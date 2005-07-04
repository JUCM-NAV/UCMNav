
// callbacks for select map dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "display.h"
#include "map.h"
#include "stub.h"
#include <string.h>
#include <ctype.h>

FD_SelectMap *pfdsm = NULL;
extern DisplayManager *display_manager;
extern void SetPromptUserForSave();
extern void DrawScreen();

static int SMDialogClose( FL_FORM *, void * );
static void UpdateMapsList();
static Map **maps_list = NULL;
static int list_size = 0;
static Stub *stub = NULL;
static map_type current_type = ROOT_MAP;
static bool type_choice_disabled = FALSE;

int strncaseequal( const char* s, const char *t, int n );

void SelectMap( Stub *s )
{
   const char *prompt;
   bool update = TRUE;
   
   stub = s;
   FREEZE_TOOL();

   if( !pfdsm ) {
      pfdsm = create_form_SelectMap();
      fl_set_browser_fontsize( pfdsm->SelectMapBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsm->SelectMapBrowser, FL_BOLD_STYLE );
      fl_set_browser_dblclick_callback( pfdsm->SelectMapBrowser, SMInstallButton_cb, 0 );
      fl_addto_choice( pfdsm->MapTypeChoice, " Root Map | Plugin Map " );
      fl_set_choice_fontsize( pfdsm->MapTypeChoice, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfdsm->MapTypeChoice, FL_BOLD_STYLE );
      fl_set_form_atclose( pfdsm->SelectMap, SMDialogClose, NULL );
      fl_set_input_return( pfdsm->MapName, FL_RETURN_CHANGED );
   }

   if( stub != NULL ) {
      if( current_type != ROOT_MAP ) {
	 fl_set_choice( pfdsm->MapTypeChoice, 1 );
	 SMMapTypeChoice_cb( pfdsm->MapTypeChoice, 0 );
	 update = FALSE;
      }
      if( type_choice_disabled == FALSE ) {
	 DISABLE_WIDGET( pfdsm->MapTypeChoice );
	 type_choice_disabled = TRUE;
      }
      prompt = "Select Root Map for Transfer";
   }
   else {
      if( type_choice_disabled == TRUE ) {
	 ENABLE_WIDGET( pfdsm->MapTypeChoice );
	 type_choice_disabled = FALSE;
      }
      prompt = "Select Map";
   }

   if( update )
      UpdateMapsList();

   fl_show_form( pfdsm->SelectMap, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT,  prompt );
}

extern "C"
void SMMapTypeChoice_cb(FL_OBJECT *choice, long)
{
   map_type old_type = current_type;
   current_type = (fl_get_choice( choice ) == 1) ? ROOT_MAP : PLUGIN;
   if( current_type != old_type ) {
      UpdateMapsList();
      if( current_type == PLUGIN ) {
	 DISABLE_WIDGET( pfdsm->DeleteButton );
	 fl_set_object_label( pfdsm->ConvertTypeButton, "Convert To Root" );
      } else { // current_type == ROOT_MAP 
	 fl_set_object_label( pfdsm->ConvertTypeButton, "Convert To Plugin" );
      }
   }
}

extern "C"
void SMInstallButton_cb(FL_OBJECT *, long)
{
   int choice;
   Map *selected_map;

   if( (choice = fl_get_browser( pfdsm->SelectMapBrowser )) == 0 ) return;
   
   SMCancelButton_cb( 0, 0 );
   selected_map = maps_list[choice-1];
   
   if( stub == NULL )
      display_manager->SetMap( selected_map );
   else
      stub->InstallRootPlugin( selected_map );
}

extern "C"
void SMDeleteButton_cb(FL_OBJECT *, long)
{
   int choice;

   if( (choice = fl_get_browser( pfdsm->SelectMapBrowser )) == 0 ) return;

   Map *current_root = display_manager->CurrentTopLevelMap();

   if( current_type == PLUGIN ) { // determine if plugin should not be deleted
      if( !maps_list[choice-1]->IsNotReferenced() )
	 return;
   }

   delete maps_list[choice-1];

   if( maps_list[choice-1] == current_root )
      display_manager->SetMap( (choice == 1) ? maps_list[1] : maps_list[0] );

   SetPromptUserForSave();
   UpdateMapsList();
}

extern "C"
void SMRenameButton_cb(FL_OBJECT *, long)
{
   int choice;
   
   if( (choice = fl_get_browser( pfdsm->SelectMapBrowser )) == 0 ) return;

   const char *entered_label = display_manager->UniqueMapName( "Enter the new name for the map. ", maps_list[choice-1]->MapLabel() );
   if( entered_label )     //  if == NULL, then the Cancel button was hit and we don't want to change anything
   {
      display_manager->RenameMap( maps_list[choice-1], entered_label );
      SetPromptUserForSave();
      UpdateMapsList();
   }
}

extern "C"
void ConvertTypeButton_cb(FL_OBJECT *, long)
{
   int choice;

   if( (choice = fl_get_browser( pfdsm->SelectMapBrowser )) == 0 ) return;
   if( current_type == ROOT_MAP ) {
      if( fl_get_browser_maxline( pfdsm->SelectMapBrowser ) == 1 )
	 return;
   }

   maps_list[choice-1]->SetMapType( (current_type == PLUGIN) ? ROOT_MAP : PLUGIN );
   UpdateMapsList();
}

extern "C"
void SMCancelButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdsm->SelectMap );
   UNFREEZE_TOOL();
}

extern "C"
void SelectMapBrowser_cb(FL_OBJECT *, long)
{
   int choice;

    if( (choice = fl_get_browser( pfdsm->SelectMapBrowser )) == 0 ) return;

  // enable Install button as something was selected
   ENABLE_WIDGET( pfdsm->InstallButton );

   if( stub != NULL ) return; // activate install button only for root->plugin transfer
   
    // enable Rename button as something was selected  
   ENABLE_WIDGET( pfdsm->RenameButton );

   if( current_type == ROOT_MAP ) { // enable Delete button if mode is root and at least two root maps exist
      if( fl_get_browser_maxline( pfdsm->SelectMapBrowser ) > 1 ) {
	 ENABLE_WIDGET( pfdsm->DeleteButton );
	 ENABLE_WIDGET( pfdsm->ConvertTypeButton ); // for conversion of root to plugin
      }
   } else { // current_type == PLUGIN
      if( maps_list[choice-1]->IsNotReferenced() ) {
	 ENABLE_WIDGET( pfdsm->DeleteButton ); 
      } else {
	 DISABLE_WIDGET( pfdsm->DeleteButton );
      }
      ENABLE_WIDGET( pfdsm->ConvertTypeButton ); // for conversion of plugin to root
   }

   fl_set_input( pfdsm->MapName, maps_list[choice-1]->MapLabel() );
}

extern "C"
void MapName_cb(FL_OBJECT *index, long)
{
   Map *current_map, *current_root;
   Cltn<Map *> *map_pool = display_manager->Maps();
   int i, len, highlight_line;
   const char *index_string = fl_get_input( index );

   len = strlen( index_string );

   if( len == 0 ) {
      fl_set_browser_topline( pfdsm->SelectMapBrowser, 1 );
      return;
   }

   current_root = display_manager->CurrentTopLevelMap();

   for( i = 0; i < len; i++ ) {
      highlight_line = 1;
      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
	 current_map = map_pool->CurrentItem();
	 if( current_map->MapType() != current_type )
	    continue;
	 if( !display_manager->MapViewable( NULL, current_map ) )
	    continue; // implement restricted navigation for ucm sets
	 if( current_map->IsRootMap() ) {
	    if( stub != NULL ) { // skip over current root map for transfer root->plugin operation
	       if( current_map == current_root )
		  continue;
	    }
	 }
	 if( strncaseequal( current_map->MapLabel(), index_string, len-i ) == 0 ) {
	    fl_set_browser_topline( pfdsm->SelectMapBrowser, highlight_line );
	    return;
	 }
	 else
	    highlight_line++; 
      }
   }
}

void UpdateMapsList()
{
   Map *current_map, *current_root, *displayed_map;
   Cltn<Map *> *map_pool = display_manager->Maps();
   char buffer[80];
   bool line_printed;
   int map_count = 0;

   // grow map list array if necessary
   if( map_pool->Size() > list_size ) {
      if( maps_list ) free( maps_list );
      list_size = map_pool->Size() + 10;
      maps_list = (Map**)malloc(list_size*sizeof(Map *));
   }

   current_root = display_manager->CurrentTopLevelMap();
   displayed_map = display_manager->CurrentMap();

   fl_freeze_form( pfdsm->SelectMap );
   fl_clear_browser( pfdsm->SelectMapBrowser );
   
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      current_map = map_pool->CurrentItem();
      if( current_map->MapType() != current_type )
	 continue;
      if( !display_manager->MapViewable( NULL, current_map ) )
	 continue; // implement restricted navigation for ucm sets
      if( current_map->IsRootMap() ) {
	 if( stub != NULL ) { // skip over current root map for transfer root->plugin operation
	    if( current_map == current_root )
	       continue;
	 }
      }
      maps_list[map_count++] = current_map;
      line_printed = FALSE;
      if( current_type == PLUGIN ) {
	 if( current_map->IsNotReferenced() ) {
	    sprintf( buffer, "@C10%s (unreferenced)", current_map->MapLabel() );
	    fl_add_browser_line( pfdsm->SelectMapBrowser, buffer );
	    line_printed = TRUE;
	 }
      }

      if( !line_printed )
	 fl_add_browser_line( pfdsm->SelectMapBrowser, current_map->MapLabel() ); 
      
      if( current_map == displayed_map ) {
	 fl_select_browser_line( pfdsm->SelectMapBrowser, map_count );
	 fl_set_browser_topline( pfdsm->SelectMapBrowser, map_count );
      }
   }

   // disable buttons initially until something is selected
   DISABLE_WIDGET( pfdsm->InstallButton );
   DISABLE_WIDGET( pfdsm->DeleteButton );
   DISABLE_WIDGET( pfdsm->RenameButton );
   DISABLE_WIDGET( pfdsm->ConvertTypeButton );
   fl_set_input( pfdsm->MapName, "" );
   fl_unfreeze_form( pfdsm->SelectMap );
}

int SMDialogClose( FL_FORM *, void * )
{
   SMCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}

int strncaseequal(const char* s, const char *t, int n)
{
   int i;

   for( i=0; (i<n) && (s[i]!='\0') && (t[i]!='\0'); i++ ){
      if( tolower(s[i]) == tolower(t[i]) )
	 continue;
      else
	 return(-1);
   }

   if(i==n||(s[i]=='\0' && t[i]=='\0'))
      return(0);
   else
      return(-1);
}
