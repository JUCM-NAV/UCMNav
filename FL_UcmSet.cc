// callbacks for UCM set management functions

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "ucm_set.h"
#include "display.h"
#include "map.h"
#include "print_mgr.h"
#include "utilities.h"

extern DisplayManager *display_manager;
extern void SetPromptUserForSave();

FD_UcmSetSpecification *pfduss = NULL;
FD_UcmSetChoice *pfdusc = NULL;
FD_AddCurrentMapUCMSet *pfdams = NULL;

void UpdateMapsList();
void UpdateUcmSets();
void ViewUCMSets( UcmSet *set, bool ns = FALSE );
void DisplayUcmSets( Cltn<UcmSet *> *sets, export_type type = NO_EXPORT );
void AddCurrentMapToSets( Cltn<UcmSet *> *sets );
static int USSDialogClose( FL_FORM *, void * );
static int USCDialogClose( FL_FORM *, void * );
static int AMSDialogClose( FL_FORM *, void * );

static bool new_set, selection_made, set_selection_made, remaining_selection_made;
static UcmSet *current_set;
static export_type extype = NO_EXPORT;
static Cltn<UcmSet *> *csl;
static Cltn<Map *> *map_pool;
static Map *current_map;
Cltn<UcmSetElement *> *set_maps;
static UcmSetElement *previous_set_map = NULL;
static map_type current_type = ROOT_MAP;

static char choice_message[][40] = {
   "Choose UCM Set for Graphics Generation",
   "Choose UCM Set for Saving",
   "Choose UCM Set for Output Report",
   "Edit UCM Set" };

void ViewUCMSets( UcmSet *set, bool ns )
{
   char message[100];
   
   FREEZE_TOOL();
   current_set = set;
   new_set = ns;
   map_pool = display_manager->Maps();
   set_maps = current_set->MapList( current_type );

   if( !pfduss ) {
      pfduss = create_form_UcmSetSpecification();
      fl_set_form_atclose( pfduss->UcmSetSpecification, USSDialogClose, NULL );
      fl_addto_choice( pfduss->MapTypeChoice, " Root Map | Plugin Map " );
      fl_set_choice_fontsize( pfduss->MapTypeChoice, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfduss->MapTypeChoice, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfduss->SetMapsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfduss->SetMapsBrowser, FL_BOLD_STYLE );
      fl_set_browser_dblclick_callback( pfduss->SetMapsBrowser, RemoveMapButton_cb, 0 ); 
      fl_set_browser_fontsize( pfduss->RemainingMapsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfduss->RemainingMapsBrowser, FL_BOLD_STYLE );
      fl_set_browser_dblclick_callback( pfduss->RemainingMapsBrowser, IncludeMapButton_cb, 0 );
      fl_set_browser_fontsize( pfduss->PrimaryMapDisplay, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfduss->PrimaryMapDisplay, FL_BOLD_STYLE );
   }

   fl_freeze_form( pfduss->UcmSetSpecification );

   fl_set_choice( pfduss->MapTypeChoice, ((current_type == ROOT_MAP) ? 1 : 2 ));
   fl_set_input( pfduss->SetNameInput, current_set->SetName() );
   fl_set_input( pfduss->SetDescriptionInput, current_set->SetDescription() );

   fl_clear_browser( pfduss->PrimaryMapDisplay );
   if( current_set->PrimaryMap() != NULL )
      fl_addto_browser( pfduss->PrimaryMapDisplay, current_set->PrimaryMap()->MapLabel() );

   fl_unfreeze_form( pfduss->UcmSetSpecification );
   
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) // reset generated flags of maps
      map_pool->CurrentItem()->ResetUCMSetIncluded();              // use set flag to mark map in current set

   current_set->MarkIncludedMaps();  // set flag for all maps in set
   UpdateMapsList();

   sprintf( message, "%s UCM Set", new_set ? "Create New" : "Edit" );
   fl_show_form( pfduss->UcmSetSpecification, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, message );
}

void UpdateMapsList()
{
   Map *current_map;

   fl_freeze_form( pfduss->UcmSetSpecification );
   
   // fill Set Maps Browser
   fl_clear_browser( pfduss->SetMapsBrowser );
   for( set_maps->First(); !set_maps->IsDone(); set_maps->Next() )
      fl_add_browser_line( pfduss->SetMapsBrowser, set_maps->CurrentItem()->SetMap()->MapLabel() );

   // fill Remaining Maps Browser
   fl_clear_browser( pfduss->RemainingMapsBrowser );
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      current_map = map_pool->CurrentItem();
      if( (!current_map->UCMSetIncluded()) && (current_map->MapType() == current_type) )
	 fl_add_browser_line( pfduss->RemainingMapsBrowser, current_map->MapLabel() );
   }

   // disable all buttons as nothing is selected
   DISABLE_WIDGET( pfduss->IncludeMapButton );
   DISABLE_WIDGET( pfduss->RemoveMapButton );
   DISABLE_WIDGET( pfduss->SetPrimaryMapButton );

   if( previous_set_map ) // save comment input as it may be changed
      previous_set_map->Comment( fl_get_input( pfduss->MapCommentInput ) );
   fl_set_input( pfduss->MapCommentInput, "" );
   DISABLE_WIDGET( pfduss->MapCommentInput );

   set_selection_made = FALSE;
   remaining_selection_made = FALSE;
   previous_set_map = NULL;
   
   fl_unfreeze_form( pfduss->UcmSetSpecification );
}

extern "C"
void RemainingMapsBrowser_cb(FL_OBJECT *, long)
{
   if( !remaining_selection_made && (fl_get_browser( pfduss->RemainingMapsBrowser ) != 0) ) {
      ENABLE_WIDGET( pfduss->IncludeMapButton );
      remaining_selection_made = TRUE;
   }
}

extern "C"
void SetMapsBrowser_cb(FL_OBJECT *, long)
{
   UcmSetElement *chosen_set_map;
   int user_choice;

   if( (user_choice = fl_get_browser( pfduss->SetMapsBrowser )) == 0) return;
   
   if( !set_selection_made ) {
      ENABLE_WIDGET( pfduss->RemoveMapButton );
      ENABLE_WIDGET( pfduss->SetPrimaryMapButton );
      ENABLE_WIDGET( pfduss->MapCommentInput );
      set_selection_made = TRUE;
   }

   chosen_set_map =  set_maps->Get( user_choice );

   if( chosen_set_map != previous_set_map ) {

      if( previous_set_map )
	 previous_set_map->Comment( fl_get_input( pfduss->MapCommentInput ) );

      fl_set_input( pfduss->MapCommentInput, chosen_set_map->Comment() );
   }

   previous_set_map = chosen_set_map;
}

extern  "C"
void SetPrimaryMapButton_cb(FL_OBJECT *, long)
{
   Map *primary_map = set_maps->Get( fl_get_browser( pfduss->SetMapsBrowser ) )->SetMap();
   current_set->PrimaryMap( primary_map );
   fl_clear_browser( pfduss->PrimaryMapDisplay );
   fl_addto_browser( pfduss->PrimaryMapDisplay, current_set->PrimaryMap()->MapLabel() );
   SetPromptUserForSave();
}

extern  "C"
void IncludeMapButton_cb(FL_OBJECT *, long)
{
   int user_choice, i = 0;
   Map *chosen_map, *map;

   user_choice = fl_get_browser( pfduss->RemainingMapsBrowser );
   
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      map = map_pool->CurrentItem();
      if( map->MapType() == current_type && !map->UCMSetIncluded() ) {
	 i++;
	 if( i == user_choice ) {
	    chosen_map = map_pool->CurrentItem();
	    break;
	 }	
      }
   }

   chosen_map->SetUCMSetIncluded();
   current_set->AddMap( chosen_map );
   SetPromptUserForSave();
   UpdateMapsList();
}

extern  "C"
void RemoveMapButton_cb(FL_OBJECT *, long)
{
   UcmSetElement *chosen_set_map =  set_maps->Get( fl_get_browser( pfduss->SetMapsBrowser ) );
   set_maps->Delete( chosen_set_map );
   chosen_set_map->SetMap()->ResetUCMSetIncluded();
   chosen_set_map->SetMap()->DeregisterUCMSet( current_set );
   if( chosen_set_map->SetMap() == current_set->PrimaryMap() ) {
      current_set->PrimaryMap( NULL );
      fl_clear_browser( pfduss->PrimaryMapDisplay );
   }
   delete chosen_set_map;
   previous_set_map = NULL;
   SetPromptUserForSave();
   UpdateMapsList();
}

extern "C"
void USSAcceptButton_cb(FL_OBJECT *, long)
{
   const char *set_name = fl_get_input( pfduss->SetNameInput );
   char message[100];
   
   // validate input and reject if incomplete
   if( !UcmSet::UniqueSetName( set_name, current_set )) {
      sprintf( message, "The name \"%s\" is not a unique name for a UCM set.", set_name );
      fl_set_resource( "flAlert.title", "Error: Duplicate Set Name Entered" );
      fl_show_alert( message, "Please choose a unique set map name.", "", 0 );
      return;
   }

   if( current_set->PrimaryMap() == NULL ) {
      fl_set_resource( "flAlert.title", "Error: Primary Map Not Specified" );
      fl_show_alert( "The primary map of this UCM set has not been specified.",
		     "Please choose a set map to be the primary map.", "", 0 );
      return;
   }
   
   if( previous_set_map ) // save comment input as it may be changed
      previous_set_map->Comment( fl_get_input( pfduss->MapCommentInput ) );

   current_set->SetName( set_name );
   current_set->SetDescription( fl_get_input( pfduss->SetDescriptionInput ) );

   if( new_set )
      UcmSet::AddUcmSet( current_set );
   
   SetPromptUserForSave();
   fl_hide_form( pfduss->UcmSetSpecification );
   UNFREEZE_TOOL();
}

extern "C"
void MapTypeChoice_cb(FL_OBJECT *choice, long)
{
   map_type old_type = current_type;
   current_type = (fl_get_choice( choice ) == 1) ? ROOT_MAP : PLUGIN;
   set_maps = current_set->MapList( current_type );
   if( current_type != old_type )
      UpdateMapsList();
}

extern  "C"
void USSRemoveButton_cb(FL_OBJECT *, long)
{
   if( new_set ) {
      if( !QuestionDialog( "Confirm UCM Set Deletion", "Removing dialog will delete newly created UCM sets.",
			   "Do you wish to delete this UCM set ?", "Delete UCM Set", "Cancel Dialog Removal", FALSE ) )
	 return;
      else
	 delete current_set;
   }
   fl_hide_form( pfduss->UcmSetSpecification );
   UNFREEZE_TOOL();
   SetPromptUserForSave();
}

int USSDialogClose( FL_FORM *, void * )
{
   USSRemoveButton_cb( 0, 0 );
   return( FL_IGNORE );
}

void AllowViewingUCMSets( bool allow )
{
   fl_set_menu_item_mode( pfdmMain->MapsMenu, 7, ( allow ?  FL_PUP_NONE : FL_PUP_GREY ) );
   fl_set_menu_item_mode( pfdmMain->MapsMenu, 8, ( allow ?  FL_PUP_NONE : FL_PUP_GREY ) );
}

// functions for Ucm Set choice dialog

void DisplayUcmSets( Cltn<UcmSet *> *sets, export_type type )
{
   FREEZE_TOOL();
   csl = sets;
   selection_made = FALSE;
   
   if( !pfdusc ) {
      pfdusc = create_form_UcmSetChoice();
      fl_set_form_atclose( pfdusc->UcmSetChoice, USCDialogClose, NULL );
      fl_set_browser_fontsize( pfdusc->UcmSetBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdusc->UcmSetBrowser, FL_BOLD_STYLE );
      fl_set_browser_dblclick_callback( pfdusc->UcmSetBrowser, EditSetButton_cb, 0 );
   }      

   if( (type == NO_EXPORT) && (extype != NO_EXPORT) ) { // reset from previous invocation if necessary
      fl_show_object( pfdusc->RestrictNavigation);
      fl_show_object( pfdusc->DeleteSetButton);
      fl_set_object_label( pfdusc->EditSetButton, "Edit Set" );
   }
   else if( type != NO_EXPORT ) {
      fl_hide_object( pfdusc->RestrictNavigation);
      fl_hide_object( pfdusc->DeleteSetButton);
      fl_set_object_label( pfdusc->EditSetButton, "Choose Set" );
   }
   
   extype = type;
   UpdateUcmSets();
   
   fl_show_form( pfdusc->UcmSetChoice, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, choice_message[extype] );
}

void UpdateUcmSets()
{
   fl_clear_browser( pfdusc->UcmSetBrowser );
   for( csl->First(); !csl->IsDone(); csl->Next() )
      fl_addto_browser( pfdusc->UcmSetBrowser, csl->CurrentItem()->SetName() );

   DISABLE_WIDGET( pfdusc->ModificationButtons );
   selection_made = FALSE;
}

extern "C"
void UcmSetBrowser_cb(FL_OBJECT *, long)
{
   if( !selection_made ) {
      ENABLE_WIDGET( pfdusc->ModificationButtons );
      selection_made = TRUE;
   }
}

extern "C"
void EditSetButton_cb(FL_OBJECT *, long)
{
   int choice;
   UcmSet *chosen_set;
   Cltn<Map *> *map_list;
   char default_name[75];
   
   if( (choice = fl_get_browser( pfdusc->UcmSetBrowser )) == 0 ) return;
   fl_hide_form( pfdusc->UcmSetChoice );
   UNFREEZE_TOOL();

   chosen_set = csl->Get( choice );
   if( extype != NO_EXPORT )
      map_list = chosen_set->ExportList();
   
   if( extype == SET_GRAPHICS ) 
      PrintManager::GenerateMapGraphics( UCM_SET, map_list );
   else if( extype == SET_SAVE ) {
      sprintf( default_name, "%s.ucm", chosen_set->SetName() );      
      display_manager->ExportMapList( map_list, default_name );      
   }
   else if( extype == SET_REPORT )
      PrintManager::GeneratePostScriptFile( map_list );
   else // NO_EXPORT
      ViewUCMSets( chosen_set );
}

extern "C"
void RestrictNavigation_cb(FL_OBJECT *, long)
{
   char message[100];
   int choice;
   
   if( (choice = fl_get_browser( pfdusc->UcmSetBrowser )) == 0 ) return;   
   UcmSet *chosen_set = csl->Get( choice );
   
   if( chosen_set->PrimaryMap() == NULL ) {
      sprintf( message, "The UCM Set %s", chosen_set->SetName() );
      fl_set_resource( "flAlert.title", "Error: Primary Map Not Specified" );
      fl_show_alert( "The primary map of this UCM set has not been specified.",
		     "Please choose a set map to be the primary map.", "", 0 );
      return;
   }
   
   USCancelButton_cb( 0, 0 ); // remove dialog
   display_manager->SetCurrentUCMSet( chosen_set );
}

extern "C"
void DeleteSetButton_cb(FL_OBJECT *, long)
{
   int choice;
   
   if( (choice = fl_get_browser( pfdusc->UcmSetBrowser )) == 0 ) return;

   UcmSet::DeleteUcmSet( csl->Get( choice ) );
   SetPromptUserForSave();

   if( csl->Size() == 0 )
      USCancelButton_cb( 0, 0 ); // remove dialog as there are no more sets
   else
      UpdateUcmSets(); // update list to remove set
}

extern "C"
void USCancelButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdusc->UcmSetChoice );
   UNFREEZE_TOOL();
}

int USCDialogClose( FL_FORM *, void * )
{
   USCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}

void AddCurrentMapToSets( Cltn<UcmSet *> *sets )
{
   UcmSet *current_set;
   char buffer[75];

   FREEZE_TOOL();
   csl = sets;
   current_map = display_manager->CurrentMap();

   if( !pfdams ) {
      pfdams = create_form_AddCurrentMapUCMSet();
      fl_set_form_atclose( pfdams->AddCurrentMapUCMSet, AMSDialogClose, NULL );
      fl_set_browser_fontsize( pfdams->CurrentMapDisplay, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdams->CurrentMapDisplay, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdams->UCMSetsList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdams->UCMSetsList, FL_BOLD_STYLE );

   }

   fl_freeze_form( pfdams->AddCurrentMapUCMSet );

   // output name of current map
   fl_clear_browser( pfdams->CurrentMapDisplay );
   fl_add_browser_line( pfdams->CurrentMapDisplay, current_map->MapLabel() );

   // disable add, remove buttons initially
   DISABLE_WIDGET( pfdams->AddCurrentMapButton );
   DISABLE_WIDGET( pfdams->RemoveCurrentMapButton );

   // output list of ucm sets, gray out ones already containing this map
   fl_clear_browser( pfdams->UCMSetsList ); 
   for( csl->First(); !csl->IsDone(); csl->Next() ) {
      current_set = csl->CurrentItem();
      if( current_set->IncludesMap( current_map ) ) {
	 sprintf( buffer, "@C11%s", current_set->SetName() );
	 fl_add_browser_line( pfdams->UCMSetsList, buffer );
      }
      else
	 fl_add_browser_line( pfdams->UCMSetsList, current_set->SetName() );
   }

   fl_unfreeze_form( pfdams->AddCurrentMapUCMSet );
   fl_show_form( pfdams->AddCurrentMapUCMSet, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Add Current Map To UCM Set" );
}

extern "C"
void UCMSetsList_cb(FL_OBJECT *, long)
{
   int choice;
   
   if( (choice = fl_get_browser( pfdams->UCMSetsList )) == 0 ) return;

   if( csl->Get( choice )->IncludesMap( current_map ) ) {
      ENABLE_WIDGET( pfdams->RemoveCurrentMapButton );
      DISABLE_WIDGET( pfdams->AddCurrentMapButton );
   } else {
      ENABLE_WIDGET( pfdams->AddCurrentMapButton );
      DISABLE_WIDGET( pfdams->RemoveCurrentMapButton );
   } 
}

extern "C"
void AddCurrentMapButton_cb(FL_OBJECT *, long)
{
   int choice;
   UcmSet *chosen_set;
   char buffer[75];

   if( (choice = fl_get_browser( pfdams->UCMSetsList )) == 0 ) return;
   chosen_set = csl->Get( choice );
   chosen_set->AddMap( current_map );
   sprintf( buffer, "@C11%s", chosen_set->SetName() );
   fl_replace_browser_line( pfdams->UCMSetsList, choice, buffer );
   ENABLE_WIDGET( pfdams->RemoveCurrentMapButton );
   DISABLE_WIDGET( pfdams->AddCurrentMapButton );
   SetPromptUserForSave();
}

extern "C"
void RemoveCurrentMapButton_cb(FL_OBJECT *, long)
{
   int choice;
   UcmSet *chosen_set;

   if( (choice = fl_get_browser( pfdams->UCMSetsList )) == 0 ) return;
   chosen_set = csl->Get( choice );
   chosen_set->RemoveSetMap( current_map );
   fl_replace_browser_line( pfdams->UCMSetsList, choice, chosen_set->SetName() );
   ENABLE_WIDGET( pfdams->AddCurrentMapButton );
   DISABLE_WIDGET( pfdams->RemoveCurrentMapButton );
   SetPromptUserForSave();
}

extern "C"
void RemoveAMSDialog_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdams->AddCurrentMapUCMSet );
   UNFREEZE_TOOL();
}

int AMSDialogClose( FL_FORM *, void * )
{
   RemoveAMSDialog_cb( 0, 0 );
   return( FL_IGNORE );
}
