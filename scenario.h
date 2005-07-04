/***********************************************************
 *
 * File:			scenario.h
 * Author:			Andrew Miga
 * Created:			September 2000
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef SCENARIO_H
#define SCENARIO_H

#include "collection.h"
#include <stdio.h>
#include <string.h>
#include "scenario_generator.h"

class Map;
class Hyperedge;
class Start;
class BooleanVariable;
class ScenarioGroup;

class VariableInit {
public:

   VariableInit( BooleanVariable *bvar, int val ) { variable = bvar; value = val; }
   VariableInit( int cvar, int val );

   int Value() { return( value ); }
   void ToggleValue() { value = 1 - value; }
   BooleanVariable * Variable() { return( variable ); }

private:

   BooleanVariable *variable;
   int value;
};

class Scenario {

public:

   Scenario( const char *name, const char *description );
   ~Scenario();

   const char * Name() { return( scenario_name ); }
   void Name( const char *new_name ) { strncpy( scenario_name, new_name, LABEL_SIZE ); }
   const char * Description() { return( scenario_description ); }
   void Description( const char *new_description );
   ScenarioGroup * Group() { return( group ); }
   void SetGroup( ScenarioGroup *sg ) { group = sg; }
   void AddVariableInitialization( int bool_id, int value ) { variable_initializations.Add( new VariableInit( bool_id, value ) ); }
   void AddScenarioPostcondition( int bool_id, int value ) { scenario_postconditions.Add( new VariableInit( bool_id, value ) ); }
   void SaveScenario( FILE *f, Cltn<Map *> *map_listp );
   void SaveCSMScenario( FILE *f, Cltn<Map *> *map_listp );
   void GeneratePostScript( FILE *ps_file );
   Cltn<VariableInit *> * VariableInitializations() { return( &variable_initializations ); }
   Cltn<VariableInit *> * ScenarioPostconditions() { return( &scenario_postconditions ); }
   Cltn<Start *> * StartingPoints() { return( &starting_points ); }
   void AddStartingPoint( Start *new_start );
   void RemoveStartingPoint( Start *start ) { starting_points.Delete( start ); }
   void DeleteStartingPoint( Start *start );
   bool HasStartingPoints() { return( (starting_points.Size() > 0) ? TRUE: FALSE ); }
   void DuplicateScenario( Scenario *original );
   bool IsContainedIn( Cltn<Map *> *map_list );
   int ScenarioId( );

private:

   Cltn<VariableInit *> variable_initializations, scenario_postconditions;
   Cltn<Start *> starting_points;
   char scenario_name[LABEL_SIZE+1];
   char *scenario_description;
   ScenarioGroup *group;
};

class ScenarioGroup {

public:

   ScenarioGroup( const char *name, const char *description );
   ~ScenarioGroup();

   const char * GroupName() { return( group_name ); }
   void GroupName( const char *new_name ) { strncpy( group_name, new_name, LABEL_SIZE ); }
   const char * GroupDescription() { return( group_description ); }
   void GroupDescription( const char *new_description );
   Scenario * AddScenario( const char *name, const char *description );
   int ScenarioCount() { return( scenarios.Size() ); }
   Cltn<Scenario *> * ScenarioSet() { return( &scenarios ); }
   bool NameExists( const char *new_name );
   void SaveGroup( FILE *fp, Cltn<Map *> *map_list );
   void SaveCSMGroup( FILE *fp, Cltn<Map *> *map_list );
   void GeneratePostScript( FILE *ps_file );
   void GenerateMSCs();
   void AddStartingPoint( Start *start );
   bool IsContainedIn( Cltn<Map *> *map_list );
   void GenerateGroupXML();
   void GenerateGroupDXL(FILE *dxl_file );   // Added by Bo Jiang, for DXL exporting.  July, 2004

private:

   Cltn<Scenario *> scenarios;
   char group_name[LABEL_SIZE+1];
   char *group_description;
};

class ScenarioList {

public:

   static void ViewScenarioList( Hyperedge *selected_start = NULL );
   static ScenarioGroup * AddScenarioGroup( const char *name, const char *description );
   static bool NameExists( const char *new_name );
   static bool ScenariosDefined();
   static void ViewStartScenarios( Start *start );
   static void ClearScenarioList();
   static void SaveXML( FILE *fp, Cltn<Map *> *map_list );
   static void SaveDXL( FILE *fp );   // Added by Bo Jiang, for DXL exporting.  July, 2004
   static void SaveCSMXML( FILE *fp, Cltn<Map *> *map_list );
   static void GeneratePostScript( FILE *ps_file );

   static void SetScenarioTracing( Scenario *scenario );
   static void ResetScenarioTracing() { tracing_mode = FALSE; scenario_tracing_id++; }
   static bool ScenarioTracingMode() { return( tracing_mode ); }
   static unsigned int ScenarioTracingId() { return( scenario_tracing_id ); }
   static int ScenarioGroupId( ScenarioGroup * );

private:

   static Cltn<ScenarioGroup *> scenario_groups;
   static unsigned int scenario_tracing_id;
   static bool tracing_mode;
};

#endif
