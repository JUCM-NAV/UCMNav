/***********************************************************
 *
 * File:			service.h
 * Author:			Andrew Miga
 * Created:			June 2000
 *
 * Modification history:	June 2000
 *				Separated from responsibility
 *				definition
 *
 ***********************************************************/

#ifndef SERVICE_H
#define SERVICE_H

#include "collection.h"
#include <string.h>
#include <stdio.h>

class ServiceRequest {

public:

   ServiceRequest( int device, const char *amt );
   ~ServiceRequest() {}

   int DeviceId() { return( device_id ); }
   void DeviceId( int device ) { device_id = device; }
   char * Amount() { return( amount ); }
   void Amount( const char *amt ) { strncpy( amount, amt, 29 ); }

   static void SaveRequests( Cltn<ServiceRequest *> *requests, FILE *fp );
   
private:

   int device_id;   // integer reference to device stores in DeviceDirectory
   char amount[30]; // specification of numerical amount or variable reference
   
};

#endif
