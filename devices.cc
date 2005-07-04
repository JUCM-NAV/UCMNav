/***********************************************************
 *
 * File:			devices.cc
 * Author:			Andrew Miga
 * Created:			November 1997
 *
 * Modification history:
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "devices.h"
#include "defines.h"
#include "xml_generator.h"
#include "print_mgr.h"
#include <stdlib.h>

static DeviceDirectory * SoleDeviceDirectory; // class variable for sole instance of directory object


extern FD_DeviceDirectory *pfddd;
extern void ShowDevices();

int Device::number_devices = 0;
int Device::max_id = -1;

static char device_types[][10]= {

   "processor",
   "disk",
   "dsp",
   "other" };

static char device_names[][26]= {

   "Processors",
   "Hard Disks",
   "Digital Signal Processors",
   "Other Hardware Devices" };

Device::Device( int device_id, device_type type, const char *name, float optime, const char *ct )
{
   dtype = type;
   op_time = optime;
   device_name[0] = 0;
   device_name[19] = 0;
   strncpy( device_name, name, 19 );
   characteristics = strdup( ct );
   device_number = device_id;

   if( device_id > max_id ) {  // avoid reallocating the same identifier
      max_id = device_id;
      number_devices = max_id+1;
   }
}

Device::Device( device_type type, const char *name, float optime, const char *ct )
{
   dtype = type;
   op_time = optime;
   device_name[0] = 0;
   device_name[19] = 0;
   strncpy( device_name, name, 19 );
   characteristics = strdup( ct );
   device_number = number_devices++;
}

Device::~Device()
{
   if( characteristics )
      free( characteristics );
}

const char * Device::Type()
{
   return( device_types[dtype] );
}

void Device::Characteristics( const char *new_characteristics )
{
   if( characteristics )
      free( characteristics );

   characteristics = strdup( new_characteristics );
}

DeviceDirectory * DeviceDirectory::Instance()
{
   if( SoleDeviceDirectory == NULL )
      SoleDeviceDirectory = new DeviceDirectory();

   return( SoleDeviceDirectory );
}

void DeviceDirectory::ClearDirectory()
{
   while( !devices.is_empty() )
      delete devices.Detach();

   Device::ResetCount();
}

void DeviceDirectory::ViewDevices()
{
   ShowDevices();  // cause device editor dialog to be displayed
}

void DeviceDirectory::ListDevices( device_type type )
{
   char buffer[100], fl[50], *ct, *null_index, *bp;
   Device *device;
   
   fl_clear_browser( pfddd->DeviceBrowser );

   for( devices.First(); !devices.IsDone(); devices.Next() ) {

      device = devices.CurrentItem();
      if( device->dtype == type ) {

	 null_index = NULL;
	 null_index = strstr( device->characteristics, "\n" );

	 if( (null_index == 0) || (null_index == (device->characteristics + strlen(device->characteristics)-1) )) {
	    sprintf( buffer, "%s\t%e\t%s", device->device_name, device->op_time, device->characteristics );
	    if( null_index != 0 ) buffer[strlen(buffer)-1]=0; // remove terminating null
	 }
	 else {
	    
	    bp = fl;
	    ct = device->characteristics;
	    while( ct != null_index )
	       *(bp++) = *(ct++);
	    *bp = 0;

	    sprintf( buffer, "%s\t%e\t%s ...", device->device_name, device->op_time, fl );
	 }
	     
	 fl_addto_browser( pfddd->DeviceBrowser, buffer );
      }
   }

}

void DeviceDirectory::ListProcessors( void *browser )
{
   fl_clear_browser( (FL_OBJECT *)browser );

   for( devices.First(); !devices.IsDone(); devices.Next() ) {
      if( devices.CurrentItem()->DeviceType() == PROCESSOR )
	 fl_addto_browser( (FL_OBJECT *)browser, devices.CurrentItem()->Name() );
   }
}

Device * DeviceDirectory::SelectedDevice( device_type type, int index )
{
   int count = 0;
   Device *device;

   for( devices.First(); !devices.IsDone(); devices.Next() ) {
      device = devices.CurrentItem();
      if( device->dtype == type ) {
	 count++;
	 if( count == index )
	    return( device );
      }
   }

   return( NULL );
}

int DeviceDirectory::DeviceId( device_type type, int selection )
{
   int count = 0;
   Device *device;

   for( devices.First(); !devices.IsDone(); devices.Next() ) {
      device = devices.CurrentItem();
      if( device->dtype == type ) {
	 count++;
	 if( count == selection )
	    return( device->Number() );
      }
   }

   return( NA );
}

int DeviceDirectory::DeviceIndex( device_type type, int reference )
{
   int count = 0;
   Device *device;

   for( devices.First(); !devices.IsDone(); devices.Next() ) {
      device = devices.CurrentItem();
      if( device->dtype == type ) {
	 count++;
	 if( device->Number() == reference )
	    return( count );
      }
   }

   return( 0 );
}

Device * DeviceDirectory::DeviceWithName( const char *device_name )
{
   for( devices.First(); !devices.IsDone(); devices.Next() ) {
      if( strcmp( devices.CurrentItem()->Name(), device_name ) == 0 )
	 return( devices.CurrentItem() );
   }

   return( NULL );  // if nothing was found
}

const char * DeviceDirectory::DeviceName( int device_id )
{
   for( devices.First(); !devices.IsDone(); devices.Next() ) {
      if( devices.CurrentItem()->Number() == device_id )
	 return( devices.CurrentItem()->Name() );
   }

   return( NULL );
}

device_type DeviceDirectory::DeviceType( int device_id )
{
   if( device_id == PROCESSOR_DEVICE )
      return( PROCESSOR );

   for( devices.First(); !devices.IsDone(); devices.Next() ) {
      if( devices.CurrentItem()->Number() == device_id )
	 return( devices.CurrentItem()->DeviceType() );
   }

   std::cout << "Warning: problem in DeviceDirectory::DeviceType... should not get here!";
   return( OTHER );  // DA: Added August 2004
}


void DeviceDirectory::SaveXML( FILE *fp )
{
   Device *device;
   char buffer[300];
   
   if( devices.Size() != 0 ) {

      PrintNewXMLBlock( fp, "device-directory" );

      for( devices.First(); !devices.IsDone(); devices.Next() ) {
	 device = devices.CurrentItem();
	 sprintf( buffer, "<device device-id=\"dv%d\" device-type=\"%s\" device-name=\"%s\"",
		  device->Number(), device->Type(), PrintDescription( device->Name() ) );
	 PrintXMLText( fp, buffer );
	 fprintf( fp, " description=\"%s\" op-time=\"%e\"/>\n", PrintDescription( device->Characteristics() ), device->OpTime() );
      }
      
      PrintEndXMLBlock( fp, "device-directory" );
      LinebreakXML( fp );
   }
}

void DeviceDirectory::SaveCSMXML( FILE *fp )
{
   Device *device;
   char buffer[300];
   
   if( devices.Size() != 0 ) {

      //PrintNewXMLBlock( fp, "active-resource" );

      for( devices.First(); !devices.IsDone(); devices.Next() ) {
	 device = devices.CurrentItem();
	 sprintf( buffer, "<ProcessingResource id=\"dv%d\" device-type=\"%s\" name=\"%s\"",
		  device->Number(), device->Type(), PrintDescription( device->Name() ) );
	 PrintXMLText( fp, buffer );
	 fprintf( fp, " description=\"%s\" op_time=\"%e\"/>\n", PrintDescription( device->Characteristics() ), device->OpTime() );
      }
      
      //PrintEndXMLBlock( fp, "active-resource" );
      LinebreakXML( fp );
   }
}

void DeviceDirectory::GeneratePostScriptDescription( FILE *ps_file )
{
   Device *device;
   bool subtype_exists;
   
   if( devices.Size() == 0 )
      return;

   PrintManager::PrintHeading( "Hardware Devices" );
   fprintf( ps_file, "[/Title(Hardware Devices) /OUT pdfmark\n" );

   for( int i = 0; i < 4; i++ ) {

      subtype_exists = FALSE;
      
      for( devices.First(); !devices.IsDone(); devices.Next() ) {

	 device = devices.CurrentItem();
	 if( (int)device->dtype == i ) {
	    if( !subtype_exists ) {
	       fprintf( ps_file, "%d bol (%s ) P L\n", PrintManager::text_font_size, device_names[i] );
	       subtype_exists = TRUE;
	    }
	    fprintf( ps_file, "1 IN ID %d rom (Device Name -  ) S %d bol (%s ) P OD\n",
		     PrintManager::text_font_size, PrintManager::text_font_size, device->device_name );
	    fprintf( ps_file, "2 IN ID %d rom ( - %s ) P OD\n", PrintManager::text_font_size, device->characteristics );
	    fprintf( ps_file, "1 IN ID %d rom (Operation Time -  ) S %d bol (%e ) P OD L\n",
		     PrintManager::text_font_size, PrintManager::text_font_size, device->op_time );
	 }
      }
      if( subtype_exists )
	 fprintf( ps_file, "L\n" );
   }
}

bool DeviceDirectory::IsReferenceValid( int device_id )
{
   for( devices.First(); !devices.IsDone(); devices.Next() ) {
      if( devices.CurrentItem()->Number() == device_id )
	 return( TRUE );
   }
      
   return( FALSE );
}

device_type DeviceDirectory::DeviceType( const char *type_string )
{
   int dt,
      len = strlen( type_string );

   switch( len ) {
   case 9:
      dt = 0; break;
   case 4:
      dt = 1; break;
   case 3:
      dt = 2; break;
   case 5:
      dt = 3; break;
   }
   return( (device_type)dt );
}


void DeviceDirectory::OutputDXL( std::ofstream &dxl_file )
{
   Device* device = NULL;
    
   if( devices.Size() != 0 ) {
      for( devices.First(); !devices.IsDone(); devices.Next() ) {
	     device = devices.CurrentItem();
         dxl_file << "device( " << "\"d" << device->Number() << "\", ";
         dxl_file << "\"" << device->Name() << "\", " << "\"" << device->Characteristics() << "\", ";
         dxl_file << "\"" << device->OpTime() << "\" )\n";
      }
   }
}
