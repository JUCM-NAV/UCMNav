// bison input file for logical expression parser

%{
   static int evaluated_value;
   extern int name_error;
   int evlerror(char*s); // To avoid implicit declarations
   int evllex(); // To avoid implicit declarations
%}

%token VARIABLE NOT_EQUAL
%left '+' '&'
%left NOT_EQUAL '=' '~'
%nonassoc NEGATION

%%

statement: expression { evaluated_value = $1; }
;

expression: expression '+' expression { $$ = ($1 || $3); }
| expression '&' expression { $$ = ($1 && $3); }
| expression '=' expression { $$ = (($1 == $3) ? 1 : 0); }
| expression NOT_EQUAL expression { $$ = (($1 != $3) ? 1 : 0); }
| expression '~' expression { $$ = (($1 != $3) ? 1 : 0); }
| '!' expression %prec NEGATION { $$ = 1 - $2; }
| '(' expression ')' { $$ = $2; }
| VARIABLE
;

%%

extern void ScanString( const char *expression );
extern void DeleteBuffer();
extern void EvaluateValue();

int EvaluateExpression( const char *expression, int evaluate )
{
   if( evaluate )
      EvaluateValue();
   name_error = 0;   
   ScanString( expression );
   yyparse();
   DeleteBuffer();
   return( evaluated_value );
}

int yyerror( char *s )
{
   evaluated_value = (name_error ? 3 : 2);
   return( evaluated_value );
}

int yywrap()
{
   return( 1 );
}
