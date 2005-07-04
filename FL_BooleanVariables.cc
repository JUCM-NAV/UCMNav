// callbacks for bool variables dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "variable.h"
#include "print_mgr.h"

extern void TerminateBVEditing( bool changed );
extern void SetPromptUserForSave();

static FD_BooleanVariables *pfdbv = NULL;
static Cltn<BooleanVariable *> *variable_list;
static int mode, current_index;
static bool subdialog_mode;
static BooleanVariable *current_variable;

extern void TerminateBVEditing( bool changed );

void ListBooleanVariables( Cltn<BooleanVariable *> *variables, bool subdialog = FALSE );
void UpdateVariableDisplay();
static int InputFilter( FL_OBJECT *, const char *, const char *, int );
static int BVDialogClose( FL_FORM *, void * );

void ListBooleanVariables( Cltn<BooleanVariable *> *variables, bool subdialog )
{
   variable_list = variables;
   subdialog_mode = subdialog;
   if( subdialog_mode == FALSE )
      FREEZE_TOOL();

   if( !pfdbv ) {
      pfdbv = create_form_BooleanVariables();
      fl_set_form_atclose( pfdbv->BooleanVariables, BVDialogClose, NULL );
      fl_set_browser_fontsize( pfdbv->BooleanList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdbv->BooleanList, FL_BOLD_STYLE );
      fl_set_input_maxchars( pfdbv->CurrentBoolean, VARIABLE_NAME );
      fl_set_input_filter( pfdbv->CurrentBoolean, InputFilter );
   }

   UpdateVariableDisplay();
   fl_show_form( pfdbv->BooleanVariables, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Global Boolean Variables" );   
}

void UpdateVariableDisplay()
{
   fl_freeze_form( pfdbv->BooleanVariables );
   fl_clear_browser( pfdbv->BooleanList );
   DISABLE_WIDGET( pfdbv->BVDeleteButton );
   DISABLE_WIDGET( pfdbv->BVAcceptButton );
   DISABLE_WIDGET( pfdbv->BVCancelButton );
   fl_set_object_label( pfdbv->CurrentBoolean, "" );
   fl_set_input( pfdbv->CurrentBoolean, "" );
   DISABLE_WIDGET( pfdbv->CurrentBoolean );
   mode = NO_MODE;

   for( variable_list->First(); !variable_list->IsDone(); variable_list->Next() )
      fl_add_browser_line( pfdbv->BooleanList, variable_list->CurrentItem()->BooleanName() );

   fl_unfreeze_form( pfdbv->BooleanVariables );
}

extern "C"
void BooleanList_cb(FL_OBJECT *vlist, long)
{
   ENABLE_WIDGET( pfdbv->BVAcceptButton );
   ENABLE_WIDGET( pfdbv->BVCancelButton );
   ENABLE_WIDGET( pfdbv->CurrentBoolean );

   if( mode != EDIT_EXISTING ) {
      fl_set_object_label( pfdbv->CurrentBoolean, "Edit Existing Variable Name" );
      mode = EDIT_EXISTING;
   }

   current_index = fl_get_browser( vlist );
   current_variable = variable_list->Get( current_index );
   
   if( !current_variable->IsTimeoutVariable()  && (current_variable->ReferenceCount() == 0) ) {
      ENABLE_WIDGET( pfdbv->BVDeleteButton );
   } else {
      DISABLE_WIDGET( pfdbv->BVDeleteButton );
   }

   fl_set_input( pfdbv->CurrentBoolean, current_variable->BooleanName() );
}

extern "C"
void BVAcceptButton_cb(FL_OBJECT *, long)
{
   char buffer[200];

   if( mode == NO_MODE ) return;

   const char *input = fl_get_input( pfdbv->CurrentBoolean );
   
   if( !PrintManager::TextNonempty( input ) )
      return;

   if( mode == EDIT_EXISTING ) {
      if( current_variable->IsTimeoutVariable() ) 
	 return;
      if( strequal( input, current_variable->BooleanName() ) )
	 return; // as old and new names are the same do nothing
   }

   if( strequal( input, "T" ) || strequal( input, "F" ) ) {
      fl_set_resource( "flAlert.title", "Error: Invalid Variable Name Specified" );
      sprintf( buffer, "The specified name \"%s\" is not allowed.", input );      
      fl_show_alert( buffer, "The letters \"T\" and \"F\" are reserved to represent true and false values",
		     "in logical expressions. Please choose another name.", 0 );
      return;
   }

   if( BooleanVariable::NameIndex( input ) != NOT_CONTAINED ) {
      fl_set_resource( "flAlert.title", "Error: Duplicate Variable Name Detected" );
      sprintf( buffer, "The specified name \"%s\" already exists.", input );      
      fl_show_alert( buffer, "Please choose a unique name.", "", 0 );
      return;
   }

   if( mode == EDIT_EXISTING )
      current_variable->BooleanName( input );
   else if( mode == CREATE_NEW )
      new BooleanVariable( input );
   
   UpdateVariableDisplay();
   SetPromptUserForSave();
}

extern "C"
void BVAddButton_cb(FL_OBJECT *, long)
{
   if( mode != CREATE_NEW ) {
      ENABLE_WIDGET( pfdbv->CurrentBoolean );
      ENABLE_WIDGET( pfdbv->BVAcceptButton );
      ENABLE_WIDGET( pfdbv->BVCancelButton );
      fl_set_object_label( pfdbv->CurrentBoolean, "Specify New Variable Name" ); 
      fl_set_input( pfdbv->CurrentBoolean, "" );
      mode = CREATE_NEW;
   }
}

extern "C"
void BVDeleteButton_cb(FL_OBJECT *, long)
{
   if( current_variable->ReferenceCount() > 0 ) return;
   if( current_variable->IsTimeoutVariable() ) return;
   delete current_variable;
   UpdateVariableDisplay();
   SetPromptUserForSave();
}

extern "C"
void BVCancelButton_cb(FL_OBJECT *, long)
{
   if( mode == CREATE_NEW ) {
      fl_set_object_label( pfdbv->CurrentBoolean, "" );
      fl_set_input( pfdbv->CurrentBoolean, "" );
      mode = NO_MODE;
   } else if( mode == EDIT_EXISTING )
      fl_set_input( pfdbv->CurrentBoolean, current_variable->BooleanName() );
}

int InputFilter( FL_OBJECT *input, const char *old, const char *cur, int character )
{
   if( strlen( old ) == 0 ) {
      if( strchr( "0123456789-_", character ) != NULL )
	 return( FL_INVALID );
   }
   
   return( (strchr( "() +&!=~", character ) == NULL ) ? FL_VALID : FL_INVALID );
}

extern "C"
void BVRemoveDialogButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdbv->BooleanVariables );
   if( subdialog_mode )
      TerminateBVEditing( TRUE );
   else {
      UNFREEZE_TOOL();
   }
}

int BVDialogClose( FL_FORM *, void * )
{
   BVRemoveDialogButton_cb( 0, 0 );
   return( FL_IGNORE );
}
