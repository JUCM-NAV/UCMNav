#ifndef LQNTASK_H
#define LQNTASK_H

#include "collection.h"
#include "defines.h"
#include "hyperedge.h"
#include "loop.h"
#include "resp_ref.h"
#include "responsibility.h"
#include "stub.h"

#include "lqndefs.h"

class Lqn;
class LqnActivity;
class LqnEntry;
class LqnDevice;
class LqnMstack;

class LqnTask {
public:
  LqnTask( const char* name, LqnDevice* proc );  // constructor
  ~LqnTask();  // destructor

  // return the name of the task
  const char* GetName() { return my_name; }
  // return the task's processor
  LqnDevice* GetProcessor() { return my_proc; }

  // create a new task with the same characteristics
  LqnTask* Copy();
  void SetCopy() { am_copy = true; }
  bool IsCopy() { return am_copy; }
  void SetOriginal( LqnTask* task ) { my_original = task; }
  LqnTask* GetOriginal() { return my_original; }

  // set task as reference task
  void SetAsReference() { reference_task = true; }
  // unset task as reference task
  void UnsetAsReference() { reference_task = false; }
  // return whether the reference status of the task
  bool IsReference() { return reference_task; }

  // add an entry to the task
  LqnEntry* AddEntry();
  // add a reference entry to the task
  LqnEntry* AddRefEntry();
  // return the entries for the task
  Cltn<LqnEntry*>* GetEntries() { return my_entries; }
  // return the number of entries for the task
  int CountEntries() { return my_entries->Size(); }

  // create a valid activity name for the edge name
  void CreateActivityName( const char* edge_name, char* activity_name );
  // create a default activity
  LqnActivity* CreateDefaultActivity();
  // create a default activity for the given hyperedge
  LqnActivity* CreateDefaultActivity( Hyperedge* edge );
  // create a default activity with the given name
  LqnActivity* CreateDefaultActivity( const char* include_name );
  // create a default activity for the given hyperedge with the given name
  LqnActivity* CreateDefaultActivity( Hyperedge* edge,\
				      const char* include_name );

  // add the given activity to the task
  void AddActivity( LqnActivity* activity );
  // connect the first activity to the second activity
  void ConnectActivities( LqnActivity* first, LqnActivity* second );

  // add a default activity to the task
  LqnActivity* AddDefaultActivity();
  //add  a default activity to the task for the given hyperedge
  LqnActivity* AddDefaultActivity( Hyperedge* edge );
  // add a default activity to the task with the given name
  LqnActivity* AddDefaultActivity( const char* include_name );
  // add a default activity for the given hyperedge with the given name 
  // to the task
  LqnActivity* AddDefaultActivity( Hyperedge* edge, const char* include_name );
  // add an activity for the responsibility
  LqnActivity* AddResponsibilityActivity( ResponsibilityReference* resp_ref );
  // create an activity for the stub to the task
  LqnActivity* CreateStubActivity( Stub* stub );
  // add an activity to handle the loop head
  LqnActivity* AddLoopHeadActivity( Loop* loop );

  // get the activity corresponding to the given hyperedge
  LqnActivity* GetActivity( Hyperedge* edge );
  // get the last activity added to the task
  LqnActivity* GetLastActivity();

  // return the activities for the task
  Cltn<LqnActivity*>* GetActivities() { return my_activities; }
  // return the number of activities for the task
  int CountActivities() { return my_activities->Size(); }

  // make the activity the messaging candidate for the task
  void SetMsgCandidate( LqnActivity* activity );
  // make the activity the connection candidate for the task
  void SetConnectCandidate( LqnActivity* activity );
  // return the messaging candidate for the task
  LqnActivity* GetMsgCandidate();
  // return the connection candidate for the task
  LqnActivity* GetConnectCandidate();

  // print the task's information to the specified file
  void FilePrint( FILE* outfile );
  // print the task's entry information to the specified file
  void FilePrintEntries( FILE* outfile );
  // print the task's activity information list to the specified file
  void FilePrintActivities( FILE* outfile );

protected:
  char my_name[TASK_NAME_LENGTH];  // task name
  int my_entry_count;
  int my_ref_entry_count;
  int my_copy_count;
  int my_def_activities;  // number of default activities added to the task

  // elements of a task
  Cltn<LqnEntry*>* my_entries;
  Cltn<LqnActivity*>* my_activities;
  Cltn<LqnActivity*>* my_unconnected_activities;
  LqnDevice* my_proc;
  LqnTask* my_original;

  LqnActivity* msg_candidate;
  LqnActivity* connect_candidate;

  // other internal data
  bool reference_task;  // flag to indicate whether this is a reference task
  bool am_copy;
  bool print;
};
#endif
