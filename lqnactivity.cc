#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lqnactivity.h"
#include "lqntask.h"
#include "lqnentry.h"


// constructors
LqnActivity::LqnActivity( const char* name, LqnTask* task,  bool print )
{
  Init();  // initialize all the variables
  strcpy( my_name, name );
  my_task = task;
  print_me = print;

  if( print_me ) {
    printf( "Activity '%s' for task '%s' created ", my_name, task->GetName() );
    printf( "with service time %lf.\n", my_service_time );
  }
}

LqnActivity::LqnActivity( const char* name, LqnTask* task, Hyperedge* edge,\
			  bool print )
{
  Init();  // initialize all the variables
  strcpy( my_name, name );
  my_task = task;
  my_edge = edge;
  print_me = print;

  if( print_me ) {
    printf( "Activity '%s' for task '%s' created ", my_name, task->GetName() );
    printf( "with service time %lf.\n", my_service_time );
  }
}

LqnActivity::LqnActivity( const char* name, LqnTask* task,\
			  double service_time, bool print )
{
  Init();  // initialize all the variables
  strcpy( my_name, name );
  my_task = task;
  my_service_time = service_time;
  print_me = print;

  if( print_me ) {
    printf( "Activity '%s' for task '%s' created ", my_name, task->GetName() );
    printf( "with service time %lf.\n", my_service_time );
  }
}

LqnActivity::LqnActivity( const char* name, LqnTask* task,\
			  Hyperedge* edge, double service_time, bool print )
{
  Init();  // initialize all the variables
  strcpy( my_name, name );
  my_task = task;
  my_edge = edge;
  my_service_time = service_time;
  print_me = print;

  if( print_me ) {
    printf( "Activity '%s' for task '%s' created ", my_name, task->GetName() );
    printf( "with service time %lf.\n", my_service_time );
  }
}


// destructor
LqnActivity::~LqnActivity()
{
  if( print_me ) {
    printf( "%s ", my_name );
  }
  delete my_predecessors;  // delete collection of predecessor activities
  delete my_successors;  // delete collection of successor activities
}


void LqnActivity::Init()
{
  // initialize the name
  my_name[0] = 0;
  my_name[ACTIVITY_NAME_LENGTH - 1] = 0;

  my_edge = NULL;  // the activity does not correspond to a hyperedge
  my_entry = NULL;  // the activity does not belong to any specific entry

  my_num_edge_visits = 1;  // the edge gets visited only once
                           // when the activity is created
  my_service_time = 0.0;  // default service time of 0 seconds
  my_num_reps = 1.0;  // default of 1.0 repetitions
  my_connection = NORMAL;  // the activity will have successor(s)

  my_call_type = NOCALL;  // the activity does not make any calls
  my_call_target = NULL;  // the activity does not call any entry

  my_num_calls = 1.0;  // default of 1.0 calls if the activity does make a call

  my_reply_target = NULL;  // the activity does not reply to any entry
  my_reply_prob = 0.0;

  my_branch_prob = 1.0;  // default branching probability of 1.0

  am_first_activity = false;  // not the first activity for an entry
  am_stochastic = false;  // the activity is deterministic

  my_predecessors = new Cltn<LqnActivity*>;
  my_successors = new Cltn<LqnActivity*>;
}


void LqnActivity::AddToName( const char* add )
{
  strcat( my_name, "_" );
  strcat( my_name, add );
}


bool LqnActivity::FindActivity( LqnActivity* activity,\
				Cltn<LqnActivity*>* activities )
{
  if( activities->Size() ) {  // there are activities to look at
    for( activities->First(); !activities->IsDone(); activities->Next() ) {  // go through all the activities
      if( activity == activities->CurrentItem() ) {
	return true;  // found the given activity
      }	
    }
  }
  return false;  // did not find the given activity
}


// add a predecessor to the activity
void LqnActivity::AddPredecessor( LqnActivity* predecessor ) 
{
  if( FindActivity( predecessor, my_predecessors ) ) {
    printf( "(Warning: activity '%s' already has a predecessor '%s'.)\n",\
	    my_name, predecessor->GetName() );
  }
  else {
    if( am_first_activity ) {
      printf( "ERROR: first activity '%s' cannot have a predecessor '%s'.\n",\
	      my_name, predecessor->GetName() );
    }
    else {
      my_predecessors->Add( predecessor );
    }
  }
}

 
// add a successor to the activity
void LqnActivity::AddSuccessor( LqnActivity* successor ) 
{
  if( FindActivity( successor, my_successors ) ) {
    printf( "(Warning: activity '%s' already has a successor '%s'.\n",\
	    my_name, successor->GetName() );
  }
  else {
    if( my_connection == NOCONNECTION ) {
      printf( "ERROR: end activity '%s' cannot have a successor '%s'.\n",\
	      my_name, successor->GetName() );
    }
    else {
      my_successors->Add( successor );
    }
  }
}


void LqnActivity::SetReply()
{
  my_connection = REPLY;
  my_call_type = NOCALL;
  my_reply_prob = 1.0;
}


void LqnActivity::SetReplyTarget( LqnEntry* entry )
{
  SetReply();
  my_reply_target = entry;
}


// smart change of the call type to the new call type
bool LqnActivity::SetCallType( call_type call )
{
  bool success = false;

  switch( my_call_type ) {  // old call type
  case NOCALL:
    my_call_type = call;  // change to new call type
    success = true;
    break;

  case UNRESOLVED:
    switch( call ) {  // new call type
    case NOCALL:
      my_call_type = call;  // change to new call type
      success = true;
      printf( "(Warning: call type for %s changed ", my_name );
      printf( "from UNRESOLVED to NOCALL.)\n" );
      break;
    default:
      my_call_type = call;  // change to new call type
      success = true;
      break;
    }
    break;

  case SYNC:
    switch( call ) {  // new call type
    case NOCALL:
      printf( "ERROR: call type for %s cannot be changed ", my_name );
      printf( "from SYNC to NOCALL!\n" );
      break;
    case UNRESOLVED:
      printf( "ERROR: call type for %s cannot be changed ", my_name );
      printf( "from SYNC to UNRESOLVED!\n" );
      break;
    case ASYNC:
      printf( "ERROR: call type for %s cannot be changed ", my_name );
      printf( "from SYNC to ASYNC!\n" );
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }
    break;

  case ASYNC:
    switch( call ) {  // new call type
    case UNRESOLVED:
      printf( "ERROR: call type for %s cannot be changed ", my_name );
      printf( "from ASYNC to UNRESOLVED!\n" );
      break;
    case NOCALL:
      printf( "ERROR: call type for %s cannot be changed ", my_name );
      printf( "from ASYNC to NOCALL!\n" );
      break;
    case SYNC:
      my_call_type = call;  // change to new call type
      success = true;
      printf( "(Warning: call type for %s changed ", my_name );
      printf( "from ASYNC to SYNC.)\n" );
      break;
    default: ; // DA: Added August 2004 (to satisfy GCC)
    }
    break;
  }
  return success;
}


void LqnActivity::SetLoopCall( double num_calls )
{
  SetNumCalls( num_calls );
  SetCallType( SYNC );
}


void LqnActivity::SetNumCalls( double num_calls )
{ 
  my_num_calls = num_calls;
}


void LqnActivity::SetCallTarget( LqnEntry* entry )
{ 
  my_call_target = entry;
  if( my_call_type == NOCALL ) {
    my_call_type = UNRESOLVED;
  }
}


void LqnActivity::SetCallTarget( LqnEntry* entry, call_type call )
{ 
  my_call_target = entry; 
  SetCallType( call );
}


void LqnActivity::SetLoopCallTarget( LqnEntry* entry )
{ 
  my_call_target = entry; 
  SetCallType( SYNC );
}


void LqnActivity::SetLoopCallTarget( LqnEntry* entry,  double num_calls )
{ 
  my_call_target = entry; 
  SetNumCalls( num_calls );
  SetCallType( SYNC );
}


// updates the activity name before printing
void LqnActivity::UpdateName()
{
  switch( my_call_type ) {
  case NOCALL:
    break;

  case UNRESOLVED:
    break;

  case ASYNC:
    AddToName( "AsyncCall" );
    break;

  case SYNC:
    AddToName( "SyncCall" );
    break;
  }  // end switch

  switch( my_connection ) {
  case NOCONNECTION:
    break;

  case NORMAL:
    break;

  case ORFORK:
    AddToName( "OrFork" );
    break;

  case ORJOIN:
    AddToName( "OrJoin" );
    break;

  case ANDFORK:
    AddToName( "AndFork" );
    break;

  case ANDJOIN:
    AddToName( "AndJoin" );
    break;

  case REPLY:
    AddToName( "SendReply" );
    break;
  default: ; // DA: Added August 2004 (to satisfy GCC)
  }  // end switch 
}


// prints activity definition information to the specified file
void LqnActivity::FilePrint( FILE* outfile )
{
  if( print_me ){
    printf( "Activity '%s'.\n", my_name );  // debug info
  }

  // if it is a stochastic activity
  if( am_stochastic ) {
    fprintf( outfile, "f %s 0\n", my_name );
  }
  // else it is a deterministic activity with only a first phase
  else {
    fprintf( outfile, "f %s 1\n", my_name );
  }

  // service time for activity
  fprintf( outfile, "s %s %f\n", my_name, my_service_time );

  switch( my_call_type ) {
  case NOCALL:
    break;

  case UNRESOLVED:
    fprintf( outfile, "z %s %s %lf",\
	     my_name, my_call_target->GetName(), my_num_calls );
    fprintf( outfile,\
	     "\t\t# Warning: %s makes a call that was not properly resolved",\
	     my_name );
    fprintf( outfile, ", assuming asynchronous\n" );
    break;

  case ASYNC:
    fprintf( outfile, "z %s %s %lf\n",\
	     my_name, my_call_target->GetName(), my_num_calls );
    break;

  case SYNC:
    fprintf( outfile, "y %s %s %lf\n",\
	     my_name, my_call_target->GetName(), my_num_calls );
    break;
  }  // end switch
}


// prints activity connection information to the specified file
bool LqnActivity::FilePrintConnections( FILE* outfile )
{
  int num_successors = my_successors->Size();
  LqnActivity* branch = NULL;
  LqnActivity* last_branch = NULL;
  // LqnActivity* loop_head = NULL; // DA: August 2004
  bool printed_something = false;

  if( print_me ){
    printf( "LqnActivity::FilePrintConnections '%s'.\n", my_name );
  }

  switch( my_connection ) {
  case NOCONNECTION:
    // Jul2005 gM: replaced 's' with '%s' to get rid of GCC warning
	printf( "WARNING LqnActivity::FilePrintConnections: activity '%s' has no connections.\n", my_name );
    break;

  case NORMAL:
    // if there is only one successor to this activity
    if( num_successors == 1 ) {
      fprintf( outfile, "\n%s -> %s",\
	       my_name, my_successors->GetFirst()->GetName() );
      printed_something = true;
    }
    // else there are multiple successors to this activity
    else  {
      fprintf( outfile,\
	       "\n# Warning: %s has %d successors, showing only the first one",\
	       my_name, num_successors );
      fprintf( outfile, "\n%s -> %s",\
	       my_name, my_successors->GetFirst()->GetName() );
      printed_something = true;
    }
    break;

  case ORFORK:
    fprintf( outfile, "\n%s -> ", my_name );
    last_branch = my_successors ->GetLast();

    for( my_successors->First(); !my_successors->IsDone(); my_successors->Next() ) {
      branch = my_successors ->CurrentItem();

      if( branch == last_branch ) {
	fprintf( outfile, "(%lf) %s",\
		 branch->GetBranchProb(), branch->GetName() );
      }
      else {
	fprintf( outfile, "(%lf) %s + ",\
		 branch->GetBranchProb(), branch->GetName() );
      }
    }

    printed_something = true;
    break;

  case ORJOIN:
    fprintf( outfile, "\n" );
    last_branch = my_predecessors->GetLast();

    for( my_predecessors->First(); !my_predecessors->IsDone(); my_predecessors->Next() ) {
      branch = my_predecessors ->CurrentItem();

      if( branch == last_branch ) {
	fprintf( outfile, "%s", branch->GetName() );
      }
      else {
	fprintf( outfile, "%s + ", branch->GetName() );
      }
    }

    fprintf( outfile, " -> %s;", my_name );

    // if there is only one successor to this activity
    if( num_successors == 1 ) {
      fprintf( outfile, "\n%s -> %s",\
	       my_name, my_successors->GetFirst()->GetName() );
    }
    // else there are multiple successors to this activity
    else {
      fprintf( outfile,\
	       "\n# activity '%s' has %d successors, showing only the first one",\
	       my_name, num_successors );
      fprintf( outfile, "\n%s -> %s",\
	       my_name, my_successors->GetFirst()->GetName() );
    }

    printed_something = true;
    break;

  case ANDFORK:
    fprintf( outfile, "\n%s -> ", my_name );
    last_branch = my_successors ->GetLast();

    for( my_successors->First(); !my_successors->IsDone(); my_successors->Next() ) {
      branch = my_successors ->CurrentItem();

      if( branch == last_branch ) {
	fprintf( outfile, "%s", branch->GetName() );
      }
      else {
	fprintf( outfile, "%s & ", branch->GetName() );
      }
    }

    printed_something = true;
    break;

  case ANDJOIN:
    fprintf( outfile, "\n" );
    last_branch = my_predecessors ->GetLast();

    for( my_predecessors->First(); !my_predecessors->IsDone(); my_predecessors->Next() ) {
      branch = my_predecessors ->CurrentItem();

      if( branch == last_branch ) {
	fprintf( outfile, "%s", branch->GetName() );
      }
      else {
	fprintf( outfile, "%s & ", branch->GetName() );
      }
    }

    fprintf( outfile, " -> %s;", my_name );

    // if there is only one successor to this activity
    if( num_successors == 1 ) {
      fprintf( outfile, "\n%s -> %s",\
	       my_name, my_successors->GetFirst()->GetName() );
    }
    // else there are multiple successors to this activity
    else {
      fprintf( outfile,\
	       "\n# Warning: '%s' has %d successors, showing only the first one",\
	       my_name, num_successors );
      fprintf( outfile, "\n%s -> %s",\
	       my_name, my_successors->GetFirst()->GetName() );
    }

    printed_something = true;
    break;

  case REPLY:
    if( print_me ) {
      printf( "Activity '%s' replies to entry '%s'.\n",\
	      my_name, my_reply_target->GetName() );
    }
    fprintf( outfile, "\n%s[ %s ]", my_name, my_reply_target->GetName() );

//    if( num_successors == 1 ) {  // there are successors to this activity
//      fprintf( outfile, ";\n%s -> %s", my_name, my_successors->GetFirst()->GetName() );
//    }
//    else if( num_successors > 1 ) {  // there are too many successors to this activity
//      fprintf( outfile, "\n# activity '%s' has %d successors, showing only the first one", my_name, num_successors );
//      fprintf( outfile, ";\n%s -> %s", my_name, my_successors->GetFirst()->GetName() );
//    }

    printed_something = true;
    break;
  default: ; // DA: Added August 2004 (to satisfy GCC)
  }  // end switch

  return printed_something;
}


// determines if there is activity connection information to print
bool LqnActivity::CanFilePrintConnections()
{
  bool can_print_something = false;

  switch( my_connection ) {
  case NOCONNECTION:
    can_print_something = false;
    break;

  case NORMAL:
    if( my_successors->Size() ) {
      can_print_something = true;
    }
    break;

  case ORFORK:
    can_print_something = true;
    break;

  case ORJOIN:
    can_print_something = true;
    break;

  case ANDFORK:
    can_print_something = true;
    break;

  case ANDJOIN:
    can_print_something = true;
    break;

  case REPLY:
    can_print_something = true;
    break;
  default: ; // DA: Added August 2004 (to satisfy GCC)
  }  // end switch

  return can_print_something;
}
