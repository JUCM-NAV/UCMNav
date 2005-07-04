/* flex file for tokenizing logical expressions */
%s EVALUATE

%{
#include <string.h>
#include "evaluate.tab.h"
#define yylval evllval
extern int VariableValue( const char * );
extern int VariableExists( const char * );
char nonexistent_variable[100];
int name_error = 0;
%}


%%

T  { yylval = 1; return VARIABLE; }
F  { yylval = 0; return VARIABLE; }
!= { return NOT_EQUAL; }
<EVALUATE>bv[0-9]+  { yylval = VariableValue( yytext ); return VARIABLE; }
[a-zA-Z][^&+=!~ ()\n\t]* {
if( VariableExists( yytext ) ) {
   yylval = 0;
   return VARIABLE;
}
else {
   strcpy( nonexistent_variable, yytext );
   name_error = 1;
   return 0;
   }
}
[\n\t ]+
.  { return yytext[0]; }
%%

static YY_BUFFER_STATE sb;

void EvaluateValue()
{
   BEGIN(EVALUATE);
   if (1==0) yyunput(0, 0); // DA: August 2004. To avoid annoying gcc warning
}

void ScanString( const char *expression )
{
   sb = yy_scan_string( expression );
}

void DeleteBuffer()
{
   yy_delete_buffer( sb );
}
