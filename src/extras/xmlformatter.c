/*
 * $Id: xmlformatter.c,v 1.3 2002/07/04 01:46:14 sean_stuckless Exp $
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>
#include <libxml/SAX.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>

#include <unistd.h>

#include "../support.h"

/* different parse states */
#define STATE_UNKNOWN   0 /* unknown/uninitialized state */
#define STATE_STARTTAG  1 /* handling start tag */
#define STATE_TEXT      2 /* handling text */
#define STATE_ENDTAG    3 /* handling end tag */
#define STATE_CDATA     4 /* handling cdata */
#define STATE_COMMENT   5 /* handling comment */


/* internal Application Parse State structure, used to track the parsing */
typedef struct _AppParseState AppParseState;

struct _AppParseState {
   int state;               /* keep track of state, will match one of STATE_XXX above */
   /* text containers for various types of data */
   GString *start_tag;
   GString *end_tag;
   GString *body_text;
};

/* SAX call back functions */
static void handleStartDocument(AppParseState *state);
static void handleEndDocument(AppParseState *state);
static void handleStartElement(AppParseState *state, char *name, char **attr);
static void handleEndElement(AppParseState *state, char *name);
static void handleCharacters(AppParseState *state, char *data, int len);
static void handleCdataBlock(AppParseState *state, char *data, int len);
static void handleComment (AppParseState *state, char *data);
static void handleDTD(AppParseState *state, char *name, char *publicId, char *systemId);


/* prototypes */
inline void addNode(AppParseState * state);
inline void addTextNode(AppParseState * state);
inline void addStartNode(AppParseState * state);
inline void addEndNode(AppParseState * state);
inline void forgetNode(AppParseState * state);
inline void setState(AppParseState * state, int stateValue);
inline void addText(AppParseState * state, char *strText);
inline GString *createTextContainer();
inline GString *getCurrentTextContainer(AppParseState * state);
static void show_help();


/* SAX Structure */
static xmlSAXHandler appSAXParser = {
    (internalSubsetSAXFunc) handleDTD,	/* internalSubset */
    0,				/* isStandalone */
    0,				/* hasInternalSubset */
    0,				/* hasExternalSubset */
    0,				/* resolveEntity */
    (getEntitySAXFunc) NULL,	/* getEntity */
    0,				/* entityDecl */
    0,				/* notationDecl */
    0,				/* attributeDecl */
    0,				/* elementDecl */
    0,				/* unparsedEntityDecl */
    0,				/* setDocumentLocator */
    (startDocumentSAXFunc) handleStartDocument,	/* startDocument */
    (endDocumentSAXFunc) handleEndDocument,	/* endDocument */
    (startElementSAXFunc) handleStartElement,	/* startElement */
    (endElementSAXFunc) handleEndElement,	/* endElement */
    0,				/* reference */
    (charactersSAXFunc) handleCharacters,	/* characters */
    0,				/* ignorableWhitespace */
    0,				/* processingInstruction */
    (commentSAXFunc) handleComment,	/* comment */
    (warningSAXFunc) NULL,	/* warning */
    (errorSAXFunc) NULL,	/* error */
    (fatalErrorSAXFunc) NULL,	/* fatalError */
    (getParameterEntitySAXFunc) NULL,	/* getParameterEntity */
    (cdataBlockSAXFunc) handleCdataBlock,	/* cdataBlock */
    (externalSubsetSAXFunc) NULL,	/* externalSubset */
};

/* reset current node information */
inline void forgetNode(AppParseState * state)
{
    if (state->body_text != NULL) g_string_free(state->body_text, FALSE);
    if (state->start_tag != NULL) g_string_free(state->start_tag, FALSE);
    if (state->end_tag != NULL) g_string_free(state->end_tag, FALSE);
	
    state->body_text = NULL;
    state->start_tag = NULL;
    state->end_tag = NULL;
    state->state = STATE_UNKNOWN;
}

int gnIndent = 0;
int gnSpaces = 3;

inline void indent() {
   int i = 0;
   for (i=0; i<gnIndent; i++) {
      printf(" ");
   }
}

/* add text (no additional indent)*/
inline void addTextNode(AppParseState * state)
{
    indent();
    printf("%s\n", state->body_text->str);
}

/* add a start element node (create an indentation)*/
inline void addStartNode(AppParseState * state)
{
    indent();
    printf("%s\n", state->start_tag->str);
    gnIndent += gnSpaces;
}

/* add a end element node (close an indentation)*/
inline void addEndNode(AppParseState * state)
{
   gnIndent -= gnSpaces;
   indent();
   printf("%s\n", state->end_tag->str);
}

/* general add node method.  It will figure out what type of node to add
 * and then proceed to build the appropriate node */
inline void addNode(AppParseState * state)
{
    if (state->state == STATE_UNKNOWN) {
	DEBUG_CMD(g_message("gxmlviewer_add_node(): state == UNKNOWN, node not added."));
	return;
    }

    /* check for start tag without end tag */
    if (state->start_tag != NULL && state->end_tag == NULL) {
	addStartNode(state);
	if (state->body_text != NULL) {
	    addTextNode(state);
	}
	
    /* check for an end tage without a start tag */
    } else if (state->start_tag == NULL && state->end_tag != NULL) {
	if (state->body_text != NULL) {
	    addTextNode(state);
	}
	addEndNode(state);
    
    /* have both start and end tags, and maybe some text */
    } else if (state->start_tag != NULL && state->end_tag != NULL) {
	DEBUG_CMD(g_message("adding start tag and end tag."));
	if (state->body_text == NULL) {
	    DEBUG_CMD(g_message("no text so will compress the nodes"));
	    g_string_insert(state->start_tag, strlen(state->start_tag->str)-1, "/");
	    indent();
            printf("%s\n", state->start_tag->str);
	} else {
	    DEBUG_CMD(g_message("Added text before the end node"));
	    indent();
            printf("%s%s%s\n", state->start_tag->str, state->body_text->str, state->end_tag->str);
	}

    /* have text only */
    } else if (state->body_text != NULL) {
	addTextNode(state);

    /* this should never happen */
    } else {
	g_message("Unhandled case for addNode(): ");
    }

    /* now that the node is added, lets forget its state */
    forgetNode(state);
}

/* adds colored text to a text container */
inline void addText(AppParseState * state, char *strText)
{
    GString *textBox = getCurrentTextContainer(state);
    g_string_append(textBox, strText);
}

/* sets the parser state */
inline void setState(AppParseState * state, int stateValue)
{
    state->state = stateValue;
}

inline GString *createTextContainer() {
    return g_string_new("");
}

/* given the current parsing state, get the container to put text */
inline GString *getCurrentTextContainer(AppParseState * state)
{
   GString *text = NULL;
   switch (state->state) {
      case STATE_STARTTAG:
         if (state->start_tag == NULL) state->start_tag = createTextContainer();
         text = state->start_tag;
         break;
      case STATE_ENDTAG:
         if (state->end_tag == NULL) state->end_tag = createTextContainer();
         text = state->end_tag;
         break;
      case STATE_TEXT:
         if (state->body_text == NULL) state->body_text = createTextContainer();
         text = state->body_text;
         break;
      default:
         g_error("Uknown State: %d! unable to get a text container",
         state->state);
   }
   return text;
}



/* SAX handlers */
static void handleStartDocument(AppParseState * state)
{
    DEBUG_CMD(g_message("starting to parse xml document"));

    /* initialize the state */
    state->start_tag = NULL;
    state->end_tag = NULL;
    state->body_text = NULL;
    forgetNode(state);
}

static void handleEndDocument(AppParseState * state)
{
    DEBUG_CMD(g_message("Finished parsing document and building xml tree."));
}

static void handleStartElement(AppParseState * state, char *name, char **attr)
{
    addNode(state);
    setState(state, STATE_STARTTAG);

    addText(state,  "<");
    addText(state,  name);

    while (attr != NULL && *attr != NULL) {
	addText(state,  " ");
	addText(state,  *attr);
	addText(state,  "=");

	attr++;
	addText(state,  "\"");
	addText(state,  *attr);
	addText(state,  "\"");

	attr++;
    };
    addText(state,  ">");
}

static void handleEndElement(AppParseState * state, char *name)
{
    setState(state, STATE_ENDTAG);
    addText(state,  "</");
    addText(state,  name);
    addText(state,  ">");
    addNode(state);
}

static void handleCharacters(AppParseState * state, char *data, int len)
{
    gchar *buf = g_strndup(data, len);
    g_strstrip(buf);
    if (strlen(buf) != 0) {
	setState(state, STATE_TEXT);
	addText(state,  buf);
	DEBUG_CMD(g_message("Adding text [%s]", buf));
    } else {
	DEBUG_CMD(g_message("String is empty, will not add"));
    }
    g_free(buf);
}

static void handleCdataBlock(AppParseState * state, char *data, int len)
{
    gchar *buf = g_strndup(data, len);
    g_strstrip(buf);
    DEBUG_CMD(g_message("adding cdata [%s]", buf));
    if (strstr(buf, "\n") != NULL) {
	DEBUG_CMD(g_message("adding a multiline CDATA"));
	addNode(state);
	setState(state, STATE_STARTTAG);
	addText(state,  "<![CDATA[");
	addNode(state);

	setState(state, STATE_TEXT);
	addText(state,  buf);

	setState(state, STATE_ENDTAG);
	addText(state,  "]]>");
	addNode(state);
    } else {
	DEBUG_CMD(g_message("adding single cdata to text node"));
	setState(state, STATE_TEXT);
	addText(state,  "<![CDATA[");
	addText(state,  buf);
	addText(state,  "]]>");
    }
    g_free(buf);
}

static void handleComment(AppParseState * state, char *data)
{
    gchar *buf = g_strdup(data);
    buf = g_strstrip(buf);
    DEBUG_CMD(g_message("adding comment [%s]", buf));
    if (strstr(buf, "\n") != NULL) {
	DEBUG_CMD(g_message("adding a multiline comment"));
	addNode(state);
	setState(state, STATE_STARTTAG);
	addText(state,  "<!--");
	addNode(state);

	setState(state, STATE_TEXT);
	addText(state,  buf);

	setState(state, STATE_ENDTAG);
	addText(state,  "-->");
	addNode(state);
    } else {
	if (state->body_text != NULL) {
	    DEBUG_CMD(g_message("adding single comment to text node"));
	    setState(state, STATE_TEXT);
	    addText(state,  "<!-- ");
	    addText(state,  buf);
	    addText(state,  " -->");
	} else {
	    DEBUG_CMD(g_message("adding single comment node"));
	    addNode(state);
	    setState(state, STATE_STARTTAG);
	    addText(state,  "<!-- ");

	    setState(state, STATE_TEXT);
	    addText(state,  buf);

	    setState(state, STATE_ENDTAG);
	    addText(state,  " -->");
	    addNode(state);
	}
    }
    g_free(buf);
}

static void handleDTD(AppParseState * state, char *name, char *publicId,
	   char *systemId)
{

    setState(state, STATE_TEXT);

    addText(state,  "<!DOCTYPE ");
    addText(state,  name);
    if (publicId != NULL) {
        addText(state,  " PUBLIC ");
        addText(state,  "\"");
        addText(state,  publicId);
        addText(state,  "\"");
    }
    if (systemId != NULL) {
        addText(state,  " SYSTEM ");
        addText(state,  "\"");
        addText(state,  systemId);
        addText(state,  "\"");
    }
    addText(state,  ">");
    addNode(state);
}

/* show the parsed xml file in a tree */
int format_xmlfile(const char *filename)
{
    xmlParserCtxtPtr ctxt;
    AppParseState state;
    const char *fileName = filename;
    int errNo = 0;
    char *strBuf = NULL;
    
    ctxt = xmlCreateFileParserCtxt(fileName);
    if (!ctxt) {
	strBuf = g_strdup_printf("Unable to parse xml file: [%s]. The filename may not exist.", fileName);
	errNo = 1;
    }
    if (errNo == 0) {
	ctxt->sax = &appSAXParser;
	ctxt->userData = &state;
	ctxt->validate = 1;
	ctxt->replaceEntities = 0;
	xmlParseDocument(ctxt);
	if (!ctxt->wellFormed) {
	    strBuf = g_strdup_printf("Parse error in XML file: [%s]",fileName);
	    errNo = 2;
	}
	ctxt->sax = NULL;
	xmlFreeParserCtxt(ctxt);
    }

    /* if we have errors, the lets show them */
    if (errNo != 0) {
       g_error(strBuf);
    }

    /* free the string buffer */
    if (strBuf != NULL)
	g_free(strBuf);
    return errNo;
}


static void show_help() {
   printf("\nxmlformatter %s Sean Stuckless <sean@stuckless.org>\n", VERSION);
   printf("xmlformatter will reformat the file and send to stdout.\n");
   printf("Reformatting xml is not generally a good idea.  Use at your own risk!\n\n");
   printf("Usage: xmlformatter [-s num_spaces] -x xmlfile\n");
   printf("  -s num_spaces: number of spaces to use for indent\n");
   printf("  -x xmlfile   : xml file to format\n");
   printf("\n");
}

int main(int argc, char **argv) {
   const char *getopt_string = "s:x:";
   int getopt_char;
   char *filename = NULL;
   
   while(1) {
      getopt_char = getopt(argc, argv, getopt_string);
      if (getopt_char == -1) break;
      switch (getopt_char) {
         case 's':
            gnSpaces = atoi(optarg);
            break;
         case 'x':
	    filename = g_strdup(optarg);
            break;
	 case '?':
	    show_help();
	    exit(1);

      }
   }
  
   if (filename == NULL) {
      show_help();
      exit(1);
   }

   format_xmlfile(filename);
   g_free(filename);
}
