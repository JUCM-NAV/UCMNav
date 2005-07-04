// prototypes for global XML functions

extern void PrintNewXMLBlock( FILE *fpx, const char *stbp );   // Prints the line, then indents 3 spaces for a new block
extern void PrintEndXMLBlock( FILE *fpx, const char *stbp );   // Unindents 3 spaces, and prints the line
extern void PrintXML( FILE *fpx, const char *stbp );           // Just prints the line.
extern void PrintXMLText( FILE *fpx, const char *stbp );       // Prints only one line, but does not auto-append < > to text
extern void IndentNewXMLBlock( FILE *fpx );              // Indents 3 spaces for a new block
extern void IndentEndXMLBlock( FILE *fpx );              // Unindents 3 spaces
extern void LinebreakXML( FILE *fpx );                   // skips a line in the XML output
extern char * PrintDescription( const char *description );  // removes redundant linebreaks from description
extern char * removeNewlineinString( const char *description );  // removes redundant linebreaks from description for DXL
