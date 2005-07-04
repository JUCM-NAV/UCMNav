#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lqntask.h"
#include "lqn.h"
#include "lqnactivity.h"
#include "lqnentry.h"
#include "lqndevice.h"


// constructor
LqnTask::LqnTask( const char* name, LqnDevice* proc )
{
  my_entries = new Cltn<LqnEntry*>;
  my_activities = new Cltn<LqnActivity*>;
  my_unconnected_activities = new Cltn<LqnActivity*>;

  my_name[0] = 0;  // initialize the name
  my_name[TASK_NAME_LENGTH - 1] = 0;
  strcpy( my_name, name );
  my_proc = proc;

  msg_candidate = NULL;
  connect_candidate = NULL;

  reference_task = false;  // assume this is not a reference task
  am_copy = false;
  my_original = NULL;
  my_entry_count = 0;
  my_ref_entry_count = 0;
  my_def_activities = 0;
  my_copy_count = 0;

  print = true;
  if( print ) {
    printf( "Task '%s' running on processor '%s' (id %d) created.\n", \
	    my_name, my_proc->GetName(), my_proc->GetId() );
  }
}


// destructor
LqnTask::~LqnTask()
{
  printf( "Deleting task '%s' with %d entries", my_name, my_entries->Size() );
  while( my_entries->Size() ) {
    delete my_entries->Detach();
  }
  delete my_entries;

  printf( "...%d activities", my_activities->Size() );
  while( my_activities->Size() ) {
    delete my_activities->Detach();
  }
  delete my_activities;

  printf( "...and %d unconnected activities", my_unconnected_activities->Size() );
  while( my_unconnected_activities->Size() ) {
    delete my_unconnected_activities->Detach();
  }
  delete my_unconnected_activities;

  printf( "...done!\n" );
}


// creates a new task with the same characteristics
LqnTask* LqnTask::Copy()
{
  char copy_name[TASK_NAME_LENGTH];
  copy_name[0] = 0;  // initialize the name
  copy_name[TASK_NAME_LENGTH - 1] = 0;
  char copy_number[NUMBER_LENGTH];
  copy_number[0] = 0;  // initialize the number
  copy_number[NUMBER_LENGTH - 1] = 0;
  LqnTask* copy_task = NULL;

  my_copy_count++;
  strcpy( copy_name, my_name );  // name the copy after this task
  strcat( copy_name, "_Loop" );  // identify that this a copy for a loop body
  sprintf( copy_number, "%d", my_copy_count );
  strcat( copy_name, copy_number );  // add a number for the copy

  copy_task = new LqnTask( copy_name, my_proc );
  copy_task->SetOriginal( this );

  return copy_task;
}


// adds an entry to the task
// returns a pointer to the entry
LqnEntry* LqnTask::AddEntry()
{
  char name[ENTRY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ENTRY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;
  LqnEntry* entry = NULL;
  // LqnActivity* first_activity = NULL; // DA: August 2004

  my_entry_count++;
  strcpy( name, my_name );  // get the task name
  strcat( name, "_E" );  // identify that this an entry
  sprintf( number, "%d", my_entry_count );
  strcat( name, number );  // add a number for the entry

  // create a new entry for the task, don't print it
  entry = new LqnEntry( name, this, false );
  my_entries->Add( entry );

  return entry;
}


// adds an entry to the task
// returns a pointer to the entry
LqnEntry* LqnTask::AddRefEntry()
{
  char name[ENTRY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ENTRY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;
  LqnEntry* ref_entry = NULL;
  // LqnActivity* first_activity = NULL;  // DA: August 2004

  my_ref_entry_count++;
  strcpy( name, my_name );  // get the task name
  strcat( name, "_RefE" );  // identify that this a reference entry
  if( my_ref_entry_count > 1 ) {
    sprintf( number, "%d", my_ref_entry_count );
    strcat( name, number );  // add a number for the entry
  }
  // create a new entry for the task, don't print it
  ref_entry = new LqnEntry( name, this, false );
  my_entries->Add( ref_entry );

  return ref_entry;
}


// create a valid activity name for the given edge name
void LqnTask::CreateActivityName( const char* edge_name, char* activity_name )
{
  strcpy( activity_name, edge_name );

  // replace spaces in the name with underscores
  for( size_t i = 0; i < strlen( activity_name ) ; i++ ) {
    if( activity_name[i] == ' ' ) {
      activity_name[i] = '_';
    }
  }
}


// creates a default activity for the task
LqnActivity* LqnTask::CreateDefaultActivity()
{
  LqnActivity* default_activity = NULL;
  char name[ACTIVITY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ACTIVITY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  my_def_activities++;  // increment the number of default activities
  strcpy( name, my_name );  // name the activity after the task
  strcat( name, "_A" );
  sprintf( number, "%d", my_def_activities );
  strcat( name, number );  // add a number for the activity

  // create a new activity, do not print it
  default_activity = new LqnActivity( name, this, false );
  my_unconnected_activities->Add( default_activity );

  return default_activity;
}


// creates a default activity for the given hyperedge
LqnActivity* LqnTask::CreateDefaultActivity( Hyperedge* edge )
{
  LqnActivity* default_activity = NULL;
  char name[ACTIVITY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ACTIVITY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  my_def_activities++;  // increment the number of default activities
  strcpy( name, my_name );  // name the activity after the task
  strcat( name, "_A" );
  sprintf( number, "%d", my_def_activities );
  strcat( name, number );  // add a number for the activity
  // strcat( name, "_h");
  // sprintf( number, "%d", edge->GetNumber() );
  // strcat( name, number );  // add a hyperedge number for the activity

  // create a new activity, do not print it
  default_activity = new LqnActivity( name, this, false );
  default_activity->SetEdge( edge );
  my_unconnected_activities->Add( default_activity );

  return default_activity;
}


// creates a default activity with the given name
LqnActivity* LqnTask::CreateDefaultActivity( const char* include_name )
{
  LqnActivity* default_activity = NULL;
  char name[ACTIVITY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ACTIVITY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  my_def_activities++;  // increment the number of default activities
  strcpy( name, my_name );  // name the activity after the task
  strcat( name, "_A" );
  sprintf( number, "%d", my_def_activities );
  strcat( name, number );  // add a number for the activity
  strcat( name, "_" );
  strcat( name, include_name );  // add the specified name

  // create a new activity, do not print it
  default_activity = new LqnActivity( name, this, false );
  my_unconnected_activities->Add( default_activity );

  return default_activity;
}


// creates a default activity for the given hyperedge with the given name
LqnActivity* LqnTask::CreateDefaultActivity( Hyperedge* edge,
					     const char* include_name )
{
  LqnActivity* default_activity = NULL;
  char name[ACTIVITY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ACTIVITY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  my_def_activities++;  // increment the number of default activities
  strcpy( name, my_name );  // name the activity after the task
  strcat( name, "_A" );
  sprintf( number, "%d", my_def_activities );
  strcat( name, number );  // add a number for the activity
  strcat( name, "_" );
  strcat( name, include_name );  // add the specified name
  // strcat( name, "_h");
  // sprintf( number, "%d", edge->GetNumber() );
  // strcat( name, number );  // add a hyperedge number for the activity

  // create a new activity, do not print it
  default_activity = new LqnActivity( name, this, false );
  default_activity->SetEdge( edge );
  my_unconnected_activities->Add( default_activity );

  return default_activity;
}
  

// add the given activity to the task
void LqnTask::AddActivity( LqnActivity* activity )
{
  LqnEntry* entry = NULL;

  // if there already are activities in the task
  if( my_activities->Size() ) {
    // if the activity does not have any predecessors
    if( !activity->PredecessorCount() ) {
      // connect the activity to the last activity in the task
      ConnectActivities( my_activities->GetLast(), activity );
    }
    // else the activity already has predecessors
    else {
      printf( "(LqnTask::AddActivity: cannot connect activity '%s', ", 
	      activity->GetName() );
      printf( "it already has %d predecessors.)\n",\
	      activity->GetPredecessors()->Size() );
    }
    // add the activity to the list of connected activities
    my_activities->Add( activity );

    // if there are unconnected activities in the task
    if( my_unconnected_activities->Size() ) {
      // go through all the unconnected activities
      for( my_unconnected_activities->First(); !my_unconnected_activities->IsDone(); my_unconnected_activities->Next() ) {
	if( activity == my_unconnected_activities->CurrentItem() ) {
	  // remove the activity from the list of unconnected activities
	  my_unconnected_activities->RemoveCurrentItem();
	}
      }
    }
  }
  // else there are no other activities in the task
  else {
    // if the task has entries
    if( my_entries->Size() ) {
      // this is the first activity for the last entry added to the task
      entry = my_entries->GetLast();
      entry->SetFirstActivity( activity );
      activity->SetEntry( entry );
      activity->SetFirstActivity();
      // add the activity to the list of connected activities
      my_activities->Add( activity );

      // if there are unconnected activities in the task
      if( my_unconnected_activities->Size() ) {
	// go through all the unconnected activities
	for( my_unconnected_activities->First(); !my_unconnected_activities->IsDone(); my_unconnected_activities->Next() ) {
	  if( activity == my_unconnected_activities->CurrentItem() ) {
	    // remove the activity from the list of unconnected activities
	    my_unconnected_activities->RemoveCurrentItem();
	  }
	}
      }
    }
    // else the task has no entries
    else {
      printf( "ERROR LqnTask::AddActivity: cannot add activity '%s' ", 
	      activity->GetName() );
      printf( "to task '%s' with no entries.\n", my_name );
    }
  }
}


// connects the first activity to the second activity
void LqnTask::ConnectActivities( LqnActivity* first, LqnActivity* second )
{
  first->AddSuccessor( second );
  second->AddPredecessor( first );    
}


// adds a default activity to the task
LqnActivity* LqnTask::AddDefaultActivity()
{
  LqnActivity* default_activity = CreateDefaultActivity();
  AddActivity( default_activity );
  return default_activity;
}


// adds a default activity to the task for the given hyperedge
LqnActivity* LqnTask::AddDefaultActivity( Hyperedge* edge )
{
  LqnActivity* default_activity = CreateDefaultActivity( edge );
  AddActivity( default_activity );
  return default_activity;
}


// adds a default activity to the task with the given name
LqnActivity* LqnTask::AddDefaultActivity( const char* include_name )
{
  LqnActivity* default_activity = CreateDefaultActivity( include_name );
  AddActivity( default_activity );
  return default_activity;
}

// adds activity to the task for the given hyperedge with the given name
LqnActivity* LqnTask::AddDefaultActivity( Hyperedge* edge,
					  const char* include_name )
{
  LqnActivity* default_activity = CreateDefaultActivity( edge, include_name );
  AddActivity( default_activity );
  return default_activity;
}
  

// adds an activity for the responsibility to the task
// returns a pointer to the activity
LqnActivity* LqnTask::AddResponsibilityActivity( ResponsibilityReference* resp_ref )
{
  Cltn<ServiceRequest*>* service_reqs = NULL;
  LqnActivity* resp_activity = NULL;
  int edge_id = resp_ref->GetNumber();
  double service_time;
  char name[ACTIVITY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ACTIVITY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  // if there are activities in the task
  if( my_activities->Size() ) {
    // go through all the existing activities
    for( my_activities->First(); !my_activities->IsDone(); my_activities->Next() ) {
      // if an activity already exists for this responsibility
      if( my_activities->CurrentItem()->GetEdge() == resp_ref ) {
	resp_activity = my_activities->CurrentItem();
	printf( "(LqnTask::AddResponsibilityActivity: activity '%s' already exists in task '%s'.)\n",\
		resp_activity->GetName(), edge_id, my_name );
      }
    }
  }

  // if there is no existing activity corresponding to this responsbility
  if ( !resp_activity ) {
    CreateActivityName( resp_ref->ParentResponsibility()->Name(), name );
    strcat( name, "_h" );
    sprintf( number, "%d", edge_id );
    strcat( name, number );

    // get the service requests
    service_reqs = resp_ref->ParentResponsibility()->ServiceRequests();

    // if there are service requests
    if( service_reqs->Size() ) {
      service_time = atof( resp_ref->ParentResponsibility()->ServiceRequests()->GetFirst()->Amount() );

      // if the service time is specified correctly
      if( service_time > 0 ) {
	// create a new activity, print it
	resp_activity = new LqnActivity( name, this, resp_ref, service_time,\
					 true );
	my_unconnected_activities->Add( resp_activity );
      }
      // else the service time specified incorrectly
      else {
	printf( "(LqnTask::AddResponsibilityActivity: activity '%s' service requests ", name );
	printf( "are incorrectly specified as %lf, using 1.0 instead.)\n",\
		service_time );
	// create a new activity, print it
	resp_activity = new LqnActivity( name, this, resp_ref, 1.0, true );
	my_unconnected_activities->Add( resp_activity );
      }
    }
    // else there are no service requests
    else {
      printf( "(LqnTask::AddResponsibilityActivity: activity '%s' ", name );
      printf( "has no service requests specified, using 1.0.)\n");
      // create a new activity, print it
      resp_activity = new LqnActivity( name, this, resp_ref, 1.0, true );
      my_unconnected_activities->Add( resp_activity );
    }
    AddActivity( resp_activity );
  }

  return resp_activity;
}


// creates an activity for the stub to the task
// returns a pointer to the activity
LqnActivity* LqnTask::CreateStubActivity( Stub* stub )
{
  Cltn<ServiceRequest*>* service_reqs = NULL;
  LqnActivity* stub_activity = NULL;
  int edge_id = stub->GetNumber();
  double service_time;
  char name[ACTIVITY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ACTIVITY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  CreateActivityName( stub->StubName(), name );
  strcat( name, "_Stub" );
  sprintf( number, "%d", edge_id );
  strcat( name, number );

  // if there are activities in the task
  if( my_activities->Size() ) {
    // go through all the existing activities
    for( my_activities->First(); !my_activities->IsDone(); my_activities->Next() ) {
      // if an activity already exists for this stub
      if( name == my_activities->CurrentItem()->GetName() ) {
	stub_activity = my_activities->CurrentItem();
	printf( "(LqnTask::CreateStubActivity: stub activity '%s' ", name );
	printf( "already exists in task '%s'.)\n", my_name );
      }
    }
  }

  // if there is no existing activity corresponding to this stub
  if ( !stub_activity ) {
    service_reqs = stub->ServiceRequests();  // get the service requests

    // if there are service requests
    if( service_reqs->Size() ) {
      service_time = atof( stub->ServiceRequests()->GetFirst()->Amount() );

      // if the service time specified correctly
      if( service_time > 0 ) {
	// create a new activity, print it
	stub_activity = new LqnActivity( name, this, stub, service_time,\
					 true );
	my_unconnected_activities->Add( stub_activity );
      }
      // else the service time specified incorrectly
      else {
	printf( "(LqnTask::CreateStubActivity: stub '%s' service requests ", name );
	printf( "are incorrectly specified as %lf, using 1.0 instead.)\n",\
		service_time );
	// create a new activity, print it
	stub_activity = new LqnActivity( name, this, stub, 1.0, true );
	my_unconnected_activities->Add( stub_activity );
      }
    }
    // else there are no service requests
    else {
      printf( "(LqnTask::CreateStubActivity: stub '%s' ", name );
      printf( "has no service requests specified, using 1.0.)\n");
      // create a new activity, print it
      stub_activity = new LqnActivity( name, this, stub, 1.0, true );
      my_unconnected_activities->Add( stub_activity );
    }
  }

  return stub_activity;
}


// adds an activity to handle the loop head
// returns a pointer to the activity
LqnActivity* LqnTask::AddLoopHeadActivity( Loop* loop )
{
  // LqnActivity* last_activity = NULL; // DA: August 2004
  LqnActivity* loop_activity = NULL;
  int edge_id = loop->GetNumber();
  double loop_count = 1.0;
  char name[ACTIVITY_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[ACTIVITY_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  // if there are activities in the task
  if( my_activities->Size() ) {
    // go through all the existing activities
    for( my_activities->First(); !my_activities->IsDone(); my_activities->Next() ) {
      // if an activity already exists for this loop head
      if( loop == my_activities->CurrentItem()->GetEdge() ) {
	loop_activity = my_activities->CurrentItem();
	printf( "(LqnTask::AddLoopHeadActivity: loop '%s' already exists in task '%s'.)\n",\
		loop_activity->GetName(), my_name );
      }
    }
  }

  // if no activity exists for this loop head
  if( !loop_activity ) {
    // if the loop has a name
    if( loop->HasName() ) {
      CreateActivityName( loop->HyperedgeName(), name );
    }
    // else the loop does not have a name
    else {
      strcpy( name, "LoopHead" );  // identify that this a loophead
    }

    strcat( name, "_h" );
    sprintf( number, "%d", edge_id );
    strcat( name, number );
    // create a new activity, print it
    loop_activity = new LqnActivity( name, this, loop, true );
    loop_activity->SetStochastic();

    // if the loop has a count specified
    if( loop->HasCount() ) {
      loop_count = atof( loop->LoopCount() );

      // if the loop count is specified correctly
      if( loop_count > 0 ) {
	loop_activity->SetLoopCall( loop_count );
      }
      // else the loop count is not specified correctly
      else {
	printf( "(LqnTask::AddLoopHeadActivity: loop count of %lf is incorrect, using 1.0.)\n",\
		loop_count );
	loop_activity->SetLoopCall( 1.0 );
      }
    }
    AddActivity( loop_activity );
  }

  return loop_activity;
}


// finds the activity corresponding to the given hyperedge
LqnActivity* LqnTask::GetActivity( Hyperedge* edge )
{
  // if there are activities in the task
  if( my_activities->Size() ) {
    // go through all the activities
    for( my_activities->First(); !my_activities->IsDone(); my_activities->Next() ) {
      if( edge == my_activities->CurrentItem()->GetEdge() ) {
	return my_activities->CurrentItem();
      }
    }
  }
  // if there are unconnected activities in the task
  if( my_unconnected_activities->Size() ) {
    // go through all the unconnected activities
    for( my_unconnected_activities->First(); !my_unconnected_activities->IsDone(); my_unconnected_activities->Next() ) {
      if( edge == my_unconnected_activities->CurrentItem()->GetEdge() ) {
	return my_unconnected_activities->CurrentItem();
      }
    }
  }
  // if we get here, the activity was not found 
  return NULL;
}


// gets the last activity added to the task
LqnActivity* LqnTask::GetLastActivity()
{
  LqnActivity* last_activity = NULL;

  // if there are activities in the task
  if( my_activities->Size() ) {
    last_activity = my_activities->GetLast();
  }

  return last_activity;
}


// make the activity the messaging candidate for the task
void LqnTask::SetMsgCandidate( LqnActivity* activity )
{
  msg_candidate = activity;
}


// make the activity the connection candidate for the task
void LqnTask::SetConnectCandidate( LqnActivity* activity )
{
  connect_candidate = activity;
}


// return the messaging candidate for the task
LqnActivity* LqnTask::GetMsgCandidate()
{
  LqnActivity* candidate_activity = msg_candidate;

  msg_candidate = NULL;  // can only use a candidate once

  return candidate_activity;
}


// return the connection candidate for the task
LqnActivity* LqnTask::GetConnectCandidate()
{
  LqnActivity* candidate_activity = connect_candidate;

  connect_candidate = NULL;  // can only use a candidate once

  return candidate_activity;
}


// prints the task's information to the specified file
void LqnTask::FilePrint( FILE* outfile )
{
  // if the task has entries
  if( my_entries->Size() ) {
    // if it is a reference task
    if( reference_task ) {
      fprintf( outfile, "t %s r ", my_name );
    } 
    // else it is a FIFO task
    else {
      fprintf( outfile, "t %s f ", my_name );
    }

    // print the entry list
    for( my_entries->First(); !my_entries->IsDone(); my_entries->Next() ) {
      fprintf( outfile, "%s ", my_entries->CurrentItem()->GetName() );
    }    
    fprintf( outfile, "-1  " );

    // print the processor the task is running on
    fprintf( outfile, "%s\n", my_proc->GetName() );
  }
  // else the task has no entries
  else {
    fprintf( outfile, "# Warning: %s  has no entries.\n",\
	     my_name, my_proc->GetName(), my_proc->GetId() );
  }

  // update the names of all the activities
  for( my_activities->First(); !my_activities->IsDone(); my_activities->Next() ) {
    my_activities->CurrentItem()->UpdateName();
  }
}


// prints the task's entry information to the specified file
void LqnTask::FilePrintEntries( FILE* outfile )
{
  // go through all the entries
  for( my_entries->First(); !my_entries->IsDone(); my_entries->Next() ) {
    my_entries->CurrentItem()->FilePrint( outfile );  // output entry info
  }
}


// prints the task's activity information list to the specified file
void LqnTask::FilePrintActivities( FILE* outfile )
{
  LqnActivity* activity = NULL;
  bool previously_printed_something = false;

  if( print ) {
    printf( "Task '%s' with %d activities.\n",\
	    my_name, my_activities->Size() );
  }

  // if the task has activities
  if( my_activities->Size() ) {
    // output the activity definition list
    fprintf( outfile, "A %s\n", my_name );

    // go through all the activities
    for( my_activities->First(); !my_activities->IsDone(); my_activities->Next() ) {
      // output activity info
      my_activities->CurrentItem()->FilePrint( outfile );
    }

    // if there are multiple activities
    if( my_activities->Size() > 1 ) {
      fprintf( outfile, ":" );

      // output the activity connection list
      for( my_activities->First(); !my_activities->IsDone(); my_activities->Next() ) {
	activity = my_activities->CurrentItem();
	if( activity->CanFilePrintConnections() ) {
	  if( previously_printed_something ) { 
	    fprintf( outfile, ";" );
	    previously_printed_something = activity->FilePrintConnections( outfile );
	  }
	  else {
    	    previously_printed_something = activity->FilePrintConnections( outfile );
	  }
	}
      }
    }
    // end task activity information list
    fprintf( outfile, "\n-1\n\n" );
  }
}
