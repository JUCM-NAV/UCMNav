// callbacks for variable operations dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "variable.h"
#include "responsibility.h"

static FD_VariableOperations *pfdvo = NULL;
extern FD_EditRespPopup *pfderp;

extern const char * EditLogicalCondition( const char *condition );
extern void SetPromptUserForSave();
extern void UpdateOperationsList();

static Cltn<VariableOperation *> *variable_operations;
static VariableOperation *current_operation;
static FL_FORM *parent_form;
static int selected_operation = 0;
static int selected_variable_index = 0;
static int selected_value = 0;  // default value to FALSE
static char *current_lexpression = NULL;

static int VODialogClose( FL_FORM *, void * );

void UpdateVariableOperations();

void EditVariableOperations( Responsibility *responsibility, FL_FORM *parent )
{ // display dialog and fill with data
   char title[150];

   variable_operations = responsibility->VariableOperations();
   parent_form = parent;
   fl_deactivate_form( parent_form ); // freeze main responsibility popup

   if( pfdvo == NULL ) {
      pfdvo = create_form_VariableOperations();
      fl_set_form_atclose( pfdvo->VariableOperations, VODialogClose, NULL );
      fl_set_browser_fontsize( pfdvo->OperationsList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdvo->OperationsList, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdvo->UBVList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdvo->UBVList, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdvo->ExpressionDisplay, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdvo->ExpressionDisplay, FL_BOLD_STYLE );
   }

   sprintf( title, "Variable operations by responsibility \"%s\"", responsibility->Name() );
   UpdateVariableOperations();

   fl_show_form( pfdvo->VariableOperations, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, title );
}

void UpdateVariableOperations()
{
   char buffer[300];
   Cltn<BooleanVariable *> *bool_variables;
   BooleanVariable *cv;
   VariableOperation *cvo;
   char *logical_expression;

   fl_clear_browser( pfdvo->OperationsList );
   fl_clear_browser( pfdvo->UBVList );
   fl_clear_browser( pfdvo->ExpressionDisplay );

   bool_variables = BooleanVariable::VariableList();
   BooleanVariable::DereferenceVariableList();
   
   for( variable_operations->First(); !variable_operations->IsDone(); variable_operations->Next() ) {
      cvo = variable_operations->CurrentItem();
      if( cvo->Value() != EVALUATED_EXPRESSION )
	 sprintf( buffer, "%s -> %s", cvo->Variable()->BooleanName(), ((cvo->Value() == TRUE) ? "T" : "F" ));
      else {
	 logical_expression = cvo->LogicalExpression();
	 sprintf( buffer, "%s -> EVAL(%s)", cvo->Variable()->BooleanName(), logical_expression );
	 free( logical_expression );
      }
      cvo->Variable()->Reference();
      fl_add_browser_line( pfdvo->OperationsList, buffer );
   }

   for( bool_variables->First(); !bool_variables->IsDone(); bool_variables->Next() ) {
      cv = bool_variables->CurrentItem();
      if( cv->IsNotReferenced() )
	 fl_add_browser_line( pfdvo->UBVList, cv->BooleanName() );
   }

   fl_set_object_label( pfdvo->AddOperation, "Add/Edit Variable Operation" );
   DISABLE_WIDGET( pfdvo->AddOperation );
   DISABLE_WIDGET( pfdvo->DeleteOperation );
   selected_operation = 0;
   current_operation = NULL;
   selected_variable_index = 0;
   current_lexpression = NULL;
   selected_value = FALSE;
  
   fl_set_button( pfdvo->TrueButton, 0 );
   fl_set_button( pfdvo->FalseButton, 1 );
   fl_set_button( pfdvo->ExpressionButton, 0 );
   DISABLE_WIDGET( pfdvo->EditExpression );
   DISABLE_WIDGET( pfdvo->ExpressionDisplay );
}

extern "C"
void OperationsList_cb(FL_OBJECT *ol, long)
{
   int index, value;
   char *expression;

   if( (index = fl_get_browser( ol )) == 0 ) return;
   if( index == selected_operation )
      return;
   else
      selected_operation = index;

   current_operation = variable_operations->Get( selected_operation );
   ENABLE_WIDGET( pfdvo->AddOperation );
   fl_set_object_label( pfdvo->AddOperation, "Edit Selected Variable Operation" );
   ENABLE_WIDGET( pfdvo->DeleteOperation );
   fl_deselect_browser( pfdvo->UBVList );
   selected_variable_index = 0;

   fl_set_button( pfdvo->TrueButton, 0 );
   fl_set_button( pfdvo->FalseButton, 0 );
   fl_set_button( pfdvo->ExpressionButton, 0 );
   
   fl_clear_browser( pfdvo->ExpressionDisplay );
   if( current_lexpression != NULL ) free( current_lexpression );

   value = current_operation->Value();

   if( value == TRUE ) {
      fl_set_button( pfdvo->TrueButton, 1 );
      fl_call_object_callback( pfdvo->TrueButton );
   }
   else if( value == FALSE ) {
      fl_set_button( pfdvo->FalseButton, 1 );
      fl_call_object_callback( pfdvo->FalseButton ); 
   }
   else { // EVALUATED_EXPRESSION
      fl_set_button( pfdvo->ExpressionButton, 1 );
      fl_call_object_callback( pfdvo->ExpressionButton ); 
     
      if( (expression = current_operation->LogicalExpression()) != NULL )
	 fl_add_browser_line( pfdvo->ExpressionDisplay, expression );

      current_lexpression = expression;
   }

   if( value == TRUE || value == FALSE ) 
      current_lexpression = NULL;
}

extern "C"
void DeleteOperation_cb(FL_OBJECT *, long)
{
   if( current_operation != NULL ) {
      current_operation->Variable()->DecrementReferenceCount();
      variable_operations->Delete( current_operation );
      delete current_operation;
      UpdateVariableOperations();
      SetPromptUserForSave();
   }
}

extern "C"
void UBVList_cb(FL_OBJECT *vl, long)
{
   int index;

   if( (index = fl_get_browser( vl )) == 0 ) return;
   if( index == selected_variable_index )
      return;
   else
      selected_variable_index = index;

   fl_deselect_browser( pfdvo->OperationsList );
   selected_operation = 0;
   current_operation = NULL; // switch mode of Add/Edit button
   ENABLE_WIDGET( pfdvo->AddOperation );
   fl_set_object_label( pfdvo->AddOperation, "Add New Variable Operation" );
}

extern "C"
void AddOperation_cb(FL_OBJECT *, long)
{
   bool expression_empty = FALSE;

   if( selected_operation == 0 && selected_variable_index == 0 ) return;

   if( selected_value == EVALUATED_EXPRESSION ) {
      if( current_lexpression == NULL )
	 expression_empty = TRUE;
      else {
	 if( strequal( current_lexpression, "" ) )
	    expression_empty = TRUE;
      }
      if( expression_empty ) {
	 fl_set_resource( "flAlert.title", "Error: Empty logical expression entered" );
	 fl_show_alert( "An empty logical expression has been entered.",
			"As this results in indeterminate operations it is disallowed.",
			"Please enter a meaningful expression.", 0 );
	 return;
      }
   }

   if( current_operation != NULL ) { // Edit existing variable operation
      current_operation->Value( selected_value );
      if( selected_value == EVALUATED_EXPRESSION )
	 current_operation->LogicalExpression( current_lexpression );

   } else { // Add New variable operation
      variable_operations->Add( new VariableOperation( BooleanVariable::SelectedVariable( selected_variable_index ), selected_value,
						 ((selected_value == EVALUATED_EXPRESSION) ? current_lexpression : NULL), TRUE));
   }
   UpdateVariableOperations();
   SetPromptUserForSave();
}

extern "C"
void EditExpression_cb(FL_OBJECT *, long)
{
   const char *edited_expression;

   if( selected_operation == 0 && selected_variable_index == 0 ) return;

   if( (edited_expression = EditLogicalCondition( current_lexpression )) != NULL ) {
      if( current_lexpression != NULL ) free( current_lexpression );
      current_lexpression = strdup( edited_expression );
      fl_clear_browser( pfdvo->ExpressionDisplay );
      fl_add_browser_line( pfdvo->ExpressionDisplay, edited_expression );
      SetPromptUserForSave();
   }
}

extern "C" 
void TrueButton_cb(FL_OBJECT *, long)
{
   if( selected_value != TRUE ) {
      if( selected_value == EVALUATED_EXPRESSION ) {
	 DISABLE_WIDGET( pfdvo->EditExpression );
	 DISABLE_WIDGET( pfdvo->ExpressionDisplay );
      }
      selected_value = TRUE;
   }
}

extern "C"
void FalseButton_cb(FL_OBJECT *, long)
{
   if( selected_value != FALSE ) {
      if( selected_value == EVALUATED_EXPRESSION ) {
	 DISABLE_WIDGET( pfdvo->EditExpression );
	 DISABLE_WIDGET( pfdvo->ExpressionDisplay );
      }
      selected_value = FALSE;
   }
}

extern "C"
void ExpressionButton_cb(FL_OBJECT *, long)
{
   if( selected_value != EVALUATED_EXPRESSION ) {
      ENABLE_WIDGET( pfdvo->EditExpression );
      ENABLE_WIDGET( pfdvo->ExpressionDisplay );
      selected_value = EVALUATED_EXPRESSION;
   }
}

extern "C"
void VariableOperationsClose_cb(FL_OBJECT *, long)
{
   UpdateOperationsList();
   fl_hide_form( pfdvo->VariableOperations );
   fl_activate_form( parent_form ); // reactivate main responsibility popup
}

int VODialogClose( FL_FORM *, void * )
{
   VariableOperationsClose_cb( 0, 0 );
   return( FL_IGNORE );
}
