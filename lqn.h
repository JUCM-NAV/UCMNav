#ifndef LQN_H
#define LQN_H

#include "defines.h"
#include "collection.h"
#include "component.h"
#include "component_ref.h"

#include "lqndefs.h"
#include "lqnactivity.h"
#include "lqndevice.h"
#include "lqnentry.h"
#include "lqnmessage.h"
#include "lqnmstack.h"
#include "lqntask.h"


class Lqn {
public:
  Lqn();  // constructor
  ~Lqn();  // destructor

  // create a valid task name for the given component reference
  void CreateTaskName( ComponentReference* comp_ref, char* task_name );

  // add a new task
  LqnTask* AddTask( ComponentReference* comp_ref, LqnDevice* proc );
  // add a new task running on the infinite processor
  LqnTask* AddTaskInfProc( ComponentReference* comp_ref );
  // add a new default task
  LqnTask* AddDefaultTask();
  // add a new reference task for the given start point 
  LqnTask* AddRefTask( Hyperedge* start_pt );
  // add a new task with the same characteristics as the given task
  LqnTask* CopyTask( LqnTask* task );

  // return the task corresponding to the given name
  LqnTask* GetTask( const char* name );
  // return the task corresponding to the component reference
  LqnTask* GetTaskForComp( ComponentReference* comp_ref );

  // finds the activity corresponding to the given hyperedge
  LqnActivity* GetActivity( Hyperedge* edge );

  // add a new device
  LqnDevice* AddDevice( const char* name, int id );
  // add a new device with the given speed factor
  LqnDevice* AddDevice( const char* name, int id, float spd_fctr );
  // returns the device with the specified id
  LqnDevice* GetDevice( int id );

  void SetClosed() { closed_system = true; }
  void UnsetClosed() { closed_system = false; }
  bool IsClosed() { return closed_system; }
  void SetPopulation( int size ) { system_population = size; }
  int GetPopulation() { return system_population; }

  // prints LQN information to the specified file
  void FilePrint( FILE* outfile );

protected:
  // elements of an LQN
  Cltn<LqnTask*>* my_tasks;  // tasks
  Cltn<LqnDevice*>* my_devices;  // devices, includes processors, disks, etc...
  LqnDevice* my_inf_proc;  // infinite processor
  LqnTask* current_task;  // current task
  LqnDevice* current_device;  // current device

  bool closed_system;
  int system_population;

  int num_default_tasks;  // number of default tasks added to the LQN
  int num_ref_tasks;  // number of reference tasks added to the LQN
  
  // general LQN information (from Greg Frank's previous version)
  double conv_val;  // convergence value
  int it_limit;  // iteration limit
  int print_int;  // intermed. res. print interval
  double underrelax_coeff;  // under-relaxation coefficient
};

#endif
