
// callbacks for device characteristics dialogs

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "devices.h"
#include <stdlib.h>

void ShowDevices();
void UpdateDevicesList();

static int mode;
#define ADD  0
#define EDIT 1

FD_DeviceDirectory *pfddd = NULL;
static Device *current_device = NULL;
static device_type current_type;
int DDvisible;

static int DeviceDialogClose( FL_FORM *, void * );

extern void SetPromptUserForSave();

void ShowDevices()
{
   FREEZE_TOOL();

   if( !pfddd ) {
      pfddd = create_form_DeviceDirectory();
      fl_set_form_atclose( pfddd->DeviceDirectory, DeviceDialogClose, NULL );
      fl_set_browser_fontsize( pfddd->DeviceBrowser, FL_NORMAL_SIZE );
      fl_set_choice_fontsize( pfddd->DeviceTypeChoice, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfddd->DeviceTypeChoice, FL_BOLD_STYLE );
      fl_addto_choice( pfddd->DeviceTypeChoice, " Processor | Disk | DSP | Service " );
      fl_set_tabstop( "aaaaaaaaaaaaaaaa" );
   }

   UpdateDevicesList();   
   
   fl_show_form( pfddd->DeviceDirectory, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, " Device Characteristics " );
   DDvisible = 1;
}

void DeviceBrowser_cb(FL_OBJECT *, long)
{
   char op_time[20];
   DeviceDirectory *dd = DeviceDirectory::Instance();
   mode = EDIT;
   
   current_device = dd->SelectedDevice( current_type, fl_get_browser( pfddd->DeviceBrowser ) );

   if( current_device == NULL )
      return;
   
   fl_set_input( pfddd->DeviceNameInput, current_device->Name() );
   fl_set_input( pfddd->DeviceCharacteristics, current_device->Characteristics() );   
   sprintf( op_time, "%e", current_device->OpTime() );
   fl_set_input( pfddd->OpTime, op_time );
   fl_set_object_label( pfddd->EditDeviceBox, "Characteristics of Selected Device" );
   ENABLE_WIDGET( pfddd->EditDevice );
   ENABLE_WIDGET( pfddd->DDDeleteButton );
}

void DeviceTypeChoice_cb(FL_OBJECT *, long)
{

   switch( fl_get_choice( pfddd->DeviceTypeChoice ) ) {

   case 1:
      current_type = PROCESSOR;
      break;
   case 2:
      current_type = DISK;
      break;
   case 3:
      current_type = DSP;
      break;
   case 4:
      current_type = OTHER;
      break;
   }

   UpdateDevicesList();

}

void UpdateDevicesList()
{
   DeviceDirectory *dd = DeviceDirectory::Instance();
   dd->ListDevices( current_type );

   // deactivate editing buttons initially and clear inputs
   DISABLE_WIDGET( pfddd->EditDevice );
   DISABLE_WIDGET( pfddd->DDDeleteButton );
   fl_set_input( pfddd->DeviceNameInput, "" );
   fl_set_input( pfddd->DeviceCharacteristics, "" );   
   fl_set_input( pfddd->OpTime, "" );
}

void AddDeviceButton_cb(FL_OBJECT *, long)
{
   mode = ADD;
   current_device = NULL;


   fl_set_object_label( pfddd->EditDeviceBox, "Enter New Device Characteristics" );
   
   // reactivate editing buttons and clear inputs
   ENABLE_WIDGET( pfddd->EditDevice );
   fl_set_input( pfddd->DeviceNameInput, "" );
   fl_set_input( pfddd->DeviceCharacteristics, "" );
   fl_set_input( pfddd->OpTime, "" );

   SetPromptUserForSave();
}

void DDRemoveDialogButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfddd->DeviceDirectory );
   DDvisible = 0;
   UNFREEZE_TOOL();
   SetPromptUserForSave();
}

void DDAcceptButton_cb(FL_OBJECT *, long)
{
   DeviceDirectory *dd = DeviceDirectory::Instance();

   if( mode == ADD ) {
      
      dd->AddDevice( current_type, fl_get_input( pfddd->DeviceNameInput ), atof( fl_get_input( pfddd->OpTime ) ),
		     fl_get_input( pfddd->DeviceCharacteristics ) );
      fl_set_object_label( pfddd->EditDeviceBox, "Characteristics of Selected Device" );
   }
   else {   // mode == EDIT
      current_device->Name( fl_get_input( pfddd->DeviceNameInput ) );
      current_device->Characteristics( fl_get_input( pfddd->DeviceCharacteristics ) );
      current_device->OpTime( atof( fl_get_input( pfddd->OpTime ) ) );
   }

   UpdateDevicesList();
   SetPromptUserForSave();
   
}

void DDCancelButton_cb(FL_OBJECT *, long)
{
   char op_time[20];

   if( mode == ADD ) {
      // deactivate editing buttons and clear inputs
      fl_set_input( pfddd->DeviceNameInput, "" );
      fl_set_input( pfddd->DeviceCharacteristics, "" );
      fl_set_input( pfddd->OpTime, "" );
      fl_set_object_label( pfddd->EditDeviceBox, "Characteristics of Selected Device" );
      DISABLE_WIDGET( pfddd->EditDevice );
   }
   else {  // mode == EDIT
      fl_set_input( pfddd->DeviceNameInput, current_device->Name() );
      fl_set_input( pfddd->DeviceCharacteristics, current_device->Characteristics() );   
      sprintf( op_time, "%f", current_device->OpTime() );
      fl_set_input( pfddd->OpTime, op_time );
   }
}

void DDDeleteButton_cb(FL_OBJECT *, long)
{
   DeviceDirectory *dd = DeviceDirectory::Instance();
   dd->DeleteDevice( current_device );
   current_device = NULL;
   UpdateDevicesList();
}

int DeviceDialogClose( FL_FORM *, void * )
{
   DDRemoveDialogButton_cb( 0, 0 );
   return( FL_IGNORE );
}
