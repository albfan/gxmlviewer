#ifndef _XMLPARSER_H_
#define _XMLPARSER_H_

typedef struct _AppParseState AppParseState;

struct _AppParseState {
   int state;        /* keep track of state */
   char *startTag;   /* buffer for start tag */
   char *text;       /* buffer for text */
   char *endTag;     /* buffer to end tag */
   char *cdata;       /* buffer to cdata */
	 char *buffer;
   char *pstart;     /* pointer to start tag */
   char *pend;       /* pointer to end tag */
   char *ptext;      /* pointer to text */
   char *pcdata;       /* pointer to cdata */
   GtkWidget *tree;  /* gtk_tree to fill */
};

/* SAX call back functions */
void handleStartDocument(AppParseState *state);
void handleEndDocument(AppParseState *state);
void handleStartElement(AppParseState *state, char *name, char **attr);
void handleEndElement(AppParseState *state, char *name);
void handleCharacters(AppParseState *state, char *data, int len);

void handleCdataBlock(AppParseState *state, char *data, int len);
void handleComment (AppParseState *state, char *data);

int show_xmlfile(const char *filename, GtkWidget *tree);

#endif
