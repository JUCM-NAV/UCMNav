#ifndef LQNMSTACK_H
#define LQNMSTACK_H

#include "defines.h"
#include "collection.h"

#include "lqndefs.h"


class LqnActivity;
class LqnEntry;
class LqnMessage;
class LqnTask;

class LqnMstack {
public:
  LqnMstack( int id );  // constructor
  ~LqnMstack();  // destructor

  void Push( LqnMessage* message );  // push the given message on the stack
  LqnMessage* Pop();  // pop the last message off the stack
  LqnMessage* Pop( msg_type type );  // pop the last message off the stack and set it to be of the given message type
  LqnActivity* PopActivity();  // pop the last activity off the stack, only if it does not have a target entry
  LqnEntry* PopEntry();  // pop the last entry off the stack

  LqnMessage* GetLastMessage();  // return the last message on the Mstack
  LqnTask* GetLastTask();  // return the last task on the Mstack
  LqnTask* GetTaskPreviousTo( LqnTask* task );  // return the task previous to the given task on the Mstack
  int FindTask( LqnTask* task );  // return the number of times an message belonging to the task is found on the Mstack

  LqnMstack* Copy( int id );
  void Print();
  int GetId() { return my_id; }
  int Size() { return my_messages->Size(); }

protected:
  int my_id;
  Cltn<LqnMessage*>* my_messages;

  int printlevel;
};

#endif
