#ifndef LQNMESSAGE_H
#define LQNMESSAGE_H

#include "collection.h"
#include "defines.h"

#include "lqndefs.h"

class LqnActivity;
class LqnEntry;
class LqnTask;

class LqnMessage {
public:
  LqnMessage( LqnActivity* activity );
  LqnMessage( LqnEntry* entry );
  LqnMessage( LqnActivity* activity, LqnEntry* entry );
  LqnMessage( LqnMessage* message );
  ~LqnMessage();

  void SetSource( LqnActivity* activity );
  void SetTarget( LqnEntry* entry );
  void SetType( msg_type type );
  void UnsetTarget( LqnEntry* entry );

  LqnActivity* GetSource() { return my_source; }
  LqnEntry* GetTarget() { return my_target; }
  LqnTask* GetSourceTask() { return my_source_task; }
  LqnTask* GetTargetTask() { return my_target_task; }
  msg_type GetType() { return my_type; }

  void Print();

private:
  LqnActivity* my_source;
  LqnEntry* my_target;
  LqnTask* my_source_task;
  LqnTask* my_target_task;
  msg_type my_type;
};

#endif
