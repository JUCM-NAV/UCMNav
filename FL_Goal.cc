
// callbacks for goal dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "goal.h"
#include "goal_tag.h"
#include "utilities.h"

FD_GoalEditor *pfdge = NULL;

static GoalTag *goal_tag1, *goal_tag2;
static Goal *current_goal;
static int operation, current_type;
static char *selected_condition[2];
static char buffer[2000];

extern int ConditionDialogClose( FL_FORM *, void * );
extern void SetPromptUserForSave();

static void OpenDialog();
static void CloseDialog();
static void UpdateConditionDisplay( int current_condition );
static void EditGoalConditions( int index, const char *default_name = NULL, const char *default_description = NULL );
 
void EditNewGoal( GoalTag *gs, GoalTag *ge )
{
   char buffer[100];

   goal_tag1 = gs;
   goal_tag2 = ge;

   OpenDialog();   
   operation = CREATE_NEW;

   sprintf( buffer, "Create goal between goal tags\n%s and %s.", gs->Name(), ge->Name() );
   fl_set_object_label( pfdge->Label, buffer );
   fl_set_input( pfdge->GoalName, "" );
   fl_set_input( pfdge->GoalDescription, "" );
   fl_clear_browser( pfdge->GoalPreconditions );
   fl_clear_browser( pfdge->GoalPostconditions );

   // deactivate edit and delete buttons
   DISABLE_WIDGET( pfdge->GoalPreconditionsEdit );
   DISABLE_WIDGET( pfdge->GoalPreconditionsDelete );
   DISABLE_WIDGET( pfdge->GoalPostconditionsEdit );
   DISABLE_WIDGET( pfdge->GoalPostconditionsDelete );
   
   fl_show_form( pfdge->GoalEditor, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Create Goal" );
}

void EditExistingGoal( Goal *goal )
{
   char buffer[100];

   current_goal = goal;
   OpenDialog();
   
   sprintf( buffer, "Edit goal between goal tags\n%s and %s.",
	    goal->StartGoal()->Name(), goal->EndGoal()->Name() );
   fl_set_object_label( pfdge->Label, buffer );
   fl_set_input( pfdge->GoalName, goal->Name() );
   fl_set_input( pfdge->GoalDescription, goal->Description() );

   for( int i = 0; i < 2; i ++ )
      UpdateConditionDisplay(i);

   fl_show_form( pfdge->GoalEditor, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Edit Goal" );

   operation = EDIT_EXISTING;
}

void GoalAcceptButton_cb(FL_OBJECT *, long)
{
   const char *name = fl_get_input( pfdge->GoalName ),
      *description = fl_get_input( pfdge->GoalDescription );
   
   if( operation == CREATE_NEW ) {

      GoalList *goal_list = GoalList::Instance();
      goal_list->AddGoal( new Goal( goal_tag1, goal_tag2, name, description ) );
   }
   else { // operation == EDIT_EXISTING

      current_goal->Name( name );
      current_goal->Description( description );
   }
   CloseDialog();
}

void GoalCancelButton_cb(FL_OBJECT *, long)
{
   CloseDialog();
}

void OpenDialog()
{
   FREEZE_TOOL();

   if( pfdge == NULL )
      pfdge = create_form_GoalEditor();
}

void CloseDialog()
{
   fl_hide_form( pfdge->GoalEditor );
   UNFREEZE_TOOL();
}

void UpdateConditionDisplay( int current_condition )
{
   char *cc, *cp;
   Cltn<char *> *cl = current_goal->Conditions( current_condition );
   FL_OBJECT *browser = ((current_condition == 0) ? pfdge->GoalPreconditions : pfdge->GoalPostconditions );
   char name[40];
   
   fl_freeze_form( pfdge->GoalEditor );
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

   FL_OBJECT *edit_button = ((current_condition == 0) ? pfdge->GoalPreconditionsEdit : pfdge->GoalPostconditionsEdit );
   FL_OBJECT *delete_button = ((current_condition == 0) ? pfdge->GoalPreconditionsDelete : pfdge->GoalPostconditionsDelete );

   DISABLE_WIDGET( edit_button );
   DISABLE_WIDGET( delete_button );
   
   selected_condition[current_condition]  = NULL;
   fl_unfreeze_form( pfdge->GoalEditor );
}

void GoalConditions_cb(FL_OBJECT *, long type )
{
   FL_OBJECT *browser = ((type == 0) ? pfdge->GoalPreconditions : pfdge->GoalPostconditions );
   int i, ordinal =0, selected;
   
   if( current_goal == NULL ) return;

   selected = fl_get_browser( browser );
   fl_freeze_form( pfdge->GoalEditor ); 
  
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

   // find selected condition, if any
   selected_condition[type] = NULL;
   
   for( i = 1; i <=fl_get_browser_maxline( browser ); i++ ) {
      if( strncmp( fl_get_browser_line( browser, i ), "@b", 2 ) == 0 ) {
	 ordinal++;
      }

      if( fl_isselected_browser_line( browser, i ) ) {
	 selected_condition[type] = current_goal->Conditions( type )->Get( ordinal );
	 break;
      }
   }

   FL_OBJECT *edit_button = ((type == 0) ? pfdge->GoalPreconditionsEdit : pfdge->GoalPostconditionsEdit );
   FL_OBJECT *delete_button = ((type == 0) ? pfdge->GoalPreconditionsDelete : pfdge->GoalPostconditionsDelete );
   
   if( selected_condition[type] != NULL ) {
      ENABLE_WIDGET( edit_button );
      ENABLE_WIDGET( delete_button );
   }
   else {
      DISABLE_WIDGET( edit_button );
      DISABLE_WIDGET( delete_button );
   }      
   
   fl_unfreeze_form( pfdge->GoalEditor );  
}

void GoalConditionsAdd_cb(FL_OBJECT *, long type )
{
   // check if goal is being created
   if( operation == CREATE_NEW ) {

      GoalList *goal_list = GoalList::Instance();
      current_goal = new Goal( goal_tag1, goal_tag2, fl_get_input( pfdge->GoalName ), fl_get_input( pfdge->GoalDescription ) );
      goal_list->AddGoal( current_goal );
      operation = EDIT_EXISTING;
   }
   
   current_type = type;
   selected_condition[type] = NULL;
   
   EditGoalConditions( type );
}

void GoalConditionsEdit_cb(FL_OBJECT *, long type )
{
   char *cp;

   current_type = type;
   strcpy( buffer, selected_condition[type] );
   cp = buffer;

   while( *(cp++) != '&' );
   cp--;
   *cp = 0;                       // null terminate name at head of buffer
   cp += 2;                      // skip over remainder of separator ( && )

   EditGoalConditions( type, buffer, cp );
}

void EditGoalConditions( int type, const char *default_name, const char *default_description )
{
   char prompt[100];
   char *name, *description;

   fl_deactivate_form( pfdge->GoalEditor );  
   sprintf( prompt, "Enter Goal %s", (type == PRECONDITIONS) ? "Precondition" : "Postcondition" );

   if( StringParagraphDialog( prompt, "Condition Name", "Description",
			      default_name, default_description, &name, &description ) == TRUE ) {

      // read input from dialog box, form into buffer and add to appropriate condition list   
      *buffer = 0;
      strcat( buffer, name );
      strcat( buffer, "&&" );
      strcat( buffer, description );

      if( selected_condition[current_type] == NULL )
	 current_goal->AddCondition( current_type, buffer );
      else
	 current_goal->ReplaceCondition( current_type, selected_condition[current_type], buffer );

      UpdateConditionDisplay( current_type );
      SetPromptUserForSave();
   }

   fl_activate_form( pfdge->GoalEditor );  
}

void GoalConditionsDelete_cb(FL_OBJECT *, long type )
{
   current_goal->Conditions( type )->Delete( selected_condition[type] );
   free( selected_condition[type] );
   UpdateConditionDisplay( type );
   SetPromptUserForSave();
}
