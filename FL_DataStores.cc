

// callbacks for data store specification functions

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "data.h"

extern void SetPromptUserForSave();

FD_DataStores *pfddsd = NULL;
int DSvisible;
static editing_mode edit_mode = DATA_STORES;

static int mode, current_index;
#define ADD  0
#define EDIT 1
#define INACTIVE 3

void UpdateDataStoresList();

static int DataDialogClose( FL_FORM *, void * );

void EditDataStores()
{

   FREEZE_TOOL();

   if( !pfddsd ) {
      pfddsd = create_form_DataStores();
      fl_set_form_atclose( pfddsd->DataStores, DataDialogClose, NULL );
      fl_set_object_label( pfddsd->DSBrowser, "Data Store Directory" );
      fl_set_browser_fontsize( pfddsd->DSBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfddsd->DSBrowser, FL_BOLD_STYLE );
      fl_set_choice_fontsize( pfddsd->DataStoreChoice, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfddsd->DataStoreChoice, FL_BOLD_STYLE );
      fl_addto_choice( pfddsd->DataStoreChoice, " Data Stores | Access Modes " );
   }

   UpdateDataStoresList();
   
   fl_show_form( pfddsd->DataStores, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, " Layered Data Store Characteristics " );
   DSvisible = 1;

}

void UpdateDataStoresList()
{
   DataStoreDirectory *dsd = DataStoreDirectory::Instance();
   dsd->List();
   mode = INACTIVE;
   
   // deactivate editing buttons initially and clear input
   fl_freeze_form( pfddsd->DataStores );
   fl_set_input( pfddsd->DSInput, "" );
   fl_set_object_label( pfddsd->DSInput, "" );
   DISABLE_WIDGET( pfddsd->InputFields );
   DISABLE_WIDGET( pfddsd->DSDeleteButton );
   fl_unfreeze_form( pfddsd->DataStores );
}

void DSChoice_cb(FL_OBJECT *pflo, long)
{
   editing_mode new_mode;
   
   switch( fl_get_choice( pflo ) ) {

   case 1:
      new_mode = DATA_STORES;
      break;
   case 2:
      new_mode = ACCESS_MODES;
      break;
   }

   if( new_mode != edit_mode ) {

      edit_mode = new_mode;
      DataStoreDirectory *dsd = DataStoreDirectory::Instance();
      dsd->SetMode( edit_mode );

      if( edit_mode == DATA_STORES )
	 fl_set_object_label( pfddsd->DSBrowser, "Data Store Directory" );
      else
	 fl_set_object_label( pfddsd->DSBrowser, "Data Store Access Modes" );
      
      UpdateDataStoresList();
   }
}

void DSBrowser_cb(FL_OBJECT *pflo, long)
{
   DataStoreDirectory *dsd = DataStoreDirectory::Instance();
   
   current_index = fl_get_browser( pflo );

   char *item = dsd->Item( current_index );

   if( mode == INACTIVE ) {
      ENABLE_WIDGET( pfddsd->InputFields );
   }
   
   if( edit_mode == DATA_STORES )
      fl_set_object_label( pfddsd->DSInput, "Edit Data Store Identifier" );
   else
      fl_set_object_label( pfddsd->DSInput, "Edit Data Access Mode" );

   ENABLE_WIDGET( pfddsd->DSDeleteButton );
   fl_set_input( pfddsd->DSInput, item );

   mode = EDIT;
  
}

void DSAddButton_cb(FL_OBJECT *, long)
{
   if( mode == INACTIVE ) {
      ENABLE_WIDGET( pfddsd->InputFields );
   }
   
   if( edit_mode == DATA_STORES )
      fl_set_object_label( pfddsd->DSInput, "Enter New Data Store Identifier" );
   else 
      fl_set_object_label( pfddsd->DSInput, "Enter New Data Access Mode" );

   fl_set_input( pfddsd->DSInput, "" );
   mode = ADD;

}

void DSCancelButton_cb(FL_OBJECT *, long)
{
   if( mode == ADD ) {
      mode = INACTIVE;
      fl_set_input( pfddsd->DSInput, "" );
      fl_set_object_label( pfddsd->DSInput, "" );
      DISABLE_WIDGET( pfddsd->InputFields );
   }
   else {
      DataStoreDirectory *dsd = DataStoreDirectory::Instance();
      fl_set_input( pfddsd->DSInput, dsd->Item( current_index ) );
   }
}

void DSDeleteButton_cb(FL_OBJECT *, long)
{
   DataStoreDirectory *dsd = DataStoreDirectory::Instance();
   dsd->DeleteItem( current_index );   
   UpdateDataStoresList();
   SetPromptUserForSave();
}

void DSAcceptButton_cb(FL_OBJECT *, long)
{
   DataStoreDirectory *dsd = DataStoreDirectory::Instance();


   if( mode == INACTIVE ) return; // kluge bug fix
   
   if( mode == ADD )
      dsd->AddItem( fl_get_input( pfddsd->DSInput ) );
   else
      dsd->EditItem( fl_get_input( pfddsd->DSInput ), current_index );

   UpdateDataStoresList();
   SetPromptUserForSave();
  
}

void DSRemoveDialog_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfddsd->DataStores );
   DSvisible = 0;
   UNFREEZE_TOOL();
}

int DataDialogClose( FL_FORM *, void * )
{
   DSRemoveDialog_cb( 0, 0 );
   return( FL_IGNORE );
}
