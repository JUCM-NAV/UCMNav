#include <stdio.h>

#include "lqnmstack.h"
#include "lqnmessage.h"
#include "lqnactivity.h"
#include "lqnentry.h"
#include "lqntask.h"


/* 
 * class LqnMstack
 */

// constructor
LqnMstack::LqnMstack( int id )
{
  my_id = id;
  my_messages = new Cltn<LqnMessage*>;
  printlevel = 0;
  if( printlevel >= 1 ) {
    printf( "\tMstack%d created.\n", my_id );
  }
}


// destructor
LqnMstack::~LqnMstack()
{
  if( printlevel >= 1 ) {
    printf( "Deleting Mstack%d with %d messages...", my_id, my_messages->Size() );
  }
  while( my_messages->Size() ) {
    delete my_messages->Detach();
    if( printlevel >= 1 ) {
      printf( "*" );
    }
  }
  delete my_messages;
  if( printlevel >= 1 ) {
    printf( "...done!\n" );
  }
}


// push the given message on the stack
void LqnMstack::Push( LqnMessage* message )
{
  my_messages->Add( message );
  if( printlevel >= 1 ) {
    printf( "\tMstack%d PUSH: ", my_id );
    message->Print();
    printf( ".\n" );
  }
}


// pop the last message off the stack
LqnMessage* LqnMstack::Pop()
{
  LqnMessage* message = NULL;

  if( my_messages->Size() ) {  // there are messages on the stack
    message = my_messages->Detach();
    if( printlevel >= 1 ) {
      printf( "\tMstack%d POP: ", my_id );
      message->Print();
      printf( ".\n" );
    }
  }
  else {  // there are no messages on the stack
    printf( "ERROR stack%d POP: no messages on stack!\n\n", my_id );
  }
  return message;
}


// pop the last message off the stack and set it to be of the given message type
LqnMessage* LqnMstack::Pop( msg_type type )
{
  LqnMessage* message = Pop();
  if( message ) {
    message->SetType( type );
  }
  return message;
}


// pop the last activity off the stack, only if it does not have a target entry
LqnActivity* LqnMstack::PopActivity()
{
  LqnMessage* message = NULL;
  LqnActivity* activity = NULL;

  if( my_messages->Size() ) {  // there are messages on the stack
    if( !my_messages->GetLast()->GetTarget() ) {  // the last message does not have a target entry
      message = my_messages->Detach();
      activity = message->GetSource();
      delete message;
      if( printlevel >= 1 ) {
	printf( "\tMstack%d PopActivity '%s'.\n", my_id, activity->GetName() );
      }
    }
    else {  // the last message does have a target entry
      printf( "ERROR Mstack%d PopActivity: the last message has a target entry!\n\n", my_id );
    }
  }
  else {  // there are no messages on the stack
    printf( "ERROR Mstack%d PopActivity: no messages on stack!\n\n", my_id );
  }
  return activity;
}


// pop the last entry off the stack
LqnEntry* LqnMstack::PopEntry()
{
  LqnEntry* entry = NULL;
  LqnMessage* message = NULL;

  if( my_messages->Size() ) {  // there are messages on the stack
    message = my_messages->GetLast();
    if( message->GetSource() ) {  // the last message has a source activity
      entry = message->GetTarget();
      message->UnsetTarget( entry );  // leave the source activity on the stack
    }
    else {  // the last message does not have a source activity
      message = my_messages->Detach();  // remove the last message from the stack
      entry = message->GetTarget();
      delete message;
    }
    if( printlevel >= 1 ) {
      printf( "\tMstack%d PopEntry '%s'.\n", my_id, entry->GetName() );
    }
  }
  else {  // there are no messages on the stack
    printf( "ERROR Mstack%d PopEntry: no messages on stack!\n\n", my_id );
  }
  return entry;
}


// return the last message on the Mstack
LqnMessage* LqnMstack::GetLastMessage()
{
  LqnMessage* message = NULL;
  
  if( my_messages->Size() ) {  // there are messages on the stack
    message = my_messages->GetLast();  // get the last message
    if( printlevel >= 2 ) {
		// Jul2005 gM: added ", my_id" to argument list to get rid of GCC warning
      printf( "\tMstack%d GetLastMessage: ", my_id );
      message->Print();
      printf( ".\n" );
    }
  }
  else {  // there are no messages on the stack
    printf( "ERROR Mstack%d GetLastMessage: no messages on stack!\n\n", my_id );
  }
  return message;
}


// return the last task on the Mstack
LqnTask* LqnMstack::GetLastTask()
{
  LqnTask* task = NULL;
  LqnMessage* message = NULL;

  if( my_messages->Size() ) {  // there are messages on the stack
    message = my_messages->GetLast();  // get the last message
    if( printlevel >= 2 ) {
      printf( "\tMstack%d GetLastTask: ", my_id );
      message->Print();
      printf( "...\n" );
    }
    if( message->GetTarget() ) {  // there is a message target
      task = message->GetTargetTask();  // the last task is the target task of the message
      if( task ) {
	if( printlevel >= 1 ) {
	  printf( "\tMstack%d GetLastTask: the last target task is '%s'.\n", my_id, task->GetName() );
	}
      }
      else {
	printf( "ERROR Mstack%d GetLastTask: the last message has a target entry with no task!\n\n", my_id );
      }
    }
    else if( message->GetSource() ) {  // no message target, but there is a message source 
      task = message->GetSourceTask();  // the last task is the source task of the message
      if( task ) {
	if( printlevel >= 1 ) {
	  printf( "\tMstack%d GetLastTask: the last source task is '%s'.\n", my_id, task->GetName() );
	}
      }
      else {
	printf( "ERROR Mstack%d GetLastTask: the last message has a source activity with no task!\n\n", my_id );
      }
    }
    else {  // the last task was not found from the message
      printf( "ERROR Mstack%d GetLastTask: the last message on the stack has no target or source task!\n\n", my_id );
    }
  }
  else {  // there are no messages on the stack
    printf( "ERROR Mstack%d GetLastTask: no messages on stack!\n\n", my_id );
  }
  return task;
}  


// return the number of times a task is found on the stack
int LqnMstack::FindTask( LqnTask* task )
{
  LqnMessage* message = NULL;
  int count = 0;

  if( task ) {
    if( printlevel >= 1 ) {
      printf( "\tMstack%d FindTask: looking for task '%s'...", my_id, task->GetName() );
    }
    for( my_messages->First(); !my_messages->IsDone(); my_messages->Next() ){
      message = my_messages->CurrentItem();
      if( task == message->GetSourceTask() ) {
	count++;
	if( printlevel >= 2 ) {
	  printf( "O" );
	}
      }
      else if( printlevel >= 2 ) {
	printf( "*" );
      }
    }
    if( printlevel >= 1 ) {
      if( count == 1 ) {  // found once
	printf( "...found once.\n" );
      }
      else if( count > 1 ) {  // found more than once
	printf( "...found %d times.\n", count );
      }
      else {  // not found
	printf( "...not found.\n" );
      }
    }
  }
  else {
    printf( "ERROR Mstack%d FindTask: cannot find NULL task!\n", my_id );
  }
  if( count > 1 ) {
    printf( "ERROR Mstack%d FindTask: found task %s %d times on the stack (it should not be there more than once)!\n\n",
	    my_id, task->GetName(), count );
  }
  return count;
}


// return the task previous to the given task on the Mstack
LqnTask* LqnMstack::GetTaskPreviousTo( LqnTask* task )
{
  LqnTask* previous_task = NULL;
  LqnMessage* message = NULL;
  LqnMessage* first_message = NULL;
//  LqnMstack* working_mstack = NULL;

  if( task ) {  // the given task exists
    if( my_messages->Size() ) {  // there are messages on the stack
      if( printlevel >= 2 ) {
	printf( "\tMstack%d GetTaskPreviousTo: looking for task previous to '%s'...\n", my_id, task->GetName() );
      }
      first_message = my_messages->GetFirst();
      message = my_messages->GetLast();
      while( !previous_task && ( message != first_message ) ) {  // go through the messages backwards
	if( task == message->GetTargetTask() ) {  // the given task is the target task of the message
	  previous_task = message->GetSourceTask();  // the previous task is the source task of the message
	}
	message = my_messages->GetPrevious( message );  // get the previous message
      }
      if( task == message->GetTargetTask() ) {  // the given task is the target task of the message
	previous_task = message->GetSourceTask();  // the previous task is the source task of the message
      }
      if( printlevel >= 1 ) {
	if( previous_task ) {
	  printf( "\tMstack%d GetTaskPreviousTo: task previous to '%s' is '%s'.\n", \
		  my_id, task->GetName(), previous_task->GetName() );
	}
	else {
	  printf( "\tMstack%d GetTaskPreviousTo: there is no task previous to '%s'.\n",  my_id, task->GetName() );
	}
      }
    }
    else {  // there are no messages on the stack
      printf( "ERROR Mstack%d GetTaskPreviousTo: no messages on stack!\n", my_id );
    }
  }
  else {  // the given task does not exist
    printf( "ERROR Mstack%d GetTaskPreviousTo: cannot find task previous to NULL task!\n", my_id );
  }
//  if( task ) {
//    if( printlevel >= 2 ) {
//      printf( "\tMstack%d GetTaskPreviousTo: looking for task previous to '%s'...\n", my_id, task->GetName() );
//    }
//    working_mstack = Copy( -1 );  // working copy of the Mstack
//    while( !previous_task && working_mstack->Size() ) {  // the previous task has not been found and the stack is not empty
//      message = working_mstack->Pop();
//      if( task == message->GetTargetTask() ) {  // the given task is the target task of the message
//	previous_task = message->GetSourceTask();  // the previous task  the source task of the message
//      }
//      delete message;  // done with the current message
//    }
//    if( printlevel >= 1 ) {
//      if( previous_task ) {
//	printf( "\tMstack%d GetTaskPreviousTo: task previous to '%s' is '%s'.\n", my_id, task->GetName(), previous_task->GetName() );
//      }
//      else {
//	printf( "\tMstack%d GetTaskPreviousTo: there is no task previous to '%s'.\n",  my_id, task->GetName() );
//      }
//    }
//    delete working_mstack;
//  }
//  else {
//    printf( "ERROR Mstack%d GetTaskPreviousTo: cannot find task previous to NULL task!\n", my_id );
//  }
  return previous_task;
}


// return a copy of the current Mstack with the given id
LqnMstack* LqnMstack::Copy( int id )
{
  LqnMessage* msg_copy = NULL;
  LqnMstack* mstack_copy = new LqnMstack( id );
  for( my_messages->First(); !my_messages->IsDone(); my_messages->Next() ){
    msg_copy = new LqnMessage( my_messages->CurrentItem() );
    mstack_copy->Push( msg_copy );
  }
  return mstack_copy;
}


// print the Mstack messages
void LqnMstack::Print()
{
  LqnMessage* message;
  LqnMessage* first_message;

  printf( "\tMstack%d contents:\t", my_id );
  // check that the Mstack is not empty
  if( my_messages->Size() ) {
    // start with the last message
    message = my_messages->GetLast();
    message->Print();
    printf( " [top]" );
    // go through the other messages backwards
    first_message = my_messages->GetFirst();
    while( message != first_message ) {
      printf( "\n\t\t\t\t" );
      message = my_messages->GetPrevious( message );
      message->Print();
    }
    printf( " [bottom]\n" );
  }
  // else the Mstack is empty
  else {
    printf( "-- empty --\n" );
  }
}
