#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lqnentry.h"
#include "lqnactivity.h"
#include "lqntask.h"


// constructors
LqnEntry::LqnEntry( const char* name, bool print )
{
  my_name[0] = 0;  // initialize the name
  my_name[ENTRY_NAME_LENGTH - 1] = 0;
  strcpy( my_name, name );
  my_task = NULL;
  my_first_activity = NULL;
  print_me = print;

  calling_activity = NULL;
  fwd = false;
  fwd_target = NULL;

  if( print_me ) {
    printf( "Entry '%s' created.\n", my_name );
  }
}


LqnEntry::LqnEntry( const char* name, LqnTask* task, bool print )
{
  my_name[0] = 0;  // initialize the name
  my_name[ENTRY_NAME_LENGTH - 1] = 0;
  strcpy( my_name, name );
  my_task = task;
  print_me = print;

  calling_activity = NULL;
  fwd = false;
  fwd_target = NULL;

  if( print_me ) {
    printf( "Entry '%s' created for task '%s'.\n",\
	    my_name, my_task->GetName() );
  }
}


LqnEntry::LqnEntry( const char* name, LqnTask* task, LqnActivity* first_activity, bool print )
{
  my_name[0] = 0;  // initialize the name
  my_name[ENTRY_NAME_LENGTH - 1] = 0;
  strcpy( my_name, name );
  my_task = task;
  my_first_activity = first_activity;
  print_me = print;

  calling_activity = NULL;
  fwd = false;
  fwd_target = NULL;

  if( print_me ) {
    printf( "Entry '%s' created for task '%s' with first activity '%s'.\n",\
	    my_name, my_task->GetName(), my_first_activity->GetName() );
  }
}


// destructor
LqnEntry::~LqnEntry()
{
  if( print_me ) {
    printf( "%s ", my_name );
  }
}


// prints entry information to the specified file
void LqnEntry::FilePrint( FILE* outfile )
{
  fprintf( outfile, "A %s %s\n", my_name, my_first_activity->GetName() );

  if( fwd ) {
    fprintf( outfile, "F %s %s 1.0 -1\n", my_name, fwd_target->GetName() );    
  }
}
