// callbacks for scenario definition dialogs

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "scenario.h"
#include "variable.h"
#include "msc_generator.h"
#include "display.h"
#include "utilities.h"
#include "map.h"
#include "start.h"
#include "print_mgr.h"
#include "hypergraph.h"
#include "scenario_generator.h"

extern DisplayManager *display_manager;
extern void ListBooleanVariables( Cltn<BooleanVariable *> *variables, bool subdialog = FALSE );
extern void SetPromptUserForSave();
extern void DrawScreen();

static FD_ScenarioSpecification *pfdss = NULL;
static FD_StartScenarios *pfdsl = NULL;
static FD_EditStartList *pfdesl = NULL;
static Cltn<ScenarioGroup *> *scenario_groups;
static Cltn<Scenario *> *scenarios;
static Cltn<Start *> *start_list;
static ScenarioGroup *current_group = NULL;
static Scenario *current_scenario;
static VariableInit *current_reference;
static Cltn<VariableInit *> *variable_references;
static BooleanVariable *current_variable;
static Hyperedge *selected_start = NULL;
static int selected_group = 0;
static int selected_scenario = 0;
static int selected_reference = 0;
static int selected_map = 0;
static int selected_scenario_start = 0;

static bool group_change, initialization_edit_mode;
static Map *original_map;
static Map *current_map;
static const char *prompt;

static int VSDialogClose( FL_FORM *, void * );
static int LSSDialogClose( FL_FORM *, void * );
static int ESLDialogClose( FL_FORM *, void * );
static void UpdateScenarioGroups();
static void UpdateScenarios();
static void UpdateSelectedScenario();
static void UpdateStartingPoints();
static bool EditGroup( const char *default_name, const char *default_description, char **name, char **description );
static bool EditScenario( const char *default_name, const char *default_description, char **name, char **description );
static void HighlightScenario( Scenario *hs );
static void TerminateGroupChange();
static void EditPathStartList();
static void UpdateScenarioStartList( bool deselect = TRUE );
static void CreateMapStartList( Map *map );
static void DisplayMap( Map *map );

void ViewScenarios( Cltn<ScenarioGroup *> *groups, Hyperedge *start )
{
   FREEZE_TOOL();

   scenario_groups = groups;
   selected_start = start;

   if( pfdss == NULL ) {
      pfdss = create_form_ScenarioSpecification();
      fl_set_form_atclose( pfdss->ScenarioSpecification, VSDialogClose, NULL );
      fl_set_browser_fontsize( pfdss->ScenariosList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdss->ScenariosList, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdss->GroupsList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdss->GroupsList, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdss->VariableInitializations, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdss->VariableInitializations, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdss->BooleanVariables, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdss->BooleanVariables, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdss->StartingPoints, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdss->StartingPoints, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdss->ScenarioDescription, FL_NORMAL_SIZE );
      fl_set_browser_fontsize( pfdss->ScenarioGroupDescription, FL_NORMAL_SIZE );
   }

   if( selected_start == NULL )
      prompt = "Scenario Definitions";
   else
      prompt = "Scenario Definitions : Path Start Selected";

   // reset previous selections
   selected_scenario = 0;
   selected_group = 0;

   UpdateScenarioGroups();
   fl_show_form( pfdss->ScenarioSpecification, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, prompt );   
}

void UpdateScenarioGroups()
{
   fl_freeze_form( pfdss->ScenarioSpecification );
   fl_clear_browser( pfdss->GroupsList );
   fl_clear_browser( pfdss->ScenarioGroupDescription );

   for( scenario_groups->First(); !scenario_groups->IsDone(); scenario_groups->Next() )
      fl_add_browser_line( pfdss->GroupsList, scenario_groups->CurrentItem()->GroupName() );

   fl_unfreeze_form( pfdss->ScenarioSpecification );
   DISABLE_WIDGET( pfdss->DeleteGroupButton );
   DISABLE_WIDGET( pfdss->EditGroupButton );
   DISABLE_WIDGET( pfdss->GroupMSCButton );
   DISABLE_WIDGET( pfdss->GroupXMLButton );
   DISABLE_WIDGET( pfdss->AddSelectedGroupButton );
   current_group = NULL;
   UpdateScenarios();
}

extern "C"
void GroupsList_cb(FL_OBJECT *gl, long)
{
   const char *description;
   int index;

   if( (index = fl_get_browser( gl )) == 0 ) return;
   if( index == selected_group )
      return;
   else
      selected_group = index; 

   selected_scenario = 0;
   current_group = scenario_groups->Get( selected_group );

   if( current_group->ScenarioCount() > 0 ) {
      DISABLE_WIDGET( pfdss->DeleteGroupButton );
   } else {  
      ENABLE_WIDGET( pfdss->DeleteGroupButton );
   }

   fl_freeze_form( pfdss->ScenarioSpecification );
   fl_clear_browser( pfdss->ScenarioGroupDescription );
   if( (description = current_group->GroupDescription()) != NULL )
      fl_add_browser_line( pfdss->ScenarioGroupDescription, description );
   fl_unfreeze_form( pfdss->ScenarioSpecification );
   
   ENABLE_WIDGET( pfdss->EditGroupButton );
   ENABLE_WIDGET( pfdss->GroupMSCButton );
   ENABLE_WIDGET( pfdss->GroupXMLButton );
   UpdateScenarios();
}

extern "C"
void AddGroupButton_cb(FL_OBJECT *, long)
{
   char *entered_name, *entered_description;
   
   if( group_change ) return; // disable during group change mode of dialog
   if( EditGroup( NULL, NULL, &entered_name, &entered_description ) == FALSE )
      return;

   scenario_groups->Add( new ScenarioGroup( entered_name, entered_description ) );
   UpdateScenarioGroups();
   SetPromptUserForSave();
}

extern "C"
void EditGroupButton_cb(FL_OBJECT *, long)
{
   char *entered_name, *entered_description;

   if( group_change ) return; // disable during group change mode of dialog
   if( current_group == NULL ) return;

   if( EditGroup( current_group->GroupName(), current_group->GroupDescription(), &entered_name, &entered_description ) == FALSE )
      return;

   current_group->GroupName( entered_name );
   current_group->GroupDescription( entered_description );

   fl_replace_browser_line( pfdss->GroupsList, selected_group, entered_name );
   fl_clear_browser( pfdss->ScenarioGroupDescription );
   fl_add_browser_line( pfdss->ScenarioGroupDescription, entered_description );
   SetPromptUserForSave();
}

bool EditGroup( const char *default_name, const char *default_description, char **name, char **description )
{
   bool duplicate_name;
   char buffer[200], old_name[100];
   const char *prompt, *name_text, *description_text;

   if( default_name == NULL )
      prompt = "Add New Scenario Group";
   else {
      prompt = "Edit Selected Scenario Group";
      strcpy( old_name, default_name);
   }

   name_text = default_name;
   description_text = default_description;

   do {
      duplicate_name = FALSE;
      if( StringParagraphDialog( prompt, "Group Name", "Group Description", name_text, description_text, name, description ) == FALSE )
	 return FALSE;
      if( default_name != NULL ) {
	 if( strequal( old_name, *name ) )
	    return TRUE;
      }
      if( !PrintManager::TextNonempty( *name ) ) {
	 fl_set_resource( "flAlert.title", "Error: Empty Scenario Group Name Detected" );
	 sprintf( buffer, "The entered name \"%s\" is disallowed. ", *name );
	 fl_show_alert( buffer, "Please enter a nonempty and unique name." , "", 0 );
	 duplicate_name = TRUE;
	 name_text = *name;
	 description_text = *description;
      }
      else if( ScenarioList::NameExists( *name ) ) {
	 fl_set_resource( "flAlert.title", "Error: Duplicate Scenario Group Name Detected" );
	 sprintf( buffer, "The entered name \"%s\" already exists. ", *name );
	 fl_show_alert( buffer, "Please enter a unique name." , "", 0 );
	 duplicate_name = TRUE;
	 name_text = *name;
	 description_text = *description;
      }
   } while(duplicate_name == TRUE) ;

   return TRUE;
}

extern "C"
void DeleteGroupButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   if( current_group != NULL ) {
      if( current_group->ScenarioCount() == 0 ) {
	 scenario_groups->Delete( current_group );
	 delete current_group;
	 selected_group = 0;
	 UpdateScenarioGroups(); 
	 SetPromptUserForSave();
      }
   }
}

void UpdateScenarios()
{
   fl_freeze_form( pfdss->ScenarioSpecification );
   fl_clear_browser( pfdss->ScenariosList );
   fl_clear_browser( pfdss->ScenarioDescription );

   if( current_group ) {
      scenarios = current_group->ScenarioSet();
      for( scenarios->First(); !scenarios->IsDone(); scenarios->Next() )
	 fl_add_browser_line( pfdss->ScenariosList, scenarios->CurrentItem()->Name() );
      fl_unfreeze_form( pfdss->ScenarioSpecification );
      ENABLE_WIDGET( pfdss->AddScenarioButton );
   } else {
      fl_unfreeze_form( pfdss->ScenarioSpecification );
      DISABLE_WIDGET( pfdss->AddScenarioButton );
   }

   DISABLE_WIDGET( pfdss->EditScenarioButton );
   DISABLE_WIDGET( pfdss->DeleteScenarioButton );
   DISABLE_WIDGET( pfdss->ChangeGroupButton );
   current_scenario = NULL;
   initialization_edit_mode = TRUE;
   fl_set_button( pfdss->InitializationType, 1 );
   fl_set_button( pfdss->PostconditionType, 0 );
   UpdateSelectedScenario();
}

extern "C"
void ScenariosList_cb(FL_OBJECT *sl, long)
{
   const char *description;
   ScenarioGroup *new_group;
   int index;

   if( group_change ) {
      if( (index = fl_get_browser( sl )) != 0 ) {
	 new_group = scenario_groups->Get( index );
	 new_group->ScenarioSet()->Add( current_scenario );
	 current_group->ScenarioSet()->Delete( current_scenario );
      }
      TerminateGroupChange();
      return;
   }

   if( (index = fl_get_browser( sl )) == 0 ) return;
   if( index == selected_scenario )
      return;
   else
      selected_scenario = index; 
   current_scenario = scenarios->Get( selected_scenario );

   ENABLE_WIDGET( pfdss->EditScenarioButton );
   ENABLE_WIDGET( pfdss->DeleteScenarioButton );
   ENABLE_WIDGET( pfdss->ChangeGroupButton );

   fl_clear_browser( pfdss->ScenarioDescription );
   if( (description = current_scenario->Description()) != NULL )
      fl_add_browser_line( pfdss->ScenarioDescription, description );
   
   UpdateSelectedScenario();
}

extern "C"
void AddScenarioButton_cb(FL_OBJECT *, long)
{
   char *entered_name, *entered_description;
   
   if( group_change ) return; // disable during group change mode of dialog
   if( EditScenario( NULL, NULL, &entered_name, &entered_description ) == FALSE )
      return;

   if( current_group->ScenarioCount() == 0 ) {
      DISABLE_WIDGET( pfdss->DeleteGroupButton );
   } 
   current_group->AddScenario( entered_name, entered_description );
   UpdateScenarios();
   SetPromptUserForSave();
}

extern "C"
void EditScenarioButton_cb(FL_OBJECT *, long)
{
   char *entered_name, *entered_description;

   if( group_change ) return; // disable during group change mode of dialog
   if( current_scenario == NULL ) return;

   if( EditScenario( current_scenario->Name(), current_scenario->Description(), &entered_name, &entered_description ) == FALSE )
      return;

   current_scenario->Name( entered_name );
   current_scenario->Description( entered_description );

   fl_replace_browser_line( pfdss->ScenariosList, selected_scenario, entered_name );
   fl_clear_browser( pfdss->ScenarioDescription );
   fl_add_browser_line( pfdss->ScenarioDescription, entered_description );
   SetPromptUserForSave();
}

bool EditScenario( const char *default_name, const char *default_description, char **name, char **description )
{
   bool duplicate_name;
   char buffer[200], old_name[100];
   const char *prompt, *name_text, *description_text;

   if( default_name == NULL )
      prompt = "Add New Scenario";
   else {
      prompt = "Edit Selected Scenario";
      strcpy( old_name, default_name);
   }

   name_text = default_name;
   description_text = default_description;

   do {
      duplicate_name = FALSE;
      if( StringParagraphDialog( prompt, "Scenario Name", "Scenario Description", name_text, description_text, name, description ) == FALSE )
	 return FALSE;
      if( default_name != NULL ) {
	 if( strequal( old_name, *name ) )
	    return TRUE;
      }
      if( !PrintManager::TextNonempty( *name ) ) {
	 fl_set_resource( "flAlert.title", "Error: Empty Scenario Name Detected" );
	 sprintf( buffer, "The entered name \"%s\" is disallowed. ", *name );
	 fl_show_alert( buffer, "Please enter a nonempty and unique name." , "", 0 );
	 duplicate_name = TRUE;
	 name_text = *name;
	 description_text = *description;
      }
      else if( current_group->NameExists( *name ) ) {
	 fl_set_resource( "flAlert.title", "Error: Duplicate Scenario Name Detected" );
	 sprintf( buffer, "The entered name \"%s\" already exists. ", *name );
	 fl_show_alert( buffer, "Please enter a unique name." , "", 0 );
	 duplicate_name = TRUE;
	 name_text = *name;
	 description_text = *description;
      }
   } while(duplicate_name == TRUE) ;

   return TRUE;
}

extern "C"
void DeleteScenarioButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   if( current_scenario != NULL ) {
      scenarios->Delete( current_scenario );
      delete current_scenario;
      if( scenarios->Size() == 0 ) {
	 ENABLE_WIDGET( pfdss->DeleteGroupButton );
      } 
      UpdateScenarios(); 
      SetPromptUserForSave();
   }
}

extern "C"
void ChangeGroupButton_cb(FL_OBJECT *, long)
{
   ScenarioGroup *cg;
   char buffer[70];

   if( group_change ) {
      TerminateGroupChange();
      return;
   }

   if( current_scenario == NULL ) return;

   // temporarily change labels of group and scenario browsers and contents of scenario browser
   fl_freeze_form( pfdss->ScenarioSpecification );
   fl_set_object_label( pfdss->GroupsBox, "Current Group of Selected Scenario(s)" );
   fl_set_object_label( pfdss->ScenariosBox, "Select New Group for Selected Scenario(s)\nPress \"Change Group\" button again to cancel operation." );
   fl_clear_browser( pfdss->ScenariosList );

   for( scenario_groups->First(); !scenario_groups->IsDone(); scenario_groups->Next() ) {
      cg = scenario_groups->CurrentItem();
      if( cg == current_group ) {
	 sprintf( buffer, "@N%s", cg->GroupName() );
	 fl_add_browser_line( pfdss->ScenariosList, buffer );
      } else
	 fl_add_browser_line( pfdss->ScenariosList, cg->GroupName() );
   }

   fl_deactivate_object( pfdss->GroupsList );
   fl_deactivate_object( pfdss->BooleanVariables );
   fl_deactivate_object( pfdss->VariableInitializations );
   fl_unfreeze_form( pfdss->ScenarioSpecification );
   group_change = TRUE;
}

void TerminateGroupChange()
{
   fl_freeze_form( pfdss->ScenarioSpecification );
   fl_activate_object( pfdss->GroupsList );
   fl_activate_object( pfdss->BooleanVariables );
   fl_activate_object( pfdss->VariableInitializations );
   fl_set_object_label( pfdss->GroupsBox, "Scenario Groups" ); // reset browser labels
   fl_set_object_label( pfdss->ScenariosBox, "Scenario" );

   fl_clear_browser( pfdss->ScenariosList );
   for( scenarios->First(); !scenarios->IsDone(); scenarios->Next() )
      fl_add_browser_line( pfdss->ScenariosList, scenarios->CurrentItem()->Name() );

   fl_unfreeze_form( pfdss->ScenarioSpecification );
   group_change = FALSE;
}

void UpdateSelectedScenario()
{
   Cltn<BooleanVariable *> *bool_variables;
   VariableInit *current_vref;
   BooleanVariable *cv;
   char buffer[200];

   fl_freeze_form( pfdss->ScenarioSpecification );
   fl_clear_browser( pfdss->VariableInitializations );
   fl_clear_browser( pfdss->BooleanVariables );

   if( current_scenario != NULL ) {

      bool_variables = BooleanVariable::VariableList();

      if( initialization_edit_mode )
	 variable_references = current_scenario->VariableInitializations();
      else
	 variable_references = current_scenario->ScenarioPostconditions();

      BooleanVariable::DereferenceVariableList();

      for( variable_references->First(); !variable_references->IsDone(); variable_references->Next() ) {
	 current_vref = variable_references->CurrentItem();
	 sprintf( buffer, "%s = %s", current_vref->Variable()->BooleanName(), ((current_vref->Value() == TRUE) ? "T" : "F") );
	 fl_add_browser_line( pfdss->VariableInitializations, buffer );
	 current_vref->Variable()->Reference();
      }

      for( bool_variables->First(); !bool_variables->IsDone(); bool_variables->Next() ) {
	 cv = bool_variables->CurrentItem();
	 if( cv->IsNotReferenced() )
	    fl_add_browser_line( pfdss->BooleanVariables, cv->BooleanName() );
      }
      
      UpdateStartingPoints();
      fl_unfreeze_form( pfdss->ScenarioSpecification );
      ENABLE_WIDGET( pfdss->ScenarioOperationGroup );
      ENABLE_WIDGET( pfdss->AddInitButton );
      ENABLE_WIDGET( pfdss->EditStartList );
   }
   else {
      fl_clear_browser( pfdss->StartingPoints );
      fl_unfreeze_form( pfdss->ScenarioSpecification );
      DISABLE_WIDGET( pfdss->ScenarioOperationGroup );
      DISABLE_WIDGET( pfdss->AddInitButton );
      DISABLE_WIDGET( pfdss->EditStartList );
   }

   DISABLE_WIDGET( pfdss->DeleteInitButton );
   DISABLE_WIDGET( pfdss->ToggleInitButton );
   DISABLE_WIDGET( pfdss->AddSelectedGroupButton );
   
   current_reference = NULL;
   current_variable = NULL;
   selected_reference = 0;
}

void UpdateStartingPoints()
{
   Cltn<Start *> *starting_points = current_scenario->StartingPoints();
   Start *start;
   char buffer[200];

   fl_freeze_form( pfdss->ScenarioSpecification );
   fl_clear_browser( pfdss->StartingPoints );

   for( starting_points->First(); !starting_points->IsDone(); starting_points->Next() ) {
      start = starting_points->CurrentItem();
      sprintf( buffer, "%s (%s)", start->HyperedgeName(), start->ParentMap()->MapLabel() );
      fl_add_browser_line( pfdss->StartingPoints, buffer );
   }

   fl_unfreeze_form( pfdss->ScenarioSpecification );
}

extern "C"
void InitializationType_cb(FL_OBJECT *, long)
{
   if( initialization_edit_mode == FALSE ) {
      initialization_edit_mode = TRUE;
      fl_set_object_label( pfdss->VariableInitializations, "Variable Initializations" );
      fl_set_object_label( pfdss->AddInitButton, "Add Variable Initialization" );
      fl_set_object_label( pfdss->DeleteInitButton, "Delete Initialization" );
      UpdateSelectedScenario();
   }
}

extern "C"
void PostconditionType_cb(FL_OBJECT *, long)
{
   if( initialization_edit_mode == TRUE ) {
      initialization_edit_mode = FALSE;
      fl_set_object_label( pfdss->VariableInitializations, "Scenario Postconditions" );
      fl_set_object_label( pfdss->AddInitButton, "Add Scenario Postcondition" );
      fl_set_object_label( pfdss->DeleteInitButton, "Delete Postcondition" );
      UpdateSelectedScenario();
   }
}

extern "C"
void VariableInitializations_cb(FL_OBJECT *rl, long)
{
   int index;

   if( (index = fl_get_browser( rl )) == 0 ) return;
   if( index == selected_reference )
      return;
   else
      selected_reference = index;

   current_reference = variable_references->Get( selected_reference );

   ENABLE_WIDGET( pfdss->DeleteInitButton );
   ENABLE_WIDGET( pfdss->ToggleInitButton );
}

extern "C"
void ViewBVButton_cb(FL_OBJECT *, long)
{
   fl_deactivate_form( pfdss->ScenarioSpecification );
   ListBooleanVariables( BooleanVariable::VariableList(), TRUE );
}

void TerminateBVEditing( bool changed )
{
   if( changed == TRUE )
      UpdateSelectedScenario();
   fl_activate_form( pfdss->ScenarioSpecification );
}

extern "C"
void DeleteInitButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   if( current_reference != NULL ) {
      current_reference->Variable()->DecrementReferenceCount();
      variable_references->Delete( current_reference );
      delete current_reference;
      UpdateSelectedScenario();
      SetPromptUserForSave();
   }
}

extern "C"
void ToggleInitButton_cb(FL_OBJECT *, long)
{
   char buffer[100];

   if( current_reference != NULL ) {
      current_reference->ToggleValue();
      sprintf( buffer, "%s = %s", current_reference->Variable()->BooleanName(), ((current_reference->Value() == TRUE) ? "T" : "F") );
      fl_replace_browser_line( pfdss->VariableInitializations, selected_reference, buffer );
      SetPromptUserForSave();
   }
}

extern "C"
void AddInitButton_cb(FL_OBJECT *, long)
{
   int index;
   BooleanVariable *selected_variable;

   if( group_change ) return; // disable during group change mode of dialog
   if( (index = fl_get_browser( pfdss->BooleanVariables )) == 0 ) return;

   selected_variable = BooleanVariable::SelectedVariable( index );
   variable_references->Add( new VariableInit( selected_variable, fl_get_button( pfdss->TrueButton )));
   selected_variable->IncrementReferenceCount();
   UpdateSelectedScenario();
   SetPromptUserForSave();
}

extern "C"
void EditStartList_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   if( current_scenario == NULL ) return;

   // invoke start point list dialog
   EditPathStartList();
}

extern "C"
void AddSelectedGroupButton_cb(FL_OBJECT *, long)
{
   int index;

   if( group_change ) return; // disable during group change mode of dialog
   if( (index = fl_get_browser( pfdss->StartingPoints )) == 0 ) return;

   current_group->AddStartingPoint( current_scenario->StartingPoints()->Get( index ) );
   SetPromptUserForSave();
//   DISABLE_WIDGET( pfdss->AddSelectedGroupButton );
}

extern "C"
void StartingPoints_cb(FL_OBJECT *spl, long)
{
   if( fl_get_browser( spl ) != 0 ) {
      ENABLE_WIDGET( pfdss->AddSelectedGroupButton );
   }
}

extern "C"
void TraceButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   if( current_scenario == NULL ) return;
   if( !current_scenario->HasStartingPoints() ) return;
   HighlightScenario( current_scenario );
}

void HighlightScenario( Scenario *hs )
{
   ScenarioList::SetScenarioTracing( hs );
   display_manager->SetScenarioHighlight( hs );
   DrawScreen();
}

extern "C"
void GroupMSCButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   current_group->GenerateMSCs();
}

extern "C"
void GroupXMLButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   current_group->GenerateGroupXML();
}

extern "C"
void ScenarioMSCButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   if( current_scenario == NULL ) return;

   if( !current_scenario->HasStartingPoints() ) {
      fl_set_resource( "flAlert.title", "Error: Starting Point for Scenario Not Defined" );
      fl_show_alert( "The selected scenario does not have a start point specified.", "Please specify a starting point." , "", 0 );
      return;
   }

   MSCGenerator::Instance()->GenerateScenarioMSC( current_scenario );
   
}

extern "C"
void ScenarioXMLButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   if( current_scenario == NULL ) return;

   if( !current_scenario->HasStartingPoints() ) {
      fl_set_resource( "flAlert.title", "Error: Starting Point for Scenario Not Defined" );
      fl_show_alert( "The selected scenario does not have a start point specified.", "Please specify a starting point." , "", 0 );
      return;
   }

   SCENARIOGenerator::Instance()->TraverseScenario( current_scenario );
}

extern "C"
void Duplicate_cb(FL_OBJECT *, long)
{
   Scenario *duplicate_scenario;
   char *entered_name, *entered_description;
   
   if( group_change ) return; // disable during group change mode of dialog
   if( current_scenario == NULL ) return;

   if( EditScenario( NULL, NULL, &entered_name, &entered_description ) == FALSE )
      return;

   duplicate_scenario = current_group->AddScenario( entered_name, entered_description );
   duplicate_scenario->DuplicateScenario( current_scenario );
   UpdateScenarios(); 
   SetPromptUserForSave();
}

extern "C"
void VSRemoveDialogButton_cb(FL_OBJECT *, long)
{
   if( group_change ) return; // disable during group change mode of dialog
   fl_hide_form( pfdss->ScenarioSpecification );
   UNFREEZE_TOOL();
}

int VSDialogClose( FL_FORM *, void * )
{
   VSRemoveDialogButton_cb( 0, 0 );
   return( FL_IGNORE );
}

// callbacks for start scenario dialog

void ListStartScenarios( Start *start )
{
   char buffer[200];
   Scenario *cs;

   FREEZE_TOOL();
   scenarios = start->StartScenarios();

   if( pfdsl == NULL ) {
      pfdsl = create_form_StartScenarios();
      fl_set_browser_fontsize( pfdsl->ScenarioList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsl->ScenarioList, FL_BOLD_STYLE );
      fl_set_form_atclose( pfdsl->StartScenarios, LSSDialogClose, NULL );
   }

   fl_clear_browser( pfdsl->ScenarioList );

   for( scenarios->First(); !scenarios->IsDone(); scenarios->Next() ) {
      cs = scenarios->CurrentItem();
      sprintf( buffer, "%s (%s)", cs->Name(), cs->Group()->GroupName() );
      fl_add_browser_line( pfdsl->ScenarioList, buffer );
   }

   selected_scenario = 0;
   DISABLE_WIDGET( pfdsl->Generate );
   DISABLE_WIDGET( pfdsl->GenerateXML );
   DISABLE_WIDGET( pfdsl->HighlightPath );

   sprintf( buffer, "Scenarios for Start Point \"%s\"", start->HyperedgeName() );

   fl_show_form( pfdsl->StartScenarios, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, buffer );
}

extern "C"
void ScenarioList_cb(FL_OBJECT *sl, long)
{
   if( (selected_scenario = fl_get_browser( sl )) == 0 ) return;
   ENABLE_WIDGET( pfdsl->Generate );
   ENABLE_WIDGET( pfdsl->GenerateXML );
   ENABLE_WIDGET( pfdsl->HighlightPath );
}

extern "C"
void HighlightPath_cb(FL_OBJECT *, long)
{
   if( selected_scenario == 0 ) return;
   HighlightScenario( scenarios->Get( selected_scenario ) );
}

extern "C"
void Generate_cb(FL_OBJECT *, long)
{
   if( selected_scenario == 0 ) return;
   MSCGenerator::Instance()->GenerateScenarioMSC( scenarios->Get( selected_scenario ) );
   //SCENARIOGenerator::Instance()->TraverseScenario( scenarios->Get( selected_scenario ) );
}

extern "C"
void GenerateXML_cb(FL_OBJECT *, long)
{
   SCENARIOGenerator * sce = SCENARIOGenerator::Instance(); 
   
   if( selected_scenario == 0 ) return;
   // for( scenarios->First(); !scenarios->IsDone(); scenarios->Next() )
   sce->TraverseScenario(  scenarios->Get( selected_scenario ));
}

extern "C"
void LSSClose_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdsl->StartScenarios );
   UNFREEZE_TOOL();
}

int LSSDialogClose( FL_FORM *, void * )
{
   LSSClose_cb( 0, 0 );
   return( FL_IGNORE );
}

// callbacks for scenario start list editor dialog

void EditPathStartList()
{
   char title[200], buffer[200];
   Cltn<Map *> *map_list;
   Map *map;
   int index;

   original_map = display_manager->CurrentMap();
   fl_hide_form( pfdss->ScenarioSpecification );

   if( pfdesl == NULL ) {
      pfdesl = create_form_EditStartList();
      fl_set_form_atclose( pfdesl->EditStartList, ESLDialogClose, NULL );
      fl_set_browser_fontsize( pfdesl->ScenarioStartList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdesl->ScenarioStartList, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdesl->MapsList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdesl->MapsList, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdesl->PathStartList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdesl->PathStartList, FL_BOLD_STYLE );
   }

   start_list = current_scenario->StartingPoints();
   UpdateScenarioStartList();
   DrawScreen();

   // fill maps list and start point list
   map_list = display_manager->Maps();
   fl_clear_browser( pfdesl->MapsList );

   for( map_list->First(); !map_list->IsDone(); map_list->Next() ) {
      map = map_list->CurrentItem();
      sprintf( buffer, "%s (%s)", map->MapLabel(), ((map->IsRootMap()) ? "Root" : "Plugin"));
      fl_add_browser_line( pfdesl->MapsList, buffer );
   }

   index = map_list->in( original_map ) + 1;
   fl_select_browser_line( pfdesl->MapsList, index );
   fl_set_browser_topline( pfdesl->MapsList, index );
   selected_map = index;
   current_map = original_map;
   CreateMapStartList( original_map );

   sprintf( title, "Start Points for Scenario \"%s\"", current_scenario->Name() );
   fl_show_form( pfdesl->EditStartList, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, title );
}

void UpdateScenarioStartList( bool deselect )
{
   Start *start;
   char buffer[200];

   fl_freeze_form( pfdesl->EditStartList );
   fl_clear_browser( pfdesl->ScenarioStartList );
   selected_scenario_start = 0;

   for( start_list->First(); !start_list->IsDone(); start_list->Next() ) {
      start = start_list->CurrentItem();
      sprintf( buffer, "%s (%s)", start->HyperedgeName(), start->ParentMap()->MapLabel() );
      fl_add_browser_line( pfdesl->ScenarioStartList, buffer );
      start->HighlightEntry();
   }

   fl_unfreeze_form( pfdesl->EditStartList );
   if( deselect ) {
      DISABLE_WIDGET( pfdesl->RemoveStart );
      DISABLE_WIDGET( pfdesl->Lower );
      DISABLE_WIDGET( pfdesl->Raise );
   }
}

extern "C"
void ScenarioStartList_cb(FL_OBJECT *sl, long)
{
   int index;

   if( (index = fl_get_browser( sl )) == 0 ) return;
   if( index == selected_scenario_start )
      return;
   else {
      if( selected_scenario_start == 0 ) {
	 ENABLE_WIDGET( pfdesl->RemoveStart );
	 ENABLE_WIDGET( pfdesl->Lower );
	 ENABLE_WIDGET( pfdesl->Raise );
      }
      selected_scenario_start = index;
   }

   current_map = start_list->Get( selected_scenario_start )->ParentMap();
   DisplayMap( current_map );
   index = display_manager->Maps()->in( current_map ) + 1;
   fl_select_browser_line( pfdesl->MapsList, index );
   fl_set_browser_topline( pfdesl->MapsList, index );
}

extern "C"
void MapsList_cb(FL_OBJECT *ml, long)
{
   int index;

   if( (index = fl_get_browser( ml )) == 0 ) return;
   if( index == selected_map )
      return;
   else
      selected_map = index;

   current_map = display_manager->Maps()->Get( selected_map );
   DisplayMap( current_map );
}

void DisplayMap( Map *map )
{
   CreateMapStartList( current_map );
   display_manager->DisplayOnlyMode( current_map );
}

void CreateMapStartList( Map *map )
{
   Cltn<Hyperedge *> *edges = map->MapHypergraph()->Hyperedges();
   Hyperedge *current_edge;

   fl_freeze_form( pfdesl->EditStartList );
   fl_clear_browser( pfdesl->PathStartList );

   for( edges->First(); !edges->IsDone(); edges->Next() ) {
      current_edge = edges->CurrentItem();
      if( current_edge->EdgeType() == START ) {
	 if( !start_list->Includes( (Start *)current_edge ))
	    fl_add_browser_line( pfdesl->PathStartList, current_edge->HyperedgeName() );
      }
   }

   fl_unfreeze_form( pfdesl->EditStartList );
   DISABLE_WIDGET( pfdesl->AddStart );
}

extern "C"
void PathStartList_cb(FL_OBJECT *psl, long)
{
   if( fl_get_browser( psl ) != 0 ) {
      ENABLE_WIDGET( pfdesl->AddStart );
   }
}

extern "C"
void RemoveStart_cb(FL_OBJECT *, long)
{
   int index;
   Start *start;

   if( (index = fl_get_browser( pfdesl->ScenarioStartList )) == 0 ) return;
   start = start_list->Get( index );
   current_scenario->DeleteStartingPoint( start );
   UpdateScenarioStartList();
   if( start->ParentMap() == current_map )
      CreateMapStartList( current_map ); // add deleted start back to list
   SetPromptUserForSave();
   start->ResetHighlight();
   DrawScreen();
}

extern "C"
void AddStart_cb(FL_OBJECT *, long)
{
   int index, i = 1;
   Start *start;
   Cltn<Hyperedge *> *edges;
   Hyperedge *current_edge;

   if( (index = fl_get_browser( pfdesl->PathStartList )) == 0 ) return;
   edges = current_map->MapHypergraph()->Hyperedges();

   for( edges->First(); !edges->IsDone(); edges->Next() ) {
      current_edge = edges->CurrentItem();
      if( current_edge->EdgeType() == START ) {
	 if( !start_list->Includes( (Start *)current_edge )) {
	    if( i == index ) {
	       start = (Start *)current_edge;
	       break;
	    }
	    i++;
	 }
      }
   }

   current_scenario->AddStartingPoint( start );
   UpdateScenarioStartList();
   CreateMapStartList( current_map ); //remove start from available list
   SetPromptUserForSave();
   DrawScreen();
}

extern "C"
void Lower_cb(FL_OBJECT *, long)
{
   int index;
   Start *start;

   if( (index = fl_get_browser( pfdesl->ScenarioStartList )) == 0 ) return;
   if( index == fl_get_browser_maxline( pfdesl->ScenarioStartList ) ) return; // can't lower anymore
   start = start_list->Get( index );
   start_list->Delete( start );
   start_list->insertnth( index-1, start );
   UpdateScenarioStartList( FALSE );
   fl_select_browser_line( pfdesl->ScenarioStartList, index+1 );
   SetPromptUserForSave();
}

extern "C"
void Raise_cb(FL_OBJECT *, long)
{
   int index;
   Start *start;

   if( (index = fl_get_browser( pfdesl->ScenarioStartList )) < 2 ) return;
   start = start_list->Get( index );
   start_list->Delete( start );
   start_list->insertnth( index-3, start );
   UpdateScenarioStartList( FALSE );
   fl_select_browser_line( pfdesl->ScenarioStartList, index-1 );
   SetPromptUserForSave();
}

extern "C"
void SPClose_cb(FL_OBJECT *, long)
{
   for( start_list->First(); !start_list->IsDone(); start_list->Next() )
      start_list->CurrentItem()->ResetHighlight();
  
   display_manager->RestoreMapNavigation();
   DrawScreen();
   fl_hide_form( pfdesl->EditStartList );
   UpdateStartingPoints();

   fl_show_form( pfdss->ScenarioSpecification, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, prompt );
}

int ESLDialogClose( FL_FORM *, void * )
{
   SPClose_cb( 0, 0 );
   return( FL_IGNORE );
}

