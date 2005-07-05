/***********************************************************
 *
 * File:			display.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "display.h"
#include "defines.h"
#include "hyperedge.h"
#include "or_figure.h"
#include "figure.h"
#include "empty.h"
#include "synchronization.h"
#include "or_fork.h"
#include "stub.h"
#include "start.h"
#include "result.h"
#include "loop.h"
#include "responsibility.h"
#include "resp_ref.h"
#include "resp_mgr.h"
#include "timer.h"
#include "or_null_figure.h"
#include "synch_null_figure.h"
#include "stub_figure.h"
#include "path.h"
#include "hypergraph.h"
#include "synch_figure.h"
#include "loop_figure.h"
#include "loop_null_figure.h"
#include "transformation.h"
#include "marker_figure.h"
#include "label.h"
#include "map.h"
#include "component_mgr.h"
#include "devices.h"
#include "data.h"
#include "print_mgr.h"
#include "xml_mgr.h"
#include "ucm_set.h"
#include "variable.h"
#include "scenario.h"
#include "utilities.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef TIME
#include <sys/time.h>
#include <sys/resource.h>

extern long time_difference( struct timeval time1, struct timeval time2 );
#endif

extern void SetActiveFigure( HyperedgeFigure *new_figure );
extern void DrawScreen();
extern void ResetScrollbars();
extern void UpdatePathMenu();
extern void UpdateResponsibilityList();
extern void EditResponsibilityDesc( ResponsibilityReference *new_responsibility, bool new_ref = FALSE );
extern void ResetConditions();
extern void SelectMap( Stub *s );
extern void ListParents( Map *map, Hyperedge *submap_endpoint = NULL );
extern void MapGroupDialog( list_operation lo, const char *ft = NULL );
extern void SetPromptUserForSave();
extern void FitMap();

extern HyperedgeFigure *first_figure;
extern MarkerFigure *marker_figure; // global pointer to sole marker Figure
extern int where_description;

Label *current_label;         // global label of current path
ComponentManager *component_manager; // global pointer to sole Component Manager
float selection_radius = SMALL_SELECTION_RADIUS; // global variable for selection radius
bool paths_complete = TRUE;
bool design_deletion;

extern char loaded_file_path[128];
extern char loaded_file[64];
extern int showing_labels;
extern int auto_map_scale;
extern bool FixedAllPositions;

#define NO_SELECTION_BOX -1

DisplayManager::DisplayManager( TransformationManager *manager ) : XSpacing(0.04) , YSpacing(0.04)
{ 
   trans_manager = manager;
   trans_manager->InstallDisplayManager( this );
   component_manager = new ComponentManager;
   map_pool = new Cltn<Map *>;
   active_figure = NULL;
   current_path = NULL;
   SetCurrentUCMSet( NULL );
   decomposition_levels = 0;
   displayed_map_level = 0;
   design_name[0] = '\0';
   design_name[LABEL_SIZE] = '\0';
   sbx = NO_SELECTION_BOX;
   design_id = 0;
   design_description = NULL;
   base_map = NULL;
   current_scenario = NULL;
}

void DisplayManager::SetFirstMap()
{
   Map *first_root;

   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      if( map_pool->CurrentItem()->IsRootMap() ) {
	 first_root = map_pool->CurrentItem();
	 break;
      }
   }
   
   this->SetMap( first_root );
}

void DisplayManager::SetCurrentUCMSet( UcmSet *new_set, bool update )
{
   char mode[100];

   current_ucm_set = new_set;

   if( ScenarioList::ScenarioTracingMode() ) { // cancel scenario trace mode if enabled
      ScenarioList::ResetScenarioTracing(); 
      if( update )
	 DrawScreen();
   }
   
   fl_set_menu_item_mode( pfdmMain->ScenariosMenu, 3, FL_PUP_GREY );
   fl_clear_browser( pfdmMain->NavigationMode );
   fl_set_menu_item_mode( pfdmMain->MapsMenu, 9, ( current_ucm_set ?  FL_PUP_NONE : FL_PUP_GREY ) ); // disable restore navigation list item if not applicable
   
   if( current_ucm_set ) {
      sprintf( mode, "UCM Set: %s", current_ucm_set->SetName() );
      fl_addto_browser( pfdmMain->NavigationMode, mode );
      this->SetMap( current_ucm_set->PrimaryMap() );
   }
   else
      fl_addto_browser( pfdmMain->NavigationMode, "Full UCM Design Navigation" );
}

void DisplayManager::SetScenarioHighlight( Scenario *new_scenario )
{
   char mode[200];
   Map *first_start_map;

   current_ucm_set = NULL; // disable ucm set mode if active
   fl_set_menu_item_mode( pfdmMain->MapsMenu, 9, ( new_scenario ?  FL_PUP_NONE : FL_PUP_GREY ) ); // disable restore navigation list item if not applicable
   fl_set_menu_item_mode( pfdmMain->ScenariosMenu, 3, ( new_scenario ?  FL_PUP_NONE : FL_PUP_GREY ) ); // disable remove highlight list item if not applicable
   fl_clear_browser( pfdmMain->NavigationMode );

   if( new_scenario ) {
      sprintf( mode, "Scenario Trace: %s (%s)", new_scenario->Name(), new_scenario->Group()->GroupName() );
      fl_addto_browser( pfdmMain->NavigationMode, mode );
      first_start_map = new_scenario->StartingPoints()->GetFirst()->ParentMap();
      if( first_start_map != current_map )
	 this->SetMap( first_start_map );
      current_scenario = new_scenario;
   }
   else {
      ScenarioList::ResetScenarioTracing();
      fl_addto_browser( pfdmMain->NavigationMode, "Full UCM Design Navigation" );
      current_scenario = NULL;
   }
}

void DisplayManager::SetMap( Map *new_map )
{
   char buffer[100];

   // determine if map is the first ancestor map
   if( displayed_map_level > 0 ) {
      if( new_map == hierarchy[displayed_map_level-1].map ) {
	 InstallParent();
	 return;
      }
   }
   
   // replace the map hierarchy with a new root or plugin map
   current_map = new_map;
   InstallMapElements( current_map );

   decomposition_levels = 0;
   displayed_map_level = 0;
   hierarchy[0].map = new_map;
   
   fl_freeze_form( pfdmMain->Main );
   fl_clear_browser( pfdmMain->DecompositionLevel );
   if( new_map->IsRootMap() )
      sprintf( buffer, "0   %s", new_map->MapLabel() );
   else
      sprintf( buffer, "N       %s (Plugin)", new_map->MapLabel() );
   fl_addto_browser( pfdmMain->DecompositionLevel, buffer );
   fl_select_browser_line( pfdmMain->DecompositionLevel, 1 );   
   fl_unfreeze_form( pfdmMain->Main );  
   DrawScreen();
}

void DisplayManager::RenameMap( Map *map, const char *new_label )
{
   map->MapLabel( new_label );

   for( int i = 0; i <= decomposition_levels; i++ ) { // update decomposition list if necessary
      if( map == hierarchy[i].map )
	 fl_replace_browser_line( pfdmMain->DecompositionLevel, i+1, map->MapLabel() );
   }
}

void DisplayManager::PlaceMap( Map *map )
{
   current_map = map;
   trans_manager->InstallCurrentGraph( map->MapHypergraph() );
   figure_pool = map->Figures();
   path_pool = map->Paths();
   responsibility_pool = map->Responsibilities();
   component_manager->ComponentReferences( map->Components() );
}

void DisplayManager::InstallParent()
{
   if( displayed_map_level > 0 ) {
      fl_freeze_form( pfdmMain->Main );
      fl_deselect_browser_line( pfdmMain->DecompositionLevel, displayed_map_level+1 );
      fl_select_browser_line( pfdmMain->DecompositionLevel, displayed_map_level );
      fl_set_browser_topline( pfdmMain->DecompositionLevel, ((displayed_map_level-1) > 0) ? displayed_map_level-1 : displayed_map_level );
      fl_unfreeze_form( pfdmMain->Main ); 
      InstallMap( displayed_map_level-1 );
      DrawScreen();
   }
}

void DisplayManager::InstallSubmap()
{ // called by keyboard shortcut
   if( decomposition_levels > displayed_map_level ) {
      fl_freeze_form( pfdmMain->Main );
      fl_deselect_browser_line( pfdmMain->DecompositionLevel, displayed_map_level+1 );
      fl_select_browser_line( pfdmMain->DecompositionLevel, displayed_map_level+2 );
      fl_set_browser_topline( pfdmMain->DecompositionLevel, displayed_map_level  );
      fl_unfreeze_form( pfdmMain->Main ); 
      InstallMap( displayed_map_level+1 );
      DrawScreen();
   }
}

void DisplayManager::FollowPath()
{ // called by keyboard shortcut
   Hyperedge *selected_edge;
   edge_type etype;

   if( selected_figures.Size() != 1 ) return;

   selected_edge = selected_figures.GetFirst()->Edge();
   etype = selected_edge->EdgeType();

   if( etype == EMPTY ) {
      if( ((Empty *)selected_edge)->FollowPathInSubmap( FALSE ) ) {
	 ((Empty *)selected_edge)->FollowPathInSubmap( TRUE );
	 DrawScreen();
      }
   }
   else if( etype == RESULT ) {
      if( ((Result *)selected_edge)->FollowPathInParentMap( FALSE ) ) {
	 ((Result *)selected_edge)->FollowPathInParentMap( TRUE );
	 DrawScreen();
      }
   }
   else if( etype == START ) {
      if( ((Start *)selected_edge)->FollowPathInParentMap( FALSE ) ) {
	 ((Start *)selected_edge)->FollowPathInParentMap( TRUE );
	 DrawScreen();
      }
   }
}

void DisplayManager::InstallMap( int map_id )
{
   if( map_id == displayed_map_level ) return;  // the same map was selected by the user

   displayed_map_level = map_id;   // set new displayed map level
   Map *map = hierarchy[displayed_map_level].map;
   ResetConditions();
   InstallMapElements( map );
   DrawScreen();
}

void DisplayManager::InstallSubmap( Map *submap, Stub *stub, Empty *empty )
{
   // disallow operation if maximum levels have been reached
   if( displayed_map_level+1 == MAXIMUM_HIERARCHY_LEVELS ) {
      fl_set_resource( "flAlert.title", "Maximum hierarchy levels reached" );
      fl_show_alert( "The maximum level of decomposition has been reached.",
		     "This Install Submap operation is disallowed.", "", 0 );
      return;
   }

   decomposition_levels = ++displayed_map_level;
   hierarchy[decomposition_levels].map = submap;
   hierarchy[decomposition_levels].stub = stub;

   FillDecompositionList();

   where_description = DESC_MAP;
   fl_set_input( pfdmMain->Description, ( (submap->MapDescription() != NULL) ? submap->MapDescription() : "" ) );
   fl_set_object_label( pfdmMain->Description, "Description of Map" );
   ResetConditions();

   submap->HighlightEntryPoint( empty, stub );
   InstallMapElements( submap );
}

void DisplayManager::FillDecompositionList()
{
   int i;
   char selection[350] = "";
   ComponentReference *stub_component;

   fl_freeze_form( pfdmMain->Main );
   fl_clear_browser( pfdmMain->DecompositionLevel );

   bool root = hierarchy[0].map->IsRootMap(); // determine if top level map is root or plugin

   for( i = 0; i <= decomposition_levels; i++ ) {
      if( i == 0 )
	 sprintf( selection, "%s%s%s",  ( root ? "0   " : "N       " ), hierarchy[i].map->MapLabel(),  ( root ? "" : " (Plugin)" ) ); 
      else {
	 stub_component = hierarchy[i].stub->GetFigure()->GetContainingComponent();
	 sprintf( selection, "%s%d%*s%s (%s) [%s]", (root ? "" : "N+" ), i, ( (i > 9) ? 2 : 3 ), "",
		  hierarchy[i].map->MapLabel(), hierarchy[i].stub->StubName(), 
		  ((stub_component != NULL) ? stub_component->ReferenceName() : "" ) );
      }
      fl_addto_browser( pfdmMain->DecompositionLevel, selection );
   }

   fl_select_browser_line( pfdmMain->DecompositionLevel, decomposition_levels+1 );
   fl_unfreeze_form( pfdmMain->Main ); 
}

void DisplayManager::RemoveStubFromDecomposition( Stub *deleted_stub )
{
   for( int i = 0; i <= decomposition_levels; i++ ) {
      if( hierarchy[i].stub == deleted_stub ) {
	 decomposition_levels = i-1;
	 FillDecompositionList();
	 break;
      }
   }
}

void DisplayManager::InstallMapElements( Map *map )
{
   // clean up display from old map
   SetActive( NULL );
   if( marker_figure ) {
      delete marker_figure;
      marker_figure = NULL;
   }

   component_manager->Reset();
   ResetSelectedElements();

   // install elements of new map
   current_map = map;
   trans_manager->InstallCurrentGraph( map->MapHypergraph() );
   figure_pool = map->Figures();
   path_pool = map->Paths();
   responsibility_pool = map->Responsibilities();
   component_manager->ComponentReferences( map->Components() );

   fl_set_input( pfdmMain->Title, ( map->MapTitle() ? map->MapTitle() : "" ) );
   fl_set_input( pfdmMain->Description, ( map->MapDescription() ? map->MapDescription() : "" ) );
   fl_set_menu_item_mode( pfdmMain->MapsMenu, 5, ( map->IsTopLevelMap() ? FL_PUP_GREY : FL_PUP_NONE ) ); // disable parent list item if applicable
   
   UpdateResponsibilityList();
   if( auto_map_scale )
      FitMap();
}

void DisplayManager::DisplayOnlyMode( Map *map )
{
   char selection[350] = "";
   bool root;

   if( map == current_map ) return;

   if( base_map == NULL ) { // store 
      base_map = current_map;
      fl_clear_browser( pfdmMain->NavigationMode );
      fl_addto_browser( pfdmMain->NavigationMode, "Display Only Mode" );
   }

   fl_clear_browser( pfdmMain->DecompositionLevel );
   root = map->IsRootMap();
   sprintf( selection, "%s%s%s",  ( root ? "0   " : "N       " ), map->MapLabel(),  ( root ? "" : " (Plugin)" ) ); 
   fl_addto_browser( pfdmMain->DecompositionLevel, selection );

   InstallMapElements( map );
   DrawScreen();
}

void DisplayManager::RestoreMapNavigation()
{
   char mode[100];

   if( base_map == NULL ) return;

   current_map = base_map;
   InstallMapElements( current_map );
   DrawScreen();
   FillDecompositionList();

   // restore navigation mode display
   fl_clear_browser( pfdmMain->NavigationMode );
   if( current_ucm_set ) {
      sprintf( mode, "UCM Set: %s", current_ucm_set->SetName() );
      fl_addto_browser( pfdmMain->NavigationMode, mode );
   } else if( current_scenario  ) {
      sprintf( mode, "Scenario Trace: %s (%s)", current_scenario->Name(), current_scenario->Group()->GroupName() );
      fl_addto_browser( pfdmMain->NavigationMode, mode );
   } else
      fl_addto_browser( pfdmMain->NavigationMode, "Full UCM Design Navigation" );

   base_map = NULL;
}

void DisplayManager::Submaps( design_plugin *plugins, int &num_plugins, Stub *stub )
{
   Map *map;
   int i, count = 0;
   bool ancestor_map;
   
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      map = map_pool->CurrentItem();
      if( stub ) {
	 if( stub->ContainsSubmap( map ) )
	    continue; // skip over plugins already contained in the stub
      }
      ancestor_map = FALSE;
      for( i = 0; i <= displayed_map_level; i++ ) { // determine if map is a parent of current map
	 if( map == hierarchy[i].map ) {            // or is the current map
	    ancestor_map = TRUE;
	    break;
	 }
      }
      plugins[count].plugin =  map;
      plugins[count++].parent = ancestor_map;
   }

   num_plugins = count;
}

bool DisplayManager::HasSubmaps( Stub *stub )
{
   Map *map;
   int i;
   bool valid_map;

   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      map = map_pool->CurrentItem();
      if( map->IsPlugIn() ) {
	 valid_map = TRUE;
	 for( i = 0; i <= displayed_map_level; i++ ) { // determine if map is a parent of current map
	    if( map == hierarchy[i].map ) {            // or is the current map
	       valid_map = FALSE;
	       break;
	    }
	 }
	 if( valid_map ) {
	    if( !stub->ContainsSubmap( map ) )
	       return( TRUE );  // return with positive result if a plug-in was found that is not the current map
	 }                      //  or any of its parent maps or is already contained in the stub
      }
   }   

   return( FALSE );
}

const char * DisplayManager::UniqueMapName( const char *prompt, const char *default_name )
{
   const char *entered_name, *new_name = NULL;
   bool quit = FALSE, duplicate_name = FALSE;
   char buffer[150];
   
   do {
      
      entered_name = fl_show_input( prompt, default_name );

      if( entered_name == NULL ) // user has pressed cancel button to cancel operation
	 quit = TRUE;
      else {
	 if( strequal( entered_name, "" ) )
	    duplicate_name = TRUE;
	 else {
	    if( MapNameExists( entered_name ) ) {
	       if( !strequal( entered_name, default_name ) )
		  duplicate_name = TRUE;  // the name duplicates an existing name that is not the old name, set flag
	       else
		  entered_name = NULL; // the user has reentered the same name, quit
	       break;
	    }
	 }
	 if( duplicate_name ) {
	    sprintf( buffer, "The name \"%s\"", entered_name );
	    fl_set_resource( "flAlert.title", "Error: Duplicate Map Name Entered" );
	    fl_show_alert( buffer, "is not a unique name for a new map. Please choose a new label", "", 0 );
	    duplicate_name = FALSE;
	 }
	 else {
	    new_name = entered_name;
	    quit = TRUE;
	 }
      }
   } while( !quit );
   
   return( new_name );
}

void DisplayManager::CreateNewRootMap()
{
   const char *new_root_name;
   char default_name[10];
   int i = 2;
   
   if( !MapNameExists( "root" ) )
      strcpy( default_name, "root" );
   else {
      do {
	 sprintf( default_name, "root%d", i++ );
      } while( MapNameExists( default_name ) );
   }
   
   new_root_name = UniqueMapName( "Enter the name of the new root map.", default_name );
   if( new_root_name == NULL ) return;

   ResetConditions();
   SetMap( new Map( new_root_name ) );
}

bool DisplayManager::MapNameExists( const char *map_name )
{
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      if( strequal( map_pool->CurrentItem()->MapLabel(), map_name ) )
	 return TRUE;
   }

   return FALSE;
}

Map * DisplayManager::FindMap( const char *map_name )
{
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      if( strequal( map_pool->CurrentItem()->MapLabel(), map_name ) )
	 return map_pool->CurrentItem();
   }

   return NULL;
}

void DisplayManager::ExportMaps()
{
   MapGroupDialog( EXPORT );
}

void DisplayManager::ExportGroup( map_list mlist )
{
   char default_name[75];
   Cltn<Map *> *sublist;
   map_type old_type;
   
   sprintf( default_name, "%s.ucm", current_map->MapLabel() );
   sublist = new Cltn<Map *>;   
   if( mlist == CURRENT_MAP )
      sublist->Add( current_map ); // fill 'sublist' with this map only
   else if( mlist == CURRENT_SUBTREE )
      current_map->ListSubmaps( sublist ); // fill sublist with current map and all of its submaps

   old_type = current_map->MapType();
   current_map->SetMapType( ROOT_MAP ); // explicitly set current map to be the root map of sub design
   this->ExportMapList( sublist, default_name );
   current_map->SetMapType( old_type ); // reset the map type
}

void DisplayManager::ExportCSMGroup( map_list mlist )
{
   char default_name[75];
   Cltn<Map *> *sublist;
   map_type old_type;
   
   sprintf( default_name, "%s.ucm", current_map->MapLabel() );
   sublist = new Cltn<Map *>;   
   if( mlist == CURRENT_MAP )
      sublist->Add( current_map ); // fill 'sublist' with this map only
   else if( mlist == CURRENT_SUBTREE )
      current_map->ListSubmaps( sublist ); // fill sublist with current map and all of its submaps

   old_type = current_map->MapType();
   current_map->SetMapType( ROOT_MAP ); // explicitly set current map to be the root map of sub design
   this->ExportCSMList( sublist, default_name );
   current_map->SetMapType( old_type ); // reset the map type
}

void DisplayManager::ExportMapList( Cltn<Map *> *map_list, const char *default_name )
{
   const char *export_filename;
   FILE *fp;
   char buffer[100], design[75];

   freeze_editor = TRUE;
   fl_set_fselector_title( "Export File" );
   export_filename = fl_show_fselector( "Export Filename", "", "*.ucm", default_name );

   if( export_filename == NULL ) {
      freeze_editor = FALSE;
      return;
   }

   if( !(fp = fopen( export_filename, "w" )) ) {
      sprintf( buffer, "The file %s ", export_filename );
      fl_set_resource( "flAlert.title", "Error: File Save Failed" );
      fl_show_alert( buffer, "could not be opened. Check file/directory permissions.", "", 0 );
      freeze_editor = FALSE;
      return;
   }

   strcpy( design, default_name );
   design[strlen(design)-4] = 0; // remove .ucm extension
   XmlManager::SaveFileXML( fp, map_list, design );
   fl_invalidate_fselector_cache();
   freeze_editor = FALSE;
}

void DisplayManager::ExportCSMList( Cltn<Map *> *map_list, const char *default_name )
{
   const char *export_filename;
   FILE *fp;
   char buffer[100], design[75];

   freeze_editor = TRUE;
   fl_set_fselector_title( "Export File" );
   export_filename = fl_show_fselector( "Export Filename", "", "*.ucm", default_name );

   if( export_filename == NULL ) {
      freeze_editor = FALSE;
      return;
   }

   if( !(fp = fopen( export_filename, "w" )) ) {
      sprintf( buffer, "The file %s ", export_filename );
      fl_set_resource( "flAlert.title", "Error: File Save Failed" );
      fl_show_alert( buffer, "could not be opened. Check file/directory permissions.", "", 0 );
      freeze_editor = FALSE;
      return;
   }

   strcpy( design, default_name );
   design[strlen(design)-4] = 0; // remove .ucm extension
   XmlManager::SaveCSMXML( fp, map_list, design );
   fl_invalidate_fselector_cache();
   freeze_editor = FALSE;
}

void DisplayManager::ImportMaps( map_type mtype, Stub *parent_stub )
{
   const char *filename, *entered_label;
   char buffer[100];
   FILE *fp;
   int rc;
   Map *imported_map, *displayed_map = current_map;;

   freeze_editor = TRUE;
   fl_set_fselector_title( "Import File" );
   filename = fl_show_fselector( "Import File", "", "*.ucm", "" );

   if( filename == NULL ) {
      freeze_editor = FALSE;
      return;
   }

   if( !(fp = fopen( filename, "r" )) ) {
      sprintf( buffer, "The file %s ", filename );
      fl_set_resource( "flAlert.title", "Error: File Load Failed" );
      fl_show_alert( buffer, "could not be found.", "", 0 );
      freeze_editor = FALSE;
      return;
   }
   
   rc = XmlManager::LoadFile( fp, IMPORT_MAPS );

   if( rc == FILE_ERROR ) {
      fl_set_resource( "flAlert.title", "Error: File Format Not Understood" );
      fl_show_alert( "The format of the following file was not understood:", filename, "", 0 );
      return;
   }
   
   imported_map = XmlManager::MainImportMap();
   imported_map->SetMapType( mtype ); // explicitly set type

   if( mtype == PLUGIN ) {
      parent_stub->InstallNewSubmap( imported_map );
      PlaceMap( displayed_map );
   }
   else
      SetMap( imported_map ); // install main imported map in editor window

   entered_label = this->UniqueMapName( "Enter the new name for the primary imported map. ", imported_map->MapLabel() );
   if( entered_label ) {
      if( mtype == PLUGIN )
	 imported_map->MapLabel( entered_label );
      else
	 this->RenameMap( imported_map, entered_label );
   }
   
   SetPromptUserForSave();
   freeze_editor = FALSE;
}

bool DisplayManager::TransferableRootMaps()
{
   Map *map;

   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      map = map_pool->CurrentItem();
      if( map->IsRootMap() && map != CurrentTopLevelMap() )
	 return( TRUE );
   }

   return( FALSE );
}

void DisplayManager::ChooseMap()
{
   SelectMap( NULL );
}

void DisplayManager::VerifyAnnotations()
{ // verifies existence of performance annotations for certain map elements
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
      map_pool->CurrentItem()->VerifyAnnotations();

   DrawScreen();
}

void DisplayManager::DeactivateHighlighting()
{  // removes higlighting for incomplete elements
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
      map_pool->CurrentItem()->DeactivateHighlighting();

   ResponsibilityManager::Instance()->DeactivateHighlighting();
   
   DrawScreen();  
}

void DisplayManager::ListParentMaps()
{
   ListParents( current_map );
}

void DisplayManager::RegisterMap( Map *new_map )
{ 
   Map *map; // add map in proper alphabetical order

   if( current_ucm_set ) current_ucm_set->AddMap( new_map );

   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      map = map_pool->CurrentItem();
      if( strcmp( new_map->MapLabel(), map->MapLabel() ) < 0 ) {
	 map_pool->AddBefore( new_map, map );
	 return;
      }
   }

   map_pool->Add( new_map ); // add map at end of list if greater than all existing names
}

void DisplayManager::PurgeMap( Map *map )
{ 
   map_pool->Delete( map );
}

void DisplayManager::ExpandUCMSet( Map *map )
{
   if( current_ucm_set ) current_ucm_set->AddNewSubmap( map );
}
 
void DisplayManager::RegisterFigure( HyperedgeFigure *new_figure )
{ 
   figure_pool->Add( new_figure );
}

void DisplayManager::PurgeFigure( HyperedgeFigure *figure )
{
   figure_pool->Delete( figure );
   selected_figures.Delete( figure );
}

void DisplayManager::RegisterPath( Path *new_path )
{ 
   path_pool->Add( new_path );
}

void DisplayManager::PurgePath( Path *path )
{ 
   path_pool->Delete( path );
}

void DisplayManager::Draw( Presentation *ppr )
{    
   Path *path;
   HyperedgeFigure *figure;
   
   for( path_pool->First(); !path_pool->IsDone(); path_pool->Next() ) {
      path = path_pool->CurrentItem();
      if( path->Destroyed() ) {
	 path_pool->RemoveCurrentItem();
	 delete path;
      } else {
	 if( path->IsVisible() ) {
	    if( path->Changed() )	       
	       path->Interpolate();
	    path->DrawPath( ppr );
	 }
      }
   }

   for( figure_pool->First(); !figure_pool->IsDone(); figure_pool->Next() ) {
      figure = figure_pool->CurrentItem();
      if( figure->IsVisible() )
	 figure->Draw( ppr );
   }

   if( marker_figure )
      marker_figure->Draw( ppr );

   if( sbx != NO_SELECTION_BOX ) {  // draw selection box if set
      
      float X[4] = { sbx, ebx, ebx, sbx };
      float Y[4] = { sby, sby, eby, eby };

      ppr->SetLinePattern( LINE_DOUBLE_DASHED );
      ppr->DrawPoly( X, Y, 4 );
      ppr->SetLinePattern( LINE_SOLID );
      sbx = NO_SELECTION_BOX;
   }
}

void DisplayManager::DrawSelectionBox( Presentation *ppr, float x1, float y1, float x2, float y2  )
{
   float x, y, lb, rb, tb, bb;
   HyperedgeFigure *figure;

// set coordinates to draw selection box
   sbx = x1;
   sby = y1;
   ebx = x2;
   eby = y2;

   if( x2 > x1 ) {
      lb = x1;
      rb = x2;
   }
   else {
      lb = x2;
      rb = x1;
   }

   if( y2 > y1 ) {
      tb = y1;
      bb = y2;
   }
   else {
      tb = y2;
      bb = y1;
   }

   
   // determine figures inside selection area

   while( !selected_figures.is_empty() )
      selected_figures.Detach();
   
   for( figure_pool->First(); !figure_pool->IsDone(); figure_pool->Next() ){
      figure = figure_pool->CurrentItem();
      figure->GetPosition( x, y );
      if( x > lb && x < rb && y > tb && y < bb ) {
	 figure->SetSelected();
	 selected_figures.Add( figure );
      }
      else
	figure->ResetSelected(); 
   }
   
   // determine components inside selection area
   component_manager->DetermineSelectedComponents( lb, rb, tb, bb );

   DrawScreen();
   
}

void DisplayManager::ResetSelectedElements()
{
   while( !selected_figures.is_empty() )
      selected_figures.Detach()->ResetSelected();

   component_manager->ResetSelectedComponents();
   current_path = NULL;
}

void DisplayManager::SelectAllElements()
{
   HyperedgeFigure *cf;

   ResetSelectedElements(); 

   for( figure_pool->First(); !figure_pool->IsDone(); figure_pool->Next() ){
      cf = figure_pool->CurrentItem();
      cf->SetSelected();
      selected_figures.Add( cf );
   }

   component_manager->SelectAllComponents();
   DrawScreen();
}

void DisplayManager::AddSelectedFigure( HyperedgeFigure *new_figure )
{
   if( selected_figures.Includes( new_figure ) ) {
      selected_figures.Remove( new_figure );  // perform de-selection
      new_figure->ResetSelected();
   }
   else
      selected_figures.Add( new_figure ); // add to selected list
}

bool DisplayManager::TransformationsApplicable()
{
   int num_selected;

   if( component_manager->ComponentsSelected() )
      return FALSE;

   num_selected = selected_figures.Size();

   if( (num_selected == 1) || (num_selected == 2) ) {
      current_path = selected_figures.Get( num_selected )->GetPath();
      return TRUE;
   }
   else
      return FALSE;
}

bool DisplayManager::SingleComponentSelection()
{
   if( selected_figures.Size() > 0 )
      return FALSE;

   if( component_manager->SingleSelection() )
      return TRUE;
   else
      return FALSE;      
}

void DisplayManager::ShiftSelectedElements( float x_offset, float y_offset )
{
   float x_limit, y_limit, max_x_limit = 0, max_y_limit = 0;

   if( FixedAllPositions == true ) return; // do not move anything as all positions should be fixed
   
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->ValidatePositionShift( x_offset, y_offset, x_limit, y_limit );
      if( x_limit > max_x_limit ) max_x_limit = x_limit;
      if( y_limit > max_y_limit ) max_y_limit = y_limit;
   }

   component_manager->ValidatePositionShifts( x_offset, y_offset, x_limit, y_limit );
   if( x_limit > max_x_limit ) max_x_limit = x_limit;
   if( y_limit > max_y_limit ) max_y_limit = y_limit;

   if( x_offset > 0 )
      x_offset -= max_x_limit;
   else
      x_offset += max_x_limit;

   if( y_offset > 0 )
      y_offset -= max_y_limit;
   else
      y_offset += max_y_limit;

   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() )
      selected_figures.CurrentItem()->ShiftPosition( x_offset, y_offset );
   
   component_manager->ShiftSelectedComponents( x_offset, y_offset );

   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() )
      selected_figures.CurrentItem()->BindComponents();
   
   DrawScreen();
   SetPromptUserForSave();
}

void DisplayManager::ClearDisplay( bool create_map )
{
   design_deletion = TRUE;
   while( !map_pool->is_empty() ) // delete all of the maps in the current design
      delete map_pool->Detach();
   design_deletion = FALSE;
   
   while( !selected_figures.is_empty() )
      selected_figures.Detach();

   component_manager->Reset();
   current_path = NULL;
   
   // clear device and data store directories, UCMSets, control variable list
   DeviceDirectory::Instance()->ClearDirectory();
   DataStoreDirectory::Instance()->ClearDirectory();
   UcmSet::ClearDirectory();
   ScenarioList::ClearScenarioList();
   PathVariableList::ClearVariableList();
   ResponsibilityManager::Instance()->ClearResponsibilityList();
   component_manager->ClearComponentList();
   SetCurrentUCMSet( NULL, FALSE );
   
   design_name[0] = '\0'; // clear previous design name
   if( design_description != NULL ) {
      free( design_description ); // clear previous design description
      design_description = NULL;
   }
   
   // reset global counters 
   Node::ResetNodeCount();
   Hyperedge::ResetHyperedgeCount();
   Responsibility::ResetResponsibilityCount();
   Figure::ResetFigureCount();
   Path::ResetCounter();
   Label::ResetLabelCount();
   Component::ResetComponentCount();
   ComponentReference::ResetComponentReferenceCount();
   Map::ResetMapCount();
   design_id = 0;
   
   ResetScrollbars();

   where_description = DESC_MAP;
   fl_set_input( pfdmMain->Description, "" );
   fl_set_object_label( pfdmMain->Description, "Description of Map" );
   
   if( create_map ) { // create and install new map
      Map *new_map = new Map( "root" );
      SetMap( new_map );
      current_map = new_map;
   }   
}

void DisplayManager::Update()
{ 
   current_path->PathChanged();
}

void DisplayManager::SetActive( HyperedgeFigure *figure )
{
   active_figure = figure;
   SetActiveFigure( active_figure );
   if( figure ) {
      if( !selected_figures.Includes( figure ) )
	 selected_figures.Add( figure );
   }
}

HyperedgeFigure *DisplayManager::FindFigure( float XCoord, float YCoord )
{
   float x, y, current_distance, min_distance = 2.0;
   HyperedgeFigure *closest_figure = NULL;

   for( figure_pool->First(); !figure_pool->IsDone(); figure_pool->Next() ){
      figure_pool->CurrentItem()->GetPosition( x, y );
      current_distance = sqrt( pow((x - XCoord), 2) + pow((y - YCoord), 2) );
      if( current_distance < min_distance ) {
	 min_distance = current_distance;
	 closest_figure = figure_pool->CurrentItem();
      }
   }

   if( min_distance < selection_radius ) { // a path element has been found within the selection distance

      closest_figure->SetSelected();
      active_figure = closest_figure;
      current_label = active_figure->Edge()->PathLabel();
	    
      if( current_path != active_figure->GetPath() )
	 current_path = active_figure->GetPath();
   }
   else { // no elements have been found within range
      active_figure = NULL;
      current_path = NULL;
   }

#ifdef DEBUG	       
   if ( closest_figure ) {
      closest_figure->GetPosition( x, y );
      cout <<  "\n\nEdge Type: " <<  closest_figure->Edge()->EdgeName()
	   << " Name: " << (( closest_figure->Edge()->HyperedgeName() != NULL ) ? closest_figure->Edge()->HyperedgeName() : "" )
	   << " number: " << closest_figure->Edge()->GetNumber() << endl << flush;
//  	   <<  " Minimum distance: " << min_distance
//  	   << ( ( min_distance < selection_radius ) ? " selected" : " not selected" )
//  	   << " label: "  << closest_figure->Edge()->PathLabel()->TextLabel()
//  	   << "Path: " << (void *)closest_figure->GetPath() << " number: " << closest_figure->GetPath()->NumberElements()
//  	   << "\nFigure x: " << x << " y: " << y << " XCoord: " << XCoord << " YCoord: " << YCoord << endl << endl << flush;
   }
#endif

   return( active_figure );
}

void DisplayManager::HandleDoubleClick( float XCoord, float YCoord )
{
   HyperedgeFigure *figure;
   
   if( (figure = this->FindFigure( XCoord, YCoord )) == NULL ) return;
   if( figure->Edge()->DoubleClickAction() == MODIFIED ) {
      DrawScreen();
      SetPromptUserForSave();
   }
}

void DisplayManager::CreateNewPath( float XCoord, float YCoord )
{ 
  marker_figure = new MarkerFigure( XCoord, YCoord );
}

void DisplayManager::ExtendPath( float XCoord, float YCoord )
{ 
   if( current_path ) {
      if( current_path->GetPathEnd()->EdgeType() != RESULT )
	 return;
   }

   if( marker_figure ){

      float mx, my;

      Path *new_path = new Path( this );
      current_path = new_path;
      current_label = new Label( "_", current_map );     // create new label for new path
      
      edges = trans_manager->CreateNewPath();

      HyperedgeFigure *figure1 = edges->GetFirst()->GetFigure();
      HyperedgeFigure *figure2 = edges->Get(2)->GetFigure();
      HyperedgeFigure *figure3 = edges->Get(3)->GetFigure();

      marker_figure->GetPosition( mx, my );

      figure1->SetPosition( mx, my );
      figure2->SetPosition( (XCoord+mx)/2, (YCoord+my)/2 );
      figure3->SetPosition( XCoord, YCoord );

      current_path->AddFigure( figure1 );
      current_path->AddFigure( figure2 );
      current_path->AddFigure( figure3 );

      figure3->SetSelected();
      active_figure = figure3;
      selected_figures.Add( figure3 );
      SetActiveFigure( figure3 );

      if( showing_labels )
	 current_map->NewLabel( edges->Get(2) );

      // delete the marker figure
      delete marker_figure;
      marker_figure = NULL;
      delete edges;    // release collection
      
   } 
   else {

      float eX, eY;
  
      Empty *new_empty = new Empty;
      Node *node1 = new Node( B );
      Node *node2 = new Node( A );
      new_empty->AttachSource( node1 );
      new_empty->AttachTarget( node2 );
    
      Hyperedge *result = current_path->GetPathEnd();
      Node *old_node = result->SourceSet()->GetFirst();
      HyperedgeFigure *result_fig = result->GetFigure();  
      Hyperedge *prev_edge = old_node->PreviousEdge();
      prev_edge->DetachTarget( old_node );
      prev_edge->AttachTarget( node1 );
      result->DetachSource( old_node );
      result->AttachSource( node2 );

      current_map->MapHypergraph()->PurgeNode( old_node );

      result_fig->GetPosition( eX, eY );
      new_empty->GetFigure()->SetPosition( eX, eY );
      result_fig->SetPosition( XCoord, YCoord );
      current_path->AddBeforeFigure( new_empty->GetFigure(), result_fig );
    
      if( active_figure != result_fig )
      {
	 if( active_figure )
	    active_figure->ResetSelected();
	 active_figure = result_fig;
	 active_figure->SetSelected();
	 SetActiveFigure( active_figure );
	 selected_figures.Add( result_fig );
      }
   }

}

void DisplayManager::CreateNewSegment()
{
   current_path = new Path( this );
   current_label = new Label( "-", current_map );     // create new label for new path
}

void DisplayManager::TransApplicableTo( Hyperedge *edge )
{ 
   trans_manager->TransApplicableTo( edge );
}

void DisplayManager::TransApplicableToPair( Hyperedge *edge1, Hyperedge *edge2 )
{ 
   trans_manager->TransApplicableToPair( edge1, edge2 );
}

void DisplayManager::Add( Hyperedge *new_edge, Hyperedge *ref_edge )
{ 
   float fX, fY;
   ref_edge->GetFigure()->GetPosition( fX, fY );
   new_edge->GetFigure()->SetPosition( fX, fY );
   current_path->AddBeforeFigure( new_edge->GetFigure(), ref_edge->GetFigure() );
}

void DisplayManager::AddBL( Hyperedge *new_edge, Hyperedge *ref_edge )
{ 
   float fX1, fY1, fX2, fY2;

   ref_edge->GetFigure()->GetPosition( fX1, fY1 );
   HyperedgeFigure *prev_figure = ref_edge->FirstInput()->GetFigure();
   prev_figure->GetPosition( fX2, fY2 );
   new_edge->GetFigure()->SetPosition( (fX1+fX2)/2, (fY1+fY2)/2 );
   current_path->AddBeforeFigure( new_edge->GetFigure(), ref_edge->GetFigure() );
}

void DisplayManager::AddBR( Hyperedge *new_edge, Hyperedge *ref_edge )
{ 
   float fX1, fY1, fX2, fY2;

   ref_edge->GetFigure()->GetPosition( fX1, fY1 );
   HyperedgeFigure *next_figure = ref_edge->FirstOutput()->GetFigure();
   next_figure->GetPosition( fX2, fY2 );
   new_edge->GetFigure()->SetPosition( (fX1+fX2)/2, (fY1+fY2)/2 );
   current_path->AddAfterFigure( new_edge->GetFigure(), ref_edge->GetFigure() );
}

void DisplayManager::AddBetween( Hyperedge *new_edge, Hyperedge *left_edge, Hyperedge *right_edge )
{
   float fX1, fY1, fX2, fY2;

   left_edge->GetFigure()->GetPosition( fX1, fY1 );
   right_edge->GetFigure()->GetPosition( fX2, fY2 );
   new_edge->GetFigure()->SetPosition( (fX1+fX2)/2, (fY1+fY2)/2 );
   current_path->AddAfterFigure( new_edge->GetFigure(), left_edge->GetFigure() );
}

void DisplayManager::AddRight( Hyperedge *new_edge, Hyperedge *ref_edge )
{ 
   float fX, fY;

   ref_edge->GetFigure()->GetPosition( fX, fY );
   new_edge->GetFigure()->SetPosition( fX+XSpacing, fY+YSpacing );
   current_path->AddAfterFigure( new_edge->GetFigure(), ref_edge->GetFigure() );
}

void DisplayManager::AddRight( Figure *new_figure, Figure *ref_figure )
{ 

   float fX, fY;
   ref_figure->GetPosition( fX, fY );
   ((HyperedgeFigure *)new_figure)->SetPosition( fX+XSpacing, fY+YSpacing );
   current_path->AddAfterFigure( new_figure, ref_figure );
   
}

void DisplayManager::AddLeft( Hyperedge *new_edge, Hyperedge *ref_edge )
{ 

   float fX, fY;
   ref_edge->GetFigure()->GetPosition( fX, fY );
   new_edge->GetFigure()->SetPosition( fX-XSpacing, fY-YSpacing );
   current_path->AddBeforeFigure( new_edge->GetFigure(), ref_edge->GetFigure() );
}

void DisplayManager::AddFirst( Hyperedge *edge )
{
   current_path->AddFigure( edge->GetFigure() );
}

void DisplayManager::AddAfter( Hyperedge *new_edge, Hyperedge *ref_edge )
{ 

   float fX1, fY1, fX2, fY2;
   ref_edge->GetFigure()->GetPosition( fX1, fY1 );
   HyperedgeFigure *next_figure = ref_edge->FirstOutput()->GetFigure();
   next_figure->GetPosition( fX2, fY2 );
   new_edge->GetFigure()->SetPosition( (fX1+fX2)/2, (fY1+fY2)/2 );
   current_path->AddAfterFigure( new_edge->GetFigure(), ref_edge->GetFigure() );
}

void DisplayManager::AddAtPosition(  Hyperedge *new_edge, Hyperedge *ref_edge, Hyperedge *next_edge )
{
   float fX, fY;

   ref_edge->GetFigure()->GetPosition( fX, fY );
   new_edge->GetFigure()->SetPosition( fX, fY );

   current_path->AddBeforeFigure(  new_edge->GetFigure(), next_edge->GetFigure() );
}

void DisplayManager::AddFirstToRight( Hyperedge *new_edge, Hyperedge *ref_edge )
{
   float fX, fY;

   ref_edge->GetFigure()->GetPosition( fX, fY );
   new_edge->GetFigure()->SetPosition( fX+XSpacing, fY+YSpacing );

   current_path->AddFigure( new_edge->GetFigure() );
}

void DisplayManager::CreateStub( Stub *stub )
{  
   // split main path 
   Hyperedge *next_empty = stub->FirstOutput();
   display_manager->SplitPaths( stub, next_empty );

   // add stub as start of new path
   Path *new_path = next_empty->GetFigure()->GetPath();
   new_path->AddStartFigure( stub->GetFigure() );
   new_path->SetPathStart( stub );
   ((StubFigure *)stub->GetFigure())->GetCurves()->Add( new_path );
   current_path = NULL;
}

void DisplayManager::DeleteStub( Stub *stub )
{
   Hyperedge *next_empty = stub->FirstOutput();
   Path *next_path = next_empty->GetFigure()->GetPath();
   
   next_path->PurgeFigure( stub->GetFigure() );
   ((StubFigure *)stub->GetFigure())->GetCurves()->Delete( next_path );

   JoinPaths( stub, next_empty ); 
}

void DisplayManager::CreateWaitSynch( WaitSynch *wait_synch )
{
   // split main path 
   Hyperedge *next_empty = wait_synch->FirstOutput();
   display_manager->SplitPaths( wait_synch, next_empty );

   // add wait_synch as start of new path
   Path *new_path = next_empty->GetFigure()->GetPath();
   new_path->AddStartFigure( wait_synch->GetFigure() );
   new_path->SetPathStart( wait_synch );
   current_path = NULL;
}

void DisplayManager::DeleteWaitSynch( WaitSynch *wait_synch )
{
   Hyperedge *next_empty = wait_synch->FirstOutput();
   Path *next_path = next_empty->GetFigure()->GetPath();
   
   next_path->PurgeFigure( wait_synch->GetFigure() );
   JoinPaths( wait_synch, next_empty ); 
}

void DisplayManager::CreateStubJoin( Hyperedge *edge, Hyperedge *stub )
{
   HyperedgeFigure *figure = edge->GetFigure();
   edge_type type = edge->EdgeType();
   
   if( type == RESULT ) {
      figure->GetPath()->AddFigure( stub->GetFigure() );
      figure->GetPath()->SetPathEnd( stub );
   }
   else {  // edge is a start
      figure->GetPath()->AddStartFigure( stub->GetFigure() );
      figure->GetPath()->SetPathStart( stub );
   }
   ((StubFigure *)stub->GetFigure())->GetCurves()->Add( figure->GetPath() );

}

void DisplayManager::DisconnectStubJoin( Hyperedge *empty, Hyperedge *stub, Hyperedge *result )
{  
   empty->GetFigure()->GetPath()->PurgeFigure( stub->GetFigure() );
   ((StubFigure *)stub->GetFigure())->GetCurves()->Delete( empty->GetFigure()->GetPath() );
   empty->GetFigure()->GetPath()->SetPathEnd( result );
}

void DisplayManager::DisconnectStubFork( Hyperedge *empty, Hyperedge *stub, Hyperedge *start )
{  
   empty->GetFigure()->GetPath()->PurgeFigure( stub->GetFigure() );
   ((StubFigure *)stub->GetFigure())->GetCurves()->Delete( empty->GetFigure()->GetPath() );
   empty->GetFigure()->GetPath()->SetPathStart( start );
}

void DisplayManager::DeleteEndNull( Hyperedge *edge )
{
   // delete the end figure of the path corresponding to the null figure 
   // entering an and join
   edge->GetFigure()->GetPath()->DeleteEndFigure();
}

void DisplayManager::DeleteStartNull( Hyperedge *edge )
{
   // delete the start figure of the path corresponding to the null figure 
   // exiting an and fork
   edge->GetFigure()->GetPath()->DeleteStartFigure();
}

void DisplayManager::CreateAndFork( Synchronization *synch, Empty *empty )
{

   SynchNullFigure *null_figure;
   SynchronizationFigure *synch_figure;
   float fx, fy, ex, ey;

   // split main path 

   Hyperedge *prev_empty = synch->FirstInput();
   Hyperedge *next_empty = synch->FirstOutput();
   SplitPaths( synch, next_empty );

   prev_empty->GetFigure()->GetPath()->SetPathEnd( synch );

   // install last point of path entering and fork

   synch_figure = (SynchronizationFigure *)synch->GetFigure();
   current_path = prev_empty->GetFigure()->GetPath();
   current_path->SetPathEnd( synch );
   current_path->PurgeFigure( synch_figure );
   null_figure = new SynchNullFigure( synch_figure, current_path, INPUT );
   null_figure->SetPosition( -STRAIGHT_PATH_SEGMENT, CENTERED );
   current_path->AddEndFigure( null_figure );
   
  // install first point of new path
   
   current_path = next_empty->GetFigure()->GetPath();
   current_path->SetPathStart( synch );
   null_figure = new SynchNullFigure( synch_figure, current_path, OUTPUT );
   null_figure->SetPosition( STRAIGHT_PATH_SEGMENT, AND_FORK_PATH_TOP );
   current_path->AddStartFigure( null_figure );

   // raise height of next empty if necessary
   synch->GetFigure()->GetPosition( fx, fy );
   next_empty->GetFigure()->GetPosition( ex, ey );
   if( ey > ( fy+AND_FORK_PATH_TOP ) )
      next_empty->GetFigure()->SetPosition(ex, (fy+AND_FORK_PATH_TOP) ); 
   
   current_path->PathChanged();
  
   // install first points of fork path

   current_path = empty->GetFigure()->GetPath();
   null_figure = new SynchNullFigure( synch_figure, current_path, OUTPUT );
   null_figure->SetPosition( STRAIGHT_PATH_SEGMENT, AND_FORK_PATH_BOTTOM ); 
   current_path->AddStartFigure( null_figure );
   current_path->PathChanged();
   if( showing_labels )
      current_map->EditLabel( empty, "Enter new Label for And Fork path" );  // prompt user to enter label of new fork path 
}

void DisplayManager::CreateOrFork( OrFork *fork, Empty *empty )
{
   OrNullFigure *null_figure;
   OrFigure *or_figure;
   float fx, fy, ex, ey;
   
   // split main path and install first point of new path

   Hyperedge *previous_empty = fork->FirstInput();
   Hyperedge *next_empty = fork->FirstOutput();
   SplitPaths( fork, next_empty );

   previous_empty->GetFigure()->GetPath()->SetPathEnd( fork );

   current_path = next_empty->GetFigure()->GetPath();
   current_path->SetPathStart( fork );
   or_figure = (OrFigure *)fork->GetFigure();
   null_figure = new OrNullFigure( or_figure, current_path );
   current_path->AddStartFigure( null_figure );

   // raise height of next empty if necessary
   fork->GetFigure()->GetPosition( fx, fy );
   next_empty->GetFigure()->GetPosition( ex, ey );
   if( ey > ( fy-OR_FORK_PATH ) )
      next_empty->GetFigure()->SetPosition( ex, (fy-OR_FORK_PATH) ); 
   
   current_path->PathChanged();

   // install first points of fork path

   current_path = empty->GetFigure()->GetPath();
   null_figure = new OrNullFigure( or_figure, current_path );
   current_path->AddStartFigure( null_figure );
   current_path->PathChanged();
   
   if( showing_labels ) {
      DrawScreen();  // redraw screen to show branch for labelling
      current_map->EditLabel( empty, "Enter new Label for Or Fork branch" ); // prompt user to enter path label of new branch
   }
}

void DisplayManager::AddOrForkBranch( OrFork *fork, Empty *empty )
{
   float fX, fY;
   
   OrNullFigure *null_figure;
   OrFigure *or_figure = (OrFigure *)fork->GetFigure();

   // add the null figure as the first figure of the spline
   paths_complete = FALSE;
   null_figure = new OrNullFigure( or_figure, current_path, FALSE );
   current_path->AddStartFigure( null_figure );
   fork->GetFigure()->GetPosition( fX, fY );
   empty->GetFigure()->SetPosition( fX+XSpacing, fY+YSpacing+OR_FORK_PATH );
   current_path->AddFigure( empty->GetFigure() );
   AddRight( empty->FirstOutput(), empty );
   paths_complete = TRUE;
   or_figure->OrderPaths();
   
   if( showing_labels ) {
      DrawScreen();  // redraw screen to show branch for labelling
      current_map->EditLabel( empty, "Enter new Label for Or Fork branch" ); // prompt user to enter path label of new branch
   }

}

void DisplayManager::CreateLoop( Loop *loop, Empty *in_empty, Empty *out_empty )
{
   LoopFigure *loop_figure;
   LoopNullFigure *null_figure;
   HyperedgeFigure *figure;
   float x, y;

   loop->GetFigure()->GetPosition( x, y );
   
   // split main path
   Hyperedge *previous_empty = loop->FirstInput();
   Hyperedge *next_empty = loop->FirstOutput();
   SplitPaths( loop, next_empty );

   // install start of outgoing main path
   current_path = next_empty->GetFigure()->GetPath();
   current_path->SetPathStart( loop );
   loop_figure = (LoopFigure *)loop->GetFigure();
   null_figure = new LoopNullFigure( loop_figure, current_path, MAIN_OUT );
   current_path->AddStartFigure( null_figure );

   // install end of incoming main path
   current_path = previous_empty->GetFigure()->GetPath();
   current_path->SetPathEnd( loop );
   null_figure = new LoopNullFigure( loop_figure, current_path, MAIN_IN );
   current_path->PurgeFigure( loop_figure );
   current_path->AddEndFigure( null_figure );
   
   // install start of outgoing loop path
   current_path = out_empty->GetFigure()->GetPath();
   current_path->SetPathStart( loop );
   null_figure = new LoopNullFigure( loop_figure, current_path, LOOP_OUT );
   current_path->AddStartFigure( null_figure );
   out_empty->GetFigure()->SetPosition( x+2*XSpacing, y-2*YSpacing ); // position of loop output empty
   current_path->AddAfterFigure( out_empty->GetFigure(), null_figure );
   figure = out_empty->FirstOutput()->GetFigure();
   figure->SetPosition( x, y-4*YSpacing ); //  position of middle empty
   current_path->AddAfterFigure( figure, out_empty->GetFigure() );

   // install end of incoming loop path
   current_path->SetPathEnd( loop );
   null_figure = new LoopNullFigure( loop_figure, current_path, LOOP_IN );
   current_path->AddEndFigure( null_figure );
   in_empty->GetFigure()->SetPosition( x-2*XSpacing, y-2*YSpacing ); // position of loop input empty
   current_path->AddBeforeFigure( in_empty->GetFigure(), null_figure );
   loop_figure->PathChanged(); // notify all connected paths
}

void DisplayManager::AddTimeoutPath( Timer *timer, Empty *empty )
{
   float fX, fY;
   HyperedgeFigure *result_fig;
    
   // add timer as start of new path
   current_path->AddStartFigure( timer->GetFigure() );
   current_path->SetPathStart( timer ); // set path start to timer

   timer->GetFigure()->GetPosition( fX, fY );
   empty->GetFigure()->SetPosition( fX+XSpacing, fY-YSpacing );
   current_path->AddFigure( empty->GetFigure() );
   result_fig = empty->FirstOutput()->GetFigure();
   result_fig->SetPosition( fX+2*XSpacing, fY-2*YSpacing );
   current_path->AddAfterFigure( result_fig, empty->GetFigure() );    
   current_path = NULL;
}

void DisplayManager::DeleteTimeoutPath( Timer *timer )
{
   Path *tp = timer->ExceptionPath()->NextEdge()->FirstOutput()->GetFigure()->GetPath();
   tp->PurgeFigure( timer->GetFigure() );
}

void DisplayManager::CreateOrJoin( OrJoin *join, Empty *empty )
{
  OrNullFigure *null_figure;
  OrFigure *or_figure;
  
   // split main path and install last points of incoming path
   Hyperedge *prev_empty = join->FirstInput();
   SplitPaths( prev_empty, join );
  
   // change the path stored in the null figure at the beginning of the recently
   // split path to that of the new first section  
   prev_empty->GetFigure()->GetPath()->SetPathEnd( join );

   current_path = prev_empty->GetFigure()->GetPath();
   or_figure = (OrFigure *)join->GetFigure();
   null_figure = new OrNullFigure( or_figure, current_path );
   current_path->AddEndFigure( null_figure );
   current_path->PathChanged();

   // install last points of fork path

   current_path = empty->GetFigure()->GetPath();
   null_figure = new OrNullFigure( or_figure, current_path );  
   current_path->AddEndFigure( null_figure );
   current_path->PathChanged();

   if( showing_labels ) {
      DrawScreen();
      current_map->EditLabel( join, "Enter new label for or join output path" );
   }

}

void DisplayManager::CreateAndJoin( Synchronization *synch, Empty *empty )
{
   SynchNullFigure *null_figure;
   SynchronizationFigure *synch_figure;
   Path *path;
  
   // split main path 
   Hyperedge *prev_empty = synch->FirstInput();
   SplitPaths( prev_empty, synch );
  
   // change the path stored in the null figure at the beginning of the recently
   // split path to that of the new first section
   path = prev_empty->GetFigure()->GetPath();
   path->SetPathEnd( synch );

   // install first point of path exiting and join
   synch_figure = (SynchronizationFigure *)synch->GetFigure();
   current_path = synch->FirstOutput()->GetFigure()->GetPath();
   current_path->SetPathStart( synch );
   current_path->PurgeFigure( synch_figure );
   null_figure = new SynchNullFigure( synch_figure, current_path, OUTPUT );
   null_figure->SetPosition( STRAIGHT_PATH_SEGMENT, CENTERED );
   current_path->AddStartFigure( null_figure );
   
   // install last points of incoming path
   current_path = prev_empty->GetFigure()->GetPath();
   null_figure = new SynchNullFigure( synch_figure, current_path, INPUT );
   null_figure->SetPosition( -STRAIGHT_PATH_SEGMENT, -AND_FORK_PATH );
   current_path->AddEndFigure( null_figure );
   current_path->PathChanged();

   // install last points of fork path
   current_path = empty->GetFigure()->GetPath();
   null_figure = new SynchNullFigure( synch_figure, current_path, INPUT );  
   null_figure->SetPosition( -STRAIGHT_PATH_SEGMENT, AND_FORK_PATH ); 
   current_path->AddEndFigure( null_figure );
   current_path->PathChanged();

   if( showing_labels ) {
      DrawScreen();
      current_map->EditLabel( synch, "Enter new Label for And Join path" );  // prompt user to enter label of join path 
   }
}

void DisplayManager::DisconnectJoin( Hyperedge *empty, Hyperedge *result )
{
   DeleteEndNull( empty );
   empty->GetFigure()->GetPath()->SetPathEnd( result );
}

void DisplayManager::AddAndForkBranch( Synchronization *synch, Empty *empty, Result *result  )
{

   float fX, fY, Yoffset;
   SynchronizationFigure *synch_figure;
   SynchNullFigure *null_figure;

   synch_figure = (SynchronizationFigure *)synch->GetFigure();
   int num_paths = synch->OutputCount();
   
   Yoffset = (floor( (num_paths-1)/2.0 )*2+1)*AND_FORK_PATH;
   paths_complete = FALSE;
   synch->GetFigure()->GetPosition( fX, fY );
   empty->GetFigure()->SetPosition( fX+XSpacing, fY+YSpacing+Yoffset );

   current_path->AddFigure( empty->GetFigure() );  // add the empty as the first figure of the spline

   // add the null figure as the first figure of the spline
   null_figure = new SynchNullFigure( synch_figure, current_path, OUTPUT );
   current_path->AddStartFigure( null_figure );
   current_path->PathChanged();

   AddRight( (Hyperedge *)result, empty );  // add result to bottom right of empty
   paths_complete = TRUE;
   synch_figure->OrderPaths();
   
   if( showing_labels ) {
      DrawScreen();
      current_map->EditLabel( empty, "Enter new Label for And Fork path" );  // prompt user to enter label of new fork path 
   }

}

void DisplayManager::AddAndJoinBranch( Synchronization *synch, Empty *empty )
{

   SynchronizationFigure *synch_figure = (SynchronizationFigure *)synch->GetFigure();
   current_path = empty->GetFigure()->GetPath();
   
   // add the null figure as the last figure of the spline
   SynchNullFigure *null_figure = new SynchNullFigure( synch_figure, current_path, INPUT );
   empty->GetFigure()->GetPath()->SetPathEnd( (Hyperedge *)synch );
   current_path->AddEndFigure( null_figure );
   current_path->PathChanged();
   
   if( showing_labels ) {
      DrawScreen();
      current_map->EditLabel( synch, "Enter new Label for And Join path" );  // prompt user to enter label of join path 
   }
}

void DisplayManager::AddOrJoinBranch( OrJoin *join, Empty *empty )
{

   OrFigure *or_figure = (OrFigure *)join->GetFigure();
   current_path = empty->GetFigure()->GetPath();
   
   // add the null figure as the last figure of the spline
   OrNullFigure *null_figure = new OrNullFigure( or_figure, current_path );
   empty->GetFigure()->GetPath()->SetPathEnd( (Hyperedge *)join );
   current_path->AddEndFigure( null_figure );
   current_path->PathChanged();
   
   if( showing_labels ) {
      DrawScreen();
      current_map->EditLabel( join, "Enter new Label for Or Join path" );  // prompt user to enter label of join path 
   }
}

void DisplayManager::SplitPaths( Hyperedge *end_edge, Hyperedge *start_edge )
{
   Label *new_label = new Label( start_edge->PathLabel()->TextLabel(), current_map );
   Path *first_path = new Path( this, FALSE );
   Path *second_path = new Path( this, FALSE );
   Path *old_path;
   
   Cltn<Figure *> *old_path_figs;
   Cltn<Figure *> *first_path_figs = new Cltn<Figure *>;
   Cltn<Figure *> *second_path_figs = new Cltn<Figure *>;

   if( end_edge->GetFigure()->GetPath() != NULL ) {
      old_path = end_edge->GetFigure()->GetPath();      
      old_path_figs = old_path->GetFigures();
   } else {
      old_path = start_edge->GetFigure()->GetPath();
      old_path_figs = old_path->GetFigures();
   }

   old_path_figs->SplitCltn( start_edge->GetFigure(), first_path_figs, second_path_figs );

   first_path->SetFigures( first_path_figs ); 
   first_path->SetPathStart( old_path->GetPathStart() );
   first_path->SetPathEnd( end_edge );

   second_path->SetFigures( second_path_figs );
   second_path->SetPathStart( start_edge );
   second_path->SetPathEnd( old_path->GetPathEnd() );
   
   PropagateForward( start_edge, second_path, old_path, new_label );
   PropagateBackward( end_edge, first_path, old_path );
       
   delete old_path;

   first_path->PathChanged();
   second_path->PathChanged();
  
}

void DisplayManager::JoinPaths( Hyperedge *end_edge, Hyperedge *start_edge )
{
   Path *new_path = new Path( this, FALSE );
   Path *first_path = end_edge->InputPath(),
      *second_path = start_edge->GetFigure()->GetPath();
   Cltn<Figure *> *new_path_figs = new Cltn<Figure *>;

   new_path_figs->JoinCltn( first_path->GetFigures(), second_path->GetFigures() );

   new_path->SetFigures( new_path_figs );
   new_path->SetPathStart( first_path->GetPathStart() );
   new_path->SetPathEnd( second_path->GetPathEnd() );
   
   PropagateForward( start_edge, new_path, second_path, end_edge->PathLabel() );
   PropagateBackward( end_edge, new_path, first_path );
  
   current_path = new_path;
   delete first_path;
   delete second_path;
  
   new_path->PathChanged();
}

void DisplayManager::PropagateForward( Hyperedge *split_edge, Path *new_path, Path *old_path, Label *new_label  )
{
   Cltn<Node *> *target = split_edge->TargetSet();
   Hyperedge *next_edge;
  
   split_edge->ReplacePath( new_path, old_path, new_label, FORWARD_SEARCH );

   for( target->First(); !target->IsDone(); target->Next() ){
      next_edge = target->CurrentItem()->NextEdge();
      if( next_edge->ReplacePath( new_path, old_path, new_label, FORWARD_SEARCH ) )
	 PropagateForward( next_edge, new_path, old_path, new_label );
   }
}

void DisplayManager::PropagateBackward( Hyperedge *split_edge, Path *new_path, Path *old_path, Label *new_label  )
{
   Cltn<Node *> *source = split_edge->SourceSet();
   Hyperedge *previous_edge;

   split_edge->ReplacePath( new_path, old_path, new_label, BACKWARD_SEARCH );

   for( source->First(); !source->IsDone(); source->Next() ){
      previous_edge = source->CurrentItem()->PreviousEdge();
      if( previous_edge->ReplacePath( new_path, old_path, new_label, BACKWARD_SEARCH ) )
	 PropagateBackward( previous_edge, new_path, old_path, new_label );
   }
}


void DisplayManager::Connect( Hyperedge *edge1, Hyperedge *edge2 )
{
   float ftx, fty, f1x, f1y, f2x, f2y;
   HyperedgeFigure *fig1 = edge1->GetFigure(),
      *fig2 = edge2->GetFigure();

   // move the triggering edge( either result or empty ) to touch the
   // waiting place and register each as a dependent figure of each other

   fig1->GetPosition( f1x, f1y );  
   fig2->GetPosition( f2x, f2y );
 
   if( fabs((f1x-f2x)) >  fabs((f1y-f2y)) ) {
      ftx = ((f1x < f2x) ? f2x-TIMER_FIG_SIZE : f2x+TIMER_FIG_SIZE );
      fty = f2y;
   }
   else {
      ftx = f2x;
      fty = ((f1y < f2y) ? f2y-TIMER_FIG_SIZE : f2y+TIMER_FIG_SIZE );
   }
   
   fig1->SetPosition( ftx, fty );
   fig1->DependentFigure( fig2 );
   fig2->DependentFigure( fig1 );
   fig1->GetPath()->PathChanged();
   fig2->GetPath()->PathChanged();
}

void DisplayManager::Disconnect( Hyperedge *edge1, Hyperedge *edge2 )
{
   edge1->GetFigure()->DependentFigure( NULL );
   edge2->GetFigure()->DependentFigure( NULL );
}

void DisplayManager::DesignName( const char *new_name )
{
   strncpy( design_name, new_name, LABEL_SIZE );
}

void DisplayManager::DesignDescription( const char *new_description )
{
   if( new_description != NULL ) {
      if( design_description != NULL ) {
	 free( design_description );
	 design_description = NULL;
      }
      if( strlen( new_description ) > 0 )
	 design_description = strdup( new_description );
   }
}

void DisplayManager::AlignTop()
{
   float x, y, min_y = 10.0;

   component_manager->AlignTop();

   if( selected_figures.Size() < 2 ) return;
   
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      if( y < min_y ) min_y = y;
   }

   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      selected_figures.CurrentItem()->SetPosition( x, min_y );
   }
}

void DisplayManager::AlignBottom()
{
   float x, y, max_y = 0.0;

   component_manager->AlignBottom();

   if( selected_figures.Size() < 2 ) return;
   
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      if( y > max_y ) max_y = y;
   }

   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      selected_figures.CurrentItem()->SetPosition( x, max_y );
   }
}

void DisplayManager::CenterVertically()
{
   float x, y, mid_y, min_y = 10.0, max_y = 0.0;

   component_manager->CenterVertically();

   if( selected_figures.Size() < 2 ) return;
   
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      if( y < min_y ) min_y = y;
      if( y > max_y ) max_y = y;
   }

   mid_y = (min_y + max_y)/2;
   
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      selected_figures.CurrentItem()->SetPosition( x, mid_y );
   }
}

void DisplayManager::AlignLeft()
{
   float x, y, min_x = 10.0;

   component_manager->AlignLeft();

   if( selected_figures.Size() < 2 ) return;
   
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      if( x < min_x ) min_x = x;
   }

   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      selected_figures.CurrentItem()->SetPosition( min_x, y );
   }
}

void DisplayManager::AlignRight()
{
   float x, y, max_x = 0.0;

   component_manager->AlignRight();

   if( selected_figures.Size() < 2 ) return;
   
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      if( x > max_x ) max_x = x;
   }

   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      selected_figures.CurrentItem()->SetPosition( max_x, y );
   }
}

void DisplayManager::CenterHorizontally()
{
   float x, y, mid_x, min_x = 10.0, max_x = 0.0;

   component_manager->CenterHorizontally();

   if( selected_figures.Size() < 2 ) return;

   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      if( x < min_x ) min_x = x;
      if( x > max_x ) max_x = x;
   }

   mid_x = (min_x + max_x)/2;
   
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      selected_figures.CurrentItem()->GetPosition( x, y );
      selected_figures.CurrentItem()->SetPosition( mid_x, y );
   }
}

void DisplayManager::DistributeHorizontally()
{
   HyperedgeFigure *list[50], *ordered_list[50];
   float x, y, min_x, max_x, spacing;
   int i = 0, j, min_index, num_selected;
   
   component_manager->DistributeHorizontally();

   if( (num_selected = selected_figures.Size()) < 3 ) return;

   // populate unordered list
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() )
      list[i++] = selected_figures.CurrentItem();

   // create ordered list of figures in increasing x-coordinate
   for( i = 0; i < num_selected; i++ ) {
      min_x = 10.0;
      for( j = 0; j < num_selected; j++ ) {
	 if( list[j] != NULL ){
	    list[j]->GetPosition( x, y );
	    if( x < min_x ) {
	       min_x = x;
	       min_index = j;
	    }
	 }
      }
      ordered_list[i] = list[min_index];
      list[min_index] = NULL;
   }

   ordered_list[0]->GetPosition( min_x, y );
   ordered_list[num_selected-1]->GetPosition( max_x, y );
   spacing = (max_x - min_x)/(num_selected-1);
   
   // adjust x-coordinates of interior elements ignoring endpoints
   for( i = 1; i < num_selected-1; i++ ) {
      ordered_list[i]->GetPosition( x, y );
      ordered_list[i]->SetPosition( min_x+(i*spacing), y );
   }   
}

void DisplayManager::DistributeVertically()
{
   HyperedgeFigure *list[50], *ordered_list[50];
   float x, y, min_y, max_y, spacing;
   int i = 0, j, min_index, num_selected;
   
   component_manager->DistributeVertically();

   if( (num_selected = selected_figures.Size()) < 3 ) return;
   
   // populate unordered list
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() )
      list[i++] = selected_figures.CurrentItem();

   // create ordered list of figures in increasing y-coordinate
   for( i = 0; i < num_selected; i++ ) {
      min_y = 10.0;
      for( j = 0; j < num_selected; j++ ) {
	 if( list[j] != NULL ){
	    list[j]->GetPosition( x, y );
	    if( y < min_y ) {
	       min_y = y;
	       min_index = j;
	    }
	 }
      }
      ordered_list[i] = list[min_index];
      list[min_index] = NULL;
   }

   ordered_list[0]->GetPosition( x, min_y );
   ordered_list[num_selected-1]->GetPosition( x, max_y );
   spacing = (max_y - min_y)/(num_selected-1);
   
   // adjust x-coordinates of interior elements ignoring endpoints
   for( i = 1; i < num_selected-1; i++ ) {
      ordered_list[i]->GetPosition( x, y );
      ordered_list[i]->SetPosition( x, min_y+(i*spacing) );
   }   
}

bool DisplayManager::MapViewable( Stub *stub, Map *map )
{
  if( ScenarioList::ScenarioTracingMode() && (stub != NULL) )
    return( stub->ScenarioSubmaps()->Includes( map ) );
  else if( current_ucm_set )
    return( current_ucm_set->IncludesMap( map ) );
  else
    return TRUE;
}	

Map * DisplayManager::ViewableMap( Stub *stub, int user_choice )
{  // find map in stub submap list corresponding to user dialog choice depending on navigation mode
   int i = 0;
   Cltn<Map *> *submaps = stub->Submaps();
 
   if( ScenarioList::ScenarioTracingMode() ) {
      for( submaps->First(); !submaps->IsDone(); submaps->Next() ) {
	 if( !stub->ScenarioSubmaps()->Includes( submaps->CurrentItem() ))
	    continue; // remove submaps that are not part of the scenario
	 else {
	    i++;
	    if( i == user_choice )
	       return( submaps->CurrentItem() );
	 }
      }
   }
   else if( current_ucm_set ) {
      for( submaps->First(); !submaps->IsDone(); submaps->Next() ) {
	 if( !current_ucm_set->IncludesMap( submaps->CurrentItem() ) )
	    continue; // implement restricted navigation for ucm sets      
	 else {
	    i++;
	    if( i == user_choice )
	       return( submaps->CurrentItem() );
	 }
      }
   }
//   else
      return( submaps->Get( user_choice ) );      
}

bool DisplayManager::HasViewableSubmaps( Stub *stub )
{
   Cltn<Map *> *submaps = stub->Submaps();
   int submap_count = submaps->Size();

   if( submap_count == 0 )
      return FALSE;

   if( ScenarioList::ScenarioTracingMode() && (submap_count > 1) )
     return( stub->HasScenarioSubmap() );
   else if( current_ucm_set ) {
      for( submaps->First(); !submaps->IsDone(); submaps->Next() ) {
	 if( current_ucm_set->IncludesMap( submaps->CurrentItem() ) )
	    return TRUE; // if at least one submap is found
      }
      return FALSE; // if none of the submaps are in the current ucm set
   }
   else
      return TRUE; // if submaps exist and navigation mode is normal
}

Map * DisplayManager::SoleViewableSubmap( Stub *stub, Empty *empty )
{
  Cltn<Map *> *submaps;
  Cltn<Map *> *filtered_list;
  Map *map, *sole_submap = NULL;
 
  if( ScenarioList::ScenarioTracingMode() ) {
    submaps = stub->ScenarioSubmaps(); // consider only the scenario submap list created during scenario traversal
    if( submaps->Size() == 1 )
      return( submaps->GetFirst() ); // return the only submap as there is no filtration      
    if( empty == NULL )
      return( NULL ); // as there is more than one single scenario submap and no binding information to differentiate there is no single viewable map
    else { // consider binding of empty to start points in stub
      for( submaps->First(); !submaps->IsDone(); submaps->Next() ) {
	map = submaps->CurrentItem();
	if( stub->InputBound( map, empty ) ) {
	  if( sole_submap )
	    return( NULL ); // as second viewable map has been found there is no sole submap, return false with null pointer
	  else
	    sole_submap = map; // first viewable map is found
	}
      }
      return( sole_submap ); // return either the sole submap if one is found or null pointer signifying false if not found
    }
  }
  else if( current_ucm_set ) {
    submaps = stub->Submaps(); // consider all submaps
    if( empty != NULL )
      filtered_list = new Cltn<Map *>;
    for( submaps->First(); !submaps->IsDone(); submaps->Next() ) {
      map = submaps->CurrentItem();
      if( current_ucm_set->IncludesMap( map ) ) {
	if( empty == NULL ) {	    
	  if( sole_submap )
	    return( NULL ); // as second viewable map has been found there is no sole submap, return false with null pointer
	  else
	    sole_submap = map; // first viewable map is found
	}
	else
	  filtered_list->Add( map );
      }
    }
    if( empty == NULL )
      return( sole_submap ); // return either the sole submap if one is found or null pointer signifying false if not found
  }
  else {
    submaps = stub->Submaps(); // consider all submaps
    if( submaps->Size() == 1 )
      return( submaps->GetFirst() ); // return the only submap as there is no filtration
    else {
      if( empty == NULL )
	return( NULL ); // as there is no ucm set defined and there is more than one single submap or no submap at all there is no single viewable map
      else // consider binding of empty to start points in stub
	filtered_list = submaps;
    }
  }

  for( filtered_list->First(); !filtered_list->IsDone(); filtered_list->Next() ) {
    map = filtered_list->CurrentItem();
    if( stub->InputBound( map, empty ) ) {
      if( sole_submap ) {
	sole_submap = NULL; // as second viewable map has been found there is no sole submap, return false with null pointer
	break;
      }
      else
	sole_submap = map; // first viewable map is found
    }

  }

  if( current_ucm_set ) delete filtered_list; // delete the temporary filtered list

  return( sole_submap ); // return either the sole submap if one is found or null pointer signifying false if not found
}

void DisplayManager::ReplaceComponent( Component *old_component, Component *new_component )
{
   if( old_component == new_component ) return;
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
      map_pool->CurrentItem()->ReplaceComponent( old_component, new_component );
}

void DisplayManager::ReplaceResponsibility( Responsibility *old_resp, Responsibility *new_resp )
{
   if( old_resp == new_resp ) return;
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
      map_pool->CurrentItem()->ReplaceResponsibility( old_resp, new_resp );
}

void DisplayManager::DeleteSelectedElements()
{
   HyperedgeFigure *elements[100];
   int count, i;
   edge_type etype;
   bool resp_deleted = FALSE;
   int edges_deleted = 0;

   if( selected_figures.Size() == 0 ) return;

   // determine if any responsibilities are to be deleted
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      if( selected_figures.CurrentItem()->Edge()->EdgeType() == RESPONSIBILITY_REF ) {
	 resp_deleted = TRUE;
	 break;
      }
   }

   // delete start points first as this may delete paths containing other selected elements
   count = 0;
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      if( selected_figures.CurrentItem()->Edge()->EdgeType() == START )
	 elements[count++] = selected_figures.CurrentItem();
   }
   for( i = 0; i < count; i++ )
      edges_deleted += elements[i]->Edge()->DeleteHyperedge();

   // delete non empty hyperedges first as selection may include non-deletable empties
   // between other elements that are about to be deleted
   count = 0;
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      etype = selected_figures.CurrentItem()->Edge()->EdgeType();
      if( (etype != EMPTY) && (etype != START) )
	 elements[count++] = selected_figures.CurrentItem();
   }
   for( i = 0; i < count; i++ )
      edges_deleted += elements[i]->Edge()->DeleteHyperedge();

   count = 0;
   for( selected_figures.First(); !selected_figures.IsDone(); selected_figures.Next() ) {
      if( selected_figures.CurrentItem()->Edge()->EdgeType() == EMPTY )
	 elements[count++] = selected_figures.CurrentItem();
   }
   for( i = 0; i < count; i++ )
      edges_deleted += elements[i]->Edge()->DeleteHyperedge();

   if( resp_deleted )
      UpdateResponsibilityList();
  
   if( edges_deleted > 0 )
      SetPromptUserForSave();
}

Empty * DisplayManager::PathContinuedInParent( Hyperedge *io_point )
{
   if( displayed_map_level > 0 )
      return( hierarchy[displayed_map_level].stub->PathContinued( io_point, hierarchy[displayed_map_level].map ) );
   else
      return NULL;
}

void DisplayManager::ContinuePathInParent( Empty *bound_empty, edge_type type )
{
   if( type == RESULT )
      hierarchy[displayed_map_level-1].map->HighlightExitPoint( bound_empty );
   else
      hierarchy[displayed_map_level-1].map->HighlightEntryPoint( bound_empty );
   InstallParent();
}

void DisplayManager::ExportPerformanceData()
{
  char performance_file_path[128];
  const char *dn;
  
  if( strlen( loaded_file_path ) == 0 ) {
    fl_set_resource( "flAlert.title", "Error: Export Performance Data Failed" );
    fl_show_alert( "The design has not been saved.", "Please save design before perfroming this operation.", "", 0 );
    return;
  }

  strcpy( performance_file_path, loaded_file_path );
  performance_file_path[strlen(performance_file_path)-4] = 0; // remove extension
  strcat( performance_file_path, ".pfm" );

  std::ofstream performance_file( performance_file_path ); // check open file later

  if( !strequal( design_name, "" ) )
    dn = design_name;
  else {
    strcpy( performance_file_path, loaded_file ); // reuse string
    performance_file_path[strlen(performance_file_path)-4] = 0; // remove .ucm extension
    dn = performance_file_path;
  }

  performance_file << "Performance Data for design \"" << dn << "\"\n\n";

   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
     map_pool->CurrentItem()->OutputPerformanceData( performance_file );

  performance_file.close();
}

void DisplayManager::ImportPerformanceData()
{
   const char *filename; //, *entered_label;
   char buffer[200], loop_count[10];
   FILE *fp;
   Map *current_map;
   Start *current_start;
   OrFork *current_fork;
   Stub *current_stub;
   Loop *current_loop;
   ResponsibilityReference *current_resp_ref;

   freeze_editor = TRUE;
   fl_set_fselector_title( "Import Performance File" );
   filename = fl_show_fselector( "Import Performance File", "", "*.pfm", "" );

   if( filename == NULL ) {
      freeze_editor = FALSE;
      return;
   }

   if( !(fp = fopen( filename, "r" )) ) {
      sprintf( buffer, "The file %s ", filename );
      fl_set_resource( "flAlert.title", "Error: File Load Failed" );
      fl_show_alert( buffer, "could not be found.", "", 0 );
      freeze_editor = FALSE;
      return;
   }
   freeze_editor = FALSE;

   while( !feof( fp ) ) {
      if( fgets( buffer, 200, fp ) != NULL ) {
	 if( strncmp( buffer, "Map", 3 ) == 0 ) {
	    buffer[strlen(buffer)-2] = 0; // remove trailing "
	    current_map = FindMap( buffer+5 );
	    if( current_map == NULL ) {
	       strcat( buffer, " not found." );
	       ErrorMessage( buffer, TRUE );
	       ErrorMessage( "Import Performance Data aborted." );
	       fclose( fp );
	       return;
	    }
	 } else if( strncmp( buffer, "Start\t\"", 7 ) == 0 ) {
	    buffer[strlen(buffer)-2] = 0; // remove trailing "
	    current_start = (Start *)current_map->FindElement( START, buffer+7 );
	    if( current_start == NULL ) {
	       strcat( buffer, " not found." );
	       ErrorMessage( buffer, TRUE );
	       ErrorMessage( "Import Performance Data aborted." );
	       fclose( fp );
	       return;
	    }
	    LoadStartPFData( current_start, fp );
	 } else if( strncmp( buffer, "Or Fork\t\"", 9 ) == 0 ) {
	    buffer[strlen(buffer)-2] = 0; // remove trailing "
	    current_fork = (OrFork *)current_map->FindElement( OR_FORK, buffer+9 );
	    if( current_fork == NULL ) {
	       strcat( buffer, " not found." );
	       ErrorMessage( buffer, TRUE );
	       ErrorMessage( "Import Performance Data aborted." );
	       fclose( fp );
	       return;
	    }
	    LoadOrForkPFData( current_fork, fp );
	 } else if( strncmp( buffer, "Stub\t\"", 6 ) == 0 ) {
	    buffer[strlen(buffer)-2] = 0; // remove trailing "
	    current_stub = (Stub *)current_map->FindElement( STUB, buffer+6 );
	    if( current_stub == NULL ) {
	       strcat( buffer, " not found." );
	       ErrorMessage( buffer, TRUE );
	       ErrorMessage( "Import Performance Data aborted." );
	       fclose( fp );
	       return;
	    }
	    LoadStubPFData( current_stub, fp );
	 } else if( strncmp( buffer, "Loop\t\"", 6 ) == 0 ) {
	    buffer[strlen(buffer)-2] = 0; // remove trailing "
	    current_loop = (Loop *)current_map->FindElement( LOOP, buffer+6 );
	    if( current_loop == NULL ) {
	       strcat( buffer, " not found." );
	       ErrorMessage( buffer, TRUE );
	       ErrorMessage( "Import Performance Data aborted." );
	       fclose( fp );
	       return;
	    }
	    fgets( buffer, 200, fp );
		// Jul2005 gM: replaced "&loop_count" with "loop_count" to get rid of GCC warning
	    sscanf( buffer, "loop count\t%s", loop_count );
	    current_loop->LoopCount( loop_count );
	 } else if( strncmp( buffer, "Responsibility\t\"", 16 ) == 0 ) {
	    buffer[strlen(buffer)-2] = 0; // remove trailing "
	    current_resp_ref = (ResponsibilityReference *)current_map->FindElement( RESPONSIBILITY_REF, buffer+16 );
	    if( current_resp_ref == NULL ) {
	       strcat( buffer, " not found." );
	       ErrorMessage( buffer, TRUE );
	       ErrorMessage( "Import Performance Data aborted." );
	       fclose( fp );
	       return;
	    }
	    LoadRespRefPFData( current_resp_ref, fp );
	 }
      }
   }

   fclose( fp );
}

void DisplayManager::LoadStartPFData( Start *start, FILE *fp )
{
   char buffer[50], type[20];
   float input;

   arrival_type arrival = start->ArrivalType();
   
   fgets( buffer, 50, fp ); // arrival type line
   fgets( buffer, 50, fp ); // first input

   if( arrival == EXPERT ) {
      start->ExpertInput( buffer+7 );
      return;
   }
   // Jul2005 gM: replaced "&type" with "type" to get rid of GCC warning
   sscanf( buffer, "%s\t%f", type, &input );
   start->Input( input, 1 );
   
   if( (arrival == UNIFORM) || (arrival == ERLANG) ) {
      fgets( buffer, 50, fp );
      // Jul2005 gM: replaced "&type" with "type" to get rid of GCC warning
	  sscanf( buffer, "%s\t%f", type, &input );
      start->Input( input, 2 );
   }
}

void DisplayManager::LoadOrForkPFData( OrFork *fork, FILE *fp )
{
   char buffer[50];
   float input;
   Cltn<Node *> *outputs = fork->TargetSet();

   for( outputs->First(); !outputs->IsDone(); outputs->Next() ) {
      fgets( buffer, 50, fp );
      fgets( buffer, 50, fp );
      sscanf( buffer, "%f\n", &input );
      fork->BranchSpecification( outputs->CurrentItem() )->Probability( input );
   }
}

void DisplayManager::LoadStubPFData( Stub *stub, FILE *fp )
{
   char buffer[50];
   float input;
   Cltn<Map *> *submaps = stub->Submaps();

   for( submaps->First(); !submaps->IsDone(); submaps->Next() ) {
      fgets( buffer, 50, fp );
      fgets( buffer, 50, fp );
      sscanf( buffer, "%f\n", &input );
      stub->SubmapBinding( submaps->CurrentItem() )->Probability( input );
   }
}

void DisplayManager::LoadRespRefPFData( ResponsibilityReference *resp, FILE *fp )
{
   char buffer[50], amount[10];
//   float input;
   Cltn<ServiceRequest *> *service_requests = resp->ParentResponsibility()->ServiceRequests();

   for( service_requests->First(); !service_requests->IsDone(); service_requests->Next() ) {
      fgets( buffer, 50, fp );
      fgets( buffer, 50, fp );
      // Jul2005 gM: replaced "&amount" with "amount" to get rid of GCC warning
	  sscanf( buffer, "%s\n", amount );
      service_requests->CurrentItem()->Amount( amount );
   }
}


void DisplayManager::ExportDXL()
{
  char dxl_file_path[128];
  const char *dn;
  FILE *outfile;
 
 if( strlen( loaded_file_path ) == 0 ) {
    fl_set_resource( "flAlert.title", "Error: Export DXL Failed" );
    fl_show_alert( "The design has not been saved.", "Please save design before performing this operation.", "", 0 );
    return;
  }

  strcpy( dxl_file_path, loaded_file_path );
  dxl_file_path[strlen(dxl_file_path)-4] = 0; // remove extension
  strcat( dxl_file_path, ".dxl" );
  std::ofstream dxl_file( dxl_file_path ); // check open file later

  if( !strequal( design_name, "" ) )
    dn = design_name;
  else {
    strcpy( dxl_file_path, loaded_file ); // reuse string
    dxl_file_path[strlen(dxl_file_path)-4] = 0; // remove .ucm extension
    dn = dxl_file_path;
  }

  dxl_file << "#include \"addins/UCM/lib/UCMUtilities.dxl\"\n";
  dxl_file << "pragma runLim, 0\n\n";   
  dxl_file << "beginImport( \"" << dn << "\" )\n\n";
  // export devices to DXL
  DeviceDirectory::Instance()->OutputDXL( dxl_file );
  // export components to DXL
  component_manager->OutputDXL( dxl_file );
  // export responsibilities to DXL
  ResponsibilityManager::Instance()->OutputDXL( dxl_file );  
  dxl_file << "\n";  

  dxl_file.close();
  if( !( outfile = fopen( dxl_file_path, "a" )) ) {
	fl_set_resource( "flAlert.title", "Error: Export DXL Failed" );
	fl_show_alert( "The DXL file cannot be opened.", "Please ensure read/write access to disk.", "", 0 );
	return;
   }
  
  // export maps including map elements to DXL 
  XmlManager::SaveFileGraphDXL( outfile );
  // export scenarios to DXL
  ScenarioList::SaveDXL( outfile );
  fprintf( outfile, "\nendImport\n" );
  fclose(outfile);

}

void DisplayManager::ExportCSM()
{
	MapGroupDialog( EXPORTCSM );
}
