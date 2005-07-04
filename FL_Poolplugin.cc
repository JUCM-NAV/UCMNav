
// callbacks for stub management functions

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "component.h"
#include "stub.h"
#include "map.h"
#include "display.h"

extern DisplayManager *display_manager;

extern void DrawScreen();

FD_PoolPluginChoicePopup *pfdppcp = NULL;
extern FD_CompAttrPopup *pfdcap;

static design_plugin plugins[50];
int numplugins;

static int PoolPluginSelectorDialogClose( FL_FORM *, void * );

extern Component *active_component;

extern void SetPromptUserForSave();

typedef Cltn<int> IntList;
static IntList *selected_maps;

void LaunchPoolPluginChoice( )
{
   display_manager->Submaps( plugins, numplugins );

   FREEZE_TOOL();
   fl_deactivate_form( pfdcap->CompAttrPopup );
   
   if( !selected_maps )
      selected_maps = new IntList;
   else {
      while( !selected_maps->is_empty() )
	 selected_maps->Detach();
   }

   if( !pfdppcp )
   {
      pfdppcp = create_form_PoolPluginChoicePopup();
      fl_set_browser_fontsize( pfdppcp->PoolPluginSelector, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdppcp->PoolPluginSelector, FL_BOLD_STYLE );
      fl_set_form_atclose( pfdppcp->PoolPluginChoicePopup, PoolPluginSelectorDialogClose, NULL );
   }

   fl_clear_browser( pfdppcp->PoolPluginSelector );

   for( int i = 0 ; i < numplugins; i++ ) {
      fl_addto_browser( pfdppcp->PoolPluginSelector, plugins[i].plugin->MapLabel() );
      if( active_component->PoolPluginsInclude( plugins[i].plugin ) == TRUE ) {      // map already in the plugin list?
	 fl_select_browser_line( pfdppcp->PoolPluginSelector, i + 1);         // select it!
	 selected_maps->Add( i );
      }
   }

      if( selected_maps->Size() == 1 ) {
	 ENABLE_WIDGET( pfdppcp->PoolPluginChoiceRenameButton );
      } else {
	 DISABLE_WIDGET( pfdppcp->PoolPluginChoiceRenameButton );
      }
   
   fl_show_form( pfdppcp->PoolPluginChoicePopup, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Choose Plugin(s)" );
}

extern "C"
void PoolPluginChoiceOKButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdppcp->PoolPluginChoicePopup );
   active_component->ClearPoolPlugins();

   for( selected_maps->First(); !selected_maps->IsDone(); selected_maps->Next() ) {
      active_component->AddPoolPlugin( plugins[ selected_maps->CurrentItem() ].plugin );
      selected_maps->RemoveCurrentItem();
   }

   UNFREEZE_TOOL();
   fl_activate_form( pfdcap->CompAttrPopup );
   SetPromptUserForSave();
}

void PoolPluginChoiceCancelButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdppcp->PoolPluginChoicePopup );

   for( selected_maps->First(); !selected_maps->IsDone(); selected_maps->Next() )
      selected_maps->RemoveCurrentItem();

   UNFREEZE_TOOL();
   fl_activate_form( pfdcap->CompAttrPopup );
}

extern "C"
void PoolPluginSelector_cb(FL_OBJECT *pflo, long)
{
   int selection = fl_get_browser( pfdppcp->PoolPluginSelector );
   
   if( selected_maps->Size() == 0)
      selected_maps->Add( selection - 1 );
   else {
      if( selection < 0 )
	 selected_maps->Remove( abs(selection)-1 ); // remove deselected line
      else
	 selected_maps->Add( selection - 1 );
   }

   if( selected_maps->Size() == 1 ) {
      ENABLE_WIDGET( pfdppcp->PoolPluginChoiceRenameButton );
   } else {
      DISABLE_WIDGET( pfdppcp->PoolPluginChoiceRenameButton );
   }
   
}

extern "C"
void PoolPluginChoiceRenameButton_cb( FL_OBJECT *, long )
{
   Map *selected_map = plugins[ selected_maps->GetFirst() ].plugin;
   const char *entered_label = fl_show_input( "Enter the new name for the plugin. ", selected_map->MapLabel() );

   if( entered_label )     //  if == NULL, then the Cancel button was hit and we don't want to change anything
   {   
      fl_replace_browser_line( pfdppcp->PoolPluginSelector, selected_maps->GetFirst() + 1, entered_label );
      strcpy( selected_map->MapLabel(), entered_label );
      SetPromptUserForSave();
   }
}

int PoolPluginSelectorDialogClose( FL_FORM *, void * )
{
   PoolPluginChoiceCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}

