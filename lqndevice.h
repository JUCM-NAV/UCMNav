#ifndef LQNDEVICE_H
#define LQNDEVICE_H

#include "defines.h"
#include "lqndefs.h"

class LqnDevice {
public:
  LqnDevice( const char* name, int id );  // default constructor
  LqnDevice( const char* name, int id,  double speed_factor );  // constructor with a speed factor
  ~LqnDevice();  // destructor

  const char* GetName() { return my_name; }
  const int GetId() { return my_id; }

  void SetInfinite() { am_infinite = true; }
  bool IsInfinite() { return am_infinite; }

  void Print();  // prints LqnDevice information to the screen
  void FilePrint( FILE* outfile );  // prints LqnDevice information to the specified file

protected:
  char my_name[DEVICE_NAME_LENGTH];  // device name
  int my_id;  // device id
  double my_speed_factor;  // device speed factor

  bool am_infinite;
  bool print;
};

#endif
