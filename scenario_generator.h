 /***********************************************************
 *
 * File:			scenario_generator.h
 * Author:			XiangYang He
 * Created:			August 2002
 *
 * Modification history:        Mar 2003
 *                              Shuhua Cui
 *
 ***********************************************************/

#ifndef SCENARIO_GENERATOR_H
#define SCENARIO_GENERATOR_H

#include "collection.h"
#include "path_data.h"
#include <fstream>
#include <time.h>
#include <stdio.h>

class Hyperedge;
class Start;
class Synchronization;
class ComponentReference;
class OrFork;
class Empty;
class Scenario;

#define MAX_NESTING_LEVELS 20

typedef struct {
   Synchronization *fork;
   int current_branch;
   Path_data * parent;
} fork_element;

class SCENARIOGenerator {

public:

   static SCENARIOGenerator * Instance();
   void IncrementScenarioCount();
   int ScenarioNumber() { return( scenario_count ); }
   void TraverseScenario(Scenario * scenario);
   void ScanScenarios();
   void ValidatePostconditionConsistency();
   void AddComponent( ComponentReference *nc );

   void MapContext( int **context, int &depth );

   void AddParallelToStack( Synchronization *fork, Path_data * );
   void CurrentParallelBranch( Synchronization **fork, int &branch );
   Hyperedge * NextParallelBranch();
   void IncrementParallelBranch();
   void EndParallelTraversal();

   void AbortGeneration();
   bool GenerationAborted() { return( abort_generation ); }
   bool SubmapRecursionExists( Map *plugin );
   Scenario * CurrentScenario() { return( current_scenario ); }
   void RegisterPathElement( Hyperedge *element ) { path_elements.Add( element ); }
   int getScenarioStartIndex() { return scenario_start_index; }
   void IncrementScenarioStartIndex() { scenario_start_index++; }
   void ResetScenarioStartIndex() { scenario_start_index = 0; }
   void AddPathTraversal(Path_data * new_data);

   void GenerateScenario();
   Path_data * getCurrentPathData( )  { return current_path_data; }
   void ResetCurrentPathData( Path_data * new_data )  { current_path_data = new_data; }
   void SaveScenarioXML(FILE * outfile);
   void SaveScenarioDXL( FILE * outfile, char * parentID );  // Added by Bo Jiang, for DXL exporting.  July, 2004. It is called by SaveScenarioGroupDXL

   void ReplacePathTraversal(Path_data * new_root, Path_data * des);
   Path_data * EmptyPathTraversal( Path_data * );
   void DeleteParallel( Path_data * root);
   void UpdateParallelStack( Path_data * next_data, Path_data * waiting_leave );
   Empty * PreviousEmpty() { return( previous_empty ); }
   void PreviousEmpty( Empty *empty ) { previous_empty = empty; }

   void RemoveNewLine(char * , char * );
   void TraverseScenarioGroup( Cltn<Scenario *> * scenarios );
   void GenerateScenarioGroup( FILE *& outfile, Cltn<Scenario *> * scenarios );
   void SaveScenarioGroupDXL( FILE * outfile, Cltn<Scenario *>* scenarios,char * parentID );    // Added by Bo Jiang, for DXL exporting.  July, 2004.  main method for exporting
   void RegisterSynchElements( Synchronization * synch );
   void ResetSynchElementStatus( );
   void AddPathList( Path_data * );
   Path_data * EmptyPathList( );
   Stub* getCurrentStub();
   void  setCurrentStub(Stub* aStub);

private:

   SCENARIOGenerator(); // constructor, private to enforce singleton pattern
   void ClearParallelStack();
   void TerminateTraversal();

   static SCENARIOGenerator * SoleSCENARIOGenerator;
   bool scenario_start, abort_generation;
   std::ofstream *outfile;
   int scenario_count;
   Start *scenario_start_point;
   Empty *previous_empty;
   char scenario_basename[LABEL_SIZE+1];
   Cltn<Hyperedge *> path_elements;
   Cltn<Hyperedge *> synch_elements;
   int nesting_levels;
   fork_element fork_history[MAX_NESTING_LEVELS];
   Scenario *current_scenario;
   time_t start_time;
   int scenario_start_index;
   Cltn<Path_data *> path_list;
   Cltn<Path_data *> path_traversal;
   Path_data * current_path_data;

   Stub* current_stub;

};

#endif
