/***********************************************************
 *
 * File:			msc_generator.h
 * Author:			Andrew Miga
 * Created:			November 1999
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef MSC_GENERATOR_H
#define MSC_GENERATOR_H

#include "collection.h"
#include <fstream>
#include <time.h>

class MSCGenerator;
class Hyperedge;
class Start;
class Synchronization;
class ComponentReference;
class OrFork;
class Empty;
class Scenario;
class WaitSynch;
class Timer;
class Wait;
template<class T> class Cltn;

#define MAX_NESTING_LEVELS 20

typedef enum { PARALLEL, ALTERNATIVE, LOOP_SECTION } inline_type;

typedef struct {
   Synchronization *fork;
   Stub **decomposition_stack;
   int current_branch, decomposition_level;
} fork_traversal;

class GenerationState {

   friend class MSCGenerator;

public:

   GenerationState();
   ~GenerationState();
   
private:

   int parallel_depth, decomposition_level;
   Stub **decomposition_stack;
   fork_traversal parallel_stack[MAX_NESTING_LEVELS];
};

class MSCGenerator {

friend class ResponsibilityReference;

public:

   static MSCGenerator * Instance();
   void GenerateAllMSCPaths( Start *start );
   void GenerateScenarioMSC( Scenario *scenario );
   void HighlightScenario( Scenario *scenario );
   void IncrementScenarioCount();
   int ScenarioNumber() { return( scenario_count ); }
   void GenerateScenario();
   void ResetGenerationState( GenerationState *gs ); 
   void AddComponent( ComponentReference *nc );
   void ComponentCrossing( ComponentReference *cp, bool first_pass );
   void StartNewPath() { previous_component = NULL; }
   ComponentReference * PreviousComponent() { return( previous_component ); }
   void PreviousComponent( ComponentReference *cp );
   bool NewComponentRole( ComponentReference *cr );
   void CreateMessage( ComponentReference *present_component, Hyperedge *path_end = NULL );
   void CreateAction( const char *resp_name );
   void OutputStatement( const char *statement );
   void CreateCondition( const char *state_name );

   void StartInlineConstruct( inline_type itype );
   void NewInlineBranch(  inline_type itype, Stub **ss, int level, bool first_pass );
   void EndInlineConstruct( inline_type itype );

   void DecomposeStub( Stub *stub );
   Stub * GetUpperLevelStub();
   bool InsideStub() { return( decomposition_level != 0 ); }
   int CurrentDecompositionStack( Stub ***ss );
   int DecompositionLevel() { return( decomposition_level ); } // debug method
   void MapContext( int **context, int &depth );

   void AddParallelToStack( Synchronization *fork );
   void CurrentParallelBranch( Synchronization **fork, int &branch );
   int CurrentParallelStack( fork_traversal *temp_stack );
   Hyperedge * NextParallelBranch();
   Hyperedge * NextScanningPath( Hyperedge *synch );
   void IncrementParallelBranch();
   void EndParallelTraversal();

   void StartTimer( const char *timer_name );
   void StopTimer( const char *timer_name, ComponentReference *timer_component, Hyperedge *start );
   void HaltTimer( const char *timer_name );
   void Timeout( const char *timer_name );
   void EnvironmentTrigger( ComponentReference *wait_component, Hyperedge *start );
   int CurrentGenerationId() { return( generation_id ); }

   Empty * PreviousEmpty() { return( previous_empty ); }
   void PreviousEmpty( Empty *empty ) { previous_empty = empty; }
   void AbortGeneration();
   bool GenerationAborted() { return( abort_generation ); }
   bool SubmapRecursionExists( Map *plugin );
   bool ScenarioTracing() { return( scenario_tracing ); }
   bool HighlightActive() { return( highlight_active ); }
   Scenario * CurrentScenario() { return( current_scenario ); }
   ComponentReference * StubComponent();
   void EnvironmentReferenced() { environment_reference = TRUE; }
   void RegisterPathElement( Hyperedge *element ) { path_elements.Add( element ); }
   void ScenarioStartPoint( Start *start, bool first_pass );
   bool ParallelSynch() { return( parallel_synch ); }
   void SetParallelSynch() { parallel_synch = TRUE; }
   const Hyperedge * SynchronizedElement() { return( synch_element ); }
   bool WaitingSynchronization() { return(waiting_synchronization); }
   void SetWaitingSynchronization( bool ws ) { waiting_synchronization = ws; }
   
   int  getScenarioStartIndex() { return scenario_start_index; }
   void IncrementScenarioStartIndex() { scenario_start_index++; }
    
private:
   
   MSCGenerator(); // constructor, private to enforce singleton pattern
   void TraverseScenario( bool first_pass = FALSE ); 
   void ScanScenarios( scan_type type );
   const char *MscName( const char *name );
   void PrintComponentLabel();
   void ClearParallelStack();
   const char * ComponentRole( ComponentReference *ref );
   void TerminateComponent( ComponentReference *nc );
   bool UpdateElapsedTime();
   void ResetDecompositionStack( Stub **ss, int level );
   void ResetParallelStack( fork_traversal *new_stack, int depth );
   void TerminateTraversal();
   void ValidatePostconditionConsistency();

   static MSCGenerator * SoleMSCGenerator;
   bool scenario_start, abort_generation, scenario_tracing, highlight_active, environment_reference;
   std::ofstream *msc;
   int message_count, unnamed_count, scenario_count, generation_id;
   ComponentReference *previous_component, *current_component;
   Start *scenario_start_point;
   ComponentReference *scenario_start_component;
   Empty *previous_empty;
   Stub *decomposition_stack[MAXIMUM_HIERARCHY_LEVELS];
   int decomposition_level;
   Cltn<ComponentReference *> component_traversals, component_terminations;
   Cltn<Hyperedge *> path_elements;
   Hyperedge *synch_element;
   char space_string[70], msc_basename[LABEL_SIZE+1];
   int inline_construct, nesting_levels;
   fork_traversal fork_history[MAX_NESTING_LEVELS];
   Scenario *current_scenario;
   time_t start_time;
   int bytes_written;
   int scenario_start_index;
   scan_type current_scan;
   bool parallel_synch, waiting_synchronization;     
};

#endif
