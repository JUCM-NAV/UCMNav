
// callbacks for responsibility data store usage dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "responsibility.h"
#include "resp_figure.h"
#include "data.h"

void EditDataStoreUsage( Responsibility *resp );
void UpdateDataReferences();

FD_DataStoreUsage *pfddsu = NULL;
extern FD_EditRespPopup *pfderp;
extern void SetPromptUserForSave();

static Responsibility *responsibility = NULL;
static Cltn<DataReference *> *references;
static int current_data_reference;
static bool delete_inactive, add_inactive;

static int DUDialogClose( FL_FORM *, void * );

void EditDataStoreUsage( Responsibility *resp )
{
   char buffer[100];
   DataReference *dr;
   DataStoreDirectory *dsd = DataStoreDirectory::Instance();
   
   responsibility = resp;
   references = responsibility->DataReferences();
   
   fl_deactivate_form( pfderp->EditRespPopup );  // freeze main responsibility popup

   if( !pfddsu ) {

      pfddsu = create_form_DataStoreUsage();
      fl_set_form_atclose( pfddsu->DataStoreUsage, DUDialogClose, NULL );
      fl_set_browser_fontsize( pfddsu->DUBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfddsu->DUBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfddsu->AvailableData, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfddsu->AvailableData, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfddsu->AccessModes, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfddsu->AccessModes, FL_BOLD_STYLE );

   }

   sprintf( buffer, "The data stores accessed by responsibility %s are:",
	    fl_get_input( pfderp->Name ) );
   fl_set_object_label( pfddsu->Prompt, buffer );

   // check if all data stores referenced by this responsibility are still valid
   // if not remove from list

   for( references->First(); !references->IsDone(); references->Next() ) {
      dr = references->CurrentItem();
      if( !dsd->IsReferenceValid( dr )) {
	 references->RemoveCurrentItem();
	 delete dr;
      }
   }
   
   dsd->DisplayAccessModes();
   UpdateDataReferences();

   fl_show_form( pfddsu->DataStoreUsage, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, " Layered Data Stores Referenced " );

}

void UpdateDataReferences()
{
   char buffer[100];
   DataReference *dr;
   DataStoreDirectory *dsd = DataStoreDirectory::Instance();
   
   fl_freeze_form( pfddsu->DataStoreUsage );
   fl_clear_browser( pfddsu->DUBrowser );
   
   fl_set_object_lcol( pfddsu->DUDeleteButton, FL_INACTIVE );
   fl_deactivate_object( pfddsu->DUDeleteButton );
   fl_set_object_lcol( pfddsu->DUAddButton, FL_INACTIVE );
   fl_deactivate_object( pfddsu->DUAddButton );

   for( references->First(); !references->IsDone(); references->Next() ) {
      dr = references->CurrentItem();
      // Jul2005 gM: replaced "*-" with "-*" to get rid of GCC warning
	  sprintf( buffer, "@f@s@b%-*s%s", 25, dsd->Item( DATA_STORES, dr->Data() ), dsd->Item( ACCESS_MODES, dr->Access() ));
      fl_addto_browser( pfddsu->DUBrowser, buffer );
   }

   dsd->DisplayDataStores( responsibility );
   
   fl_unfreeze_form( pfddsu->DataStoreUsage );
   delete_inactive = TRUE;
   add_inactive = TRUE;
   
}

void DUAddButton_cb(FL_OBJECT *, long)
{
   DataStoreDirectory *dsd = DataStoreDirectory::Instance();   
   responsibility->AddDataReference
      ( dsd->DSItem( DATA_STORES, fl_get_browser( pfddsu->AvailableData ) )->Identifier(),
        dsd->DSItem( ACCESS_MODES, fl_get_browser( pfddsu->AccessModes ) )->Identifier() ); 
   UpdateDataReferences();
   SetPromptUserForSave();
}

void DUDeleteButton_cb(FL_OBJECT *, long)
{
   DataReference *dr = references->Get( current_data_reference );
   responsibility->DeleteDataReference( dr );
   UpdateDataReferences();
   SetPromptUserForSave();
}

void DUBrowser_cb(FL_OBJECT *, long)
{
   current_data_reference = fl_get_browser( pfddsu->DUBrowser );

   if( delete_inactive ) {
      fl_activate_object( pfddsu->DUDeleteButton );
      fl_set_object_lcol( pfddsu->DUDeleteButton, FL_BLACK );
      delete_inactive = FALSE;
   }
}

void DURemoveDialogButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfddsu->DataStoreUsage ); 
   fl_activate_form( pfderp->EditRespPopup ); // reactivate main responsibility popup
}

void AvailableData_cb(FL_OBJECT *, long)
{
   if( fl_get_browser( pfddsu->AccessModes ) == 0 )
      return;

   if( add_inactive ) {
      fl_activate_object( pfddsu->DUAddButton );
      fl_set_object_lcol( pfddsu->DUAddButton, FL_BLACK );
      add_inactive = FALSE;
   }

}

void AccessModes_cb(FL_OBJECT *, long)
{
   if( fl_get_browser( pfddsu->AvailableData ) == 0 )
      return;
   
   if( add_inactive ) {
      fl_activate_object( pfddsu->DUAddButton );
      fl_set_object_lcol( pfddsu->DUAddButton, FL_BLACK );
      add_inactive = FALSE;
   }

}

int DUDialogClose( FL_FORM *, void * )
{
   DURemoveDialogButton_cb( 0, 0 );
   return( FL_IGNORE );
}
