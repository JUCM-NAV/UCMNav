/***********************************************************
 *
 * File:			service.cc
 * Author:			Andrew Miga
 * Created:			June 2000
 *
 * Modification history:	June 2000
 *				Separated from responsibility
 *				definition
 *
 ***********************************************************/

#include "service.h"
#include "devices.h"
#include "xml_generator.h"

ServiceRequest::ServiceRequest( int device, const char *amt )
{
   amount[29] = 0;
   device_id = device;
   strncpy(  amount, amt, 29 );
}

void ServiceRequest::SaveRequests( Cltn<ServiceRequest *> *requests, FILE *fp )
{
   ServiceRequest *sr;
   char buffer[200];

   if( requests->Size() == 0 ) return;

   DeviceDirectory *dd = DeviceDirectory::Instance();
   PrintNewXMLBlock( fp, "service-request-spec" );

   for( requests->First(); !requests->IsDone(); requests->Next() ) {
      sr = requests->CurrentItem();
      if( sr->DeviceId() == PROCESSOR_DEVICE ) {
	 sprintf( buffer, "service-request service-type=\"pr\" request-number=\"%s\" /", PrintDescription( sr->Amount() ) );
	 PrintXML( fp, buffer );
      }
      else if( dd->IsReferenceValid( sr->DeviceId() ) ) {
	 sprintf( buffer, "service-request service-type=\"dv%d\" request-number=\"%s\" /", sr->DeviceId(), PrintDescription( sr->Amount() ) );
	 PrintXML( fp, buffer );
      }
   }

   PrintEndXMLBlock( fp, "service-request-spec" );
}
