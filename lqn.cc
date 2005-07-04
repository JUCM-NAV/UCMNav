#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lqn.h"

#include "lqntemplates-def.cc"


// constructor
Lqn::Lqn()
{
  closed_system = false;  // assume open system
  system_population = 1;  // assume a single user
  num_default_tasks = 0;
  num_ref_tasks = 0;

  my_tasks = new Cltn<LqnTask*>;  // tasks
  my_devices = new Cltn<LqnDevice*>;  // devices, includes processors, disks, etc...
  my_inf_proc = new LqnDevice( "P_Infinite", -1  );
  my_inf_proc->SetInfinite();

  printf( "LQN object created.\n");
}


// destructor
Lqn::~Lqn()
{
  printf( "Deleting LQN with %d tasks", my_tasks->Size() );
  while( my_tasks->Size() ) {
    printf( "\n\t" );
    delete my_tasks->Detach();
  }
  delete my_tasks;

  printf( "...and %d devices", my_devices->Size() );
  while( my_devices->Size() ) {
    printf( "\n\t" );
    delete my_devices->Detach();
  }
  delete my_devices;
  delete my_inf_proc;

  printf( "...delete LQN done!\n");
}


// create a valid task name for the given component reference
void Lqn::CreateTaskName( ComponentReference* comp_ref, char* task_name )
{
  Component* comp = NULL;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  if( comp_ref ) {  // the component reference exists
    comp = comp_ref->ReferencedComponent();  // logical component

    if( comp ) {  // the logical component  exists
      strcpy( task_name, comp->GetLabel() );
      // replace spaces in the name with underscores
      for( size_t i = 0; i < strlen( task_name ) ; i++ ) {
	if( task_name[i] == ' ' ) {
	  task_name[i] = '_';
	}
      }
    }
    // else use the component reference
    else {
      strcpy( task_name, "LqnTask" );
      sprintf( number, "%d", comp_ref->ComponentReferenceNumber() );
      strcat( task_name, number );
    }
  }
}


// add a new task corresponding to the component reference
// return a pointer to the task
LqnTask* Lqn::AddTask( ComponentReference* comp_ref, LqnDevice* proc )
{
  LqnTask* task;
  char name[TASK_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[TASK_NAME_LENGTH - 1] = 0;

  // make sure that the component reference actually exists
  if( comp_ref != NULL ) {
    CreateTaskName( comp_ref, name );
    // check if a task already exists for this component reference
    if( GetTask( name ) ) {
      task = GetTask( name );
      printf( "(Lqn::AddTask: cannot add new task '%s', task already exists.)\n", name );
    }
    // else add new task
    else {
      task = new LqnTask( name, proc );
      my_tasks->Add( task );
    }
  }

  return task;
}


// add a new task corresponding to the component reference running on the infinite processor
// return a pointer to the task
LqnTask* Lqn::AddTaskInfProc( ComponentReference* comp_ref )
{
  LqnTask* task = NULL;
  char name[TASK_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[TASK_NAME_LENGTH - 1] = 0;

  // make sure that the component reference actually exists
  if( comp_ref != NULL ) {
    CreateTaskName( comp_ref, name );
    // check if a task already exists for this component reference
    if( GetTask( name ) ) {
      task = GetTask( name );
      printf( "(Lqn::AddTaskInfProc: task '%s' already exists.)\n", name );
    }
    // else add new task
    else {
      task = new LqnTask( name, my_inf_proc );
      my_tasks->Add( task );
    }
  }

  return task;
}


// add a new default task
// return a pointer to the task
LqnTask* Lqn::AddDefaultTask()
{
  LqnTask* task;
  char name[TASK_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[TASK_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  num_default_tasks++;  // increment the number of default tasks
  strcpy( name, "DefaultTask" );
  sprintf( number, "%d", num_default_tasks );
  strcat( name, number );
  task = new LqnTask( name, my_inf_proc );
  my_tasks->Add( task );

  return task;
}


// add a new reference task for the given start point 
// return a pointer to the task
LqnTask* Lqn::AddRefTask( Hyperedge* start_pt ) 
{
  LqnTask* task;
  char name[TASK_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[TASK_NAME_LENGTH - 1] = 0;
  char number[NUMBER_LENGTH];
  number[0] = 0;  // initialize the number
  number[NUMBER_LENGTH - 1] = 0;

  num_ref_tasks++;  // increment the number of reference tasks
  strcpy( name, "RefTask" );
  sprintf( number, "%d", num_ref_tasks );
  strcat( name, number );
  task = new LqnTask( name, my_inf_proc );
  task->SetAsReference();
  my_tasks->Add( task );

  return task;
}


// add a new task with the same characteristics as the given task
LqnTask* Lqn::CopyTask( LqnTask* task )
{
  LqnTask* copy = task->Copy();
  copy->SetOriginal( task );
  my_tasks->Add( copy );

  return copy;
} 


// return the task corresponding to the given name
LqnTask* Lqn::GetTask( const char* name )
{
  LqnTask* task = NULL;

  for( my_tasks->First(); !my_tasks->IsDone() && !task; my_tasks->Next() ) {
    if( strequal( my_tasks->CurrentItem()->GetName(), name ) ) {
      task = my_tasks->CurrentItem();
    }
  }

  return task;
} 


// return the task corresponding to the component reference
LqnTask* Lqn::GetTaskForComp( ComponentReference* comp_ref )
{
  LqnTask* task = NULL;
  char name[TASK_NAME_LENGTH];
  name[0] = 0;  // initialize the name
  name[TASK_NAME_LENGTH - 1] = 0;

  // make sure that the component reference actually exists
  if( comp_ref != NULL ) {
    CreateTaskName( comp_ref, name );
    task = GetTask( name );
  }

  return task;
} 


// finds the join activity corresponding to the given hyperedge
LqnActivity* Lqn::GetActivity( Hyperedge* edge )
{
  LqnActivity* activity = NULL;

  // go through all the tasks
  for( my_tasks->First(); !my_tasks->IsDone(); my_tasks->Next() ) {  
      activity = my_tasks->CurrentItem()->GetActivity( edge );
      if( activity ) {
	return activity;  // return the activity as soon as it's found
      }
  }

  return activity;
}


// add a new device
LqnDevice* Lqn::AddDevice( const char* name, int id )
{             
  LqnDevice* device = new LqnDevice( name, id );
  my_devices->Add( device );
  return device;
}


// add a new device with the given speed factor
LqnDevice* Lqn::AddDevice( const char* name, int id, float spd_fctr )
{
  LqnDevice* device = new LqnDevice( name, id, spd_fctr );
  my_devices->Add( device );

  return device;
}


// returns the device with the specified id
LqnDevice* Lqn::GetDevice( int id )
{
  LqnDevice* device = NULL;

  for( my_devices->First(); !my_devices->IsDone(); my_devices->Next() ) {
    if( my_devices->CurrentItem()->GetId() == id ) {
      device = my_devices->CurrentItem();
    }
  }

  if( device ) {  // found the device
    return device;
  }
  else {  // did not find the device
    printf( "(Lqn::GetDevice: no device with id %d, using infinite processor.)\n\n", id );
    return my_inf_proc;
  }
}


// prints LQN information to the specified file
void Lqn::FilePrint( FILE* outfile )
{
  fprintf( outfile, "# LqnGenerator output\n\n" );

  // general information
  fprintf( outfile, "G\n" );
  fprintf( outfile, "\"\"\n" );  // comment
  fprintf( outfile, "1e-05\n" );  // convergence value
  fprintf( outfile, "50\n" );  // iteration limit
  fprintf( outfile, "5\n" );  // intermed. res. print interval
  fprintf( outfile, "0.9\n" );  // under-relaxation coefficient
  fprintf( outfile, "-1\n\n" );  // end general information list

  // processor information
  fprintf( outfile, "P 0\n" );  // P 0
  my_inf_proc->FilePrint( outfile );  // infinite processor

  // go through all the other processors
  for( my_devices->First(); !my_devices->IsDone(); my_devices->Next() ) {
    my_devices->CurrentItem()->FilePrint( outfile );  // output processor info
  }

  fprintf( outfile, "-1\n\n" );  // end processor information list
  printf( "\n" );

  // start task data
  fprintf( outfile, "T 0\n" );  // T 0

  // go through all the tasks
  for( my_tasks->First(); !my_tasks->IsDone(); my_tasks->Next() ) {
    my_tasks->CurrentItem()->FilePrint( outfile );  // output task info
  }

  fprintf( outfile, "-1\n\n" );  // end task information list
  printf( "\n" );

  // start entry data
  fprintf( outfile, "E 0\n" );  // E 0

  // go through all the tasks
  for( my_tasks->First(); !my_tasks->IsDone(); my_tasks->Next() ) {
    my_tasks->CurrentItem()->FilePrintEntries( outfile );
  }

  fprintf( outfile, "-1\n\n" );  // end entry information list
  printf( "\n" );

  // print the activity information list for each task
  for( my_tasks->First(); !my_tasks->IsDone(); my_tasks->Next() ) {
    my_tasks->CurrentItem()->FilePrintActivities( outfile );
  }
  printf( "\n" );
}
