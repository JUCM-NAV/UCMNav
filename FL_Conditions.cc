// callbacks for path condition management functions

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "condition.h"
#include "defines.h"
#include "utilities.h"

#include <stdlib.h>

extern void SetPromptUserForSave();
extern void GoalConditionDialogOK();

int ECPvisible = 0;
static Cltn<char *> *condition[2];
static char *selected_condition[2];
static int current_condition;
static char buffer[2000];
static condition_type ctype;

static void UpdateConditionDisplay( int current_condition );
// static char * FindSelectedCondition( int current_condition );
static void EditCondition( int index, const char *default_name = NULL, const char *default_description = NULL );

void InstallConditions( ConditionManager &conditions )
{
   int i;
   
   fl_freeze_form( pfdmMain->Main );
   
   fl_set_object_label( pfdmMain->Conditions1, conditions.ConditionName( 1 ) );
   fl_set_object_label( pfdmMain->Conditions2, conditions.ConditionName( 2 ) );

   ctype = conditions.ConditionType();
   
   for( i = 1; i <= 2; i++ )
      condition[i-1] = conditions.ConditionList( i );

   ENABLE_WIDGET( pfdmMain->ConditionAddControls );

   for( i = 0; i < 2; i++ )
      UpdateConditionDisplay( i );

   fl_unfreeze_form( pfdmMain->Main );

}

void ResetConditions()
{
   fl_freeze_form( pfdmMain->Main );
   fl_clear_browser( pfdmMain->Conditions1 );
   fl_clear_browser( pfdmMain->Conditions2 );
   fl_set_object_label( pfdmMain->Conditions1, "" );
   fl_set_object_label( pfdmMain->Conditions2, "" );
   DISABLE_WIDGET( pfdmMain->ConditionControls );
   DISABLE_WIDGET( pfdmMain->ConditionAddControls );
   fl_unfreeze_form( pfdmMain->Main );
}

extern "C"
void Conditions_cb(FL_OBJECT *, long cond )
{
   FL_OBJECT *browser = ((cond == 0) ? pfdmMain->Conditions1 : pfdmMain->Conditions2 );
   int i, ordinal =0,
      selected = fl_get_browser( browser );
   
   fl_freeze_form( pfdmMain->Main ); 
  
   for( i = 1; i <= fl_get_browser_maxline( browser ); i++ )
      fl_deselect_browser_line( browser, i );

   if( selected > 0 ) {
      
      if( strncmp( fl_get_browser_line( browser, selected ), "@b", 2 ) == 0 ) {
	 fl_select_browser_line( browser, selected );
	 selected++;
      }
    
      for( i=selected; i >= 1; i-- ) {
	 if( strncmp( fl_get_browser_line( browser, i ), "@b", 2 ) == 0 ) {
	    fl_select_browser_line( browser, i );
	    break;
	 }
	 if( strcmp( fl_get_browser_line( browser, i ), "" ) != 0 ) 
	    fl_select_browser_line( browser, i );
      }
    
      for( i=selected; i <= fl_get_browser_maxline( browser ); i++ ) {
	 if( strncmp( fl_get_browser_line( browser, i ), "@b", 2 ) == 0 ) {
	    break;
	 }
	 if( strcmp( fl_get_browser_line( browser, i ), "" ) != 0 ) 
	    fl_select_browser_line( browser, i );
      }
   }

   // find selected responsibility if any
   selected_condition[cond] = NULL;
   
   for( i = 1; i <=fl_get_browser_maxline( browser ); i++ ) {
      if( strncmp( fl_get_browser_line( browser, i ), "@b", 2 ) == 0 ) {
	 ordinal++;
      }

      if( fl_isselected_browser_line( browser, i ) ) {
	 selected_condition[cond] = condition[cond]->Get( ordinal );
	 break;
      }
   }

   FL_OBJECT *edit_button = ((cond == 0) ? pfdmMain->Conditions1_Edit : pfdmMain->Conditions2_Edit );
   FL_OBJECT *delete_button = ((cond == 0) ? pfdmMain->Conditions1_Delete : pfdmMain->Conditions2_Delete );
   
   if( selected_condition[cond] != NULL ) {
      ENABLE_WIDGET( edit_button );
      ENABLE_WIDGET( delete_button );
   } else {
      DISABLE_WIDGET( edit_button );
      DISABLE_WIDGET( delete_button );
   }      
   
   fl_unfreeze_form( pfdmMain->Main );  
}

extern "C"
void Conditions_Add_cb(FL_OBJECT *, long index )
{
   current_condition = index;
   selected_condition[index] = NULL;
   
   EditCondition( index );
}

extern "C"
void Conditions_Edit_cb(FL_OBJECT *, long index )
{
   char *cp;
   //Cltn<char *> *cl = condition[index];

   current_condition = index;

   strcpy( buffer, selected_condition[index] );
   cp = buffer;

   while( *(cp++) != '&' );
   cp--;
   *cp = 0;                       // null terminate name at head of buffer
   cp += 2;                      // skip over remainder of separator ( && )

   EditCondition( index, buffer, cp );
}

void EditCondition( int index, const char *default_name, const char *default_description )
{
   char *name, *description;

   if( StringParagraphDialog( ConditionManager::Prompt( ctype, index ), "Condition Name", "Description",
			      default_name, default_description, &name, &description ) == TRUE ) {

      *buffer = 0;
      strcat( buffer, name );
      strcat( buffer, "&&" );
      strcat( buffer, description );

      if( selected_condition[current_condition] == NULL )
	 condition[current_condition]->Add( strdup( buffer ) );
      else {
	 condition[current_condition]->Replace( selected_condition[current_condition], strdup( buffer ) );
	 free( selected_condition[current_condition] );
      }

      UpdateConditionDisplay( current_condition );
      SetPromptUserForSave();
   }
}

extern "C"
void Conditions_Delete_cb(FL_OBJECT *, long cond )
{
   Cltn<char *> *cl = condition[cond];
   cl->Delete( selected_condition[cond] );
   free( selected_condition[cond] );

   UpdateConditionDisplay( cond );
   SetPromptUserForSave();
}

void UpdateConditionDisplay( int current_condition )
{
   char *cc, *cp;
   Cltn<char *> *cl = condition[current_condition];
   FL_OBJECT *browser = ((current_condition == 0) ? pfdmMain->Conditions1 : pfdmMain->Conditions2 );
   char name[40];
   
   fl_clear_browser( browser );  
   
   for( cl->First(); !cl->IsDone(); cl->Next() ) {

      cc = cl->CurrentItem();
      strcpy( buffer, cc );
      cp = buffer;

      while( *(cp++) != '&' );
      cp--;
      *cp = 0;                       // null terminate name at head of buffer
      sprintf( name, "@b%s", buffer );
      fl_add_browser_line( browser, name );
      cp += 2;                      // skip over remainder of separator ( && )
      fl_add_browser_line( browser, cp );
      fl_add_browser_line( browser, "\n" );  // add blank line

   }

   FL_OBJECT *edit_button = ((current_condition == 0) ? pfdmMain->Conditions1_Edit : pfdmMain->Conditions2_Edit );
   FL_OBJECT *delete_button = ((current_condition == 0) ? pfdmMain->Conditions1_Delete : pfdmMain->Conditions2_Delete );
   
   selected_condition[current_condition]  = NULL;
   
   DISABLE_WIDGET( edit_button );
   DISABLE_WIDGET( delete_button );
}
