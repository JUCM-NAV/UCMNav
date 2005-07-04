#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lqndevice.h"


// default constructor
LqnDevice::LqnDevice( const char* name, int id )
{
  my_name[0] = 0;  // initialize the name
  my_name[DEVICE_NAME_LENGTH - 1] = 0;
  strcpy( my_name, name );
  my_id = id;
  my_speed_factor = 1.0;
  am_infinite = false;
  print = true;
  if( print ) {
    printf( "LqnDevice '%s' (id %d) with speed factor %lf created.\n", my_name, my_id, my_speed_factor );
  }
}


// constructor with speed factor
LqnDevice::LqnDevice( const char* name, int id, double speed_factor )
{
  my_name[0] = 0;  // initialize the name
  my_name[DEVICE_NAME_LENGTH - 1] = 0;
  strcpy( my_name, name );
  my_id = id;
  my_speed_factor = speed_factor;
  am_infinite = false;
  print = true;
  if( print ) {
    printf( "LqnDevice '%s' (id %d) with speed factor %lf created.\n", my_name, my_id, my_speed_factor );
  }
}


// destructor
LqnDevice::~LqnDevice()
{
  printf( "Device '%s' deleted.\n", my_name, my_id );
}


// prints LqnDevice information to the screen
void LqnDevice::Print()
{
  // if infinite processor
  if( am_infinite ) {
    printf( "p %s f i R %lf\n", my_name, my_speed_factor );
  }
  // else not an infinite processor
  else {
    printf( "p %s f R %lf\n", my_name, my_speed_factor );
  }
}


// prints LqnDevice information to the specified file
void LqnDevice::FilePrint( FILE* outfile )
{
  // check if infinite processor
  if( am_infinite ) {
    fprintf( outfile, "p %s f i R %lf\n", my_name, my_speed_factor );
  }
  // else not an infinite processor
  else {
    fprintf( outfile, "p %s f R %lf\n", my_name, my_speed_factor );
  }
}
