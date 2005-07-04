#ifndef LQNENTRY_H
#define LQNENTRY_H

#include "defines.h"
#include "lqndefs.h"

class LqnActivity;
class LqnTask;

class LqnEntry {
public:
  LqnEntry( const char* name, bool print );  // constructor
  LqnEntry( const char* name, LqnTask* task, bool print );
  LqnEntry( const char* name, LqnTask* task, LqnActivity* first_activity, bool print );
  ~LqnEntry();  // destructor
  const char* GetName() { return my_name; }

  void SetTask( LqnTask* task ) { my_task = task; }
  LqnTask* GetTask() { return my_task; }
  
  void SetCallSource( LqnActivity* activity) { calling_activity = activity; }
  LqnActivity* GetCallSource() { return calling_activity; }
  
  void SetFirstActivity( LqnActivity* activity ) { my_first_activity = activity; }
  LqnActivity* GetFirstActivity() { return my_first_activity; }

  void SetFwd() { fwd = true; }
  void SetFwdTarget( LqnEntry* entry ) { fwd = true; fwd_target = entry; }

  void FilePrint( FILE* outfile );  // prints entry information to the specified file

protected:
  char my_name[ENTRY_NAME_LENGTH];  // entry name
  LqnTask* my_task;  // pointer to the parent task of the activity
  LqnActivity* my_first_activity;  // first activity linked to this entry
  bool print_me;

  LqnActivity* calling_activity;  // activity making a call to this entry
  bool fwd;  // flag on whether this entry does any forwarding
  LqnEntry* fwd_target;  // entry being forwarded to
};

#endif
