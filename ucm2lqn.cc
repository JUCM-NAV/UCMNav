// Use Case Maps to Layered Queueing Networks converter
#include <stdio.h>
#include <stdlib.h>

#include "ucm2lqn.h"
extern char loaded_file_path[128];


// returns pointer to sole instance, creates instance on first call
Ucm2Lqn* Ucm2Lqn::Instance()
{
  if( !SoleUcm2Lqn ) {
    SoleUcm2Lqn = new Ucm2Lqn();
  }
  return( SoleUcm2Lqn );
}


// constructor
Ucm2Lqn::Ucm2Lqn()
{ 
  printf( "--------------------------------------------------------------------------------\n" );
  printf( "                               Begin Ucm2Lqn\n" );
  printf( "--------------------------------------------------------------------------------\n\n" );
  print = false;
}


// destructor
Ucm2Lqn::~Ucm2Lqn()
{
  Clear();
  SoleUcm2Lqn = NULL;
  printf( "--------------------------------------------------------------------------------\n" );
  printf( "                               End Ucm2Lqn\n" );
  printf( "--------------------------------------------------------------------------------\n\n" );
}


// initialize internal variables
void Ucm2Lqn::Init()
{
  lqn = new Lqn;  // create LQN
  mstacks = new Cltn<LqnMstack*>;
  mstack_count = 1;
  mstack = new LqnMstack( mstack_count );
  mstacks->Add( mstack );
  current_map = NULL;
  lbody_flag = false;
  lbody_task = NULL;
  parent_stub = NULL;
  stub_join_activity = NULL;
  plugin_exit = false;
  starts = new Cltn<Hyperedge*>;
  root_starts = new Cltn<Hyperedge*>;
  // TestMstack();
}


// clear internal variables
void Ucm2Lqn::Clear()
{
  // reset visited flags of all edges in hypergraph
  TransformationManager::Instance()->CurrentGraph()->ResetVisited();
  while( mstacks->Size() ) {
    delete mstacks->Detach();
  }
  delete mstacks;
  delete lqn;
  while( starts->Size() ) {
    starts->Detach();
  }
  delete starts;
  while( root_starts->Size() ) {
    root_starts->Detach();
  }
  delete root_starts;
}


// reset internal variables
void Ucm2Lqn::Reset()
{
  Clear();
  Init();
}


// transform UCM into LQN
void Ucm2Lqn::Transmorgrify( Cltn<Map*>* maps, Cltn<Device*>* devices )
{  
  Map* trans_map;  // map being transmogrified
  Cltn<Path*>* paths;  // paths on the current map
  Path* current_path;  // current path
  Cltn<ComponentReference*>* comp_refs;  // component references on the current map
  Cltn<ResponsibilityReference*>* resp_refs;  // responsibility references on the current map
  FILE* output_file;
  FILE* log_file;
  char output_file_name[128];
  output_file_name[0] = 0;  // initialize the file name
  output_file_name[127] = 0;
  char log_file_name[128];
  log_file_name[0] = 0;  // initialize the file name
  log_file_name[127] = 0;

  printf( "========================= Begin Transmorgrify ========================\n\n", maps->Size() );
  Init();  // initialize transmorgrify
  CreateDevices( devices );  // verify that all the devices are properly specified

  //fprintf( log_file, "UCM info as follows (%d maps):\n", maps->Size() );
  for( maps->First(); !maps->IsDone(); maps->Next() ) {  // go through all the maps
    trans_map = maps->CurrentItem();
    trans_map->MapHypergraph()->ResetVisited();  // // resets the visited flags for all the hyperedges
    comp_refs = trans_map->Components();  // all the components for the current map
    paths = trans_map->Paths();  // all the paths for the current map
    resp_refs = trans_map->Responsibilities();  // all the responsibilities for the current map
    for( paths->First(); !paths->IsDone(); paths->Next() ) {  // get all the start points for the current map
      current_path = paths->CurrentItem();
      if( current_path->GetPathStart()->EdgeType() == START ) {  // the start of a path segment is a start point
        starts->Add( current_path->GetPathStart() );  // add to the list of start points
      }
    }
    if( trans_map->IsRootMap() ) {  // root map
//      fprintf( log_file, "Root map%d '%s' has:\n", trans_map->GetNumber(), trans_map->MapLabel() );
    }
    else if( trans_map->IsPlugIn() ) {  // plug-in map
//      fprintf( log_file, "Plug-in map%d '%s' has:\n", trans_map->GetNumber(), trans_map->MapLabel() );
    }
    else {  // neither a root map nor a plug-in map
//      fprintf( log_file, "Map%d '%s' has:\n", trans_map->GetNumber(), trans_map->MapLabel() );
    }
//    fprintf( log_file, "\t%d component references\n", comp_refs->Size() );
//    fprintf( log_file, "\t%d paths\n", paths->Size() );
//    fprintf( log_file, "\t%d start points\n", starts->Size() );
//    fprintf( log_file, "\t%d responsibilities\n", resp_refs->Size() );

    if( trans_map->IsTopLevelMap() && trans_map->IsRootMap() ) {  // process only the top level root map
      current_map = trans_map;
//      fprintf( log_file, "Map%d '%s' is the top level root map!\n", trans_map->GetNumber(), trans_map->MapLabel() );
      parent_stub = NULL;
      for( starts->First(); !starts->IsDone(); starts->Next() ) {  // go through all start points
	root_starts->Add( starts->CurrentItem() );  // add start as a root start
      }
    }
    CreateTasks( comp_refs );

    while( starts->Size() ) {  // clear the starts for the next map
      starts->Detach();
    }
  }  // endfor

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

  printf( "*** Converting file '%s' ***\n\n", loaded_file_path );
  fprintf( log_file, "*** Converting file '%s' ***\n\n", loaded_file_path );
  printf( "\n*** Begin Path Traversal ***\n" );
  for( root_starts->First(); !root_starts->IsDone(); root_starts->Next() ) {  // traverse all the root map start points
    Start2Lqn( (Start*)root_starts->CurrentItem() );  // generate LQN
  }
  printf( "\n*** End Path Traversal ***" );
  lqn->FilePrint( output_file );
  fclose( output_file );
  fl_show_message( "", "LQN created and saved in file", output_file_name );
  fprintf( log_file, "\n*** LQN saved in file '%s' ***\n", output_file_name );
  fclose( log_file );
  fl_show_message( "", "Log comments saved in file", log_file_name );
  Clear();
  printf( "\n========================== End Transmorgrify =========================\n\n", maps->Size() );
}


void Ucm2Lqn::CreateDevices( Cltn<Device*>* devices )
{
  char name[50];
  name[0] = 0;  // initialize the name
  name[49] = 0;
  char number[10];
  number[0] = 0;  // initialize the number
  number[9] = 0;
  Device* current_device = NULL;  // current device being looked at
  int id = 0;
  double optime = 0;

  printf( "CreateDevices: %d devices.\n", devices->Size() );
  for( devices->First(); !devices->IsDone(); devices->Next() ) {  // go through all the devices
    current_device = devices->CurrentItem();
    optime = current_device->OpTime();
    id = current_device->Number();
    switch( current_device->DeviceType() ) {
    case PROCESSOR:  // the device is a processor
      strcpy( name, current_device->Name() );
      if( strequal( current_device->Name(), "" ) ) {  // the name is missing
	strcat( name, "P" );
	sprintf( number, "%d", id );
	strcat( name, number );
	printf( "WARNING: processor (id %d) has no name, using '%s'.\n", id, name );
      } 
      if( optime <= 0.0 ) {  // operation time is specified improperly
	printf( "WARNING: processor '%s' (id %d): op time improperly specified as %lf, using 1.0.\n", name, id, optime );
	optime = 1.0;
      }
      lqn->AddDevice( name, id, 1.0 / optime );  // add device to LQN using specified speed factor
    break;
    default:  // the device is not a processor
       printf( "WARNING: device '%s' is not a PROCESSOR.\n", current_device->Name() );
    break;
    }
  }  // endfor
  printf( "\n" );
} 


// verify methods to check that all the necessary information for transformation into LQN is present in UCM
void Ucm2Lqn::CreateTasks( Cltn<ComponentReference*>* component_refs )
{
  ComponentReference* current_comp_ref = NULL;  // current component reference
  Component* comp = NULL;

  printf( "CreateTasks: %d component references.\n", component_refs->Size() );
  for( component_refs->First(); !component_refs->IsDone(); component_refs->Next() ) {  // go through all the comp references
    current_comp_ref = component_refs->CurrentItem();
    if( !lqn->GetTaskForComp( current_comp_ref ) ) {  // an LQN task does not exist for this component reference
      comp = current_comp_ref->ReferencedComponent();  // get the referenced component
      if( comp ) {  // the referenced component exists
	if( comp->GetType() != PROCESS ) {  // the component is not a process.
	  printf( "WARNING: component '%s' (id %d) is not a PROCESS.\n", comp->GetLabel(), comp->GetComponentNumber() );
	}
	if( comp->ProcessorId() == NA ) {  // a processor is not specified
	  printf( "WARNING: component '%s' (id %d) has no processor specified, assigning to infinite processor.\n",\
                  comp->GetLabel(), comp->GetComponentNumber() );
	  lqn->AddTaskInfProc( current_comp_ref );  // create a task that runs on the infinite processor
	}
	else {  // a processor is specified
	  lqn->AddTask( current_comp_ref, lqn->GetDevice( comp->ProcessorId() ) );  // create a task running on the processor
	}
      }
      else {  // the referenced component does not exist
	printf( "WARNING: component reference '%s' (id %d) does not point to a logical component.\n",\
                current_comp_ref->ReferenceName(), current_comp_ref->ComponentReferenceNumber() );
	lqn->AddTaskInfProc( current_comp_ref );  // create a task that runs on the infinite processor
      }
    }
  }  // endfor
  printf( "\n" );
}


void Ucm2Lqn::Start2Lqn( Start* start_pt )
{
  Hyperedge* next_edge = NULL;
  ComponentReference* comp_ref = NULL;
  ComponentReference* next_comp_ref = NULL;
  LqnTask* task = NULL;
  LqnTask* ref_task = NULL;
  LqnEntry* ref_entry = NULL;
  LqnEntry* entry = NULL;
  LqnActivity* ref_activity = NULL;
  LqnActivity* activity = NULL;
  LqnMessage* message = NULL;
  double think_time = 1.0;
  Cltn<Hyperedge*>* next_edges = new Cltn<Hyperedge*>;

  start_pt->SetVisited();

  if( start_pt->ArrivalType() == UNDEFINED ) {  // the arrival type has been left undefined
    printf( "WARNING: start point '%s' (h%d) has an UNDEFINED arrival rate.\n",\
	    start_pt->HyperedgeName(), start_pt->GetNumber() );
  }
  else {  // the arrival type has been defined
    switch( start_pt->ArrivalType() ) {
    case EXPONENTIAL:
      if( start_pt->Input(1) < 0 ) {  // the mean arrival rate is negative
	printf( "WARNING: start point '%s' (h%d) has an EXPONENTIAL arrival rate with negative mean, using 1.0 instead.\n",\
		start_pt->HyperedgeName(), start_pt->GetNumber() );
      }
      else if( start_pt->Input(1) == 0 ) {  // the mean arrival rate is zero
	printf( "WARNING: start point '%s' (h%d) has an EXPONENTIAL arrival rate with zero mean, using 1.0 instead.\n",\
		start_pt->HyperedgeName(), start_pt->GetNumber() );
      }
      else {
	think_time = start_pt->Input(1);
      }
      break;
    case DETERMINISTIC:
      if( start_pt->Input(1) < 0 ) {  // the arrival rate is negative
	printf( "WARNING: start point '%s' (h%d) has a DETERMINISTIC arrival rate with a negative value, using 1.0 instead.\n",\
		start_pt->HyperedgeName(), start_pt->GetNumber() );
      }
      else if( start_pt->Input(1) == 0 ) {  // the arrival rate is zero
	printf( "WARNING: start point '%s' (h%d) has a DETERMINISTIC arrival rate with zero value, using 1.0 instead.\n",\
		start_pt->HyperedgeName(), start_pt->GetNumber() );
      }
      else {
	think_time = start_pt->Input(1);
      }
      break;
    case UNIFORM:
      printf( "WARNING: start point '%s' (h%d) cannot use UNIFORM arrivals, using exponential arrivals with mean 1.0.\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber() );
      break;
    case ERLANG:
      printf( "WARNING: start point '%s' (h%d) cannot use ERLANG arrivals, using exponential arrivals with mean 1.0.\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber() );
      break;
    case EXPERT:
      printf( "WARNING: start point '%s' (h%d) cannot use EXPERT arrivals, using exponential arrivals with mean 1.0.\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber() );
      break;
    }
  }
  if( start_pt->StreamType() == CLOSED_ARRIVAL ) {
    lqn->SetClosed();
    if( start_pt->PopulationSize() < 1 ) {
      printf( "WARNING: start point '%s' (h%d) has Closed Arrivals with an incorrect population size of %d, using 1 instead.\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber(), start_pt->PopulationSize() );
    }
    else {
      lqn->SetPopulation( start_pt->PopulationSize() );
    }
  }
  ref_task = lqn->AddRefTask( start_pt );  // create a new reference task
  ref_entry = ref_task->AddEntry();  // add an entry
  ref_activity = ref_entry->GetFirstActivity();
  ref_activity->SetStochastic();
  ref_activity->SetServiceTime( think_time );

  comp_ref = start_pt->GetFigure()->GetContainingComponent();  // get the component reference
  if( comp_ref ) {  // the component reference exists
    task = lqn->GetTaskForComp( comp_ref );  // get the task corresponding to the component reference
    if( task ) {
      printf( "Start point '%s' (h%d) is contained in component corresponding to task '%s'.\n",\
	      start_pt->HyperedgeName(), start_pt->GetNumber(), task->GetName() );
      entry = task->AddRefEntry();  // new entry for the reference task
      message = new LqnMessage( ref_activity, entry );
      mstack->Push( message );  // push the message on the Mstack
    }
    else {
      printf( "ERROR Start2Lqn: no task was found for component reference '%s' (id %d)!\n\n",\
              comp_ref->ReferenceName(), comp_ref->ComponentReferenceNumber() );
    }
  }
  else {  // else the component reference does not exist
    printf( "START point h%d is not contained in a component.\n", start_pt->GetNumber() );
    message = new LqnMessage( ref_activity );  // create a message from the reference activity
    mstack->Push( message );  // push the message on the Mstack
  }
  NextEdges( start_pt, next_edges );  // look ahead at the next hyperedges
  if( next_edges->Size() > 1 ) {  // error check
    printf( "ERROR Start2Lqn: start point at hyperedge h%d should not have %d next edges!\n\n",\
	    start_pt->GetNumber(), next_edges->Size() );
  }
  TraverseEdge( start_pt, next_edges->GetFirst() );  // traverse the first next edge

  while( next_edges->Size() ) {
    next_edges->Detach();
  }
  delete next_edges;
}


void Ucm2Lqn::Edge2Lqn( Hyperedge* edge, Hyperedge* in_edge )
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
  LqnMessage* lbody_message = NULL;
  LqnMstack* loop_mstack = NULL;
  LqnMstack* lbody_mstack = NULL;
  bool old_lbody_flag;
  // variables used for an end point
  Result* end_pt = NULL;
  LqnActivity* end_activity = NULL;

  NextEdges( edge, next_edges );  // get the next hyperedges
  PrevEdges( edge, prev_edges );  // get the previous hyperedges
  comp_ref = edge->GetFigure()->GetContainingComponent();  // get the component reference
  if( comp_ref ) {  // there is a component reference
    task = lqn->GetTaskForComp( comp_ref );  // get the task corresponding to the component reference
  }
  else {  // there is no component reference
    task = NULL;
  }

  switch( edge->EdgeType() ) {  // check the edge type

  case RESPONSIBILITY_REF:
    edge->SetVisited();  // mark this hyperedge as visited
    resp_ref = (ResponsibilityReference*) edge;
    // printf( "Responsibility '%s' found at hyperedge h%d.\n", resp_ref->HyperedgeName(), resp_ref->GetNumber() );

    if( task ) {  // in a task
      task = CheckForLoopBodyTask( task );
    }
    else {  // not in a task
      task = lqn->AddDefaultTask();  // create a new task for the responsibility
      entry = task->AddEntry();  // create a new entry for the task
      mstack->GetLastMessage()->SetTarget( entry );   // make a call to the entry
    }

    activity = task->AddResponsibilityActivity( resp_ref );
    activity->SetConnection( NORMAL );

    if( next_edges->Size() > 1 ) {  // do some error checking
      printf( "ERROR Responsibility2Lqn: responsibility '%s' at hyperedge h%d should have only one, not %d next edges!\n\n",\
	      resp_ref->ParentResponsibility()->Name(), edge->GetNumber(), next_edges->Size() );
    }

    TraverseEdge( edge, next_edges->GetFirst() );  // traverse the only next edge
    break;

  case OR_FORK:
    printf( "OR FORK with %d branches found at hyperedge h%d.", next_edges->Size(), edge->GetNumber() );
    Fork2Lqn( edge, next_edges, task, ORFORK );
    break;

  case OR_JOIN:
    printf( "OR JOIN with %d branches found at hyperedge h%d.", prev_edges->Size(), edge->GetNumber() );
    if( edge->Visited() ) {  // already visited
      printf( " (already visited)\n" );
      if( task ) {  // in a task
	task = CheckForLoopBodyTask( task );
      }
      else {  // not in a task
	task = lqn->AddDefaultTask();  // create a new task for the join
	entry = task->AddEntry();  // create a new entry for the task
	mstack->GetLastMessage()->SetTarget( entry );  // make a call to the entry from the last activity on the Mstack
      }
      activity = lqn->GetActivity( edge );
      activity->IncrementEdgeVisits();  // increment the number of edge visits
    }
    else {  // first visit
      printf( " (first visit)\n" );
      edge->SetVisited();  // mark this hyperedge as visited
      if( task ) {  // in a task
	task = CheckForLoopBodyTask( task );
      }
      else {  // not in a task
	task = lqn->AddDefaultTask();  // create a new task for the join
	entry = task->AddEntry();  // create a new entry for the task
	mstack->GetLastMessage()->SetTarget( entry );  // make a call to the entry from the last activity on the Mstack
      }
      activity = task->CreateDefaultActivity( edge, "OrJoin" );
      activity->SetConnection( ORJOIN );
    }
    Join2Lqn( edge, next_edges, prev_edges, task, activity, ORJOIN );
    break;

  case SYNCHRONIZATION:
    switch( SyncType( edge ) ) {
    case ANDFORK:
      printf( "AND FORK with %d branches found at hyperedge h%d.", next_edges->Size(), edge->GetNumber() );
      Fork2Lqn( edge, next_edges, task, ANDFORK );
      break;

    case ANDJOIN:
      printf( "AND JOIN with %d branches found at hyperedge h%d.", prev_edges->Size(), edge->GetNumber() );
      if( edge->Visited() ) {  // already visited
	printf( " (already visited)\n" );
	if( task ) {  // in a task
	  task = CheckForLoopBodyTask( task );
	}
	else {  // not in a task
	  task = lqn->AddDefaultTask();  // create a new task for the join
	  entry = task->AddEntry();  // create a new entry for the task
	  mstack->GetLastMessage()->SetTarget( entry );  // make a call to the entry from the last activity on the Mstack
	}
	activity = lqn->GetActivity( edge );
	activity->IncrementEdgeVisits();  // increment the number of edge visits
      }
      else {  // first visit
	printf( " (first visit)\n" );
	edge->SetVisited();  // mark this hyperedge as visited
	if( task ) {  // in a task
	  task = CheckForLoopBodyTask( task );
	}
	else {  // not in a task
	  task = lqn->AddDefaultTask();  // create a new task for the join
	  entry = task->AddEntry();  // create a new entry for the task
	  mstack->GetLastMessage()->SetTarget( entry );  // make a call to the entry from the last activity on the Mstack
	}
	activity = task->CreateDefaultActivity( edge, "AndJoin" );
	activity->SetConnection( ANDJOIN );
      }
      Join2Lqn( edge, next_edges, prev_edges, task, activity, ANDJOIN );
      break;

    case ANDSYNC:
      printf( "SYNCHRONIZATION found at hyperedge h%d [branches in = %d, branches out = %d].", \
	      edge->GetNumber(), prev_edges->Size(), next_edges->Size() );
      if( !edge->Visited() ) {  // first visit
	printf( " (first visit)\n" );
	edge->SetVisited();  // mark this hyperedge as visited
      }
      else {  // already visited
	printf( " (already visited)\n" );
      }
      break;
    }  // end synchronization switch
    break;

  case STUB:
    stub = (Stub*) edge;
//    stub->UpdateBindingsDisplay();  // update the stub's plugin bindings

    if( stub == parent_stub ) {  // same stub as the one being currently traversed
      if( parent_stub->HasMultipleSubmaps() ) {  // dynamic stub
	printf( "Exiting plugin map%d '%s' for dynamic STUB '%s' (h%d).",\
		current_map->GetNumber(), current_map->MapLabel(), parent_stub->StubName(), parent_stub->GetNumber() );
      }
      else {  // static stub
	printf( "Exiting plugin map%d '%s' for static STUB '%s' (h%d).",\
		current_map->GetNumber(), current_map->MapLabel(), parent_stub->StubName(), parent_stub->GetNumber() );
      }
      // stop traversal and collapse outside of the stub
    }
    else {  // a different stub than the one being currently traversed
      printf( "STUB '%s' found at hyperedge h%d.", stub->StubName(), stub->GetNumber() );
      if( !stub->Visited() ) {  // first visit
	printf( " (first visit)\n" );
	stub->SetVisited();  // mark this hyperedge as visited
      }
      else {  // already visited
	printf( " (already visited)\n" );
      }
      if( task ) {  // in a task
	task = CheckForLoopBodyTask( task );
      }
      else {  // not in a task
	task = lqn->AddDefaultTask();  // create a new task for the responsibility
	entry = task->AddEntry();  // create a new entry for the task
	mstack->GetLastMessage()->SetTarget( entry );   // make a call to the entry
      }
      if( stub->HasMultipleSubmaps() ) {  // dynamic stub
	stub_mstack = mstack;  // remember the mstack
	stub_fork_activity = task->AddDefaultActivity( edge, "StubOrFork" );  // OR fork for the plugins
	stub_fork_activity->SetConnection( ORFORK );
	old_stub_join_activity = stub_join_activity;  // remember the old stub join
	stub_join_activity = task->CreateDefaultActivity( edge, "StubOrJoin" );  // OR join for the plugins
	stub_join_activity->SetConnection( ORJOIN );

	plugin_maps = stub->Submaps();
	printf( "STUB '%s' (h%d) is a dynamic stub with %d plugins.\n",\
		stub->StubName(), edge->GetNumber(), plugin_maps->Size() );
	for( plugin_maps->First(); !plugin_maps->IsDone(); plugin_maps->Next() ) {  // go through all the plugins
	  plugin_bind = stub->SubmapBinding( plugin_map );
	  if( plugin_bind ) {
	    selection_prob = plugin_bind->Probability();
	  }
	  else {
	    selection_prob = 1.0;
	  }
	  selection_prob_sum = selection_prob_sum + selection_prob ;  // add the selection probabilities
	}

	last_plugin_map = plugin_maps->GetLast();
	for( plugin_maps->First(); !plugin_maps->IsDone(); plugin_maps->Next() ) {  // go through all the plugins
	  old_plugin_exit = plugin_exit;  // remember the old plugin exit
	  plugin_exit = false;  // a plugin exit has not been reached yet
	  plugin_map = plugin_maps->CurrentItem();
	  if( plugin_map == last_plugin_map ) {  // last plugin
	    mstack = stub_mstack;  // use the original Mstack
	  }
	  else {  // not the last plugin
	    mstack_count++;
	    plugin_mstack = stub_mstack->Copy( mstack_count );  // make a new mstack for the branch
	    mstacks->Add( plugin_mstack );
	    mstack = plugin_mstack;  // use the plugin Mstack
	  }
	  plugin_bind = stub->SubmapBinding( plugin_map );
	  if( plugin_bind ) {
	    selection_prob = plugin_bind->Probability();
	  }
	  else {
	    selection_prob = 1.0;
	  }
	  plugin_start = stub->SubmapPathStart( (Empty*) in_edge, plugin_map );
	  if( plugin_start ) {  // plugin is bound to the stub
	    if( plugin_start->HyperedgeName() == "" ) {  // start point does not have a name
	      plugin_entry_activity = task->CreateDefaultActivity( plugin_start, "PluginEntry" );
	    }
	    else {  // start point has a name
	      plugin_entry_activity = task->CreateDefaultActivity( plugin_start, plugin_start->HyperedgeName() );
	    }
	    plugin_entry_activity->SetBranchProb( selection_prob / selection_prob_sum );
	    task->ConnectActivities( stub_fork_activity, plugin_entry_activity );
	    task->AddActivity( plugin_entry_activity );
	    plugin_entry_activity->SetConnection( NORMAL );

	    old_parent_map = current_map;  // rememember the old parent map
	    current_map = plugin_map;  // this the current plugin map
	    old_parent_stub = parent_stub;  // rememember the old parent stub
	    parent_stub = stub;  // this is the parent stub for its plugins

	    printf( "Traversing into plugin map%d '%s' for dynamic STUB '%s' (h%d) at start point '%s' (h%d).\n",\
		    plugin_map->GetNumber(), plugin_map->MapLabel(), stub->StubName(), edge->GetNumber(),\
		    plugin_start->HyperedgeName(), plugin_start->GetNumber() );
	    TraverseEdge( edge, plugin_start );  // traverse the plugin

	    current_map = old_parent_map;  // restore the old parent map
	    parent_stub = old_parent_stub;  // restore the old parent stub

	    if( plugin_exit ) {  // the plugin did exit
	      plugin_exit_activity = task->AddDefaultActivity( stub, "PluginExit" );  // add a plugin exit
	      task->ConnectActivities( plugin_exit_activity, stub_join_activity );  // bind plugin exit to stub join
	      plugin_exit_activity->SetConnection( JOINBRANCH );
	    }
	    else {
	      printf( "ERROR: plugin map%d '%s' for dynamic STUB '%s' (h%d) starting at start point '%s' (h%d) does not have a bound exit!\n\n",\
		      plugin_map->GetNumber(), plugin_map->MapLabel(), stub->StubName(), edge->GetNumber(),\
		      plugin_start->HyperedgeName(), plugin_start->GetNumber() );
	    }
	  }
	  else {  // plugin is not bound to the stub
	    printf( "WARNING: plugin map%d '%s' is not bound to dynamic STUB '%s' (h%d).\n",\
		    plugin_map->GetNumber(), plugin_map->MapLabel(), stub->StubName(), edge->GetNumber() );
	    plugin_entry_activity = task->CreateDefaultActivity( stub, "UnboundPluginEntry" );
	    plugin_entry_activity->SetBranchProb( selection_prob / selection_prob_sum );
	    task->ConnectActivities( stub_fork_activity, plugin_entry_activity );
	    task->AddActivity( plugin_entry_activity );
	    plugin_entry_activity->SetConnection( NORMAL );

	    activity = task->CreateStubActivity( stub );  // activity for the stub itself
	    task->ConnectActivities( plugin_entry_activity, activity );
	    task->AddActivity( activity );
	    activity->SetConnection( NORMAL );

	    plugin_exit_activity = task->AddDefaultActivity( stub, "UnboundPluginExit" );  // add a plugin exit
	    task->ConnectActivities( plugin_exit_activity, stub_join_activity );  // bind plugin exit to stub join
	    plugin_exit_activity->SetConnection( JOINBRANCH );
	  }
	  if( plugin_map == last_plugin_map ) {  // last plugin
	    task->AddActivity( stub_join_activity );  // add the stub join to the task
	  }
	  else {  // not the last plugin, empty the plugin mstack
	    while( mstack->Size() ) {  // there are messages remaining on the branch stack
	      message = mstack->Pop();
	      activity = message->GetSource();
	      if( activity->GetCallType() == UNRESOLVED ) {
		activity->SetCallType( ASYNC );  // assume that any remaining unresolved messages are asynchronous
	      }
	      delete message;
	    }
	  }
	  plugin_exit = old_plugin_exit;  // restore the old plugin exit
	}  // done all the plugins
	stub_join_activity = old_stub_join_activity;  // restore the old stub join
	// continue past the stub
	for( next_edges->First(); !next_edges->IsDone(); next_edges->Next() ) {   // traverse the next edges
	  next_edge = next_edges->CurrentItem();
	  TraverseEdge( edge, next_edge );  // traverse the current next edge
	}
      }
      else if( stub->HasSubmap() ) {  // static stub
	printf( "STUB '%s' (h%d) is a static stub.\n", stub->StubName(), edge->GetNumber() );
	plugin_maps = stub->Submaps();
	plugin_map = plugin_maps->GetLast();
	plugin_bind = stub->SubmapBinding( plugin_map );
	plugin_start = stub->SubmapPathStart( (Empty*) in_edge );
	if( plugin_start ) {  // plugin is bound to the stub
	  if( plugin_start->HyperedgeName() == "" ) {  // start point does not have a name
	    plugin_entry_activity = task->AddDefaultActivity( plugin_start, "PluginEntry" );
	  }
	  else {  // start point has a name
	    plugin_entry_activity = task->AddDefaultActivity( plugin_start, plugin_start->HyperedgeName() );
	  }
	  old_plugin_exit = plugin_exit;  // remember the old plugin exit
	  plugin_exit = false;  // a plugin exit has not been reached yet
	  old_parent_map = current_map;  // rememember the old parent map
	  current_map = plugin_map;  // this the current plugin map
	  old_parent_stub = parent_stub;  // rememember the old parent stub
	  parent_stub = stub;  // this is the parent stub for its plugins

	  printf( "Traversing into plugin map%d '%s' for static STUB '%s' (h%d) at start point '%s' (h%d).\n",\
		  plugin_map->GetNumber(), plugin_map->MapLabel(), stub->StubName(), edge->GetNumber(),\
		  plugin_start->HyperedgeName(), plugin_start->GetNumber() );
	  TraverseEdge( edge, plugin_start );  // traverse the plugin

	  current_map = old_parent_map;  // restore the old parent map
	  parent_stub = old_parent_stub;  // restore the old parent stub
	  if( !plugin_exit ) {  // the plugin did not exit
	    printf( "ERROR: plugin map%d '%s' for static STUB '%s' (h%d) starting at start point '%s' (h%d) does not have a bound exit!\n\n",\
		    plugin_map->GetNumber(), plugin_map->MapLabel(), stub->StubName(), edge->GetNumber(),\
		    plugin_start->HyperedgeName(), plugin_start->GetNumber() );
	  }
	  plugin_exit = old_plugin_exit;  // restore the old plugin exit
	}
	else {  // plugin is not bound to the stub
	  printf( "WARNING: plugin map%d '%s' is not bound to static STUB '%s' (h%d).\n",\
		  plugin_map->GetNumber(), plugin_map->MapLabel(), stub->StubName(), edge->GetNumber() );
	  activity = task->CreateStubActivity( stub );
	  task->AddActivity( activity );
	  activity->SetConnection( NORMAL );
	}
	// continue past the stub
	for( next_edges->First(); !next_edges->IsDone(); next_edges->Next() ) {   // traverse the next edges
	  next_edge = next_edges->CurrentItem();
	  TraverseEdge( edge, next_edge );  // traverse the current next edge
	}
      }
      else {  // empty stub
	printf( "STUB '%s' (h%d) is empty and will be treated as a responsibility.\n",\
		stub->StubName(), edge->GetNumber() );
	activity = task->CreateStubActivity( stub );
	task->AddActivity( activity );
	activity->SetConnection( NORMAL );
	// continue past the stub
	for( next_edges->First(); !next_edges->IsDone(); next_edges->Next() ) {   // traverse the next edges
	  next_edge = next_edges->CurrentItem();
	  TraverseEdge( edge, next_edge );  // traverse the current next edge
	}
      }
    }
    break;

  case WAIT:
    printf( "WAIT found at hyperedge h%d.\n", edge->GetNumber() );
    edge->SetVisited();  // mark this hyperedge as visited
    if( next_edges->Size() > 1 ) {  // do some error checking
      printf( "ERROR Edge2Lqn: waiting place at hyperedge h%d should not have %d next edges!\n\n", \
	      edge->GetNumber(), next_edges->Size() );
    }
    TraverseEdge( edge, next_edges->GetFirst() );  // traverse the first next edge
    break;

  case TIMER:
    printf( "TIMER found at hyperedge h%d.\n", edge->GetNumber() );
    edge->SetVisited();  // mark this hyperedge as visited
    if( next_edges->Size() > 1 ) {  // do some error checking
      printf( "ERROR Edge2Lqn: timer at hyperedge h%d has %d next edges!\n\n", \
	      edge->GetNumber(), next_edges->Size() );
    }
    TraverseEdge( edge, next_edges->GetFirst() );  // traverse the first next edge
    break;

  case LOOP:
    printf( "LOOP found at hyperedge h%d.", edge->GetNumber() );
    if( edge->Visited() ) {  // already visited
      printf( " (done loop body)\n" );
    }
    else {  // first visit
      printf( " (first visit)\n" );
      edge->SetVisited();  // mark the loop head as visited
      if( next_edges->Size() == 2 ) {  // check that there are exactly 2 next edges
	printf( "Processing LOOP body.\n" );
	if( !task ) {
	  task = lqn->AddDefaultTask();  // create a new task for the loop head
	}
	task = CheckForLoopBodyTask( task );
	loop_activity = task->AddLoopHeadActivity( (Loop*)edge );  // activity for the loop head

	old_lbody_flag = lbody_flag;  // remember the global loop body variables
	lbody_flag = true;  // processing a loop body
	old_lbody_task = lbody_task;
 	lbody_task = lqn->CopyTask( task );  // loop body task
	lbody_entry = lbody_task->AddEntry();  // entry for the loop body
	loop_activity->SetLoopCallTarget( lbody_entry );  // the loop head activity calls the loop body entry

	loop_mstack = mstack;  // remember the current Mstack
	mstack_count++;
	lbody_mstack = new LqnMstack( mstack_count );  // new Mstack for the loop body
	mstacks->Add( lbody_mstack );
	mstack = lbody_mstack;  // use the loop body Mstack

	TraverseEdge( edge, next_edges->GetLast() );  // traverse the loop body

	lbody_activity = lbody_task->AddDefaultActivity( "DoneLoop" );  // reply activity at the end of the loop body
	lbody_activity->SetReplyTarget( lbody_entry );  // the reply activity replies to the loop body entry
	lbody_activity->SetConnection( REPLY );

	if( mstack->Size() ) {  // there are still messages on the loop body Mstack
	  printf( "ERROR Edge2Lqn: LOOP body done with UNRESOLVED messages still on the Mstack!\n\n" );
	  mstack->Print();
	}
	mstack = loop_mstack;  // restore the Mstack for the main path
	lbody_flag = old_lbody_flag;  // restore the global loop body variables
	lbody_task = old_lbody_task;
	printf( "LOOP body processed.\n" );
	TraverseEdge( edge, next_edges->GetFirst() );  // traverse the first next edge
      }
      // else there are not exactly 2 next edges
      else {
	printf( "ERROR Edge2Lqn: Loop head at hyperedge h%d should have exaclty 2, not %d next edges!\n\n",\
		edge->GetNumber(), next_edges->Size() );
      }
    }
    break;

  case RESULT:
    end_pt = (Result*) edge;
    end_pt->SetVisited();  // mark this endpoint as visited

    if( parent_stub ) {  // dealing with a plugin map
      if( end_pt->IsBound() ) {  // the end point is bound to the stub
	next_edge = parent_stub->StubExitPath( end_pt, current_map );
	if( next_edge ) {  
	  printf( "Plugin END '%s' (h%d) exits from stub '%s' (h%d) at hyperedge h%d.\n",\
		  end_pt->HyperedgeName(), end_pt->GetNumber(), parent_stub->StubName(), parent_stub->GetNumber(),\
		  next_edge->GetNumber() );
	  plugin_exit = true;
	  TraverseEdge( end_pt, parent_stub );  // traverse back to the stub
	}
      }
      else {  // the end point is not bound to the stub
	printf( "Path END '%s' (h%d) terminates inside stub '%s' (h%d).\n",\
		end_pt->HyperedgeName(), end_pt->GetNumber(), parent_stub->StubName(), parent_stub->GetNumber() );

	if( task ) {  //contained in a task
	  task = CheckForLoopBodyTask( task );
	}
	else {  // not in a task
	  task = lqn->AddDefaultTask();  // create a new task for the end point
	  entry = task->AddEntry();  // create a new entry for the task
	  mstack->GetLastMessage()->SetTarget( entry );   // make a call to the entry
	}

	if( end_pt->HyperedgeName() == "" ) {  // end point does not have a name
	  end_activity = task->AddDefaultActivity( end_pt, "PluginEnd" ); 
	}
	else {  // end point has a name
	  end_activity = task->AddDefaultActivity( end_pt, end_pt->HyperedgeName() ); 
	}
	end_activity->SetConnection( NOCONNECTION );
	while( mstack->Size() ) {  // go through all the remaining messages on the Mstack
	  message = mstack->Pop();
	  activity = message->GetSource();
	  if( activity->GetCallType() == UNRESOLVED ) {
	    activity->SetCallType( ASYNC );  // assume that any remaining unresolved messages are asynchronous
	  }
	  delete message;
	}
      } 
      printf( "\n" );
    }
    else {  // dealing with the root map
      printf( "Path END '%s' (h%d) reached.\n", end_pt->HyperedgeName(), end_pt->GetNumber() );

      if( task ) {  //contained in a task
	task = CheckForLoopBodyTask( task );
      }
      else {  // not in a task
	task = lqn->AddDefaultTask();  // create a new task for the end point
	entry = task->AddEntry();  // create a new entry for the task
	mstack->GetLastMessage()->SetTarget( entry );   // make a call to the entry
      }

      if( end_pt->HyperedgeName() == "" ) {  // end point does not have a name
	end_activity = task->AddDefaultActivity( end_pt, "End" ); 
      }
      else {  // end point has a name
	end_activity = task->AddDefaultActivity( end_pt, end_pt->HyperedgeName() ); 
      }
      end_activity->SetConnection( NOCONNECTION );
      while( mstack->Size() ) {  // go through all the remaining messages on the Mstack
	message = mstack->Pop();
	activity = message->GetSource();
	if( activity->GetCallType() == UNRESOLVED ) {
	  activity->SetCallType( ASYNC );  // assume that any remaining unresolved messages are asynchronous
	}
	delete message;
      } 
      printf( "\n" );
    }
    break;

  default:
    edge->SetVisited();  // mark this hyperedge as visited
    if( next_edges->Size() > 1 ) {  // do some error checking
      printf( "ERROR Edge2Lqn: hyperedge h%d should only have a single next edge, not %d next edges!\n\n", \
	      edge->GetNumber(), next_edges->Size() );
    }
    TraverseEdge( edge, next_edges->GetFirst() );  // traverse the first next edge
    break;
  }  // end switch

  while( next_edges->Size() ) {
    next_edges->Detach();
  }
  while( prev_edges->Size() ) {
    prev_edges->Detach();
  }
  delete next_edges;
  delete prev_edges;
}


void Ucm2Lqn::Fork2Lqn( Hyperedge* fork_edge, Cltn<Hyperedge*>* next_edges, LqnTask* fork_task,\
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

  if( fork_edge->Visited() ) {  // already visited
    printf( " (already visited)\n" );
  }
  else {  // first visit
    printf( " (first visit)\n" );
    fork_edge->SetVisited();  // mark this hyperedge as visited
    if( fork_task ) {  // in a task
      fork_task = CheckForLoopBodyTask( fork_task );  // check if dealing with a loop body
    }
    else {  // not in a task
      fork_task = lqn->AddDefaultTask();  // create a new task for the fork
      fork_entry = fork_task->AddEntry();  // create a new entry for the task
      mstack->GetLastMessage()->SetTarget( fork_entry );  // make a call to the entry from the last activity on the Mstack
    }
    switch( edge_connection ) {
    case ORFORK:
      or_fork = (OrFork*) fork_edge;
      fork_activity = fork_task->AddDefaultActivity( or_fork, "OrFork" ); 
      for( output_nodes->First(); !output_nodes->IsDone(); output_nodes->Next() ) {  // go through all the output branches
	branch_prob = or_fork->BranchSpecification( output_nodes->CurrentItem() )->Probability();
	branch_prob_sum = branch_prob_sum + branch_prob;  // add the branch probabilities
      }
      break;
    case ANDFORK:
      fork_activity = fork_task->AddDefaultActivity( fork_edge, "AndFork" ); 
      break;
    }
    fork_activity->SetConnection( edge_connection );

    last_branch_edge = next_edges->GetLast();
    for( next_edges->First(), output_nodes->First();  // go through all the branches
         !next_edges->IsDone() && !output_nodes->IsDone();
         next_edges->Next(), output_nodes->Next() ) {  
      branch_edge = next_edges->CurrentItem();  // get the current next edge
      switch( edge_connection ) {
      case ORFORK:
	or_fork = (OrFork*) fork_edge;
	branch_activity = fork_task->CreateDefaultActivity( branch_edge, "OrForkBranch" );
	branch_prob = or_fork->BranchSpecification( output_nodes->CurrentItem() )->Probability();
	branch_activity->SetBranchProb( branch_prob / branch_prob_sum );
	break;
      case ANDFORK:
	branch_activity = fork_task->CreateDefaultActivity( branch_edge, "AndForkBranch" );
	break;
      }
      fork_task->ConnectActivities( fork_activity, branch_activity );
      fork_task->AddActivity( branch_activity );
      branch_activity->SetConnection( NORMAL );

      if( branch_edge == last_branch_edge ) {  // last outgoing branch
	mstack = fork_mstack;  // use the original Mstack
	TraverseEdge( fork_edge, branch_edge );  // traverse the last branch
      }
      else {  // not the last branch
	mstack_count++;
	branch_mstack = mstack->Copy( mstack_count );  // make a new mstack for the branch
	mstacks->Add( branch_mstack );
	mstack = branch_mstack;  // use the branch Mstack
	TraverseEdge( fork_edge, branch_edge );  // traverse the branch
      }
    }
  }  // done all the branches
}


void Ucm2Lqn::Join2Lqn( Hyperedge* join_edge, Cltn<Hyperedge*>* next_edges, Cltn<Hyperedge*>* prev_edges, LqnTask* join_task,\
			LqnActivity* join_activity, connection_type edge_connection )
{
  OrJoin* or_join = NULL;
  Synchronization* and_join = NULL;
  LqnActivity* branch_activity = NULL;
  LqnActivity* activity = NULL;
  LqnMessage* message = NULL;

  switch( edge_connection ) {
  case ORJOIN:
    or_join = (OrJoin*)join_edge;
    branch_activity = join_task->AddDefaultActivity( "OrJoinBranch" );
    join_task->ConnectActivities( branch_activity, join_activity );
    branch_activity->SetConnection( JOINBRANCH );

    if( join_activity->GetEdgeVisits() == or_join->PathCount() ) {  // this is the last incoming branch
      join_task->AddActivity( join_activity );
      if( next_edges->Size() > 1 ) {  // error check
	printf( "ERROR Edge2Lqn: OR_JOIN at h%d should have only one, not %d next edges!\n\n",\
		join_edge->GetNumber(), next_edges->Size() );
      }
      TraverseEdge( join_edge, next_edges->GetFirst() );  // traverse the next edge
    } 
    else {  // this is not the last incoming branch
      while( mstack->Size() ) {  // there are messages remaining on the branch stack
	message = mstack->Pop();
	activity = message->GetSource();
	if( activity->GetCallType() == UNRESOLVED ) {
	  activity->SetCallType( ASYNC );  // assume that any remaining unresolved messages are asynchronous
	}
	delete message;
      }
      // do not traverse any further past the join, collapse the traversal back to the fork
    }
    break;
  case ANDJOIN:
    and_join = (Synchronization*)join_edge;
    branch_activity = join_task->AddDefaultActivity( "AndJoinBranch" );
    join_task->ConnectActivities( branch_activity, join_activity );
    branch_activity->SetConnection( JOINBRANCH );

    if( join_activity->GetEdgeVisits() == and_join->InputCount() ) {  // this is the last incoming branch
      join_task->AddActivity( join_activity );
      if( next_edges->Size() > 1 ) {  // error check
	printf( "Edge2Lqn: AND_JOIN at h%d is a synchronization with %d next edges.\n",\
		join_edge->GetNumber(), next_edges->Size() );
      }
      TraverseEdge( join_edge, next_edges->GetFirst() );  // traverse the next edge
    } 
    else {  // this is not the last incoming branch
      while( mstack->Size() ) {  // there are messages remaining on the branch stack
	message = mstack->Pop();
	activity = message->GetSource();
	if( activity->GetCallType() == UNRESOLVED ) {
	  activity->SetCallType( ASYNC );  // assume that any remaining unresolved messages are asynchronous
	}
	delete message;
      }
      // do not traverse any further past the join, collapse the traversal back to the fork
    }
    break;
  }
}


void Ucm2Lqn::TraverseEdge( Hyperedge* edge, Hyperedge* next_edge ) 
{
  if( next_edge->Visited() ) {  // the next edge has been visited
    if( ( next_edge->EdgeType() == LOOP ) || ( next_edge->EdgeType() == STUB ) || ( next_edge->EdgeType() == OR_JOIN ) || \
	( ( next_edge->EdgeType() == SYNCHRONIZATION ) && ( SyncType( next_edge ) == ANDJOIN ) ) ) {
      // make an exception for loops, stubs, OR joins, and AND joins
      ComponentBoundaryXing2Lqn( edge, next_edge );
      Edge2Lqn( next_edge, edge );  // transform the next edge
    }
    else {
      // for any other kind of hyperedge
      printf( "Warning TraverseEdge: next edge at h%d has already been visited.\n", next_edge->GetNumber() );
      next_edge->GetFigure()->SetSelected();
    }
  }
  else {  // the next edge has not been visited
    ComponentBoundaryXing2Lqn( edge, next_edge );
    Edge2Lqn( next_edge, edge );  // transform the next edge
  }
}


void Ucm2Lqn::ComponentBoundaryXing2Lqn( Hyperedge* edge, Hyperedge* next_edge )
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


void Ucm2Lqn::LeaveComponent2Lqn( Hyperedge* edge )
{
  ComponentReference* comp_ref = NULL;
  LqnTask* task = NULL;
  LqnActivity* activity = NULL;

  // get the component reference for the edge, if any
  comp_ref = edge->GetFigure()->GetContainingComponent();
  if( comp_ref ) {
    // get the task corresponding to the component reference, if any
    task = lqn->GetTaskForComp( comp_ref );
    if( task ) {
      task = CheckForLoopBodyTask( task );
      printf( "Leaving task '%s'.\n", task->GetName() );
      // create a new activity to make a call or a reply
      activity = task->AddDefaultActivity( "SendMsg" );  // create a new default activity for the task
      activity->SetConnection( NORMAL );
      mstack->Push( new LqnMessage( activity ) );  // push the activity on the Mstack
    }
    else {
      printf( "ERROR LeaveComponent2Lqn: no task was found for component reference '%s' (id %d)!\n\n",\
	      comp_ref->ReferenceName(), comp_ref->ComponentReferenceNumber() );
    }
  }
  else {
    printf( "ERROR LeaveComponent2Lqn: no component reference was found for hyperedge h%d!\n\n",\
	    edge->GetNumber() );
  }
}


void Ucm2Lqn::EnterComponent2Lqn( Hyperedge* next_edge )
{
  ComponentReference* next_comp_ref = NULL;
  LqnTask* next_path_task = NULL;
  LqnTask* last_stack_task = NULL;
  LqnTask* prev_stack_task = NULL;
  LqnMessage* reply_msg = NULL;
  LqnMessage* call_msg = NULL;
  LqnActivity* reply_activity = NULL;
  LqnActivity* handle_reply_activity = NULL;
  LqnActivity* calling_activity = NULL;
  LqnActivity* fwding_activity = NULL;
  LqnEntry* called_entry = NULL;
  LqnEntry* fwding_entry = NULL;
  LqnEntry* fwd_to_entry = NULL;

  next_comp_ref = next_edge->GetFigure()->GetContainingComponent();  // get the comp ref for the next edge along the path
  if( next_comp_ref ) {  // there is a component reference for the next edge along the path
    next_path_task = lqn->GetTaskForComp( next_comp_ref );  // get the task for to the next comp ref along the path
    if( next_path_task ) {  // there is a task for the next component ref along the path
      printf( "Entering task '%s'.\n", next_path_task->GetName() );
      next_path_task = CheckForLoopBodyTask( next_path_task );
      if( mstack->FindTask( next_path_task ) ) {  // the next task along the path is already on the Mstack
	last_stack_task = mstack->GetLastTask();  // get the last task on the Mstack
	prev_stack_task = mstack->GetTaskPreviousTo( last_stack_task );  // task previous to the last Mstack task
	if( next_path_task == prev_stack_task ) {  // the next task along the path calls the last Mstack task
	  // process a synchronous reply
	  printf( "SYNC interaction detected.\n" );
	  mstack->Print();
          // process reply message
	  reply_msg = mstack->Pop( REPLY_MSG );  // message for the reply
	  if( reply_msg->GetTarget() ) {  // error check
	    printf( "ERROR EnterComponent2Lqn SYNC interaction: reply message should not have a target entry!\n\n" );
	  }
	  reply_activity = reply_msg->GetSource();
	  if( !reply_activity ) {  // error check
	    printf( "ERROR EnterComponent2Lqn SYNC interaction: reply message does not have a source activity!\n\n" );
	  }
	  delete reply_msg;  // done with the reply message
	  // process synchronous call message
	  call_msg = mstack->Pop( SYNC_MSG );  // message for the SYNC call
	  called_entry = call_msg->GetTarget();  // target entry for the SYNC call
	  reply_activity->SetReplyTarget( called_entry );  // the reply activity replies to the called entry
	  calling_activity = call_msg->GetSource();  // source activity of the SYNC call
	  delete call_msg;  // done with the call message
	  handle_reply_activity = next_path_task->CreateDefaultActivity( "ReceiveReply" );  // activity to handle the reply
	  next_path_task->ConnectActivities( calling_activity, handle_reply_activity );
	  next_path_task->AddActivity( handle_reply_activity );
	  handle_reply_activity->SetConnection( NORMAL );
	  printf( "SYNC interaction processed.\n" );
	  mstack->Print();
	}
	else {  // next_path_task != prev_stack_task
	  // process a forwarded reply
	  printf( "FWD interaction detected.\n" );
	  mstack->Print();
          // process final reply message in the forwarding chain
	  reply_msg = mstack->Pop( REPLY_MSG );  // message for the final reply 
	  if( reply_msg->GetTarget() ) {  // error check
	    printf( "ERROR EnterComponent2Lqn FWD interaction: reply message should not have a target entry!\n\n" );
	  }
	  reply_activity = reply_msg->GetSource();
	  if( !reply_activity ) {  // error check
	    printf( "ERROR EnterComponent2Lqn FWD interaction: reply message does not have a source activity!\n\n" );
	  }
	  delete reply_msg;  // done with the reply message
	  fwd_to_entry = mstack->PopEntry();  // the last forwarded to entry
	  reply_activity->SetReplyTarget( fwd_to_entry );  // the reply activity replies to the last forwarded to entry
	  // process forwarding chain
	  last_stack_task = mstack->GetLastTask();  // get the last task on the Mstack
	  prev_stack_task = mstack->GetTaskPreviousTo( last_stack_task );  // task previous to the last Mstack task
	  while( next_path_task != prev_stack_task ) {
	    fwding_activity = mstack->PopActivity();  // forwarding activity
	    fwding_entry = mstack->PopEntry();  // forwarding entry 
	    fwding_activity->SetReplyTarget( fwding_entry );  // forwarding activity replies to forwarding entry 
	    fwding_entry->SetFwdTarget( fwd_to_entry );  // forwarding entry forwards to the forwarded to entry
	  
	    last_stack_task = mstack->GetLastTask();  // get the last task on the Mstack
	    prev_stack_task = mstack->GetTaskPreviousTo( last_stack_task );  // task previous to the last Mstack task
	    fwd_to_entry = fwding_entry;  // the last forwarding entry is now the forwarded to entry
	  }  
          // now next_path_task == prev_stack_task
	  fwding_activity = mstack->PopActivity();  // first forwarding activity
	  // process original synchronous call message
	  call_msg = mstack->Pop( SYNC_MSG );  // message for the original SYNC call
	  called_entry = call_msg->GetTarget();  // target entry for the original SYNC call and first forwarding entry
	  called_entry->SetFwdTarget( fwd_to_entry );  // called entry forwards to the forwarded to entry
	  fwding_activity->SetReplyTarget( called_entry );  // first forwarding activity replies to the called entry
	  calling_activity = call_msg->GetSource();  // source activity of the original SYNC call
	  delete call_msg;  // done with the original call message
	  handle_reply_activity = next_path_task->CreateDefaultActivity( "ReceiveReply" );  // activity to handle the reply
	  next_path_task->ConnectActivities( calling_activity, handle_reply_activity );
	  next_path_task->AddActivity( handle_reply_activity );
	  handle_reply_activity->SetConnection( NORMAL );
	  printf( "FWD interaction processed.\n" );
	  mstack->Print();
	}
      }
      else {  // the next task is not on the Mstack
	// process a call
	printf( "Undetermined call detected from task '%s' to task '%s'.\n",\
		mstack->GetLastTask()->GetName(), next_path_task->GetName() );
	mstack->Print();
	called_entry = next_path_task->AddEntry();  // new entry on the next task along the path
	mstack->GetLastMessage()->SetTarget( called_entry );  // called entry is the target of the last message on the Mstack
      }
    }
    else {  // there is no next task
      printf( "ERROR EnterComponent2Lqn: no task was found for component reference '%s' (id %d)!\n\n",\
	      next_comp_ref->ReferenceName(), next_comp_ref->ComponentReferenceNumber() );
    }
  }
  else {  // there is no next component reference
    printf( "ERROR EnterComponent2Lqn: no component reference was found for hyperedge (h%d)!\n\n",\
	    next_edge->GetNumber() );
  }
}


xing_type Ucm2Lqn::FindXing( Hyperedge* edge1, Hyperedge* edge2 )
{
  xing_type xing;
  LqnTask* task1 = lqn->GetTaskForComp( edge1->GetFigure()->GetContainingComponent() );  // get tasks, if any
  LqnTask* task2 = lqn->GetTaskForComp( edge2->GetFigure()->GetContainingComponent() );

  if( !task1 && !task2 ) {  // neither edge1 nor edge2 are in a task
    xing = NO_COMP;
    if( print) {
      printf( "\tFindXing: from edge h%d to edge h%d - NO component\n", edge1->GetNumber(), edge2->GetNumber() );
    }
  }
  else if( task1 && !task2 ) {  // edge1 is in a task but edge2 is not
    xing = LEAVE;
    if( print) {
      printf( "\tFindXing: from edge h%d to edge h%d - LEAVE component\n", edge1->GetNumber(), edge2->GetNumber() );
    }
  }
  else if( !task1 && task2 ) {  // edge1 is not in a task but edge2 is
    xing = ENTER;
    if( print) {
      printf( "\tFindXing: from edge h%d to edge h%d - ENTER component\n", edge1->GetNumber(), edge2->GetNumber() );
    }
  }
  else {  // edge1 and edge2 are both in a task
    if( task1 == task2 ) {  // edge1 and edge2 are both in the same task
      xing = STAY;
      if( print) {
	printf( "\tFindXing: from edge h%d to edge h%d - STAY in component\n", edge1->GetNumber(), edge2->GetNumber() );
      }
    }
    else {  // edge1 and edge2 are in different tasks
      xing = CHANGE;
      if( print) {
	printf( "FindXing: from edge h%d to edge h%d - CHANGE components\n", edge1->GetNumber(), edge2->GetNumber() );
      }
    }
  }
  return xing;
}


// changes the task to the loop body task when processing a loop body
LqnTask* Ucm2Lqn::CheckForLoopBodyTask( LqnTask* task )
{
  LqnTask* check_task = task;

  if( lbody_flag ) {  // currently processing a loop body
    printf( "Processing loop body, " );
    if( check_task == lbody_task->GetOriginal() ) {  // the given task is the original task for the loop body task
      printf( "using loop body task '%s' for task '%s'.\n", lbody_task->GetName(), check_task->GetName() );
      check_task = lbody_task;  // use the loop body task
    }
    else {
      printf( "no loop body task '%s' for task '%s'.\n", lbody_task->GetName(), check_task->GetName() );
    }
  }
  return check_task;
}


// returns the type of synchronization encountered
connection_type Ucm2Lqn::SyncType( Hyperedge* edge )
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
  while( next_edges->Size() ) {
    next_edges->Detach();
  }
  while( prev_edges->Size() ) {
    prev_edges->Detach();
  }
  delete next_edges;
  delete prev_edges;

  return sync_type;
}


void Ucm2Lqn::NextEdges( Hyperedge* edge, Cltn<Hyperedge*>* next )
{
  Cltn<Node*>* nodes = edge->TargetSet();
  for( nodes->First(); !nodes->IsDone(); nodes->Next() ) {
    next->Add( nodes->CurrentItem()->NextEdge() );
  }
}


void Ucm2Lqn::PrevEdges( Hyperedge* edge, Cltn<Hyperedge*>* previous )
{
  Cltn<Node*>* nodes = edge->SourceSet();
  for( nodes->First(); !nodes->IsDone(); nodes->Next() ) {
    previous->Add( nodes->CurrentItem()->PreviousEdge() );
  }
}


void Ucm2Lqn::TestMstack()
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
