// Layered Queueing Network generation from Use Case Maps
#include <stdio.h>
#include <stdlib.h>

#include "lqngenerator.h"
extern char loaded_file_path[128];


// returns pointer to sole instance, creates instance on first call
LqnGenerator* LqnGenerator::Instance()
{
  if( !SoleLqnGenerator ) {
    SoleLqnGenerator = new LqnGenerator();
  }
  return( SoleLqnGenerator );
}


// constructor
LqnGenerator::LqnGenerator()
{ 
  printf( "------------------------------------------------------------\n" );
  printf( "                 Begin LqnGenerator\n" );
  printf( "------------------------------------------------------------\n\n" );
}


// destructor
LqnGenerator::~LqnGenerator()
{
  Clear();
  SoleLqnGenerator = NULL;
  printf( "------------------------------------------------------------\n" );
  printf( "                  End LqnGenerator\n" );
  printf( "------------------------------------------------------------\n\n" );
}


// initialize internal variables
void LqnGenerator::Init()
{
  print = false;

  current_map = NULL;
  lbody_flag = false;
  lbody_task = NULL;
  parent_stub = NULL;
  stub_join_activity = NULL;
  plugin_exit = false;

  mstacks = new Cltn<LqnMstack*>;
  mstack_count = 1;
  mstack = new LqnMstack( mstack_count );
  mstacks->Add( mstack );
  starts = new Cltn<Hyperedge*>;
  root_starts = new Cltn<Hyperedge*>;

  lqn = new Lqn;  // create LQN
}


// clear internal variables
void LqnGenerator::Clear()
{
  // reset visited flags of all edges in hypergraph
  TransformationManager::Instance()->CurrentGraph()->ResetVisited();

  while( mstacks->Size() ) {
    delete mstacks->Detach();
  }
  delete mstacks;

  delete starts;
  delete root_starts;
  delete lqn;
}


// reset internal variables
void LqnGenerator::Reset()
{
  Clear();
  Init();
}


// transform UCM into LQN
void LqnGenerator::Transmorgrify( Cltn<Map*>* maps, Cltn<Device*>* devices )
{  
  Map* trans_map;  // map being transmogrified
  Cltn<Path*>* paths;  // paths on the current map
  Path* current_path;  // current path
  Cltn<ComponentReference*>* comp_refs;  // component references 
                                         // on the current map
  Cltn<ResponsibilityReference*>* resp_refs;  // responsibility references
                                              // on the current map
  FILE* output_file;
  FILE* log_file;
  char output_file_name[FILE_NAME_LENGTH];
  output_file_name[0] = 0;  // initialize the file name
  output_file_name[FILE_NAME_LENGTH - 1] = 0;
  char log_file_name[FILE_NAME_LENGTH];
  log_file_name[0] = 0;  // initialize the file name
  log_file_name[FILE_NAME_LENGTH - 1] = 0;

  // create the output file name
  strcpy( output_file_name, loaded_file_path );
  output_file_name[ strlen( output_file_name ) - 3 ] = 'l';
  output_file_name[ strlen( output_file_name ) - 2 ] = 'q';
  output_file_name[ strlen( output_file_name ) - 1 ] = 'n';
  output_file = fopen( output_file_name, "w" );  // output LQN to file
  // create the log file name
  strcpy( log_file_name, loaded_file_path );
  log_file_name[ strlen( log_file_name ) - 3 ] = 'l';
  log_file_name[ strlen( log_file_name ) - 2 ] = 'o';
  log_file_name[ strlen( log_file_name ) - 1 ] = 'g';
  log_file = fopen( log_file_name, "w" );  // output comments to log file

  printf( "==================== Begin Transmorgrify ===================\n\n",\
	  maps->Size() );
  Init();  // initialize transmorgrify
  // verify that all the devices are properly specified
  CreateDevices( devices );

  //fprintf( log_file, "UCM info as follows (%d maps):\n", maps->Size() );

  // go through all the maps
  for( maps->First(); !maps->IsDone(); maps->Next() ) {
    trans_map = maps->CurrentItem();
    // reset the visited flags for all the hyperedges
    trans_map->MapHypergraph()->ResetVisited();
    // get all the components for the current map
    comp_refs = trans_map->Components();
    // get all the paths for the current map
    paths = trans_map->Paths();
    // get all the responsibilities for the current map
    resp_refs = trans_map->Responsibilities();

    // get all the start points for the current map
    for( paths->First(); !paths->IsDone(); paths->Next() ) {
      current_path = paths->CurrentItem();
      // the start of a path segment is a start point
      if( current_path->GetPathStart()->EdgeType() == START ) {
	// add to the list of start points
        starts->Add( current_path->GetPathStart() );
      }
    }

    // dealing with the root map
    if( trans_map->IsRootMap() ) {
      fprintf( log_file, "Root map%d '%s' has:\n",\
	       trans_map->GetNumber(), trans_map->MapLabel() );
    }
    // dealing with a plug-in map
    else if( trans_map->IsPlugIn() ) {  
      fprintf( log_file, "Plug-in map%d '%s' has:\n",\
	       trans_map->GetNumber(), trans_map->MapLabel() );
    }
    // neither a root map nor a plug-in map
    else {  
      fprintf( log_file, "Map%d '%s' has:\n",\
	       trans_map->GetNumber(), trans_map->MapLabel() );
    }
    fprintf( log_file, "\t%d component references\n", comp_refs->Size() );
    fprintf( log_file, "\t%d paths\n", paths->Size() );
    fprintf( log_file, "\t%d start points\n", starts->Size() );
    fprintf( log_file, "\t%d responsibilities\n", resp_refs->Size() );

    // process only the top level root map
    if( trans_map->IsTopLevelMap() && trans_map->IsRootMap() ) {
      current_map = trans_map;
      parent_stub = NULL;
      fprintf( log_file, "Map%d '%s' is the top level root map!\n",\
	       trans_map->GetNumber(), trans_map->MapLabel() );

      // go through all start points
      for( starts->First(); !starts->IsDone(); starts->Next() ) {
	// add start as a root start
	root_starts->Add( starts->CurrentItem() );
      }
    }

    CreateTasks( comp_refs );

    starts->Empty();  // clear the starts for the next map
  }  // endfor

  printf( "*** Converting file '%s' ***\n\n", loaded_file_path );
  fprintf( log_file, "*** Converting file '%s' ***\n\n", loaded_file_path );

  printf( "\n*** Begin Path Traversal ***\n" );
  // traverse all the root map start points
  for( root_starts->First(); !root_starts->IsDone(); root_starts->Next() ) {
    // generate LQN
    Start2Lqn( (Start*)root_starts->CurrentItem() );
  }

  printf( "\n*** End Path Traversal ***" );

  lqn->FilePrint( output_file );
  fclose( output_file );
  fl_show_message( "", "LQN created and saved in file", output_file_name );

  fprintf( log_file, "\n*** LQN saved in file '%s' ***\n", output_file_name );
  fclose( log_file );
  fl_show_message( "", "Log comments saved in file", log_file_name );

  printf( "\n==================== End Transmorgrify ===================\n\n",\
	  maps->Size() );
  Clear();
}


void LqnGenerator::CreateDevices( Cltn<Device*>* devices )
{
  char name[DEVICE_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[DEVICE_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;
  Device* current_device = NULL;  // current device being looked at
  int id = 0;
  double optime = 0;

  printf( "CreateDevices: %d devices.\n", devices->Size() );

  // go through all the devices
  for( devices->First(); !devices->IsDone(); devices->Next() ) {
    current_device = devices->CurrentItem();
    optime = current_device->OpTime();
    id = current_device->Number();

    switch( current_device->DeviceType() ) {
    case PROCESSOR:  // the device is a processor
      strcpy( name, current_device->Name() );

      // the name is missing
      if( strequal( current_device->Name(), "" ) ) {
	strcat( name, "P" );
	sprintf( number, "%d", id );
	strcat( name, number );
	printf( "(Warning: processor (id %d) has no name, using '%s'.)\n",\
		id, name );
      } 

      // operation time is specified improperly
      if( optime <= 0.0 ) {
	printf( "(Warning: processor '%s' (id %d): op time improperly specified as %lf, using 1.0.)\n", \
		name, id, optime );
	optime = 1.0;
      }

      // add device to LQN using specified speed factor
      lqn->AddDevice( name, id, 1.0 / optime );
    break;

    default:  // the device is not a processor
       printf( "(Warning: device '%s' is not a processor.)\n",\
	       current_device->Name() );
    break;
    }
  }  // endfor

  printf( "\n" );
} 


// verify methods to check that all the necessary information is present in
// the UCM for LQN generation
void LqnGenerator::CreateTasks( Cltn<ComponentReference*>* component_refs )
{
  ComponentReference* current_comp_ref = NULL;  // current component reference
  Component* comp = NULL;

  printf( "CreateTasks: %d component references.\n", component_refs->Size() );

  // go through all the comp references
  for( component_refs->First(); !component_refs->IsDone(); component_refs->Next() ) {
    current_comp_ref = component_refs->CurrentItem();

    // an LQN task does not exist for this component reference
    if( !lqn->GetTaskForComp( current_comp_ref ) ) {
      // get the referenced component
      comp = current_comp_ref->ReferencedComponent();

      // the referenced component exists
      if( comp ) {
	// the component is not a process.
	if( comp->GetType() != PROCESS ) {
	  printf( "(Warning: component '%s' (id %d) is not defined as a process.)\n",\
		  comp->GetLabel(), comp->GetComponentNumber() );
	}

	// a processor is not specified
	if( comp->ProcessorId() == NA ) {
	  printf( "(Warning: component '%s' (id %d) has no processor specified, assigning to infinite processor.)\n",\
                  comp->GetLabel(), comp->GetComponentNumber() );

	  // create a task that runs on the infinite processor
	  lqn->AddTaskInfProc( current_comp_ref );
	}
	// a processor is specified
	else {
	  // create a task running on the specified processor
	  lqn->AddTask( current_comp_ref, lqn->GetDevice( comp->ProcessorId() ) );
	}
      }
      // the referenced component does not exist
      else {
	printf( "(Warning: component reference '%s' (id %d) does not point to a logical component.)\n",\
                current_comp_ref->ReferenceName(),\
		current_comp_ref->ComponentReferenceNumber() );

	// create a task that runs on the infinite processor
	lqn->AddTaskInfProc( current_comp_ref );
      }
    }
  }  // endfor

  printf( "\n" );
}


void LqnGenerator::Start2Lqn( Start* start_pt )
{
  // Hyperedge* next_edge = NULL;  // DA: August 2004
  ComponentReference* comp_ref = NULL;
  // ComponentReference* next_comp_ref = NULL;  // DA: August 2004
  LqnTask* task = NULL;
  LqnTask* ref_task = NULL;
  LqnEntry* ref_entry = NULL;
  LqnEntry* entry = NULL;
  LqnActivity* ref_activity = NULL;
  // LqnActivity* activity = NULL;  // DA: August 2004
  LqnMessage* message = NULL;
  double think_time = 1.0;
  Cltn<Hyperedge*>* next_edges = new Cltn<Hyperedge*>;

  start_pt->SetVisited();

  // if the arrival type has been left undefined
  if( start_pt->ArrivalType() == UNDEFINED ) {
    printf( "(Warning: start '%s' h%d has an UNDEFINED arrival rate.)\n",\
	    start_pt->HyperedgeName(), start_pt->GetNumber() );
  }
  // else the arrival type has been defined
  else {
    switch( start_pt->ArrivalType() ) {
    case EXPONENTIAL:
      // if the mean arrival rate is negative
      if( start_pt->Input( 1 ) < 0 ) {
	printf( "(Warning: start '%s' h%d has an EXPONENTIAL arrival rate with negative mean, using 1.0 instead.)\n",\
		start_pt->HyperedgeName(), start_pt->GetNumber() );
      }
      // else if the mean arrival rate is zero
      else if( start_pt->Input( 1 ) == 0 ) {
	printf( "(Warning: start '%s' h%d has an EXPONENTIAL arrival rate with 0 mean, using 1.0 instead.)\n",\
		start_pt->HyperedgeName(), start_pt->GetNumber() );
      }
      // else the mean arrival rate is positive
      else {
	think_time = start_pt->Input(1);
      }
      break;

    case DETERMINISTIC:
      // if the mean arrival rate is negative
      if( start_pt->Input(1) < 0 ) {
	printf( "(Warning: start '%s' h%d has a DETERMINISTIC arrival rate with a negative value, using 1.0 instead.)\n",\
		start_pt->HyperedgeName(), start_pt->GetNumber() );
      }
      // else if the mean arrival rate is zero
      else if( start_pt->Input(1) == 0 ) {
	printf( "(Warning: start '%s' h%d has a DETERMINISTIC arrival rate with zero value, using 1.0 instead.)\n",\
		start_pt->HyperedgeName(), start_pt->GetNumber() );
      }
      // else the mean arrival rate is positive
      else {
	think_time = start_pt->Input(1);
      }
      break;

    case UNIFORM:
      printf( "(Warning: start '%s' h%d cannot use UNIFORM arrivals, using exponential arrivals with mean 1.0.)\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber() );
      break;

    case ERLANG:
      printf( "(Warning: start '%s' h%d cannot use ERLANG arrivals, using exponential arrivals with mean 1.0.)\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber() );
      break;

    case EXPERT:
      printf( "(Warning: start '%s' h%d cannot use EXPERT arrivals, using exponential arrivals with mean 1.0.)\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber() );
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }
  }

  // if the start point has closed arrivals
  if( start_pt->StreamType() == CLOSED_ARRIVAL ) {
    lqn->SetClosed();

    // if the population size is less than 1
    if( start_pt->PopulationSize() < 1 ) {
      printf( "(Warning: start '%s' h%d has Closed Arrivals with a population size of %d, using 1 instead.)\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber(),\
	      start_pt->PopulationSize() );
    }
    // else the population size is 1 or more
    else {
      lqn->SetPopulation( start_pt->PopulationSize() );
    }
  }

  // create a new reference task
  ref_task = lqn->AddRefTask( start_pt );
  // create an entry for the reference task
  ref_entry = ref_task->AddRefEntry();
  // create an activity for the entry
  ref_activity = ref_task->AddDefaultActivity();
  ref_activity->SetStochastic();
  ref_activity->SetServiceTime( think_time );

  // get the component reference containing the start point
  comp_ref = start_pt->GetFigure()->GetContainingComponent();

  // the component reference exists
  if( comp_ref ) {
    // get the task corresponding to the component reference
    task = lqn->GetTaskForComp( comp_ref );

    // the task exists
    if( task ) {
      printf( "Start '%s' h%d is contained in task '%s'.\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber(),\
	      task->GetName() );

      // create an entry called by the reference task
      entry = task->AddRefEntry();
      message = new LqnMessage( ref_activity, entry );
      mstack->Push( message );  // push the message on the Mstack
    }
    // the task does not exist
    else {
      printf( "ERROR: no task found for component '%s' (id %d)!\n\n",\
              comp_ref->ReferenceName(),\
	      comp_ref->ComponentReferenceNumber() );
    }
  }
  // else the component reference does not exist
  else {
    printf( "Start '%s' h%d is not contained in a component.\n",\
	    start_pt->HyperedgeName(), start_pt->GetNumber() );
    // create a message from the reference activity
    message = new LqnMessage( ref_activity );
    mstack->Push( message );  // push the message on the Mstack
  }

  // look ahead at the next hyperedges
  NextEdges( start_pt, next_edges );

  // error check
  if( next_edges->Size() > 1 ) {
    printf( "ERROR: start point at hyperedge h%d has %d next edges!\n\n",\
	    start_pt->GetNumber(), next_edges->Size() );
  }
  // traverse the first next edge
  TraverseEdge( start_pt, next_edges->GetFirst() );

  delete next_edges;
}


void LqnGenerator::Edge2Lqn( Hyperedge* edge, Hyperedge* in_edge )
{
  // variables used for all cases
  LqnTask* task = NULL;
  LqnActivity* activity = NULL;
  LqnEntry* entry = NULL;
  LqnMessage* message = NULL;
  Hyperedge* next_edge = NULL;
  ComponentReference* comp_ref = NULL;
  Cltn<Hyperedge*>* next_edges = new Cltn<Hyperedge*>;
  Cltn<Hyperedge*>* prev_edges = new Cltn<Hyperedge*>;
  // variables used for a responsibility
  ResponsibilityReference* resp_ref = NULL;
  // variables used for a stub
  Stub* stub = (Stub*) NULL;
  Stub* old_parent_stub = NULL;
  LqnActivity* stub_fork_activity = NULL;
  LqnActivity* plugin_entry_activity = NULL;
  LqnActivity* plugin_exit_activity = NULL;
  bool old_plugin_exit = false;
  LqnMstack* stub_mstack = NULL;
  LqnMstack* plugin_mstack = NULL;
  LqnActivity* old_stub_join_activity = NULL;
  Cltn<Map*>* plugin_maps = NULL;
  Map* plugin_map = NULL;
  Map* old_parent_map = NULL;
  Map* last_plugin_map = NULL;
  Start* plugin_start = NULL;
  PluginBinding* plugin_bind = NULL;
  double selection_prob = 0;
  double selection_prob_sum = 0;
  // variables used for a loop
  LqnTask* old_lbody_task = NULL;
  LqnActivity* loop_activity = NULL;
  LqnActivity* lbody_activity = NULL;
  LqnEntry* lbody_entry = NULL;
  // LqnMessage* lbody_message = NULL; // DA: August 2004
  LqnMstack* loop_mstack = NULL;
  LqnMstack* lbody_mstack = NULL;
  bool old_lbody_flag;
  // variables used for an end point
  Result* end_pt = NULL;
  LqnActivity* end_activity = NULL;

  NextEdges( edge, next_edges );  // get the next hyperedges
  PrevEdges( edge, prev_edges );  // get the previous hyperedges
  // get the component reference
  comp_ref = edge->GetFigure()->GetContainingComponent();

  // if there is a component reference
  if( comp_ref ) {
    // get the task corresponding to the component reference
    task = lqn->GetTaskForComp( comp_ref );
  }
  // else there is no component reference
  else {
    task = NULL;
  }

  // check the edge type
  switch( edge->EdgeType() ) {
  case RESPONSIBILITY_REF:
    edge->SetVisited();  // mark this hyperedge as visited
    resp_ref = (ResponsibilityReference*) edge;

    // if in a task
    if( task ) {
      task = CheckForLoopBodyTask( task );
    }
    // else not in a task
    else {
      // create a default task for the responsibility
      task = lqn->AddDefaultTask();
      // create an entry for the task
      entry = task->AddEntry();
      // make a call to the entry
      mstack->GetLastMessage()->SetTarget( entry );
    }

    // add an activity correpsonding to the responsibility to the task
    activity = task->AddResponsibilityActivity( resp_ref );
    // make the activity a candidate for messaging and connecting
    task->SetMsgCandidate( activity );
    task->SetConnectCandidate( activity );

    // error check
    if( next_edges->Size() > 1 ) {
      printf( "ERROR: responsibility %s at h%d should have only one, ",\
	      resp_ref->ParentResponsibility()->Name(), edge->GetNumber() );
      printf( "not %d next edges!\n\n", next_edges->Size() );
    }

    // traverse the only next edge
    TraverseEdge( edge, next_edges->GetFirst() );
    break;

  case OR_FORK:
    printf( "Or Fork with %d branches found at h%d.",\
	    next_edges->Size(), edge->GetNumber() );
    Fork2Lqn( edge, next_edges, task, ORFORK );
    break;

  case OR_JOIN:
    printf( "Or Join with %d branches found at h%d.",\
	    prev_edges->Size(), edge->GetNumber() );

    // if already visited
    if( edge->Visited() ) {
      printf( " (already visited)\n" );

      // if in a task
      if( task ) {
	task = CheckForLoopBodyTask( task );
      }

      // get the activity corresponding to the join
      activity = lqn->GetActivity( edge );
      // increment the number of edge visits
      activity->IncrementEdgeVisits();
    }
    // else first visit
    else {
      printf( " (first visit)\n" );
      edge->SetVisited();  // mark this hyperedge as visited

      // if in a task
      if( task ) {
	task = CheckForLoopBodyTask( task );
      }
      //  else not in a task
      else {
	// create a default task for the join
	task = lqn->AddDefaultTask();
	// create an entry for the task
	entry = task->AddEntry();
	// make a call to the entry from the last activity on the Mstack
	mstack->GetLastMessage()->SetTarget( entry );
      }

      // create an activity corresponding to the join
      activity = task->CreateDefaultActivity( edge, "OrJoin" );
      activity->SetConnection( ORJOIN );
    }

    // process the join further
    Join2Lqn( edge, next_edges, prev_edges, task, activity, ORJOIN );
    break;

  case SYNCHRONIZATION:
    // check the type of synchronization
    switch( SyncType( edge ) ) {
    case ANDFORK:
      printf( "And Fork with %d branches found at h%d.",\
	      next_edges->Size(), edge->GetNumber() );
      Fork2Lqn( edge, next_edges, task, ANDFORK );
      break;

    case ANDJOIN:
      printf( "And Join with %d branches found at h%d.",\
	      prev_edges->Size(), edge->GetNumber() );

      // if already visited
      if( edge->Visited() ) {
	printf( " (already visited)\n" );

	// if in a task
	if( task ) {
	  task = CheckForLoopBodyTask( task );
	}

	// get the activity corresponding to the join
	activity = lqn->GetActivity( edge );
	// increment the number of edge visits
	activity->IncrementEdgeVisits();
      }
      // else first visit
      else {
	printf( " (first visit)\n" );
	edge->SetVisited();  // mark this hyperedge as visited

	// if in a task
	if( task ) {
	  task = CheckForLoopBodyTask( task );
	}
	// else not in a task
	else {
	  // create a task for the join
	  task = lqn->AddDefaultTask();
	  // create an entry for the task
	  entry = task->AddEntry();
	  // make a call to the entry from the last activity on the Mstack
	  mstack->GetLastMessage()->SetTarget( entry );
	}

	// create an activity corresponding to the join
	activity = task->CreateDefaultActivity( edge, "AndJoin" );
	activity->SetConnection( ANDJOIN );
      }

      // process the join further
      Join2Lqn( edge, next_edges, prev_edges, task, activity, ANDJOIN );
      break;

    case ANDSYNC:
      printf( "And Synchronization found at h%d ", edge->GetNumber() );
      printf( "[branch in = %d, branch out = %d].",\
	      prev_edges->Size(), next_edges->Size() );

      // if already visited
      if( edge->Visited() ) {
	printf( " (already visited)\n" );
      }
      // else first visit
      else {
	printf( " (first visit)\n" );
	edge->SetVisited();  // mark this hyperedge as visited
      }
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }  // end synchronization switch
    break;

  case STUB:
    stub = (Stub*) edge;

    // if same stub as the one being currently traversed
    if( stub == parent_stub ) {
      // if dynamic stub
      if( parent_stub->HasMultipleSubmaps() ) {
	printf( "Exiting plugin map%d '%s' for dynamic stub %s h%d.",\
		current_map->GetNumber(), current_map->MapLabel(),\
		parent_stub->StubName(), parent_stub->GetNumber() );
      }
      // else static stub
      else {
	printf( "Exiting plugin map%d '%s' for static STUB %s h%d.",\
		current_map->GetNumber(), current_map->MapLabel(),\
		parent_stub->StubName(), parent_stub->GetNumber() );
      }
      // stop traversal and collapse outside of the stub
    }
    // else different stub than the one being currently traversed
    else {
      printf( "Stub %s found at h%d.",\
	      stub->StubName(), stub->GetNumber() );

      // if already visited
      if( stub->Visited() ) {
	printf( " (already visited)\n" );
      }
      // else first visit
      else {
	printf( " (first visit)\n" );
	stub->SetVisited();  // mark this hyperedge as visited
      }

      // if in a task
      if( task ) {
	task = CheckForLoopBodyTask( task );
      }
      // else not in a task
      else {
	// create a task for the stub
	task = lqn->AddDefaultTask();
	// create an entry for the task
	entry = task->AddEntry();
	// make a call to the entry
	mstack->GetLastMessage()->SetTarget( entry );
      }

      // if dynamic stub
      if( stub->HasMultipleSubmaps() ) {
	// remember the old mstack
	stub_mstack = mstack;
	// remember the old stub OR join for the plugins
	old_stub_join_activity = stub_join_activity;

	// create an OR fork for the plugins
	stub_fork_activity = task->AddDefaultActivity( edge );
	stub_fork_activity->SetConnection( ORFORK );
	// create an OR join for the plugins
	stub_join_activity = task->CreateDefaultActivity( edge );
	stub_join_activity->SetConnection( ORJOIN );

	plugin_maps = stub->Submaps();
	printf( "Stub %s h%d is a dynamic stub with %d plugins.\n",\
		stub->StubName(), edge->GetNumber(), plugin_maps->Size() );

	// go through all the plugins
	for( plugin_maps->First(); !plugin_maps->IsDone(); plugin_maps->Next() ) {
	  plugin_bind = stub->SubmapBinding( plugin_map );

	  if( plugin_bind ) {
	    selection_prob = plugin_bind->Probability();
	  }
	  else {
	    selection_prob = 1.0;
	  }
	  // sum the selection probabilities
	  selection_prob_sum = selection_prob_sum + selection_prob ;
	}

	last_plugin_map = plugin_maps->GetLast();

	// go through all the plugins
	for( plugin_maps->First(); !plugin_maps->IsDone(); plugin_maps->Next() ) {
	  old_plugin_exit = plugin_exit;  // remember the old plugin exit
	  plugin_exit = false;  // a plugin exit has not been reached yet
	  plugin_map = plugin_maps->CurrentItem();

	  // if this is the last plugin
	  if( plugin_map == last_plugin_map ) {
	    mstack = stub_mstack;  // use the original Mstack again
	  }
	  // else this is not the last plugin
	  else {
	    mstack_count++;
	    // create a mstack for the branch
	    plugin_mstack = stub_mstack->Copy( mstack_count );
	    mstacks->Add( plugin_mstack );
	    // use the plugin Mstack
	    mstack = plugin_mstack;
	  }

	  plugin_bind = stub->SubmapBinding( plugin_map );

	  if( plugin_bind ) {
	    selection_prob = plugin_bind->Probability();
	  }
	  else {
	    selection_prob = 1.0;
	  }

	  plugin_start = stub->SubmapPathStart( (Empty*) in_edge, plugin_map );

	  // if the plugin is bound to the stub
	  if( plugin_start ) {
	    // if the start point does not have a name
	    if( plugin_start->HyperedgeName() == "" ) {
	      plugin_entry_activity = task->CreateDefaultActivity( plugin_start, "PluginIn" );
	    }
	    // else the start point has a name
	    else {
	      plugin_entry_activity = task->CreateDefaultActivity( plugin_start, plugin_start->HyperedgeName() );
	    }

	    plugin_entry_activity->SetBranchProb( selection_prob / selection_prob_sum );
	    task->ConnectActivities( stub_fork_activity, plugin_entry_activity );
	    task->AddActivity( plugin_entry_activity );

	    old_parent_map = current_map;  // rememember the old parent map
	    current_map = plugin_map;  // this the current plugin map
	    old_parent_stub = parent_stub;  // rememember the old parent stub
	    parent_stub = stub;  // this is the parent stub for its plugins

	    printf( "Traversing into plugin map%d %s ",\
		    plugin_map->GetNumber(), plugin_map->MapLabel() );
	    printf( "for dynamic stub %s h%d at start point '%s' (h%d).\n",\
		    stub->StubName(), edge->GetNumber(),\
		    plugin_start->HyperedgeName(), plugin_start->GetNumber() );

	    // traverse the plugin
	    TraverseEdge( edge, plugin_start );

	    current_map = old_parent_map;  // restore the old parent map
	    parent_stub = old_parent_stub;  // restore the old parent stub

	    // if the plugin did exit
	    if( plugin_exit ) {
	      // add a plugin exit activity
	      plugin_exit_activity = task->AddDefaultActivity( stub,\
							       "PluginOut" );
	      // bind plugin exit activity to stub join
	      task->ConnectActivities( plugin_exit_activity,\
				       stub_join_activity );
	      plugin_exit_activity->SetConnection( JOINBRANCH );
	    }
	    // else the plugin did not exit
	    else {
	      printf( "ERROR in dynamic stub %s hid%d: ",\
		      stub->StubName(), edge->GetNumber() );
	      printf( "plugin map%d %s starting at start point '%s' h%d ",\
		      plugin_map->GetNumber(), plugin_map->MapLabel(),\
		      plugin_start->HyperedgeName(),\
		      plugin_start->GetNumber() );
	      printf( "does not have a bound exit!\n\n" );
	    }
	  }
	  // else the plugin is not bound to the stub
	  else {
	    printf( "(Warning: plugin map%d %s is not bound to ",\
		    plugin_map->GetNumber(), plugin_map->MapLabel() );
	    printf( "dynamic stub %s h%d.)\n",\
		    stub->StubName(), edge->GetNumber() );

	    // activity for the stub itself
	    activity = task->CreateStubActivity( stub );
	    activity->SetBranchProb( selection_prob / selection_prob_sum );
	    task->ConnectActivities( stub_fork_activity, activity );
	    task->AddActivity( activity );
	    // bind activity to stub join
	    task->ConnectActivities( activity, stub_join_activity );
	    activity->SetConnection( JOINBRANCH );
	  }

	  // if this is the last plugin
	  if( plugin_map == last_plugin_map ) {
	    // add the stub join activity to the task
	    task->AddActivity( stub_join_activity );
	  }
	  // else this is not the last plugin, empty the plugin mstack
	  else {
	    // while there are messages remaining on the plugin mstack
	    while( mstack->Size() ) {
	      message = mstack->Pop();
	      activity = message->GetSource();

	      // if there are unresolved messages
	      if( activity->GetCallType() == UNRESOLVED ) {
		// assume they are asynchronous
		activity->SetCallType( ASYNC );
	      }
	      delete message;
	    }
	  }

	  // restore the old plugin exit
	  plugin_exit = old_plugin_exit;
	}  // done all the plugins

	// restore the old stub join
	stub_join_activity = old_stub_join_activity;

	// continue past the stub and traverse the next edges
	for( next_edges->First(); !next_edges->IsDone(); next_edges->Next() ) {
	  next_edge = next_edges->CurrentItem();
	  TraverseEdge( edge, next_edge );
	}
      }
      // else if the stub is a static stub
      else if( stub->HasSubmap() ) {
	printf( "Stub %s h%d is a static stub.\n",\
		stub->StubName(), edge->GetNumber() );
	plugin_maps = stub->Submaps();
	plugin_map = plugin_maps->GetLast();
	plugin_bind = stub->SubmapBinding( plugin_map );
	plugin_start = stub->SubmapPathStart( (Empty*) in_edge );

	// if the plugin is bound to the stub
	if( plugin_start ) {
	  // if the start point does not have a name
	  if( plugin_start->HyperedgeName() == "" ) {
	    plugin_entry_activity = task->AddDefaultActivity( plugin_start,\
							      "PluginIn" );
	  }
	  // else the start point has a name
	  else {
	    plugin_entry_activity  = task->AddDefaultActivity( plugin_start,\
					   plugin_start->HyperedgeName() );
	  }

	  old_plugin_exit = plugin_exit;  // remember the old plugin exit
	  old_parent_map = current_map;  // rememember the old parent map
	  old_parent_stub = parent_stub;  // rememember the old parent stub

	  current_map = plugin_map;  // this the current plugin map
	  plugin_exit = false;  // a plugin exit has not been reached yet
	  parent_stub = stub;  // this is the parent stub for its plugins

	  printf( "Traversing into plugin map%d '%s' for static stub %s h%d ",\
		  plugin_map->GetNumber(), plugin_map->MapLabel(),\
		  stub->StubName(), edge->GetNumber() );
	  printf( "at start point %s h%d.\n",\
		  plugin_start->HyperedgeName(), plugin_start->GetNumber() );

	  // traverse into the plugin
	  TraverseEdge( edge, plugin_start );

	  current_map = old_parent_map;  // restore the old parent map
	  parent_stub = old_parent_stub;  // restore the old parent stub

	  // if the plugin did not exit
	  if( !plugin_exit ) {
	    printf( "ERROR in static stub %s h%d: plugin map%d %s ",
		    stub->StubName(), edge->GetNumber(),\
		    plugin_map->GetNumber(), plugin_map->MapLabel() );
	    printf( "starting at start point %s h%d ",\
		    plugin_start->HyperedgeName(), plugin_start->GetNumber() );
	    printf( "does not have a bound exit!\n\n" );
	  }

	  plugin_exit = old_plugin_exit;  // restore the old plugin exit
	}
	// else the plugin is not bound to the stub
	else {
	  printf( "(Warning: plugin map%d %s is not bound ",\
		  plugin_map->GetNumber(), plugin_map->MapLabel() );
	  printf( "to static stub %s h%d.\n",\
		  stub->StubName(), edge->GetNumber() );

	  activity = task->CreateStubActivity( stub );
	  task->AddActivity( activity );
	}
	// continue past the stub and traverse the next edges
	for( next_edges->First(); !next_edges->IsDone(); next_edges->Next() ) {
	  next_edge = next_edges->CurrentItem();
	  TraverseEdge( edge, next_edge ); 
	}
      }
      // else the stub is empty
      else {
	printf( "Stub %s h%d is empty - treat as a responsibility.\n",\
		stub->StubName(), edge->GetNumber() );

	// add an activity for the stub
	activity = task->CreateStubActivity( stub );
	task->AddActivity( activity );
	// make the activity a candidate for messaging and connecting
	task->SetMsgCandidate( activity );
	task->SetConnectCandidate( activity );

	// continue past the stub and traverse the next edges
	for( next_edges->First(); !next_edges->IsDone(); next_edges->Next() ) {
	  next_edge = next_edges->CurrentItem();
	  TraverseEdge( edge, next_edge );
	}
      }
    }
    break;

  case WAIT:
    printf( "Wait found at h%d.\n", edge->GetNumber() );
    edge->SetVisited();  // mark this hyperedge as visited

    // error check
    if( next_edges->Size() > 1 ) {
      printf( "ERROR: wait at h%d should not have %d next edges!\n\n", \
	      edge->GetNumber(), next_edges->Size() );
    }

    // traverse the first next edge
    TraverseEdge( edge, next_edges->GetFirst() );
    break;

  case TIMER:
    printf( "Timer found at h%d.\n", edge->GetNumber() );
    edge->SetVisited();  // mark this hyperedge as visited

    // error check
    if( next_edges->Size() > 1 ) {
      printf( "ERROR: timer at h%d should not have %d next edges!\n\n", \
	      edge->GetNumber(), next_edges->Size() );
    }

    // traverse the first next edge
    TraverseEdge( edge, next_edges->GetFirst() );
    break;

  case LOOP:
    printf( "Loop found at h%d.", edge->GetNumber() );

    // if already visited
    if( edge->Visited() ) {
      printf( " (done loop body)\n" );
    }
    // else first visit
    else {
      printf( " (first visit)\n" );
      edge->SetVisited();  // mark the loop head as visited

      // if there are exactly 2 next edges
      if( next_edges->Size() == 2 ) {
	printf( "Processing loop body.\n" );

	// if not in a task
	if( !task ) {
	  // create a task for the loop head
	  task = lqn->AddDefaultTask();
	}

	task = CheckForLoopBodyTask( task );
	// create an activity for the loop head
	loop_activity = task->AddLoopHeadActivity( (Loop*)edge );
	// make the activity a candidate for connecting only
	task->SetConnectCandidate( activity );

	// remember the global loop body variables
	old_lbody_flag = lbody_flag;
	old_lbody_task = lbody_task;

	lbody_flag = true;  // processing a loop body
	// create a loop body task
 	lbody_task = lqn->CopyTask( task );
	// create an entry for the loop body task
	lbody_entry = lbody_task->AddEntry();
	// the loop head activity calls the loop body entry
	loop_activity->SetLoopCallTarget( lbody_entry );

	loop_mstack = mstack;  // remember the current Mstack
	mstack_count++;
	// create an mstack for the loop body
	lbody_mstack = new LqnMstack( mstack_count );
	mstacks->Add( lbody_mstack );
	// use the loop body Mstack
	mstack = lbody_mstack;

	// traverse the loop body
	TraverseEdge( edge, next_edges->GetLast() );

	// reply at the end of the loop body
	// get the messaging candidate for the loop body task
	lbody_activity = lbody_task->GetMsgCandidate();

	// if there is no messaging candidate for the loop body task
	if( !lbody_activity ) {
	  // create a reply activity
	  lbody_activity = lbody_task->AddDefaultActivity();
	}

	// the reply activity replies to the loop body entry
	lbody_activity->SetReplyTarget( lbody_entry );
	lbody_activity->SetConnection( REPLY );

	// error check if there are still messages on the loop body mstack
	if( mstack->Size() ) {
	  printf( "ERROR: loop body done with unresolved messages!\n\n" );
	  mstack->Print();
	}

	mstack = loop_mstack;  // restore the Mstack for the main path
	lbody_flag = old_lbody_flag;  // restore the global loop body variables
	lbody_task = old_lbody_task;
	printf( "Loop body processed.\n" );

	// traverse the first next edge
	TraverseEdge( edge, next_edges->GetFirst() );
      }
      // else there are not exactly 2 next edges
      else {
	printf( "ERROR: loop at h%d should have exaclty 2, ",\
		edge->GetNumber() );
	printf( "not %d next edges!\n\n", next_edges->Size() );
      }
    }
    break;

  case RESULT:
    end_pt = (Result*) edge;
    end_pt->SetVisited();  // mark this endpoint as visited

    // if dealing with a plugin map
    if( parent_stub ) {
      // if the end point is bound to the stub
      if( end_pt->IsBound() ) {
	next_edge = parent_stub->StubExitPath( end_pt, current_map );

	if( next_edge ) {  
	  printf( "End %s h%d exits from stub %s h%d at h%d.\n",\
		  end_pt->HyperedgeName(), end_pt->GetNumber(),\
		  parent_stub->StubName(), parent_stub->GetNumber(),\
		  next_edge->GetNumber() );

	  // traverse back to the stub
	  plugin_exit = true;
	  TraverseEdge( end_pt, parent_stub );
	}
      }
      // else the end point is not bound to the stub
      else {
	printf( "End %s h%d terminates inside stub %s h%d.\n",\
		end_pt->HyperedgeName(), end_pt->GetNumber(),\
		parent_stub->StubName(), parent_stub->GetNumber() );

	// if in a task
	if( task ) {
	  task = CheckForLoopBodyTask( task );
	}
	// else not in a task
	else {
	  // create a task for the end point
	  task = lqn->AddDefaultTask();
	  // create an entry for the task
	  entry = task->AddEntry();
	  // make a call to the entry
	  mstack->GetLastMessage()->SetTarget( entry );
	}

	// if the end point does not have a name
	if( end_pt->HyperedgeName() == "" ) {
	  end_activity = task->AddDefaultActivity( end_pt, "End" ); 
	}
	// else the end point has a name
	else {
	  end_activity = task->AddDefaultActivity( end_pt,\
						   end_pt->HyperedgeName() ); 
	}

	end_activity->SetConnection( NOCONNECTION );

	// while there are messages remaining on the mstack
	while( mstack->Size() ) {
	  message = mstack->Pop();
	  activity = message->GetSource();

	  // if the message is unresolved
	  if( activity->GetCallType() == UNRESOLVED ) {
	    // assume the message is asynchronous
	    activity->SetCallType( ASYNC );
	  }
	  delete message;
	}
      } 

      printf( "\n" );
    }
    // else dealing with the root map
    else {
      printf( "End %s reached at h%d.\n",\
	      end_pt->HyperedgeName(), end_pt->GetNumber() );

     // if in a task
     if( task ) {
	task = CheckForLoopBodyTask( task );
      }
     // else not in a task
     else {
       // pop the last message off the stack
       message = mstack->Pop();
       activity = message->GetSource();
       activity->SetCallType( NOCALL );
       // get the last task on the stack
       task = activity->GetTask();
     }

     // if the end point does not have a name
     if( end_pt->HyperedgeName() == "" ) {
       end_activity = task->AddDefaultActivity( end_pt, "End" ); 
     }
     // else the end point has a name
      else {
	end_activity = task->AddDefaultActivity( end_pt,\
						 end_pt->HyperedgeName() ); 
      }

      end_activity->SetConnection( NOCONNECTION );

      // while there are messages remaining on the mstack
      while( mstack->Size() ) {
	message = mstack->Pop();
	activity = message->GetSource();

	// if the message is unresolved
	if( activity->GetCallType() == UNRESOLVED ) {
	  // assume the message is asynchronous
	  activity->SetCallType( ASYNC );
	}
	delete message;
      }

      printf( "\n" );
    }
    break;

  default:
    edge->SetVisited();  // mark this hyperedge as visited

    // error check
    if( next_edges->Size() > 1 ) {
      printf( "ERROR: h%d should only have one, not %d next edges!\n\n", \
	      edge->GetNumber(), next_edges->Size() );
    }

    // traverse the first next edge
    TraverseEdge( edge, next_edges->GetFirst() );
    break;
  }  // end switch

  delete next_edges;
  delete prev_edges;
}


void LqnGenerator::Fork2Lqn( Hyperedge* fork_edge,
			     Cltn<Hyperedge*>* next_edges,
			     LqnTask* fork_task,
			     connection_type edge_connection )
{
  Cltn<Node*>* output_nodes = fork_edge->TargetSet();
  OrFork* or_fork = NULL;
  Hyperedge* branch_edge = NULL;
  Hyperedge* last_branch_edge = NULL;
  LqnActivity* branch_activity = NULL;
  LqnActivity* fork_activity = NULL;
  LqnEntry* fork_entry = NULL;
  LqnMstack* branch_mstack = NULL;
  LqnMstack* fork_mstack = mstack;  // remember the mstack
  double branch_prob = 0;
  double branch_prob_sum = 0;

  // if already visited
  if( fork_edge->Visited() ) {
    printf( " (already visited)\n" );
  }
  // else first visit
  else {
    printf( " (first visit)\n" );
    fork_edge->SetVisited();  // mark this hyperedge as visited

    // if in a task
    if( fork_task ) {
      // check if dealing with a loop body
      fork_task = CheckForLoopBodyTask( fork_task );
    }
    // else not in a task
    else {
      // create a task for the fork
      fork_task = lqn->AddDefaultTask();
      // create an entry for the task
      fork_entry = fork_task->AddEntry();
      // make a call to the entry from the last activity on the stack
      mstack->GetLastMessage()->SetTarget( fork_entry );
    }

    switch( edge_connection ) {
    case ORFORK:
      or_fork = (OrFork*) fork_edge;
      fork_activity = fork_task->AddDefaultActivity( or_fork, "OrFork" ); 

      // go through all the output branches
      for( output_nodes->First();
	   !output_nodes->IsDone();
	   output_nodes->Next() ) {
	// get the probability for the current branch
	branch_prob = or_fork->BranchSpecification( output_nodes->CurrentItem() )->Probability();
	// sum the branch probabilities
	branch_prob_sum += branch_prob;
      }
      break;

    case ANDFORK:
      fork_activity = fork_task->AddDefaultActivity( fork_edge, "AndFork" ); 
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }

    fork_activity->SetConnection( edge_connection );
    last_branch_edge = next_edges->GetLast();

    // go through all the branches
    for( next_edges->First(), output_nodes->First();
         !next_edges->IsDone() && !output_nodes->IsDone();
         next_edges->Next(), output_nodes->Next() ) {  
      // get the current next edge
      branch_edge = next_edges->CurrentItem();

      switch( edge_connection ) {
      case ORFORK:
	or_fork = (OrFork*) fork_edge;
	branch_prob = or_fork->BranchSpecification( output_nodes->CurrentItem() )->Probability();

	branch_activity = fork_task->CreateDefaultActivity( branch_edge,\
							    "OrBranch" );
	branch_activity->SetBranchProb( branch_prob / branch_prob_sum );
	fork_task->ConnectActivities( fork_activity, branch_activity );
	fork_task->AddActivity( branch_activity );
	// make the branch activity a candidate for messaging and connecting
	fork_task->SetMsgCandidate( branch_activity );
	fork_task->SetConnectCandidate( branch_activity );
	break;

      case ANDFORK:
	break;

      default: ; // DA: Added August 2004 (to satisfy GCC)
      }

      // if this is the last outgoing branch
      if( branch_edge == last_branch_edge ) {
	// use the original Mstack
	mstack = fork_mstack;
	// traverse the last branch
	TraverseEdge( fork_edge, branch_edge );
      }
      // else this is not the last branch
      else {
	// make a mstack for the branch
	mstack_count++;
	branch_mstack = mstack->Copy( mstack_count );
	mstacks->Add( branch_mstack );

	// use the branch Mstack
	mstack = branch_mstack;

	// traverse the branch
	TraverseEdge( fork_edge, branch_edge );
      }
    }
  }  // done all the branches
}


void LqnGenerator::Join2Lqn( Hyperedge* join_edge,\
			     Cltn<Hyperedge*>* next_edges,\
			     Cltn<Hyperedge*>* prev_edges,\
			     LqnTask* join_task,\
			     LqnActivity* join_activity,\
			     connection_type edge_connection )
{
  OrJoin* or_join = NULL;
  Synchronization* and_join = NULL;
  LqnActivity* branch_activity = NULL;
  LqnActivity* activity = NULL;
  LqnMessage* message = NULL;

  switch( edge_connection ) {
  case ORJOIN:
    or_join = (OrJoin*)join_edge;

    // get the connection candidate for the join task
    branch_activity = join_task->GetConnectCandidate();

    // if there is no connection candidate for the join task
    if( !branch_activity ) {
      // create a branch activity
      branch_activity = join_task->AddDefaultActivity( "OrJoinBranch" );
    }

    join_task->ConnectActivities( branch_activity, join_activity );
    branch_activity->SetConnection( JOINBRANCH );

    // if this is the last incoming branch
    if( join_activity->GetEdgeVisits() == or_join->PathCount() ) {
      join_task->AddActivity( join_activity );
      // make the join activity a candidate for messaging and connecting
      join_task->SetMsgCandidate( join_activity );
      join_task->SetConnectCandidate( join_activity );

      // error check
      if( next_edges->Size() > 1 ) {
	printf( "ERROR LqnGenerator::Join2Lqn: Or Join at h%d has %d next edges!\n\n",\
		join_edge->GetNumber(), next_edges->Size() );
      }

      // traverse the next edge
      TraverseEdge( join_edge, next_edges->GetFirst() );
    } 
    // else this is not the last incoming branch
    else {
      // while there are messages remaining on the branch stack
      while( mstack->Size() ) {
	message = mstack->Pop();
	activity = message->GetSource();

	// if the remaining messages is unresolved
	if( activity->GetCallType() == UNRESOLVED ) {
	  // assume the message is asynchronous
	  activity->SetCallType( ASYNC );
	}
	delete message;
      }
      // do not traverse any further past the join
      // collapse the traversal back to the fork
    }
    break;

  case ANDJOIN:
    and_join = (Synchronization*)join_edge;

    // get the connection candidate for the join task
    branch_activity = join_task->GetConnectCandidate();

    // if there is no connection candidate for the join task
    if( !branch_activity ) {
      // create a branch activity
      branch_activity = join_task->AddDefaultActivity( "AndBranch" );
    }

    join_task->ConnectActivities( branch_activity, join_activity );
    branch_activity->SetConnection( JOINBRANCH );

    // if this is the last incoming branch
    if( join_activity->GetEdgeVisits() == and_join->InputCount() ) {
      join_task->AddActivity( join_activity );
      // make the join activity a candidate for messaging and connecting
      join_task->SetMsgCandidate( join_activity );
      join_task->SetConnectCandidate( join_activity );

      // error check
      if( next_edges->Size() > 1 ) {
	printf( "ERROR: And Join at h%d has %d next edges!\n\n",\
		join_edge->GetNumber(), next_edges->Size() );
      }

      // traverse the next edge
      TraverseEdge( join_edge, next_edges->GetFirst() );
    } 
    else {
      // while there are messages remaining on the branch stack
      while( mstack->Size() ) {
	message = mstack->Pop();
	activity = message->GetSource();

	// if the remaining messages is unresolved
	if( activity->GetCallType() == UNRESOLVED ) {
	  // assume the message is asynchronous
	  activity->SetCallType( ASYNC );
	}
	delete message;
      }
      // do not traverse any further past the join
      // collapse the traversal back to the fork
    }
    break;

    default: ; // DA: Added August 2004 (to satisfy GCC)
  }
}


void LqnGenerator::TraverseEdge( Hyperedge* edge, Hyperedge* next_edge ) 
{
  // if the next edge has already been visited
  if( next_edge->Visited() ) {
    // if the next edge is a loop, stub, OR join, or AND join
    if( ( next_edge->EdgeType() == LOOP )\
	|| ( next_edge->EdgeType() == STUB )\
	|| ( next_edge->EdgeType() == OR_JOIN )\
	|| ( ( next_edge->EdgeType() == SYNCHRONIZATION )\
             && ( SyncType( next_edge ) == ANDJOIN ) ) ) {
      // check for component boundary crossings
      ComponentBoundaryXing2Lqn( edge, next_edge );
      // transform the next edge
      Edge2Lqn( next_edge, edge );
    }
    // else the next edge is some other kind of hyperedge
    else {
      printf( "(Warning: h%d has already been visited - traversal stopped.)\n",\
	      next_edge->GetNumber() );
      next_edge->GetFigure()->SetSelected();
    }
  }
  // else the next edge has not been visited
  else {
    // check for component boundary crossings
    ComponentBoundaryXing2Lqn( edge, next_edge );
    // transform the next edge
    Edge2Lqn( next_edge, edge );
  }
}


void LqnGenerator::ComponentBoundaryXing2Lqn( Hyperedge* edge, Hyperedge* next_edge )
{
  switch( FindXing( edge, next_edge ) ) {
  case NO_COMP:
    break;

  case STAY: 
    break;

  case ENTER:
    EnterComponent2Lqn( next_edge );
    break;

  case LEAVE:
    LeaveComponent2Lqn( edge );  
    break;

  case CHANGE:
    LeaveComponent2Lqn( edge );  
    EnterComponent2Lqn( next_edge );
    break;
  }  // end switch
}


void LqnGenerator::LeaveComponent2Lqn( Hyperedge* edge )
{
  ComponentReference* comp_ref = NULL;
  LqnTask* task = NULL;
  LqnActivity* activity = NULL;

  // get the component reference for the edge, if any
  comp_ref = edge->GetFigure()->GetContainingComponent();

  // if the component reference exists
  if( comp_ref ) {
    // get the task corresponding to the component reference
    task = lqn->GetTaskForComp( comp_ref );

    // if the task exists
    if( task ) {
      task = CheckForLoopBodyTask( task );
      printf( "Leaving task %s.\n", task->GetName() );

      // get the messaging candidate for the task
      activity = task->GetMsgCandidate();

      // if there is no messaging candidate for the task
      if( !activity ) {
	// create an activity to make a call or send a reply
	activity = task->AddDefaultActivity();
      }

      // push the activity on the mstack
      mstack->Push( new LqnMessage( activity ) );
    }
    // else the task does not exist
    else {
      printf( "ERROR: no task was found for component %s (id %d)!\n\n",\
	      comp_ref->ReferenceName(),\
	      comp_ref->ComponentReferenceNumber() );
    }
  }
  // else the component reference does not exist
  else {
    printf( "ERROR: no component reference was found for h%d!\n\n",\
	    edge->GetNumber() );
  }
}


void LqnGenerator::EnterComponent2Lqn( Hyperedge* next_edge )
{
  ComponentReference* next_comp_ref = NULL;
  LqnTask* next_path_task = NULL;
  LqnTask* last_stack_task = NULL;
  LqnTask* prev_stack_task = NULL;
  LqnMessage* reply_msg = NULL;
  LqnMessage* call_msg = NULL;
  LqnActivity* reply_activity = NULL;
  // LqnActivity* handle_reply_activity = NULL; // DA: August 2004
  LqnActivity* calling_activity = NULL;
  LqnActivity* fwding_activity = NULL;
  LqnEntry* called_entry = NULL;
  LqnEntry* fwding_entry = NULL;
  LqnEntry* fwd_to_entry = NULL;

  // get the comp ref for the next edge along the path
  next_comp_ref = next_edge->GetFigure()->GetContainingComponent();

  // if there is a component reference for the next edge along the path
  if( next_comp_ref ) {
    // get the task for to the next comp ref along the path
    next_path_task = lqn->GetTaskForComp( next_comp_ref );

    // if there is a task for the next component ref along the path
    if( next_path_task ) {
      printf( "Entering task '%s'.\n", next_path_task->GetName() );
      next_path_task = CheckForLoopBodyTask( next_path_task );

      // if the next task along the path is already on the mstack
      if( mstack->FindTask( next_path_task ) ) {
	// get the last task on the mstack
	last_stack_task = mstack->GetLastTask();
	// get the task previous to the last task on the mstack
	prev_stack_task = mstack->GetTaskPreviousTo( last_stack_task );

	// if the next task along the path calls the last task on the mstack
	if( next_path_task == prev_stack_task ) {
	  // process a synchronous reply
	  printf( "SYNC interaction detected.\n" );
	  mstack->Print();

          // process reply message
	  // message for the reply
	  reply_msg = mstack->Pop( REPLY_MSG );

	  // error check, no target entry
	  if( reply_msg->GetTarget() ) {
	    printf( "ERROR SYNC interaction: reply message should not have a target entry!\n\n" );
	  }

	  reply_activity = reply_msg->GetSource();

	  // error check, no reply activity
	  if( !reply_activity ) {
	    printf( "ERROR SYNC interaction: reply message does not have a source activity!\n\n" );
	  }

	  delete reply_msg;  // done with the reply message

	  // process synchronous call message
	  // message for the SYNC call
	  call_msg = mstack->Pop( SYNC_MSG );
	  // target entry for the SYNC call
	  called_entry = call_msg->GetTarget();
	  // the reply activity replies to the called entry
	  reply_activity->SetReplyTarget( called_entry );
	  // source activity of the SYNC call
	  calling_activity = call_msg->GetSource();

	  delete call_msg;  // done with the call message

	  mstack->Print();
	  printf( "SYNC interaction processed.\n" );
	}
	// else next_path_task != prev_stack_task
	else {
	  // process a forwarded reply
	  printf( "FWD interaction detected.\n" );
	  mstack->Print();

          // process the final reply message in the forwarding chain
	  // message for the final reply 
	  reply_msg = mstack->Pop( REPLY_MSG );
	  // error check, no target entry
	  if( reply_msg->GetTarget() ) {
	    printf( "ERROR FWD interaction: reply message does not have a target entry!\n\n" );
	  }

	  reply_activity = reply_msg->GetSource();

	  // error check, no reply activity
	  if( !reply_activity ) {
	    printf( "ERROR FWD interaction: reply message does not have a source activity!\n\n" );
	  }

	  delete reply_msg;  // done with the reply message

	  // get the last forwarded to entry
	  fwd_to_entry = mstack->PopEntry();
	  // the reply activity replies to the last forwarded to entry
	  reply_activity->SetReplyTarget( fwd_to_entry );

	  // process forwarding chain
	  // get the last task on the Mstack
	  last_stack_task = mstack->GetLastTask();
	  // get the task previous to the last task on the mstack
	  prev_stack_task = mstack->GetTaskPreviousTo( last_stack_task );

	  // while the next task on the path
	  // is not the previous path on the mstack
	  while( next_path_task != prev_stack_task ) {
	    // get the forwarding activity
	    fwding_activity = mstack->PopActivity();
	    // get the forwarding entry 
	    fwding_entry = mstack->PopEntry();
	    // the forwarding activity replies to the forwarding entry 
	    fwding_activity->SetReplyTarget( fwding_entry );
	    // the forwarding entry forwards to the forwarded to entry
	    fwding_entry->SetFwdTarget( fwd_to_entry );
	  
	    // get the last task on the Mstack
	    last_stack_task = mstack->GetLastTask();
	    // task previous to the last Mstack task
	    prev_stack_task = mstack->GetTaskPreviousTo( last_stack_task );
	    // the last forwarding entry is now the entry being forwarded to
	    fwd_to_entry = fwding_entry;
	  }  
          // now next_path_task == prev_stack_task

	  // first forwarding activity
	  fwding_activity = mstack->PopActivity();

	  // process original synchronous call message
	  // message for the original SYNC call
	  call_msg = mstack->Pop( SYNC_MSG );
	  // target entry for the original SYNC call and first forwarding entry
	  called_entry = call_msg->GetTarget();
	  // called entry forwards to the forwarded to entry
	  called_entry->SetFwdTarget( fwd_to_entry );
	  // first forwarding activity replies to the called entry
	  fwding_activity->SetReplyTarget( called_entry );
	  // source activity of the original SYNC call
	  calling_activity = call_msg->GetSource();

	  delete call_msg;  // done with the original call message

	  mstack->Print();
	  printf( "FWD interaction processed.\n" );
	}
      }
      // else the next task is not on the stack
      else {
	// process a call
	printf( "Undetermined call detected from task '%s' to task '%s'.\n",\
		mstack->GetLastTask()->GetName(), next_path_task->GetName() );

	mstack->Print();
	// create an entry on the next task along the path
	called_entry = next_path_task->AddEntry();
	// the  called entry is the target of the last message on the Mstack
	mstack->GetLastMessage()->SetTarget( called_entry );
      }
    }
    // else  there is no next task
    else {
      printf( "ERROR EnterComponent2Lqn: no task was found for component reference '%s' (id %d)!\n\n",\
	      next_comp_ref->ReferenceName(),\
	      next_comp_ref->ComponentReferenceNumber() );
    }
  }
  // else there is no next component reference
  else {
    printf( "ERROR EnterComponent2Lqn: no component reference was found for hyperedge (h%d)!\n\n",\
	    next_edge->GetNumber() );
  }
}


xing_type LqnGenerator::FindXing( Hyperedge* edge1, Hyperedge* edge2 )
{
  xing_type xing;

  // get the tasks
  LqnTask* task1 = lqn->GetTaskForComp( edge1->GetFigure()->GetContainingComponent() );
  LqnTask* task2 = lqn->GetTaskForComp( edge2->GetFigure()->GetContainingComponent() );

  // if neither edge1 nor edge2 are in a task
  if( !task1 && !task2 ) {
    xing = NO_COMP;

    if( print) {
      printf( "\tFindXing: from edge h%d to edge h%d - NO component\n",\
	      edge1->GetNumber(), edge2->GetNumber() );
    }
  }
  // else if edge1 is in a task but edge2 is not
  else if( task1 && !task2 ) {
    xing = LEAVE;

    if( print) {
      printf( "\tFindXing: from edge h%d to edge h%d - LEAVE component\n",\
	      edge1->GetNumber(), edge2->GetNumber() );
    }
  }
  // else if edge1 is not in a task but edge2 is
  else if( !task1 && task2 ) {
    xing = ENTER;

    if( print) {
      printf( "\tFindXing: from edge h%d to edge h%d - ENTER component\n",\
	      edge1->GetNumber(), edge2->GetNumber() );
    }
  }
  // else edge1 and edge2 are both in a task
  else {
    // if edge1 and edge2 are both in the same task
    if( task1 == task2 ) {
      xing = STAY;

      if( print) {
	printf( "\tFindXing: from edge h%d to edge h%d - STAY in component\n",\
		edge1->GetNumber(), edge2->GetNumber() );
      }
    }
    // else edge1 and edge2 are in different tasks
    else {
      xing = CHANGE;

      if( print) {
	printf( "FindXing: from edge h%d to edge h%d - CHANGE components\n",\
		edge1->GetNumber(), edge2->GetNumber() );
      }
    }
  }

  return xing;
}


// changes the task to the loop body task when processing a loop body
LqnTask* LqnGenerator::CheckForLoopBodyTask( LqnTask* task )
{
  LqnTask* check_task = task;

  // if currently processing a loop body
  if( lbody_flag ) {
    printf( "Processing loop body, " );

    // if the given task is the original task for the loop body task
    if( check_task == lbody_task->GetOriginal() ) {
      printf( "using loop body task '%s' for task '%s'.\n",\
	      lbody_task->GetName(), check_task->GetName() );
      // use the loop body task
      check_task = lbody_task;
    }
    // else the given task is not the original task for the loop body task
    else {
      printf( "no loop body task '%s' for task '%s'.\n",\
	      lbody_task->GetName(), check_task->GetName() );
    }
  }

  return check_task;
}


// returns the type of synchronization encountered
connection_type LqnGenerator::SyncType( Hyperedge* edge )
{
  connection_type sync_type;
  Cltn<Hyperedge*>* next_edges = new Cltn<Hyperedge*>;
  Cltn<Hyperedge*>* prev_edges = new Cltn<Hyperedge*>;

  NextEdges( edge, next_edges );
  PrevEdges( edge, prev_edges );

  if( ( next_edges->Size() > 1 ) && ( prev_edges->Size() == 1 ) ) {
    sync_type = ANDFORK;
  }
  else if( ( next_edges->Size() == 1 ) && ( prev_edges->Size() > 1 ) ) {
    sync_type = ANDJOIN;
  }
  else {
    sync_type = ANDSYNC;
  }

  delete next_edges;
  delete prev_edges;

  return sync_type;
}


void LqnGenerator::NextEdges( Hyperedge* edge, Cltn<Hyperedge*>* next )
{
  Cltn<Node*>* nodes = edge->TargetSet();

  for( nodes->First(); !nodes->IsDone(); nodes->Next() ) {
    next->Add( nodes->CurrentItem()->NextEdge() );
  }
}


void LqnGenerator::PrevEdges( Hyperedge* edge, Cltn<Hyperedge*>* previous )
{
  Cltn<Node*>* nodes = edge->SourceSet();

  for( nodes->First(); !nodes->IsDone(); nodes->Next() ) {
    previous->Add( nodes->CurrentItem()->PreviousEdge() );
  }
}


void LqnGenerator::TestMstack()
{
  LqnTask* task = lqn->AddDefaultTask();
  LqnMessage* message = NULL;

  printf( "\n--------- Start Mstack test ---------\n" );
  mstack->Print();
  message = mstack->Pop();
  mstack->Print();
  mstack->Push( new LqnMessage( task->AddDefaultActivity() ) );
  mstack->Print();
  message = mstack->GetLastMessage();
  message->SetTarget( task->AddEntry() );
  mstack->Print();
  mstack->Push( new LqnMessage( task->AddDefaultActivity() ) );
  mstack->Print();
  message = mstack->Pop();
  mstack->Print();
  message = mstack->Pop();
  mstack->Print();
  message = mstack->Pop();
  mstack->Print();
  mstack->Push( new LqnMessage( task->AddDefaultActivity(), task->AddEntry() ) );
  mstack->Print();
  mstack->Push( new LqnMessage( task->AddDefaultActivity(), task->AddEntry() ) );
  mstack->Print();
  mstack->Push( new LqnMessage( task->AddDefaultActivity(), task->AddEntry() ) );
  mstack->Print();
  message = mstack->Pop();
  mstack->Print();
  message = mstack->Pop();
  mstack->Print();
  message = mstack->Pop();
  mstack->Print();
  message = mstack->Pop();
  mstack->Print();
  delete task;
  printf( "---------- End Mstack test ----------\n\n" );
}
