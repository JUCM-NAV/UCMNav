/***********************************************************
 *
 * File:			devices.h
 * Author:			Andrew Miga
 * Created:			November 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef DEVICES_H
#define DEVICES_H

#include "collection.h"
#include <string.h>
#include <stdio.h>
#include <fstream>

typedef enum { PROCESSOR, DISK, DSP, OTHER } device_type;

#define PROCESSOR_DEVICE -1

class DeviceDirectory;

class Device { // data class used by DeviceDirectory

   friend class DeviceDirectory;

public:

   Device( int device_id, device_type type, const char *name, float optime, const char *ct ); // constructor for file loading
   Device( device_type type, const char *name, float optime, const char *ct ); // constructor, accept user inputted strings as parameters
   ~Device();
   const char * Name() { return( device_name ); } // access methods for elements
   void Name( const char *new_name ) { strncpy( device_name, new_name, 19 ); }
   const char * Characteristics() { return( characteristics ); }
   void Characteristics( const char *new_characteristics );
   double OpTime() { return( op_time ); }
   void OpTime( double new_time ) { op_time = new_time; }
   int Number() { return( device_number ); }
   const char * Type();
   device_type DeviceType() { return( dtype ); }

   static void ResetCount() { number_devices = 0; max_id = -1; } // resets counters for elements
   static int NumberDevices() { return( number_devices ); } // Apr2005 gM: required for map import to avoid conflict with constant IDs
  
private:

   int device_number; // identifier for device
   device_type dtype; // enumerated variable for type of device
   char device_name[20], *characteristics; // user entered strings for device name and characteristics
   double op_time; // processing time for one operation, user given

   static int number_devices, max_id;
};


class DeviceDirectory {

public:

   ~DeviceDirectory() {}
   static DeviceDirectory * Instance(); // returns pointer to sole instance, creates instance on first call

   void ViewDevices(); // invokes device dialog
   void ListDevices( device_type type ); // displays the devices of a given type in dialog list box
   void ListProcessors( void *browser );
   int DeviceId( device_type type, int selection );
   int DeviceIndex( device_type type, int reference );
   Device * SelectedDevice( device_type type, int index ); // returns the device object corresponding to index
   void AddDevice( device_type type, const char *name, double optime, const char *ct ) // methods to add, delete device objects from directory
   { devices.Add( new Device( type, name, optime, ct ) ); }
   void AddDevice( int device_id, device_type type, const char *name, double optime, const char *ct ) // methods to add, delete device objects from directory
   { devices.Add( new Device( device_id, type, name, optime, ct ) ); }
   void DeleteDevice( Device *d ) { devices.Delete( d ); }
   void ClearDirectory(); // removes all elements from directory
   bool IsReferenceValid( int device_id ); // determines if an element still exists
   void SaveXML( FILE *fp ); // saves the directory contents in XML format
   void SaveCSMXML( FILE *fp ); // saves the directory contents in CSM XML format
   void GeneratePostScriptDescription( FILE *ps_file );
   
   Cltn<Device *> * DeviceList() { return( &devices ); } // returns the list of devices
   Device * DeviceWithName( const char *device_name ); // returns the device object with the given name
   const char * DeviceName( int device_id ); // returns the device name of the device with number device_id
   device_type DeviceType( const char *type_string ); // convert string into enumerated type
   device_type DeviceType( int device_id ); // determine type of device with given device id

   void OutputDXL( std::ofstream &dxl_file );
   
private:
   
   DeviceDirectory() {} // constructor, private to enforce singleton pattern

   Cltn<Device *> devices; // list of devices

};

#endif
