/*
 * $Id: xmlparser.c,v 1.1 2001/07/07 17:24:53 sean_stuckless Exp $
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <parser.h>
#include <parserInternals.h>

#include "interface.h"
#include "support.h"
#include "xmlparser.h"

/* different parse states */
#define STATE_UNKNOWN   0
#define STATE_STARTTAG  1 
#define STATE_TEXT      2
#define STATE_ENDTAG    3

/* size of the default string buffers */
#define MAX_BUFFER_SIZE 4096


/* state structure for sax parser */
typedef struct _AppParseState AppParseState;
struct _AppParseState {
   int state;        /* keep track of state */
   char *startTag;   /* buffer for start tag */
   char *text;       /* buffer for text */
   char *endTag;     /* buffer to end tag */
   char *buffer;     /* general purpose buffer */
   char *pstart;     /* pointer to start tag */
   char *pend;       /* pointer to end tag */
   char *ptext;      /* pointer to text */
   GtkWidget *tree;  /* gtk_tree to fill */
};

/* prototypes */
inline void append(AppParseState *state, char *str, int len);
inline char *blankIfOnlyWhitespace(char *str);
inline void forgetNode(AppParseState *state);
inline void addNode(AppParseState *state);
inline void setState(AppParseState *state, int stateValue); 
inline void allocateBuffer(AppParseState *state, int len);

/* SAX call back functions */
static void startDocument(AppParseState *state);
static void endDocument(AppParseState *state);
static void handleStartElement(AppParseState *state, char *name, char **attr);
static void handleEndElement(AppParseState *state, char *name);
static void handleCharacters(AppParseState *state, char *data, int len);
static void processDTD(AppParseState *state, char *name, char *publicId, char *systemId);

/* SAX Structure */
static xmlSAXHandler appSAXParser = {
   (internalSubsetSAXFunc)processDTD, /* internalSubset */
   0, /* isStandalone */
   0, /* hasInternalSubset */
   0, /* hasExternalSubset */
   0, /* resolveEntity */
   (getEntitySAXFunc)NULL, /* getEntity */
   0, /* entityDecl */
   0, /* notationDecl */
   0, /* attributeDecl */
   0, /* elementDecl */
   0, /* unparsedEntityDecl */
   0, /* setDocumentLocator */
   (startDocumentSAXFunc)startDocument, /* startDocument */
   (endDocumentSAXFunc)endDocument, /* endDocument */
   (startElementSAXFunc)handleStartElement, /* startElement */
   (endElementSAXFunc)handleEndElement, /* endElement */
   0, /* reference */
   (charactersSAXFunc)handleCharacters, /* characters */
   0, /* ignorableWhitespace */
   0, /* processingInstruction */
   (commentSAXFunc)NULL, /* comment */
   (warningSAXFunc)NULL, /* warning */
   (errorSAXFunc)NULL, /* error */
   (fatalErrorSAXFunc)NULL, /* fatalError */
};

/* append str to buffer in AppParseState depending on state */
inline void append(AppParseState *state, char *str, int len) {
   int i=0;
   char *ptr = NULL;

   /* set the buffer to append */
   if (state->state == STATE_STARTTAG) ptr=state->pstart;
   if (state->state == STATE_TEXT)     ptr=state->ptext;
   if (state->state == STATE_ENDTAG)   ptr=state->pend;

   /* if we have  len the use it */
   if (len != 0) {
      while (i++<len) {
         *ptr=*str;
	 ptr++; str++;
      }
   }
   /* else do until we reach a zero in the string */
   else {
      while (*str != '\0') {
         *ptr=*str;
	 ptr++; str++;
      }
   }

   /* zero terminate the string */
   *ptr='\0';

   /* update the string pinters */
   if (state->state == STATE_STARTTAG) state->pstart=ptr;
   if (state->state == STATE_TEXT)     state->ptext=ptr;
   if (state->state == STATE_ENDTAG)   state->pend=ptr;
}

/* if string is only whitespace, then return nothing*/
inline char *blankIfOnlyWhitespace(char *str) {
   int isWhiteSpace = 1;
   char *ch;
   ch=str;
   
   while (*ch != 0) {
      if (*ch != '\t' && *ch != '\n' && *ch != ' ') { 
	      isWhiteSpace = 0;
	      break;
      }
      ch++;
   }

   if (isWhiteSpace) {
      str[0]=0;
   }
   return str;
}

/* reset pointers */
inline void forgetNode(AppParseState *state) {
   *state->startTag = 0;
   *state->text     = 0;
   *state->endTag   = 0;
   *state->buffer   = 0;
   state->pstart = state->startTag;
   state->ptext  = state->text;
   state->pend   = state->endTag;
}

/* add a node to the tree */
inline void addNode(AppParseState *state) {
   GtkWidget *treeItem = NULL;

   if (state->startTag[0] != 0) {
      /* add start tag */
      char *szBuf = state->buffer;
      static GtkWidget *subTree;
      char *pbuf = szBuf;

      if (state->endTag[0] != 0) {
	 /* we have both start an end tags, so add it all */
	 strcpy(szBuf, "  ");
	 strcat(szBuf,state->startTag);
	 strcat(szBuf,blankIfOnlyWhitespace(state->text));
	 strcat(szBuf,state->endTag);
         pbuf = szBuf;
         treeItem = gtk_tree_item_new_with_label(pbuf);
         gtk_tree_append(GTK_TREE(state->tree), treeItem);
         gtk_widget_show(treeItem);

      } else {
	 /* we only have a start tag, so lets add it only */
	 strcpy(szBuf, "  ");
	 strcat(szBuf, state->startTag);
	 pbuf = szBuf;
         subTree = gtk_tree_new();
         treeItem = gtk_tree_item_new_with_label(pbuf);
         gtk_tree_append(GTK_TREE(state->tree), treeItem);
         gtk_tree_item_set_subtree (GTK_TREE_ITEM(treeItem), subTree);
         gtk_widget_show(treeItem);

         state->tree = subTree;
      }
   }
   else {
      if (state->endTag[0] != 0) {
         /* add end node. */
         treeItem = gtk_tree_item_new_with_label(state->endTag);
         gtk_tree_append(GTK_TREE(state->tree), treeItem);
         gtk_widget_show(treeItem);
	 state->tree = state->tree->parent;
      }
   }

   /* once its added, then lets reset everything */
   forgetNode(state);
}

/* sets the parser state */
inline void setState(AppParseState *state, int stateValue) {
   state->state = stateValue;
}

/* allocate the element buffers */
inline void allocateBuffer(AppParseState *state, int len) {
   state->startTag = (char *)malloc(len);
   state->text = (char *)malloc(len);
   state->endTag = (char *)malloc(len);
   state->buffer = (char *)malloc(len * 3);
}

/* SAX handlers */
static void startDocument(AppParseState *state) {
   state->state=STATE_UNKNOWN;
   state->startTag = NULL;
   state->text = NULL;
   state->endTag = NULL;
   allocateBuffer(state, 4096);
   forgetNode(state);
}

static void endDocument(AppParseState *state) {
   free(state->startTag);
   free(state->text);
   free(state->endTag);
   free(state->buffer);
}

static void handleStartElement(AppParseState *state, char *name, char **attr) {
   setState(state, STATE_STARTTAG);
   addNode(state);

   append(state, "<", 0); append(state, name, 0);
   while (attr != NULL && *attr != NULL) {

     append(state, " ", 0); append(state, *attr,0); append(state, "=", 0);
     attr++;
     append(state, "\"", 0); append(state, *attr, 0); append(state, "\"", 0);
     attr++;
   };
   append(state,">",0);
}

static void handleEndElement(AppParseState *state, char *name) {
   setState(state, STATE_ENDTAG);
   append(state, "</", 0); append(state, name, 0); append(state, ">", 0);
   addNode(state);
}

static void handleCharacters(AppParseState *state, char *data, int len) {
   setState(state, STATE_TEXT);
   append(state, data, len);
}

static void processDTD(AppParseState *state, char *name, char *publicId, char *systemId) {
	char *buf = state->buffer;
	GtkWidget *treeItem;
	*buf = 0;
	strcat(buf, "  <!DOCTYPE ");
	strcat(buf, name);
	if (publicId != NULL) {
		strcat(buf, " PUBLIC \"");
		strcat(buf, publicId);
		strcat(buf, "\"");
	}
	if (systemId != NULL) {
		strcat(buf, " SYSTEM \"");
		strcat(buf, systemId);
		strcat(buf, "\"");
	}
	strcat(buf, ">");
         treeItem = gtk_tree_item_new_with_label(buf);
         gtk_tree_append(GTK_TREE(state->tree), treeItem);
         gtk_widget_show(treeItem);	
}



/* show the parsed xml file in a tree */
int show_xmlfile(const char *filename, GtkWidget *tree) {
   xmlParserCtxtPtr ctxt;
   AppParseState    state;
   const char *fileName = filename;
   int errNo = 0;

   /* Allocate a string buffer for some messages */
   char *strBuf   = (char *)malloc(1024);
   if (strBuf == NULL) {
	   fprintf(stderr, "Out of memory!");
	   gtk_exit(99);
   }

   /* Set the default tree state: No Lines */
   gtk_tree_set_view_mode (GTK_TREE (tree), GTK_TREE_VIEW_ITEM);
   gtk_tree_set_view_lines (GTK_TREE (tree), FALSE);

   /* set the tree in the parser state */
   state.tree = tree;
   ctxt = xmlCreateFileParserCtxt(fileName);
   if (!ctxt) {
      sprintf(strBuf,"Unable to parse xml file: %s", fileName);
      errNo = 1;
   }
   if (errNo == 0) {
      ctxt->sax = &appSAXParser;
      ctxt->userData = &state;
      ctxt->validate = 1;
      xmlParseDocument(ctxt);
      if (!ctxt->wellFormed) {
         sprintf(strBuf,"Xml file: %s, is not well formed.", fileName);      
         errNo = 2;
      }
      ctxt->sax = NULL;
      xmlFreeParserCtxt(ctxt);
   }

   /* if we have errors, the lets show them */
   if (errNo != 0) {
         GtkWidget *treeItem;
         treeItem = gtk_tree_item_new_with_label((strBuf != NULL) ? strBuf : "No Message");
         gtk_tree_append(GTK_TREE(tree), treeItem);
         gtk_widget_show(treeItem);
   }

   /* free the string buffer */
   if (strBuf != NULL) free(strBuf);
   return errNo;
}
