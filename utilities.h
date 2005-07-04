// external declarations for utility dialogs
#include "defines.h"

extern bool QuestionDialog( const char *title, const char *prompt1, const char *prompt2,
			    const char *yes_button, const char *no_button, bool freeze = TRUE );

extern bool TwoStringDialog( const char *title, const char *prompt1, const char *prompt2,
			     const char *default1, const char *default2, char **string1, char **string2 );

extern bool StringParagraphDialog( const char *title, const char *prompt1, const char *prompt2,
				   const char *default1, const char *default2, char **string1, char **string2, bool freeze = TRUE );

extern void ErrorMessage( const char *message, bool new_message = FALSE );

extern void ClearErrorLog();
