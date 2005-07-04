// Utlility GUI dialogs

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "defines.h"
#include "interface.h"

#include<iostream> // temporary

static FD_TwoStringDialog *pfdtsd = NULL;
static FD_QuestionDialog *pfdqd = NULL;
static FD_StringParagraphDialog *pfdspd = NULL;
static FD_ErrorDialog *pfded = NULL;

static char s1[101], s2[101], p[2001];
static bool error_dialog_visible;
static int ErrorDialogClose( FL_FORM *, void * );

void ErrorMessage( const char *message, bool new_message = FALSE );

bool TwoStringDialog( const char *title, const char *prompt1, const char *prompt2,
		      const char *default1, const char *default2, char **string1, char **string2 )
{
   FL_OBJECT *returned;

   s1[100] = 0;
   s2[100] = 0;

   FREEZE_TOOL();
   
   if( !pfdtsd )
      pfdtsd = create_form_TwoStringDialog();

   fl_set_object_label( pfdtsd->String1, prompt1 );
   fl_set_input( pfdtsd->String1, ((default1 != NULL ) ? default1 : "") );
   fl_set_object_label( pfdtsd->String2, prompt2 );
   fl_set_input( pfdtsd->String2, ((default2 != NULL ) ? default2 : "") );

   fl_show_form( pfdtsd->TwoStringDialog, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT, title );

   do {
      returned = fl_do_forms();
   } while( returned != pfdtsd->AcceptButton && returned != pfdtsd->CancelButton );

   fl_hide_form( pfdtsd->TwoStringDialog );
   UNFREEZE_TOOL();

   if( returned == pfdtsd->AcceptButton ) {
      strncpy( s1, fl_get_input( pfdtsd->String1 ), 100 );
      strncpy( s2, fl_get_input( pfdtsd->String2 ), 100 );
      *string1 = s1;
      *string2 = s2;
      return TRUE;
   }
   else
      return FALSE;
}

bool QuestionDialog( const char *title, const char *prompt1, const char *prompt2, const char *yes_button, const char *no_button, bool freeze )
{
   FL_OBJECT *returned;

   if( freeze ) FREEZE_TOOL();
   
   if( !pfdqd )
      pfdqd = create_form_QuestionDialog();

   fl_set_object_label( pfdqd->String1, prompt1 );
   fl_set_object_label( pfdqd->String2, prompt2 );
   fl_set_object_label( pfdqd->YesButton, yes_button );
   fl_set_object_label( pfdqd->NoButton, no_button );

   fl_show_form( pfdqd->QuestionDialog, FL_PLACE_CENTER, FL_TRANSIENT, title );

   do {
      returned = fl_do_forms();
   } while( returned != pfdqd->YesButton && returned != pfdqd->NoButton );

   fl_hide_form( pfdqd->QuestionDialog );
   if( freeze ) UNFREEZE_TOOL();
   return( (returned == pfdqd->YesButton) ? TRUE : FALSE );
}

bool StringParagraphDialog( const char *title, const char *prompt1, const char *prompt2, const char *default1, const char *default2,
			       char **string, char **paragraph, bool freeze )
{
   FL_OBJECT *returned;

   s1[100] = 0;
   p[2000] = 0;

   if( freeze ) FREEZE_TOOL();

   if( pfdspd == NULL )
      pfdspd = create_form_StringParagraphDialog();

   fl_set_object_label( pfdspd->String, prompt1 );
   fl_set_input( pfdspd->String, ((default1 != NULL ) ? default1 : "") );
   fl_set_object_label( pfdspd->Paragraph, prompt2 );
   fl_set_input( pfdspd->Paragraph, ((default2 != NULL ) ? default2 : "") );

   fl_show_form( pfdspd->StringParagraphDialog, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT, title );

   do {
      returned = fl_do_forms();
   } while( returned != pfdspd->AcceptButton && returned != pfdspd->CancelButton );

   if( freeze ) UNFREEZE_TOOL();
   fl_hide_form( pfdspd->StringParagraphDialog );

   if( returned == pfdspd->AcceptButton ) {
      strncpy( s1, fl_get_input( pfdspd->String ), 100 );
      strncpy( p, fl_get_input( pfdspd->Paragraph ), 2000 );
      *string = s1;
      *paragraph = p;
      return TRUE;
   }
   else
      return FALSE;
}

void ErrorMessage( const char *message, bool new_message )
{ // add message to error message window

   if( pfded == NULL ) {
      pfded = create_form_ErrorDialog();
      fl_set_form_atclose( pfded->ErrorDialog, ErrorDialogClose, NULL );
      fl_set_browser_fontsize( pfded->ErrorLog, FL_NORMAL_SIZE );
   }

   if( error_dialog_visible == FALSE ) {
      fl_show_form( pfded->ErrorDialog, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT, "Scenario Scanning Error Messages" );
      error_dialog_visible = TRUE;
   }

   if( new_message )
      fl_add_browser_line( pfded->ErrorLog, "<------------------------------------------>" );
   fl_add_browser_line( pfded->ErrorLog, message );
}

extern "C"
void DismissButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfded->ErrorDialog );
   error_dialog_visible = FALSE;
}

extern "C"
void ClearLogButton_cb(FL_OBJECT *, long)
{
   fl_clear_browser( pfded->ErrorLog );
}

int ErrorDialogClose( FL_FORM *, void * )
{
   DismissButton_cb( 0, 0 );
   return( FL_IGNORE );
}

void ClearErrorLog()
{
   if( pfded != NULL )
      fl_clear_browser( pfded->ErrorLog );
}
