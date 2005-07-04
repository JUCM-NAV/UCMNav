
// callbacks for parent map list dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "display.h"
#include "map.h"
#include "stub.h"

extern DisplayManager *display_manager;
static FD_ParentMapList *pfdpml = NULL;
static Cltn<Stub *> *parent_stubs;
static Hyperedge *submap_io = NULL;
static bool selection_made = TRUE;
static int PMLClose( FL_FORM *, void * );

void ListParents( Map *map, Hyperedge *submap_endpoint = NULL );

void ListParents( Map *map, Hyperedge *submap_endpoint )
{
   Stub *current_stub;
   char *map_label;
   int spacing;
   char buffer[150], title[100];
   
   parent_stubs = map->ParentStubs();
   submap_io = submap_endpoint;
   FREEZE_TOOL();

   if( !pfdpml ) {
      pfdpml = create_form_ParentMapList();
      fl_set_browser_fontsize( pfdpml->ParentListBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdpml->ParentListBrowser, FL_FIXEDBOLD_STYLE );
      fl_set_browser_dblclick_callback( pfdpml->ParentListBrowser, PML_Install_cb, 0 );
      fl_set_form_atclose( pfdpml->ParentMapList, PMLClose, NULL );
   }

   fl_clear_browser( pfdpml->ParentListBrowser );

   for( parent_stubs->First(); !parent_stubs->IsDone(); parent_stubs->Next() ) {
      current_stub = parent_stubs->CurrentItem();
      if( submap_endpoint ) {
	 if( !current_stub->BoundToEndpoint( submap_endpoint, map ) )
	    continue; // skip over parent maps that are not bound to submap_endpoint
      }
      map_label = current_stub->ParentMap()->MapLabel();
      spacing = 24 - strlen( map_label );
      spacing = ( spacing >= 5 ) ? spacing : 5;
      sprintf( buffer, "%s%*s%s", map_label, spacing, "", current_stub->HyperedgeName() );
      fl_addto_browser( pfdpml->ParentListBrowser, buffer );
   }

   if( selection_made ) {
      DISABLE_WIDGET( pfdpml->InstallButton );
      selection_made = FALSE;
   }

   if( submap_endpoint ) {
      sprintf( title, "Parent Maps Bound to Path %s %s", 
	       ((submap_endpoint->EdgeType() == START) ? "Start" : "End" ), submap_endpoint->HyperedgeName() );
   } else
      strcpy( title, "Parent Maps of Current Map" );

   fl_show_form( pfdpml->ParentMapList, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT, title );
}

extern "C"
void ParentListBrowser_cb(FL_OBJECT *, long)
{
   if( !selection_made && (fl_get_browser( pfdpml->ParentListBrowser ) != 0  ) ) {
      ENABLE_WIDGET( pfdpml->InstallButton );
      selection_made = TRUE;
   }
}

extern "C"
void PML_Install_cb(FL_OBJECT *, long)
{
   Map *install_map, *current_map;
   int user_choice, i = 1;

   if( (user_choice = fl_get_browser( pfdpml->ParentListBrowser )) == 0) return;
   if( submap_io == NULL )
      install_map = parent_stubs->Get( user_choice )->ParentMap();
   else {
      current_map = submap_io->ParentMap();
      for( parent_stubs->First(); !parent_stubs->IsDone(); parent_stubs->Next() ) {
	 if( !parent_stubs->CurrentItem()->BoundToEndpoint( submap_io, current_map ) )
	    continue; // skip over parent maps that are not bound to submap_io
	 if( i++ == user_choice ) {
	    install_map = parent_stubs->CurrentItem()->ParentMap();
	    break;
	 }
      }
   }

   display_manager->SetMap( install_map );
   PML_Dismiss_cb( 0, 0 );
}

extern "C"
void PML_Dismiss_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdpml->ParentMapList );
   UNFREEZE_TOOL();
}

int PMLClose( FL_FORM *, void * )
{
   PML_Dismiss_cb( 0, 0 );
   return( FL_IGNORE );
}
