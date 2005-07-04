
// callbacks for service request dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "service.h"
#include "devices.h"

#define CANCEL 1

void UpdateServiceRequests();

FD_ServiceRequests *pfdsr = NULL;
extern FD_EditRespPopup *pfderp;
extern void SetPromptUserForSave();

static Cltn<ServiceRequest *> *service_requests;
static FL_FORM *parent_form;
static device_type service_category;
static ServiceRequest *csr;
static const char *processor_name;

static int SRDialogClose( FL_FORM *, void * );

void EditServiceRequests( Cltn<ServiceRequest *> *requests, FL_FORM *parent, const char *element, const char *processor )
{ // display dialog and fill with data
   char title[150];

   service_requests = requests;
   parent_form = parent;
   processor_name = processor;

   fl_deactivate_form( parent_form ); // freeze main  popup ( responsibility or stub )

   if( !pfdsr ) {
      pfdsr = create_form_ServiceRequests();
      fl_set_form_atclose( pfdsr->ServiceRequests, SRDialogClose, NULL );
      fl_set_browser_fontsize( pfdsr->SRBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsr->SRBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdsr->ServiceType, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsr->ServiceType, FL_BOLD_STYLE );
      fl_set_choice_fontsize( pfdsr->ServiceCategoryChoice, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfdsr->ServiceCategoryChoice, FL_BOLD_STYLE );
      fl_addto_choice( pfdsr->ServiceCategoryChoice, " Processor | Disk | DSP | Service " );
      fl_set_choice_text( pfdsr->ServiceCategoryChoice, " Processor " );
   }

   sprintf( title, "Service Requests by %s", element );

   UpdateServiceRequests();
   ServiceCategoryChoice_cb( 0, 0 ); // update list of services

   fl_show_form( pfdsr->ServiceRequests, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, title );
}

void UpdateServiceRequests()
{
   char buffer[100];
   ServiceRequest *sr;
   const char *device_name;
   DeviceDirectory *dd = DeviceDirectory::Instance();

   fl_freeze_form( pfdsr->ServiceRequests );
   fl_set_input( pfdsr->RequestNumberInput, "" );
   fl_clear_browser( pfdsr->SRBrowser );
   csr = NULL; // set current service request pointer to null

   for( service_requests->First(); !service_requests->IsDone(); service_requests->Next() ) {
      sr = service_requests->CurrentItem();

      if( sr->DeviceId() != PROCESSOR_DEVICE )
	 device_name = dd->DeviceName( sr->DeviceId() );
      else
	 device_name = processor_name;

      if( device_name ) {
	 sprintf( buffer, "@f@b%s\t%s", device_name, sr->Amount() );
	 fl_addto_browser( pfdsr->SRBrowser, buffer );
      }
      else {
	 service_requests->RemoveCurrentItem();
	 delete sr;
      }
   }

   // deactivate delete and cancel buttons
   fl_deactivate_object( pfdsr->SRCancelButton );
   fl_set_object_lcol( pfdsr->SRCancelButton, FL_INACTIVE );
   fl_deactivate_object( pfdsr->SRDeleteButton );
   fl_set_object_lcol( pfdsr->SRDeleteButton, FL_INACTIVE );
   
   fl_unfreeze_form( pfdsr->ServiceRequests );
}

void SRRemoveDialogButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdsr->ServiceRequests );
   fl_activate_form( parent_form ); // reactivate main responsibility popup
}

void SRBrowser_cb(FL_OBJECT *, long mode )
{
   int choice, id, count = 0;

   choice = fl_get_browser( pfdsr->SRBrowser );
   if( choice <= 0 ) return;
   
   DeviceDirectory *dd = DeviceDirectory::Instance();
   Device *device;
   Cltn<Device *> *devices = dd->DeviceList();

   fl_freeze_form( pfdsr->ServiceRequests );

   csr = service_requests->Get( choice );
   id = csr->DeviceId();

   if( mode != CANCEL ) {

      device_type new_category = dd->DeviceType( csr->DeviceId() );

      if( new_category != service_category ) {

	 service_category = new_category;
	 fl_clear_browser( pfdsr->ServiceType );
	 fl_set_choice( pfdsr->ServiceCategoryChoice, ((int)service_category+1) );

	 if( service_category != PROCESSOR ) {
	    for( devices->First(); !devices->IsDone(); devices->Next() ) {
	       device = devices->CurrentItem();
	       if( device->DeviceType() == service_category ) {
		  fl_addto_browser( pfdsr->ServiceType, device->Name() );
	       }
	    }
	 } else {
	    fl_addto_browser( pfdsr->ServiceType, processor_name );
	    fl_select_browser_line( pfdsr->ServiceType, 1 );
	 }
      }

      // activate delete and cancel buttons
      fl_activate_object( pfdsr->SRCancelButton );
      fl_set_object_lcol( pfdsr->SRCancelButton, FL_BLACK );
      fl_activate_object( pfdsr->SRDeleteButton );
      fl_set_object_lcol( pfdsr->SRDeleteButton, FL_BLACK );
   }

   fl_set_input( pfdsr->RequestNumberInput, csr->Amount() );

   if( id != PROCESSOR_DEVICE ) {
      for( devices->First(); !devices->IsDone(); devices->Next() ) {
	 device = devices->CurrentItem();
	 if( device->DeviceType() == service_category ) {
	    count++;
	    if( device->Number() == id ) {
	       fl_select_browser_line( pfdsr->ServiceType, count );
	       fl_set_browser_topline( pfdsr->ServiceType, count );
	       break;
	    }
	 }
      }
   }
   
   fl_unfreeze_form( pfdsr->ServiceRequests );
}

void ServiceCategoryChoice_cb(FL_OBJECT *, long)
{
   DeviceDirectory *dd = DeviceDirectory::Instance();
   Cltn<Device *> *devices = dd->DeviceList();
   Device *device;

   int choice = fl_get_choice( pfdsr->ServiceCategoryChoice );
   if( choice == 0 ) return;
   
   service_category = (device_type)(choice-1);
   csr = NULL; // set current service request pointer to null

   fl_freeze_form( pfdsr->ServiceRequests );
   fl_clear_browser( pfdsr->ServiceType );

   if( service_category != PROCESSOR ) {
      for( devices->First(); !devices->IsDone(); devices->Next() ) {
	 device = devices->CurrentItem();
	 if( device->DeviceType() == service_category )
	    fl_addto_browser( pfdsr->ServiceType, device->Name() );
      }
   } else {
      fl_addto_browser( pfdsr->ServiceType, processor_name );
      fl_select_browser_line( pfdsr->ServiceType, 1 );
   }

   fl_set_input( pfdsr->RequestNumberInput, "" );
   fl_unfreeze_form( pfdsr->ServiceRequests );
}

void SRAcceptButton_cb(FL_OBJECT *, long)
{
   int id, selected_line;
   const char *amount = fl_get_input( pfdsr->RequestNumberInput );

   if( (selected_line = fl_get_browser( pfdsr->ServiceType )) == 0 ) return;

   if( service_category != PROCESSOR )
      id = DeviceDirectory::Instance()->DeviceId( service_category, selected_line );
   else
      id = PROCESSOR_DEVICE;

   if( csr ) { // modify selected service request
      csr->DeviceId( id );
      csr->Amount( amount );
   }
   else // create new service request
      service_requests->Add( new ServiceRequest( id, amount ) );
   
   SetPromptUserForSave();
   UpdateServiceRequests();
}

void SRDeleteButton_cb(FL_OBJECT *, long)
{
   if( csr ) {
      service_requests->Delete( csr );
      delete csr;
      UpdateServiceRequests();
   }
}

void SRCancelButton_cb(FL_OBJECT *, long)
{
   if( csr )
      SRBrowser_cb( 0, CANCEL );
   else
      fl_set_input( pfdsr->RequestNumberInput, "" );
}

int SRDialogClose( FL_FORM *, void * )
{
   SRRemoveDialogButton_cb( 0, 0 );
   return( FL_IGNORE );
}
