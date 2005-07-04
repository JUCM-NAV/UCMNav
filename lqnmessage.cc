#include <stdio.h>

#include "lqnmessage.h"
#include "lqnactivity.h"
#include "lqnentry.h"
#include "lqntask.h"


/* 
 * class LqnMessage
 */

LqnMessage::LqnMessage( LqnActivity* activity )
{
  my_source = activity;
  my_source_task = activity->GetTask();
  if( !my_source_task ) {
    printf( "ERROR LqnMessage( activity ): source activity has no task!\n" );
  }
  my_target = NULL;
  my_target_task = NULL;
  my_type = UNRESOLVED_MSG;
}


LqnMessage::LqnMessage( LqnEntry* entry )
{
  my_source = NULL;
  my_source_task = NULL;
  my_target = entry;
  my_target_task = entry->GetTask();
  if( !my_target_task ) {
    printf( "ERROR LqnMessage( entry ): target entry has no task!\n" );
  }
  my_type = CALL_MSG;
}


LqnMessage::LqnMessage( LqnActivity* activity, LqnEntry* entry )
{
  my_source = activity;
  my_source_task = activity->GetTask();
  if( !my_source_task ) {
    printf( "ERROR LqnMessage( activity, entry ): source activity has no task!\n" );
  }
  my_target = entry;
  my_target_task = entry->GetTask();
  if( !my_target_task ) {
    printf( "ERROR LqnMessage( activity, entry ): target entry has no task!\n" );
  }
  my_source->SetCallTarget( my_target );  // the activity's call target is the entry
  my_target->SetCallSource( my_source );  // the entry's call source is the activity 
  my_type = CALL_MSG;
}


LqnMessage::LqnMessage( LqnMessage* message )
{
  my_source = message->GetSource();
  my_source_task = message->GetSourceTask();
  if( my_source && !my_source_task ) {
    printf( "ERROR LqnMessage( message ): source activity has no task!\n" );
  }
  my_target = message->GetTarget();
  my_target_task = message->GetTargetTask();
  if( my_target && !my_target_task ) {
    printf( "ERROR LqnMessage( message ): target entry has no task!\n" );
  }
  my_type = message->GetType();
}


LqnMessage::~LqnMessage()
{
}


void LqnMessage::SetSource( LqnActivity* activity )
{
  if( my_source ) {  // a source activity is already set
    printf( "WARNING LqnMessage::SetSource: message source activity is being changed from '%s' to '%s'.\n",\
	    my_source->GetName(), activity->GetName() );
  }
  my_source = activity;
  my_source_task = activity->GetTask();
  if( my_target ) {  // a target entry is already set
    my_source->SetCallTarget( my_target );  // the activity's call target is the entry
    my_target->SetCallSource( my_source );  // the entry's call source is the activity 
  }
  if( my_type == UNRESOLVED_MSG ) {
    my_type = CALL_MSG;
  }
}


void LqnMessage::SetTarget( LqnEntry* entry )
{
  if( my_target ) {  // a target entry is already set
    printf( "WARNING LqnMessage::SetTarget: message target entry is being changed from '%s' to '%s'.\n",\
	    my_target->GetName(), entry->GetName() );
  }
  my_target = entry;
  my_target_task = entry->GetTask();
  if( my_source ) {  // a source activity is already set
    my_source->SetCallTarget( my_target );  // the activity's call target is the entry
    my_target->SetCallSource( my_source );  // the entry's call source is the activity 
  }
  if( my_type == UNRESOLVED_MSG ) {
    my_type = CALL_MSG;
  }
}


void LqnMessage::UnsetTarget( LqnEntry* entry )
{
  if( my_target ) {  // a target entry is set
    if( my_target == entry ) {
      my_target = NULL;
      my_target_task = NULL;
    }
    else {
      printf( "ERROR LqnMessage::UnsetTarget: message has target entry '%s', cannot unset entry '%s'!\n",\
	      my_target->GetName(), entry->GetName() );
    }
  }
  else {  // there is no target entry
    printf( "ERROR LqnMessage::UnsetTarget: message does not have a target entry!\n" );
  }
}


void LqnMessage::SetType( msg_type type )
{
  switch( my_type ) {
  case UNRESOLVED_MSG:  // current message type
    switch( type ) {
    case CALL_MSG:  // new message type
      my_type = type;  // update
      if( my_source ) {  // the source activity is set
	my_source->SetCallType( UNRESOLVED );  // the activity is involved in making a call
	if( my_target ) { 
	  my_source->SetCallTarget( my_target );  // the activity's call target is the entry
	  my_target->SetCallSource( my_source );  // the entry's call source is the activity 
	}
      }
      break;
    case ASYNC_MSG:  // new message type
      my_type = type;  // update
      if( my_source ) {  // the source activity is set
	my_source->SetCallType( ASYNC );  // the activity is involved in making an asynchronous call
	if( my_target ) { 
	  my_source->SetCallTarget( my_target );  // the activity's call target is the entry
	  my_target->SetCallSource( my_source );  // the entry's call source is the activity
	}
      }
      break;
    case SYNC_MSG:  // new message type
      my_type = type;  // update
      if( my_source ) {  // the source activity is set
	my_source->SetCallType( SYNC );  // the activity is involved in making a synchronous call
	if( my_target ) { 
	  my_source->SetCallTarget( my_target );  // the activity's call target is the entry
	  my_target->SetCallSource( my_source );  // the entry's call source is the activity
	}
      }
      break;
    case REPLY_MSG:  // new message type
      my_type = type;  // update
      if( my_source ) {  // the source activity is set
	my_source->SetReply();  // the activity is involved in making a reply
	my_source->SetConnection( REPLY );
      }
      if( my_target ) {  // target entry is set
	printf( "ERROR LqnMessage::SetType: reply activity '%s' should not make a call to entry '%s'!\n.",\
		my_source->GetName(), my_target->GetName() );
      }
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }  // end new message type
    break;

  case CALL_MSG:  // current message type
    switch( type ) {
    case UNRESOLVED_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from CALL_MSG to UNRESOLVED_MSG!" );
      break;
    case ASYNC_MSG:  // new message type
      my_type = type;  // update
      if( my_source ) {  // the source activity is set
	my_source->SetCallType( ASYNC );  // the activity is involved in making an asynchronous call
	if( my_target ) { 
	  my_source->SetCallTarget( my_target );  // the activity's call target is the entry
	  my_target->SetCallSource( my_source );  // the entry's call source is the activity
	}
      }
      break;
    case SYNC_MSG:  // new message type
      my_type = type;  // update
      if( my_source ) {  // the source activity is set
	my_source->SetCallType( SYNC );  // the activity is involved in making a synchronous call
	if( my_target ) { 
	  my_source->SetCallTarget( my_target );  // the activity's call target is the entry
	  my_target->SetCallSource( my_source );  // the entry's call source is the activity
	}
      }
      break;
    case REPLY_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from CALL_MSG to REPLY_MSG!\n" );
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }
    break;

  case ASYNC_MSG:  // current message type
    switch( type ) {
    case UNRESOLVED_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from ASYNC_MSG to UNRESOLVED_MSG!\n" );
      break;
    case CALL_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from ASYNC_MSG to CALL_MSG!\n" );
      break;
    case SYNC_MSG:  // new message type
      my_type = type;  // update
      printf( "LqnMessage::SetType: changing message from ASYNC_MSG to SYNC_MSG.\n" );
      if( my_source ) {  // the source activity is set
	my_source->SetCallType( SYNC );  // the activity is involved in making a synchronous call
	if( my_target ) { 
	  my_source->SetCallTarget( my_target );  // the activity's call target is the entry
	  my_target->SetCallSource( my_source );  // the entry's call source is the activity
	}
      }
      break;
    case REPLY_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from ASYNC_MSG to REPLY_MSG!\n" );
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }
    break;

  case SYNC_MSG:  // current message type
    switch( type ) {
    case UNRESOLVED_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from SYNC_MSG to UNRESOLVED_MSG!\n" );
      break;
    case CALL_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from SYNC_MSG to CALL_MSG!\n" );
      break;
    case ASYNC_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from SYNC_MSG to ASYNC_MSG!\n" );
      break;
    case REPLY_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from SYNC_MSG to REPLY_MSG!\n" );
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }
    break;

  case REPLY_MSG:  // current message type
    switch( type ) {
    case UNRESOLVED_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from REPLY_MSG to UNRESOLVED_MSG!\n" );
      break;
    case CALL_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from REPLY_MSG to CALL_MSG!\n" );
      break;
    case ASYNC_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from REPLY_MSG to ASYNC_MSG!\n" );
      break;
    case SYNC_MSG:  // new message type
      printf( "ERROR LqnMessage::SetType: message cannot be changed from REPLY_MSG to SYNC_MSG!\n" );
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }
    if( my_target ) {
      printf( "ERROR LqnMessage::SetType: REPLY_MSG  should not have a target entry '%s'!\n.", my_target->GetName() );
    }
    break;
  }
}


void LqnMessage::Print()
{
  if( my_source && my_target ) {  // message has both a source activity and a target entry
    printf( "message from '%s' to '%s'", my_source->GetName(), my_target->GetName() );
  }
  else if( my_source ) {  // message only has a source
    printf( "message from '%s' to <unknown target>", my_source->GetName() );
  }
  else if( my_target ) {  // message only has a target
    printf( "message from <unknown source> to '%s'", my_target->GetName() );
  }
  else {  // message is empty
    printf( "\nERROR LqnMessage::Print: message is empty!\n" );
  }
}

