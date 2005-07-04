/***********************************************************
 *
 * File:			scenario.cc
 * Author:			Andrew Miga
 * Created:			September 2000
 *
 * Modification history:
 *
 ***********************************************************/

#include "scenario.h"
#include "xml_generator.h"
#include "map.h"
#include "start.h"
#include "print_mgr.h"
#include "variable.h"
#include "msc_generator.h"
#include <stdlib.h>

extern void ViewScenarios( Cltn<ScenarioGroup *> *, Hyperedge * );
extern void ListStartScenarios( Start * );

Cltn<ScenarioGroup *> ScenarioList::scenario_groups;
unsigned int ScenarioList::scenario_tracing_id = 0;
bool ScenarioList::tracing_mode = FALSE;

VariableInit::VariableInit( int cvar, int val )
{
   variable = BooleanVariable::ReferencedVariable( cvar );
   value = val;
}

Scenario::Scenario( const char *name, const char *description )
{
   scenario_name[LABEL_SIZE] = 0;
   strncpy( scenario_name, name, LABEL_SIZE );
   scenario_description = ( PrintManager::TextNonempty( description ) ? strdup( description ) : NULL );
}

Scenario::~Scenario()
{
   VariableInit *cvi;

   while( !variable_initializations.is_empty() ) {
      cvi = variable_initializations.Detach();
      cvi->Variable()->DecrementReferenceCount();
      delete cvi;
   }

   while( !scenario_postconditions.is_empty() ) {
      cvi = scenario_postconditions.Detach();
      cvi->Variable()->DecrementReferenceCount();
      delete cvi;
   }
// Modified by DA, December 2004. 
// May need to change this ???
//   

   while( !starting_points.is_empty() )
      starting_points.Detach()->RemoveScenario( this );

   if( scenario_description != NULL ) free( scenario_description );
}

void Scenario::Description( const char *new_description )
{
   if( scenario_description != NULL ) free( scenario_description );
   scenario_description = ( PrintManager::TextNonempty( new_description ) ? strdup( new_description ) : NULL );
}

void Scenario::AddStartingPoint( Start *new_start )
{
   if( new_start == NULL ) return;

   if( !starting_points.Includes( new_start ) ) {
      new_start->AddScenario( this );
   }
// Modified by DA, December 2004. This line was moved out of the previous 
// condition because it prevented the use of the same start point many times 
// in a scenario definition.
//   
   starting_points.Add( new_start );
}

void Scenario::DeleteStartingPoint( Start *start )
{
   starting_points.Delete( start );
   start->RemoveScenario( this );
}

void Scenario::DuplicateScenario( Scenario *original )
{
   Cltn<Start *> *start_list;
   Start *sp;
   Cltn<VariableInit *> *original_init;
   VariableInit *cvi;

   start_list = original->StartingPoints();

   for( start_list->First(); !start_list->IsDone(); start_list->Next() ) {
      sp = start_list->CurrentItem();
      starting_points.Add( sp );
      sp->AddScenario( this );
   }

   original_init = original->VariableInitializations();

   for( original_init->First(); !original_init->IsDone(); original_init->Next() ) {
      cvi = original_init->CurrentItem();
      variable_initializations.Add( new VariableInit( cvi->Variable(), cvi->Value() ) );
      cvi->Variable()->IncrementReferenceCount();
   }

   original_init = original->ScenarioPostconditions();

   for( original_init->First(); !original_init->IsDone(); original_init->Next() ) {
      cvi = original_init->CurrentItem();
      scenario_postconditions.Add( new VariableInit( cvi->Variable(), cvi->Value() ) );
      cvi->Variable()->IncrementReferenceCount();
   }
}

bool Scenario::IsContainedIn( Cltn<Map *> *map_list )
{
   for( starting_points.First(); !starting_points.IsDone(); starting_points.Next() ) {
      if( !map_list->Includes( starting_points.CurrentItem()->ParentMap() )) 
	 return FALSE;
   }

   return TRUE;
}

void Scenario::SaveScenario( FILE *fp, Cltn<Map *> *map_list )
{
   VariableInit *cvi;
   Start *cs;
   char buffer[300];

   if( map_list != NULL ) {
      if( !this->IsContainedIn( map_list ) )
	 return;
   }

   sprintf( buffer, "<scenario-definition name=\"%s\" ", PrintDescription( scenario_name ) );
   PrintXMLText( fp, buffer );
   if( PrintManager::TextNonempty( scenario_description ) )
      fprintf( fp, "description=\"%s\" ", PrintDescription( scenario_description ) );
   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );

   for( starting_points.First(); !starting_points.IsDone(); starting_points.Next() ) {
      cs = starting_points.CurrentItem();
      sprintf( buffer, "scenario-start map-id=\"m%d\" start-id=\"h%d\" /", cs->ParentMap()->GetNumber(), cs->GetNumber() );
      PrintXML( fp, buffer );
   }

   for( variable_initializations.First(); !variable_initializations.IsDone(); variable_initializations.Next() ) {
      cvi = variable_initializations.CurrentItem();
      sprintf( buffer, "variable-init variable-id=\"bv%d\" value=\"%s\" /", cvi->Variable()->BooleanId(), ( (cvi->Value() == TRUE) ? "T" : "F" ) );
      PrintXML( fp, buffer );
   }

   for( scenario_postconditions.First(); !scenario_postconditions.IsDone(); scenario_postconditions.Next() ) {
      cvi = scenario_postconditions.CurrentItem();
      sprintf( buffer, "postcondition variable-id=\"bv%d\" value=\"%s\" /", cvi->Variable()->BooleanId(), ( (cvi->Value() == TRUE) ? "T" : "F" ) );
      PrintXML( fp, buffer );
   }

   PrintEndXMLBlock( fp, "scenario-definition" );
}

void Scenario::SaveCSMScenario( FILE *fp, Cltn<Map *> *map_list )
{
   Start *cs;
   char buffer[300];

   if( map_list != NULL ) {
      if( !this->IsContainedIn( map_list ) )
	 return;
   }

   sprintf( buffer, "<scenario name=\"%s\" ", PrintDescription( scenario_name ) );
   PrintXMLText( fp, buffer );
   
   for( starting_points.First(); !starting_points.IsDone(); starting_points.Next() ) {
      cs = starting_points.CurrentItem();
      sprintf( buffer, " root=\"h%d\" ", cs->GetNumber() );
      PrintXMLText( fp, buffer );
   }
   
   //if( PrintManager::TextNonempty( scenario_description ) )
     // fprintf( fp, "description=\"%s\" ", PrintDescription( scenario_description ) );
   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );
/*
   for( starting_points.First(); !starting_points.IsDone(); starting_points.Next() ) {
      cs = starting_points.CurrentItem();
      sprintf( buffer, "scenario-start map-id=\"m%d\" start-id=\"h%d\" /", cs->ParentMap()->GetNumber(), cs->GetNumber() );
      PrintXML( fp, buffer );
   }

   for( variable_initializations.First(); !variable_initializations.IsDone(); variable_initializations.Next() ) {
      cvi = variable_initializations.CurrentItem();
      sprintf( buffer, "variable-init variable-id=\"bv%d\" value=\"%s\" /", cvi->Variable()->BooleanId(), ( (cvi->Value() == TRUE) ? "T" : "F" ) );
      PrintXML( fp, buffer );
   }

   for( scenario_postconditions.First(); !scenario_postconditions.IsDone(); scenario_postconditions.Next() ) {
      cvi = scenario_postconditions.CurrentItem();
      sprintf( buffer, "postcondition variable-id=\"bv%d\" value=\"%s\" /", cvi->Variable()->BooleanId(), ( (cvi->Value() == TRUE) ? "T" : "F" ) );
      PrintXML( fp, buffer );
   }
*/
   PrintEndXMLBlock( fp, "scenario" );
}


void Scenario::GeneratePostScript( FILE *ps_file )
{
   Start *cs;
   VariableInit *cvi;

   fprintf( ps_file, "%d rom (Scenario - ) S %d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, 
	    PrintManager::PrintPostScriptText( scenario_name ) );
   
   if( PrintManager::TextNonempty( scenario_description ) ){
      fprintf( ps_file, "%d rom\n1 IN ID (Scenario Description ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID\n" );
      PrintManager::PrintDescription( scenario_description );
      fprintf( ps_file, "OD\n" );
   }

   if( starting_points.Size() > 0 ) {
      fprintf( ps_file, "%d rom\n1 IN ID (Scenario Starting Points ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID\n" );
      for( starting_points.First(); !starting_points.IsDone(); starting_points.Next() ) {
	 cs = starting_points.CurrentItem();
     	 fprintf( ps_file, "%d rom (Start Point - ) S %d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size,
		  PrintManager::PrintPostScriptText( cs->HyperedgeName() ) );
	 fprintf( ps_file, "%d rom (Map - ) S %d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size,
		  PrintManager::PrintPostScriptText( cs->ParentMap()->MapLabel() ) );
      }
      fprintf( ps_file, "OD\n" );
   }

   if( variable_initializations.Size() > 0 ) {
      fprintf( ps_file, "%d rom\n1 IN ID (Variable Initializations ) P OD\n2 IN ID\n", PrintManager::text_font_size );
      for( variable_initializations.First(); !variable_initializations.IsDone(); variable_initializations.Next() ) {
	 cvi = variable_initializations.CurrentItem();
	 fprintf( ps_file, "(%s = %s ) P\n", cvi->Variable()->BooleanName(), ( (cvi->Value() == TRUE) ? "T" : "F" ) );
      }
      fprintf( ps_file, "OD\n" );
   }

   if( scenario_postconditions.Size() > 0 ) {
      fprintf( ps_file, "%d rom\n1 IN ID (Scenario Postconditions ) P OD\n2 IN ID\n", PrintManager::text_font_size );
      for( scenario_postconditions.First(); !scenario_postconditions.IsDone(); scenario_postconditions.Next() ) {
	 cvi = scenario_postconditions.CurrentItem();
	 fprintf( ps_file, "(%s = %s ) P\n", cvi->Variable()->BooleanName(), ( (cvi->Value() == TRUE) ? "T" : "F" ) );
      }
      fprintf( ps_file, "OD\n" );
   }
}

int Scenario::ScenarioId( )
{
  int id = 0;
  Cltn<Scenario *> * scenario_set = group->ScenarioSet(); 
  for( scenario_set->First(); !scenario_set->IsDone(); scenario_set->Next() ) {
     id++;
     if( scenario_set->CurrentItem() == this ) {
        return id;
     }
  }
  return -1;
}

ScenarioGroup::ScenarioGroup( const char *name, const char *description )
{
   group_name[LABEL_SIZE] = 0;
   strncpy( group_name, name, LABEL_SIZE );
   group_description = ( PrintManager::TextNonempty( description ) ? strdup( description ) : NULL );
}

ScenarioGroup::~ScenarioGroup()
{
   while( !scenarios.is_empty() )
      delete scenarios.Detach();

   if( group_description != NULL ) free( group_description );
}

void ScenarioGroup::GroupDescription( const char *new_description )
{
   if( group_description != NULL ) free( group_description );
   group_description = ( PrintManager::TextNonempty( new_description ) ? strdup( new_description ) : NULL );
}

Scenario * ScenarioGroup::AddScenario( const char *name, const char *description )
{
   Scenario *new_scenario = new Scenario( name, description );
   scenarios.Add( new_scenario );
   new_scenario->SetGroup( this );
   return( new_scenario );
}

bool ScenarioGroup::NameExists( const char *new_name )
{
   for( scenarios.First(); !scenarios.IsDone(); scenarios.Next() ) {
      if( strequal( new_name, scenarios.CurrentItem()->Name() ) )
	 return TRUE;
   }

   return FALSE;
}

bool ScenarioGroup::IsContainedIn( Cltn<Map *> *map_list )
{
   for( scenarios.First(); !scenarios.IsDone(); scenarios.Next() ) {
      if( scenarios.CurrentItem()->IsContainedIn( map_list ) )
	 return TRUE;
   }
   return FALSE;
 }

void ScenarioGroup::SaveGroup( FILE *fp, Cltn<Map *> *map_list )
{
   char buffer[300];
   
   if( map_list != NULL ) {
      if( !this->IsContainedIn( map_list ) ) // validate whether the map list contains
	 return;                              // any of the scenarios in the group
   }

   sprintf( buffer, "<scenario-group name=\"%s\" ", PrintDescription( group_name ) );
   PrintXMLText( fp, buffer );
   if( PrintManager::TextNonempty( group_description ) )
      fprintf( fp, "description=\"%s\" ", PrintDescription( group_description ) );
   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );
   
   for( scenarios.First(); !scenarios.IsDone(); scenarios.Next() )
      scenarios.CurrentItem()->SaveScenario( fp, map_list );

   PrintEndXMLBlock( fp, "scenario-group" );
}

void ScenarioGroup::SaveCSMGroup( FILE *fp, Cltn<Map *> *map_list )
{
   //char buffer[300];
   
   if( map_list != NULL ) {
      if( !this->IsContainedIn( map_list ) ) // validate whether the map list contains
	 return;                              // any of the scenarios in the group
   }

   //sprintf( buffer, "<scenario-group name=\"%s\" ", PrintDescription( group_name ) );
   //PrintXMLText( fp, buffer );
   //if( PrintManager::TextNonempty( group_description ) )
     // fprintf( fp, "description=\"%s\" ", PrintDescription( group_description ) );
   //fprintf( fp, ">\n" );
  // IndentNewXMLBlock( fp );
   
   for( scenarios.First(); !scenarios.IsDone(); scenarios.Next() )
      scenarios.CurrentItem()->SaveCSMScenario( fp, map_list );

   //PrintEndXMLBlock( fp, "scenario-group" );
}

void ScenarioGroup::GeneratePostScript( FILE *ps_file )
{
   fprintf( ps_file, "%d rom (Scenario Group - ) S %d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, 
	    PrintManager::PrintPostScriptText( group_name ) );
   
   if( PrintManager::TextNonempty( group_description ) ) {
      fprintf( ps_file, "%d rom\n1 IN ID (Group Description ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID\n" );
      PrintManager::PrintDescription( group_description );
      fprintf( ps_file, "OD\n" );
   }

   for( scenarios.First(); !scenarios.IsDone(); scenarios.Next() )
      scenarios.CurrentItem()->GeneratePostScript( ps_file );
}

void ScenarioGroup::GenerateMSCs()
{
   for( scenarios.First(); !scenarios.IsDone(); scenarios.Next() )
      MSCGenerator::Instance()->GenerateScenarioMSC( scenarios.CurrentItem() );
    //SCENARIOGenerator::Instance()->TraverseScenarioGroup( &scenarios );
}

void ScenarioGroup::GenerateGroupXML()
{
   SCENARIOGenerator::Instance()->TraverseScenarioGroup( &scenarios );
}

void ScenarioGroup::GenerateGroupDXL(FILE *dxl_file )
{
   char buffer[300];
   sprintf( buffer, "scenarioGroup( \"%s\", \"scenarioGroup-%s\"", removeNewlineinString( group_name ),removeNewlineinString( group_name ) );
   PrintXMLText( dxl_file, buffer );
    
   if( PrintManager::TextNonempty( group_description ) ){
      fprintf( dxl_file, ", \"%s\"", removeNewlineinString( group_description ) );
    }  
    else{
      fprintf( dxl_file, ", \"\"");
   } 
   fprintf( dxl_file, " )\n" );
   IndentNewXMLBlock( dxl_file );
   sprintf( buffer, "scenarioGroup-%s", removeNewlineinString( group_name ));
   SCENARIOGenerator::Instance()->SaveScenarioGroupDXL( dxl_file, &scenarios, buffer );
}

void ScenarioGroup::AddStartingPoint( Start *start )
{
   for( scenarios.First(); !scenarios.IsDone(); scenarios.Next() )
      scenarios.CurrentItem()->AddStartingPoint( start );
}

ScenarioGroup * ScenarioList::AddScenarioGroup( const char *name, const char *description )
{
   ScenarioGroup *new_group = new ScenarioGroup( name, description );
   scenario_groups.Add( new_group );
   return( new_group );
}

bool ScenarioList::NameExists( const char *new_name )
{
   for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() ) {
      if( strequal( new_name, scenario_groups.CurrentItem()->GroupName() ) )
	 return TRUE;
   }

   return FALSE;
}

bool ScenarioList::ScenariosDefined()
{
   for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() ) {
      if( scenario_groups.CurrentItem()->ScenarioCount() > 0 )
	 return TRUE;
   }

   return FALSE;
}

void ScenarioList::ViewScenarioList( Hyperedge *selected_start )
{
   ViewScenarios( &scenario_groups, selected_start );
}

void ScenarioList::ViewStartScenarios( Start *start )
{
   ListStartScenarios( start );
}

void ScenarioList::SetScenarioTracing( Scenario *scenario )
{
   tracing_mode = TRUE;
   scenario_tracing_id++;
   MSCGenerator::Instance()->HighlightScenario( scenario ); // invoke scanning algorithm
}

void ScenarioList::SaveXML( FILE *fp, Cltn<Map *> *map_list )
{
   bool valid_scenarios_exist = FALSE;

   if( scenario_groups.Size() == 0 ) return;

   if( map_list != NULL ) { // determine if any scenarios are contained in map_list
      for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() ) {
	 if( scenario_groups.CurrentItem()->IsContainedIn( map_list ) ) {
	    valid_scenarios_exist = TRUE;
	    break;
	 }
      }
      if( !valid_scenarios_exist ) return;
   }

   PrintNewXMLBlock( fp, "scenario-list" );

   for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() )
      scenario_groups.CurrentItem()->SaveGroup( fp, map_list );

   PrintEndXMLBlock( fp, "scenario-list" );
}

void ScenarioList::SaveCSMXML( FILE *fp, Cltn<Map *> *map_list )
{
   bool valid_scenarios_exist = FALSE;

   if( scenario_groups.Size() == 0 ) return;

   if( map_list != NULL ) { // determine if any scenarios are contained in map_list
      for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() ) {
	 if( scenario_groups.CurrentItem()->IsContainedIn( map_list ) ) {
	    valid_scenarios_exist = TRUE;
	    break;
	 }
      }
      if( !valid_scenarios_exist ) return;
   }

   //PrintNewXMLBlock( fp, "scenario-list" );

   for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() )
      scenario_groups.CurrentItem()->SaveCSMGroup( fp, map_list );

   //PrintEndXMLBlock( fp, "scenario-list" );
}

void ScenarioList::SaveDXL( FILE *fp )    // Added by Bo Jiang, for DXL exporting.  July, 2004
{
   if( scenario_groups.Size() == 0 ) return;
   for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() )
      scenario_groups.CurrentItem()->GenerateGroupDXL( fp );
   IndentEndXMLBlock(fp);          
}

void ScenarioList::GeneratePostScript( FILE *ps_file )
{
   if( scenario_groups.Size() == 0 ) return;

   PrintManager::PrintHeading( "Scenario Definitions" );
   fprintf( ps_file, "[/Title(Scenario Definitions) /OUT pdfmark\n" );
  
   for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() )
      scenario_groups.CurrentItem()->GeneratePostScript( ps_file );
}

void ScenarioList::ClearScenarioList()
{
   while( !scenario_groups.is_empty() )
      delete scenario_groups.Detach();

   if( tracing_mode == TRUE ) {
      tracing_mode = FALSE; 
      scenario_tracing_id++;
   }
}

int ScenarioList::ScenarioGroupId( ScenarioGroup * group )
{
   int id = 0;
   for( scenario_groups.First(); !scenario_groups.IsDone(); scenario_groups.Next() ) {
      id++;
      if( scenario_groups.CurrentItem() == group ) {
         return id;
      }
   }
   return -1;
}
