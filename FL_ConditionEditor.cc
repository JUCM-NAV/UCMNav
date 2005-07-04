// GUI callbacks for logical condition editor

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "variable.h"
#include "print_mgr.h"

#define PARSE_ERROR 2
#define NAME_ERROR 3

static FD_ConditionEditor *pfdce = NULL;
static Cltn<BooleanVariable *> *variable_list;

extern "C" int EvaluateExpression( const char *expression, int state );
extern void SetPromptUserForSave();
// static int CEDialogClose( FL_FORM *, void * );
static void BooleanVariables();
static void AcceptButton();
static void CancelButton();
static bool quit_dialog;
static const char *edited_condition;

extern char nonexistent_variable[100];

const char * EditLogicalCondition( const char *condition )
{
   FL_OBJECT *returned;

   quit_dialog = FALSE;
   variable_list = BooleanVariable::VariableList();
   edited_condition = NULL;

   if( pfdce == NULL ) {
      pfdce = create_form_ConditionEditor();
      fl_set_browser_fontsize( pfdce->BooleanVariables, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdce->BooleanVariables, FL_BOLD_STYLE );
   }

   fl_set_input( pfdce->Condition, (condition != NULL ? condition : "") );
   fl_clear_browser( pfdce->BooleanVariables );

   for( variable_list->First(); !variable_list->IsDone(); variable_list->Next() )
      fl_add_browser_line( pfdce->BooleanVariables, variable_list->CurrentItem()->BooleanName() );

   fl_show_form( pfdce->ConditionEditor, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Edit Logical Selection Condition" );   

   do {
      returned = fl_do_forms();

      if( returned == pfdce->BooleanVariables )
	 BooleanVariables();
      else if( returned == pfdce->AcceptButton )
	 AcceptButton();
      else if( returned == pfdce->CancelButton )
	 CancelButton();

   } while( !quit_dialog );

   fl_hide_form( pfdce->ConditionEditor );
   return( edited_condition );
}

void BooleanVariables()
{
   int index, xpos, line;
   const char *variable_name, *current_condition;
   char buffer[2000], *cp;

   if( (index = fl_get_browser( pfdce->BooleanVariables )) == 0 ) return;
   variable_name = variable_list->Get( index )->BooleanName();
   fl_get_input_cursorpos( pfdce->Condition, &xpos, &line );
   current_condition = fl_get_input( pfdce->Condition );
   cp = buffer;

   while( --line ) {
      while( *current_condition != '\n' )
	 *cp++ = *current_condition++;
      *cp++ = '\n';
      current_condition++;
   }

   while( xpos-- ) // move to selected x position of selected line
      *cp++ = *current_condition++;
   while( *variable_name ) // add selected variable name at proper position
      *cp++ = *variable_name++;
   while( *current_condition ) // add rest of condition
      *cp++ = *current_condition++;
   *cp = 0; // null terminate buffer

   fl_set_input( pfdce->Condition, buffer );
}

void AcceptButton()
{
   int rc;
   char buffer[200];

   edited_condition = fl_get_input( pfdce->Condition );

   if( !strequal( edited_condition, "" ) ) {

      rc = EvaluateExpression( edited_condition, FALSE );

      if( rc == PARSE_ERROR ) {
	 fl_set_resource( "flAlert.title", "Error:Invalid Logical Expression" );
	 fl_show_alert( "The entered logical expression could not be parsed successfully.", "Please re-edit the condition", "", 0 );
	 return;
      }
      else if( rc == NAME_ERROR ) {
	 fl_set_resource( "flAlert.title", "Error:Invalid Variable Name Specified" );
	 sprintf( buffer, "The variable \"%s\" has not been defined.", nonexistent_variable );
	 fl_show_alert( buffer, "Please use only defined variables." , "", 0 );
	 return;
      }
   }

   quit_dialog = TRUE;
}

void CancelButton()
{
   edited_condition = NULL;
   quit_dialog = TRUE;
}

