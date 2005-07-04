/***********************************************************
 *
 * File:			stub.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:        May 1997
 *                              Feb 2003 
 *                              Shuhua Cui
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "stub.h"
#include "stub_figure.h"
#include "map.h"
#include "empty.h"
#include "hypergraph.h"
#include "start.h"
#include "result.h"
#include "print_mgr.h"
#include "xml_mgr.h"
#include "msc_generator.h"
#include "utilities.h"
#include "variable.h"
#include "scenario.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "scenario_generator.h"

#include "path_data.h"
#include "conditions.h"


#define HIGHLIGHT_PARENT_STUBS 2

Start **Stub::plugin_starts = NULL;
Result **Stub::plugin_results = NULL;
int Stub::start_list_size = 0;
int Stub::result_list_size = 0;
int Stub::number_starts;
int Stub::number_results;

extern void DrawScreen();
extern void EditStubDescription( Stub *new_stub, bool ns = FALSE );
extern void InstallExistingStubExpansion( Stub *stub, bool copy = FALSE );
extern void InstallConditions( ConditionManager &conditions );
extern void ResetConditions();
extern void SelectStubSubmap( Stub *stub, int operation, Empty *empty = NULL );
extern void BindStubPlugin( Stub *stub, Map *plugin );
extern void SelectMap( Stub *s );

extern FD_StubBindingPopup *pfdsbp;
extern FD_Main *pfdmMain;
extern bool design_deletion;

PluginBinding::PluginBinding( Stub *stub, Map *pi, const char *lcond, float prob )
{
   Cltn<Node *> *source = stub->SourceSet();
   Cltn<Node *> *target = stub->TargetSet();
   
   plugin = pi;
   logical_condition = ((lcond != NULL) ? strdup( lcond ) : NULL);
   probability = prob;

   for( source->First(); !source->IsDone(); source->Next() )
      entry_bindings.Add( new StubBinding( source->CurrentItem() ) );
   
   for( target->First(); !target->IsDone(); target->Next() )
      exit_bindings.Add( new StubBinding( target->CurrentItem() ) );

}

PluginBinding::~PluginBinding()
{
   while( !entry_bindings.is_empty() )
      delete entry_bindings.Detach();
   
   while( !exit_bindings.is_empty() )
      delete exit_bindings.Detach();
}

void PluginBinding::AddNode( Node *new_node, end_type etype )
{
   if( etype == PATH_ENTRY )
      entry_bindings.Add( new StubBinding( new_node ) );
   else
      exit_bindings.Add( new StubBinding( new_node ) );

}

void PluginBinding::RemoveNode( Node *node, end_type etype )
{
   StubBinding *sb;
   Cltn<StubBinding *> &bindings = (etype == PATH_ENTRY) ? entry_bindings : exit_bindings;

   for( bindings.First(); !bindings.IsDone(); bindings.Next() ) {
      sb = bindings.CurrentItem();
      if( sb->boundary_node == node ) {
	 delete sb;
	 bindings.RemoveCurrentItem();
	 break;
      }
   }
}
   
void PluginBinding::AddBinding( Hyperedge *endpoint, Hyperedge *boundary_edge, end_type etype )
{
   Cltn<StubBinding *> &bindings = (etype == PATH_ENTRY) ? entry_bindings : exit_bindings;
   Node *boundary_node = ( (etype == PATH_ENTRY) ? (boundary_edge->TargetSet()->GetFirst()) : (boundary_edge->SourceSet()->GetFirst()) );

   for( bindings.First(); !bindings.IsDone(); bindings.Next() ) {
      if( bindings.CurrentItem()->boundary_node == boundary_node ) {
	 bindings.CurrentItem()->path_endpoint = endpoint;
	 return;
      }
   }

   AbortProgram( "Node corresponding to input/output hyperedge not found" );
}

Hyperedge * PluginBinding::SubmapPathStart( Node *input_node )
{
   for( entry_bindings.First(); !entry_bindings.IsDone(); entry_bindings.Next() ) {
      if( entry_bindings.CurrentItem()->boundary_node == input_node )
	 return( entry_bindings.CurrentItem()->path_endpoint );
   }

   return( NULL ); // if no path start was found
}

void PluginBinding::GeneratePostScript( FILE *ps_file )
{
   StubBinding *sb;
   int path_number;
   char *lc;
   bool has_input_bindings = FALSE, has_output_bindings = FALSE;
   
   fprintf( ps_file, "1 IN ID %d rom (Plugin Map - ) S\n%d bol (%s ) P OD\n", PrintManager::text_font_size, PrintManager::text_font_size,
	    PrintManager::PrintPostScriptText( plugin->MapLabel() ) );
   if( (lc = LogicalCondition()) != NULL ) {
      fprintf( ps_file, "2 IN ID %d rom (Selection Condition ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "3 IN ID (%s ) P OD\n", PrintManager::PrintPostScriptText( lc ) );
      free( lc );
   }
   if( PrintManager::ReportIncludesMap( plugin ) ) {
      fprintf( ps_file, "[/Dest /Plugin%s /Rect [106 tm 3 sub cpp tm 10 add]\n", PrintManager::PrintPostScriptText( plugin->MapLabel(), FALSE ) );
      fprintf( ps_file, "/Border [0 0 0] /Color [1 0 0] /InvisibleRect /I /Subtype /Link /ANN pdfmark\n" );
   }
   fprintf( ps_file, "L\n" );

   for( entry_bindings.First(); !entry_bindings.IsDone(); entry_bindings.Next() ) {
      if( entry_bindings.CurrentItem()->path_endpoint != NULL ) {
	 has_input_bindings = TRUE;
	 break;
      }
   }
   
   for( exit_bindings.First(); !exit_bindings.IsDone(); exit_bindings.Next() ) {
      if( exit_bindings.CurrentItem()->path_endpoint != NULL ) {
	 has_output_bindings = TRUE;
	 break;
      }
   }
   
   if( has_input_bindings ) {
      fprintf( ps_file, "2 IN ID %d rom (Input Bindings ) P OD\n3 IN ID %d bol\n", PrintManager::text_font_size, PrintManager::text_font_size );

      for( entry_bindings.First(); !entry_bindings.IsDone(); entry_bindings.Next() ) {
	 sb = entry_bindings.CurrentItem();
	 if( sb->path_endpoint != NULL ) {
	    path_number = ((Empty *)(sb->boundary_node->PreviousEdge()))->PathNumber();
	    fprintf( ps_file, "(IN%d <-> %s ) P\n", path_number, PrintManager::PrintPostScriptText( sb->path_endpoint->HyperedgeName()));
	 }
      }   
      fprintf( ps_file, "OD\n" );
   }

   if( has_output_bindings ) {
      fprintf( ps_file, "2 IN ID %d rom (Output Bindings ) P OD\n3 IN ID %d bol\n", PrintManager::text_font_size, PrintManager::text_font_size );

      for( exit_bindings.First(); !exit_bindings.IsDone(); exit_bindings.Next() ) {
	 sb = exit_bindings.CurrentItem();
	 if( sb->path_endpoint != NULL ) {
	    path_number = ((Empty *)(sb->boundary_node->NextEdge()))->PathNumber();
	    fprintf( ps_file, "(OUT%d <-> %s ) P\n", abs(path_number), PrintManager::PrintPostScriptText( sb->path_endpoint->HyperedgeName()));
	 }
      }
      fprintf( ps_file, "OD\n" );
   }
   
}

char * PluginBinding::LogicalCondition()
{
   return( (logical_condition != NULL) ? BooleanVariable::ReferenceToName( logical_condition ) : NULL );
}

void PluginBinding::LogicalCondition( const char *new_condition )
{
   logical_condition = BooleanVariable::NameToReference( new_condition, logical_condition );
}

Stub::Stub() : Hyperedge()
{
   stub_label[0] = 0;   
   stub_label[DISPLAYED_LABEL] = 0;
   stype = STATIC;
   shared = FALSE;
   selection_policy = NULL;
   figure = new StubFigure( this, TRUE );
   condition_manager.ConditionType( STUB_PATH );   
   annotations_complete = TRUE;
   generation_pass = FALSE;
   tracing_id = 0;
   current_submap = NULL;
}

Stub::Stub( int stub_id, const char *name, float x, float y, const char *desc, stub_type st, bool shared_stub, const char *sp ) : Hyperedge()
{
   load_number = stub_id;
   
   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = stub_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;     
   //End of the modification by Bo Jiang, April, 2005

   stub_label[DISPLAYED_LABEL] = 0;
   strncpy( stub_label, name, DISPLAYED_LABEL );
   figure = new StubFigure( this, FALSE );
   figure->SetPosition( x, y );
   if( desc != NULL ) unique_desc = strdup( desc );
   stype = st;
   shared = shared_stub;
   selection_policy = NULL;
   if( sp != NULL ) selection_policy = strdup( sp );
   condition_manager.ConditionType( STUB_PATH );
   annotations_complete = TRUE;
   generation_pass = FALSE;
   tracing_id = 0;
   current_submap = NULL;
}

Stub::~Stub()
{
   while( !continuity_bindings.is_empty() )
      delete continuity_bindings.Detach();

   if( figure->IsSelected() )  // remove conditions from lists if edge deleted
      ResetConditions();

   if( selection_policy != NULL ) free( selection_policy );

   while( !service_requests.is_empty() )
      delete service_requests.Detach();

   if( design_deletion ) return; // don't bother deregistering parent stubs
   
   if( submaps.Size() > 0 )
      display_manager->RemoveStubFromDecomposition( this );

   for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
      if( submaps.CurrentItem()->IsPlugIn() )
	 submaps.CurrentItem()->RemoveParentStub( this );
   }

   while( !scenario_submaps.is_empty() )
     scenario_submaps.Detach();
}

void Stub::SetName( const char *new_label )
{
   strncpy( stub_label, new_label, DISPLAYED_LABEL );
}

void Stub::SelectionPolicy( const char *new_sp )
{
   if( selection_policy )
      free( selection_policy );
   selection_policy = strdup( new_sp );
}

void Stub::AddServiceRequest( int device_id, const char *amount )
{
   service_requests.Add( new ServiceRequest( device_id, amount ) );
}

bool Stub::Perform( transformation trans, execution_flag execute )
{
   switch( trans ) {
      
   case DELETE_STUB:
      return( DeleteStub( execute ) );
      break;
   case EXPAND_STUB:
      return( ExpandStub( execute ) );
      break;
   case VIEW_SUB_MAP:
      return( ViewSubmap( execute ) );
      break;
   case EDIT_STUB_LABEL:
      return( EditStubLabel( execute ) );
      break;
   case INSTALL_EXISTING_PLUGIN:
      return( InstallExistingPlugin( execute ) );
      break;
   case REMOVE_SUBMAP:
      return( RemoveSubmap( execute ) );
      break;
   case RENAME_SUBMAP:
      return( RenamePlugin( execute ) );
      break;
   case BIND_STUB:
      return( BindPlugin( execute ) );
      break;
   case COPY_EXISTING_PLUGIN:
      return( CopyExistingPlugin( execute ) );
      break;
   case IMPORT_PLUGIN_FILE:
      return( ImportPluginFile( execute ) );
      break;
   case TRANSFER_ROOT_PLUGIN:
      return( TransferRootPlugin( execute ) );
      break;
   }    
   return FALSE;  
}

bool Stub::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute )
{

   switch( trans ){
  
   case CONNECT:
      return( ConnectPath( edge, execute ) );
      break;
   case DISCONNECT:
      return( DisconnectPath( edge, execute ) );
      break;
   }
   return FALSE;  
}

int Stub::DoubleClickAction()
{
   if( HasViewableSubmap() )
      this->ViewPlugin();
   else
      EditStubLabel( TRUE ); // default to description dialog if no submaps exist
   return UNMODIFIED; // routines above will update display
}

void Stub::AttachSource( Node *new_node )
{
   Hyperedge::AttachSource( new_node );

   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() )
      plugin_bindings.CurrentItem()->AddNode( new_node, PATH_ENTRY );   

   this->ShowInputPaths();
}

void Stub::DetachSource( Node *node )
{
   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() )
      plugin_bindings.CurrentItem()->RemoveNode( node, PATH_ENTRY );   

   Hyperedge::DetachSource( node );

   this->ShowInputPaths();
}

void Stub::AttachTarget( Node *new_node )
{
   Hyperedge::AttachTarget( new_node );
   
   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() )
      plugin_bindings.CurrentItem()->AddNode( new_node, PATH_EXIT );   

   this->ShowOutputPaths();
}

void Stub::DetachTarget( Node *node )
{
   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() )
      plugin_bindings.CurrentItem()->RemoveNode( node, PATH_EXIT );   

   Hyperedge::DetachTarget( node );

   this->ShowOutputPaths();
}

void Stub::Install()
{
   display_manager->CreateStub( this );
   this->ShowInputOutputPaths();
   EditStubDescription( this, TRUE );
   DrawScreen();
}

bool Stub::DeleteStub( flag execute )
{
   char prompt[100];

   if( execute ){
      if( HasSubmap() || HasConditions() || PrintManager::TextNonempty( selection_policy ) ) {
	 sprintf( prompt, "Do you wish to delete the stub %s ?", stub_label );
	 if( QuestionDialog( "Confirm Stub Deletion", prompt, "", "Yes, Delete Stub", "No, Cancel Operation" ) == FALSE )
	    return( UNMODIFIED );
      }
      
      // reset current label so that empty will be created with proper label
      current_label = target->GetFirst()->NextEdge()->PathLabel();
      // delete stub and merge curves of the incoming and outgoing paths
      Node *front_node = source->GetFirst();
      Node *end_node = target->GetFirst();
      Hyperedge *prev_edge = front_node->PreviousEdge();
      Hyperedge *next_edge = end_node->NextEdge(); 

      ((Empty *)prev_edge)->PathNumber( 0 );
      ((Empty *)next_edge)->PathNumber( 0 );
      
      display_manager->DeleteStub( this );
      
      current_label = this->PathLabel();  // reset label so empty is created with label of incoming path
      Empty *new_empty = new Empty;
      Node *node1 = new Node( B );
      Node *node2 = new Node( A );
      new_empty->AttachSource( node1 );
      new_empty->AttachTarget( node2 );
  
      next_edge->ReplaceSource( end_node, node2 );
      prev_edge->ReplaceTarget( front_node, node1 );
  
      display_manager->Add( new_empty, this );

      parent_map->MapHypergraph()->PurgeSegmentFrom( front_node, end_node );
      display_manager->SetActive( NULL );
   
      return( MODIFIED );
   }
   else {  // disable deletion if stub is multipath
      if( (source->Size() > 1) || (target->Size() > 1) )
	 return( FALSE );
      else
	 return( TRUE );
   }
   return FALSE;  
}

bool Stub::DeleteHyperedge()
{
   char prompt[100];

   if( (source->Size() > 1) || (target->Size() > 1) )
      return FALSE;

   if( HasSubmap() || HasConditions() || PrintManager::TextNonempty( selection_policy ) ) {
      sprintf( prompt, "Do you wish to delete the stub %s ?", stub_label );
      if( QuestionDialog( "Confirm Stub Deletion", prompt, "", "Yes, Delete Stub", "No, Cancel Operation" ) == FALSE )
	 return FALSE;
   }

   Node *front_node = source->GetFirst();
   Node *end_node = target->GetFirst();
   Hyperedge *prev_edge = front_node->PreviousEdge();
   Hyperedge *next_edge = end_node->NextEdge(); 

   ((Empty *)prev_edge)->PathNumber( 0 );
   ((Empty *)next_edge)->PathNumber( 0 );
 
   display_manager->DeleteStub( this );
   prev_edge->ReplaceTarget( front_node, end_node );

   parent_map->MapHypergraph()->PurgeNode( front_node );
   parent_map->MapHypergraph()->PurgeEdge( this );
   return TRUE;
}

void Stub::AddSubmap( Map *new_submap )
{
   submaps.Add( new_submap );
   plugin_bindings.Add( new PluginBinding( this, new_submap ) );
   new_submap->AddParentStub( this );
   display_manager->ExpandUCMSet( new_submap );
}   

bool Stub::ExpandStub( flag execute )
{
   if( execute ) {
      
      const char *entered_label = display_manager->UniqueMapName( "Enter the name for the new stub plugin map. ", "" );
      
      if( entered_label != NULL )     //  if == NULL, then the Cancel button was hit and we don't want to change anything
      {
	 Map *plugin = new Map( entered_label, PLUGIN );
	 this->AddSubmap( plugin );
	 display_manager->InstallSubmap( plugin, this );
	 figure->ResetSelected();
      }

   }
   else {  // disable only if it is a static stub with its expansion already filled

      if( (stype == STATIC) && (submaps.Size() == 1) )
	 return( FALSE );
      else	    
	 return( TRUE );
   }
   return FALSE;  
}

void Stub::AddPluginBinding( PluginBinding *pb )
{
   plugin_bindings.Add( pb );
   submaps.Add( pb->plugin );
   pb->plugin->AddParentStub( this );
}

bool Stub::EditStubLabel( flag execute )
{
   if( execute )
      EditStubDescription( this );
   else
      return( TRUE );
   return FALSE;  
}

bool Stub::ViewSubmap( flag execute )
{  
   if( execute )
      this->ViewPlugin();
   else
      return( display_manager->HasViewableSubmaps( this ) );
   return FALSE;  
}

bool Stub::HasViewableSubmap()
{
   return( display_manager->HasViewableSubmaps( this ) );
}

void Stub::ViewPlugin( Empty *empty )
{
   Map *sole_viewable_map;
   
   if( submaps.Size() == 1 ) {
      sole_viewable_map = submaps.GetFirst();
      display_manager->InstallSubmap( sole_viewable_map, this, empty ); // simply install the lone submap
      figure->ResetSelected();      
   }
   else if( (sole_viewable_map = display_manager->SoleViewableSubmap( this, empty ))!= NULL ) {
      display_manager->InstallSubmap( sole_viewable_map, this, empty );
      figure->ResetSelected();
   }
   else
      SelectStubSubmap( this, CHOOSE_SUBMAP, empty );  // display dialog box with browser to allow user to choose among possible plugins
   // method ends here as installation must be done after dialog returns
}

void Stub::SetSelectedMap( Map *selected_map, Empty *empty )
{
   if( selected_map ) {
      display_manager->InstallSubmap( selected_map, this, empty ); // install submap if user selected one
      figure->ResetSelected();
      DrawScreen();
   }
}

bool Stub::InstallExistingPlugin( flag execute )
{

   if( execute ) {
      InstallExistingStubExpansion( this );
   }
   else {  // disable if it is a static stub with its expansion already filled or
      // if no submaps exist
      
      if( ((stype == STATIC) && (submaps.Size() == 1)) || !(display_manager->HasSubmaps( this )) )
	 return( FALSE );
      else	    
	 return( TRUE );
   }
   return FALSE;  
}

bool Stub::CopyExistingPlugin( flag execute )
{

   if( execute ) {
      InstallExistingStubExpansion( this, TRUE );
   }
   else {  // disable if it is a static stub with its expansion already filled or
      // if no submaps exist
      
      if( ((stype == STATIC) && (submaps.Size() == 1)) || !(display_manager->HasSubmaps( this )) )
	 return( FALSE );
      else	    
	 return( TRUE );
   }
   return FALSE;  
}

bool Stub::ImportPluginFile( flag execute )
{

   if( execute ) {
      display_manager->ImportMaps( PLUGIN, this );
   }
   else {  // disable if it is a static stub with its expansion already filled
      if( (stype == STATIC) && (submaps.Size() == 1) )
	 return( FALSE );
      else	    
	 return( TRUE );
   }
   return FALSE;  
}

bool Stub::TransferRootPlugin( flag execute )
{
   if( execute )
      SelectMap( this );
   else {
      if( (stype == STATIC) && (submaps.Size() == 1) )
	 return( FALSE );
      else // disable if there are no other root maps except this map's parent
	 return( display_manager->TransferableRootMaps() );
   }
   return FALSE;  
}

void Stub::InstallRootPlugin( Map *root_map )
{
   root_map->SetMapType( PLUGIN );
   this->AddSubmap( root_map );
}

void Stub::InstallNewSubmap( Map *new_map )
{
   if( submaps.Includes( new_map ) ) {
      fl_set_resource( "flAlert.title", "Error: Plugin Exists" );
      fl_show_alert( "Stub Plug-In already exists as a submap of this stub.",
		     "Operation Ignored", "", 0 );
   }
   else
      this->AddSubmap( new_map );
}

void Stub::InstallSubmapImage( Map *new_map )
{
   const char *entered_label;
   
   if( (entered_label = display_manager->UniqueMapName( "Enter New Plugin Name", "" )) == NULL )
      return; //  if pointer is null then the Cancel button was hit and we don't want to change anything

   Map *top_map = display_manager->CurrentMap();  // save pointer to map containing this stub
   Map *image = new_map->Copy( entered_label );
   this->AddSubmap( image );
   display_manager->PlaceMap( top_map );
}

bool Stub::RemoveSubmap( flag execute )
{
   PluginBinding *pb;

   if( execute ) {
      if( submaps.Size() == 1 ) {

	 // prompt user for confirmation of removal
	 if( !QuestionDialog( "Confirm Submap Removal", "Do you wish to remove the submap for this stub ?", "",
			      "Remove Submap", "Cancel Submap Removal" ) )
	    return TRUE;
	 
	 Map *sole_map = submaps.GetFirst();
	 submaps.Detach();
	 sole_map->RemoveParentStub( this );

	 for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
	    pb = plugin_bindings.CurrentItem();
	    if( pb->plugin == sole_map ) {
	       delete pb;
	       plugin_bindings.RemoveCurrentItem();
	    }
	 }
      }
      else
	 SelectStubSubmap( this, REMOVE_PLUGIN );  // display dialog box with browser to allow user to choose among possible plugins
   }
   else  // disable only if there are no submaps
      return( display_manager->HasViewableSubmaps( this ) );
   return FALSE;  
}

bool Stub::RenamePlugin( flag execute )
{
   const char *entered_label;
   static Map *sole_map;
   
   if( execute ) {
      entered_label = display_manager->UniqueMapName( "Enter the new name for the plugin. ", sole_map->MapLabel() );
      if( entered_label != NULL )
	 sole_map->MapLabel( entered_label );
   }
   else // disable if more than one viewable map exists
      return( (sole_map = display_manager->SoleViewableSubmap( this )) != NULL );
   return FALSE;  
}

void Stub::RemovePlugin( Map *map )
{
   PluginBinding *pb;
   
   if( submaps.Includes( map ) ) {
      
      submaps.Remove( map );
      map->RemoveParentStub( this );

      for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
	 pb = plugin_bindings.CurrentItem();
	 if( pb->plugin == map ) {
	    delete pb;
	    plugin_bindings.RemoveCurrentItem();
	 }
      }
   }
}

bool Stub::BindPlugin( flag execute )
{
   static Map *sole_map;

   if( execute ) {
      this->InitiatePathBinding( sole_map );
      BindStubPlugin( this, sole_map );  // display popup dialog for stub binding
   }
   else   // disable if more than one viewable map exists
      return( (sole_map = display_manager->SoleViewableSubmap( this )) != NULL );
   return FALSE;  
}

void Stub::VerifyAnnotations()
{
   PluginBinding *pb;

   if( annotations_complete == HIGHLIGHT_PARENT_STUBS ) return;
   
   annotations_complete = TRUE;

   if( strequal( stub_label, "" ) ) { // return if name is empty
      annotations_complete = FALSE;
      parent_map->HighlightParentStubs();
      return;
   }

   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
      pb = plugin_bindings.CurrentItem();
      for( pb->entry_bindings.First(); !pb->entry_bindings.IsDone(); pb->entry_bindings.Next() ) {
	 if( pb->entry_bindings.CurrentItem()->path_endpoint == NULL ) {
	    annotations_complete = FALSE;
	    break;
	 }
      }
      for( pb->exit_bindings.First(); !pb->exit_bindings.IsDone(); pb->exit_bindings.Next() ) {
	 if( pb->exit_bindings.CurrentItem()->path_endpoint == NULL ) {
	    annotations_complete = FALSE;
	    break;
	 }
      }
   }
   
   if( annotations_complete == FALSE )
      parent_map->HighlightParentStubs();
}

void Stub::HighlightParentStubs()
{
   if( annotations_complete == HIGHLIGHT_PARENT_STUBS ) return;

   annotations_complete = HIGHLIGHT_PARENT_STUBS;
   parent_map->HighlightParentStubs();
}

void Stub::InitiatePathBinding( Map *plugin )
{
   int path_identifier = 1;
   Cltn<Hyperedge *> *edge_pool = plugin->MapHypergraph()->Hyperedges();
   Hyperedge *current_edge;
   edge_type etype;
   StubBinding *sb;

   // find the plugin binding associated with this plugin
   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
      if( plugin_bindings.CurrentItem()->plugin == plugin ) {
	 current_binding = plugin_bindings.CurrentItem();
	 break;
      }
   }
       
   Cltn<StubBinding *> &entry_bindings = current_binding->entry_bindings;
   Cltn<StubBinding *> &exit_bindings = current_binding->exit_bindings;

   // search the hypergraph of the submap for start and result edges and place their pointers in the scratch
   // space arrays that are an element of the Stub class as well as setting their initial state to unbound

   number_starts = 0;
   number_results = 0;
      
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      etype = edge_pool->CurrentItem()->EdgeType();
      if( etype == START )
	 number_starts++;
      else if( etype == RESULT )
	 number_results++;
   }
   
   if( number_starts > start_list_size ) {
      if( plugin_starts != NULL ) free( plugin_starts );
      plugin_starts = (Start **)malloc(number_starts*sizeof(Start *));
   }

   if( number_results > result_list_size ) {
      if( plugin_results != NULL ) free( plugin_results );
      plugin_results = (Result **)malloc(number_results*sizeof(Result *));
   }

   number_starts = 0;
   number_results = 0;
      
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      current_edge = edge_pool->CurrentItem();
      if( current_edge->EdgeType() == START ) {
	 plugin_starts[number_starts++] = (Start *)current_edge;
	 ((Start *)current_edge)->SetUnbound();
      }
      else if( current_edge->EdgeType() == RESULT ) {
	 plugin_results[number_results++] = (Result *)current_edge;
	 ((Result *)current_edge)->SetUnbound();
      }
   }

   // iterate through input and output access points and install path identifiers in both the bindings
   // and in the previous and next empty points for temporary display as well as setting the bound state of endpoints

   for( entry_bindings.First(); !entry_bindings.IsDone(); entry_bindings.Next() ) {
      sb = entry_bindings.CurrentItem();
      if( sb->path_endpoint != NULL ) {
	 if( PathStartValid( (Start *)sb->path_endpoint ) )
	    ((Start *)sb->path_endpoint)->SetBound();
	 else
	    sb->path_endpoint = NULL;  // clear invalid endpoint
      }
      sb->path_identifier = path_identifier;
      ((Empty *)(sb->boundary_node->PreviousEdge()))->PathNumber( path_identifier );
      path_identifier++;
   }

   path_identifier = 1;
   for( exit_bindings.First(); !exit_bindings.IsDone(); exit_bindings.Next() ) {
      sb = exit_bindings.CurrentItem();
      if( sb->path_endpoint != NULL ) {
	 if( PathEndValid( (Result *)sb->path_endpoint ) )
	    ((Result *)sb->path_endpoint)->SetBound();
	 else
	    sb->path_endpoint = NULL;  // clear invalid endpoint
      }
      sb->path_identifier = -1*path_identifier;
      ((Empty *)(sb->boundary_node->NextEdge()))->PathNumber( (-1*path_identifier) );
      path_identifier++;
   }
}

void Stub::UpdateBindingsDisplay()
{
   char browser_entry[50];
   StubBinding *sb;
   int i;
   
   Cltn<StubBinding *> &entry_bindings = current_binding->entry_bindings;
   Cltn<StubBinding *> &exit_bindings = current_binding->exit_bindings;

   for( entry_bindings.First(); !entry_bindings.IsDone(); entry_bindings.Next() ) {

      sb = entry_bindings.CurrentItem();
      if( sb->path_endpoint == NULL ) {  // entry is not bound
	 sprintf( browser_entry, "IN%d", sb->path_identifier );
	 fl_addto_browser( pfdsbp->StubEntryPointsBrowser, browser_entry );
      }
      else {   // entry is bound
	 sprintf( browser_entry, "IN%d <-> %s", sb->path_identifier, sb->path_endpoint->HyperedgeName() );
	 fl_addto_browser( pfdsbp->StubBindingsList, browser_entry );
      }
      
   }

   for( exit_bindings.First(); !exit_bindings.IsDone(); exit_bindings.Next() ) {

      sb = exit_bindings.CurrentItem();
      if( sb->path_endpoint == NULL ) {  // entry is not bound
	 sprintf( browser_entry, "OUT%d", abs(sb->path_identifier) );
	 fl_addto_browser( pfdsbp->StubExitPointsBrowser, browser_entry );
      }
      else {   // entry is bound
	 sprintf( browser_entry, "OUT%d <-> %s", abs(sb->path_identifier), sb->path_endpoint->HyperedgeName() );
	 fl_addto_browser( pfdsbp->StubBindingsList, browser_entry );
      }

   }
   
   // iterate through the arrays for the plugin start and result edges and place their labels in the appropriate
   // browser windows. For previously performed bindings flags in the start and end points must be checked to
   // see if they are already a part of the bindings

   for( i = 0; i < number_starts; i++ ) {
      if( !(plugin_starts[i]->IsBound()) )
	 fl_addto_browser( pfdsbp->PluginEntryPointsBrowser, plugin_starts[i]->HyperedgeName() );
   }

   for( i = 0; i < number_results; i++ ) {
      if( !(plugin_results[i]->IsBound()) )
	 fl_addto_browser( pfdsbp->PluginExitPointsBrowser, plugin_results[i]->HyperedgeName() );
   }

}

void Stub::BindEntryPoints( int stub_entry, int plugin_entry )
{
   Start *plst;
   int i, count = 0;
   StubBinding *sb;
   Cltn<StubBinding *> &entry_bindings = current_binding->entry_bindings;
   
   // find plugin_entry'th unbound start point

   for( i = 0; i < number_starts; i++ ) {
      if( !(plugin_starts[i]->IsBound()) ) {
	 count++;
	 if( count == plugin_entry ) {
	    plst = plugin_starts[i];
	    break;
	 }
      }      
   }
   
   plst->SetBound();

   // find stub_entry'th unbound stub entry point

   count = 0;
   for( entry_bindings.First(); !entry_bindings.IsDone(); entry_bindings.Next() ) {
      sb = entry_bindings.CurrentItem();
      if( sb->path_endpoint == NULL ) {  // entry is not bound
	 count++;
	 if( count == stub_entry ) {
	    sb->path_endpoint = plst;
	    break;
	 }
      }
   }
}

void Stub::BindExitPoints( int stub_exit, int plugin_exit )
{
   Result *plrt;
   int i, count = 0;
   StubBinding *sb;
   Cltn<StubBinding *> &exit_bindings = current_binding->exit_bindings;

   // find plugin_exit'th unbound result point

   for( i = 0; i < number_results; i++ ) {
      if( !(plugin_results[i]->IsBound()) ) {
	 count++;
	 if( count == plugin_exit ) {
	    plrt = plugin_results[i];
	    break;
	 }
      }      
   }
   
   plrt->SetBound();
   
   // find stub_exit'th unbound stub exit point

   count = 0;
   for( exit_bindings.First(); !exit_bindings.IsDone(); exit_bindings.Next() ) {
      sb = exit_bindings.CurrentItem();
      if( sb->path_endpoint == NULL ) {  // exit is not bound
	 count++;
	 if( count == stub_exit ) {
	    sb->path_endpoint = plrt;
	    break;
	 }
      }
   }
}

void Stub::BreakBinding( int binding )
{
   Start *plst;
   Result *plrt;
   StubBinding *sb;
   Cltn<StubBinding *> &entry_bindings = current_binding->entry_bindings;
   Cltn<StubBinding *> &exit_bindings = current_binding->exit_bindings;
   int count = 0;

   // check if selected binding is that of an entry point

   for( entry_bindings.First(); !entry_bindings.IsDone(); entry_bindings.Next() ) {
      sb = entry_bindings.CurrentItem();
      if( sb->path_endpoint != NULL ) {  // entry is bound
	 count++;
	 if( count == binding ) {
	    plst = (Start *)sb->path_endpoint;
	    sb->path_endpoint = NULL;
	    plst->SetUnbound();
	    return;
	 }
      }
   }
   
   // check if selected binding is that of an exit point

   for( exit_bindings.First(); !exit_bindings.IsDone(); exit_bindings.Next() ) {
      sb = exit_bindings.CurrentItem();
      if( sb->path_endpoint != NULL ) {  // exit is bound
	 count++;
	 if( count == binding ) {
	    plrt = (Result *)sb->path_endpoint;
	    sb->path_endpoint = NULL;
	    plrt->SetUnbound();
	    return;
	 }
      }
   }
}

bool Stub::PathStartValid( Start *start )
{
   for( int i = 0; i < number_starts; i++ ) {
      if( plugin_starts[i] == start )
	 return( TRUE );
   }

   return( FALSE );
}

bool Stub::PathEndValid( Result *result )
{
   for( int i = 0; i < number_results; i++ ) {
      if( plugin_results[i] == result )
	 return( TRUE );
   }

   return( FALSE );
}

bool Stub::IgnoredEndingsExist()
{
   int i;
   
   for( i = 0; i < number_starts; i++ ) {
      if( !plugin_starts[i]->IsBound() )
	 return( TRUE );
   }

   for( i = 0; i < number_results; i++ ) {
      if( !plugin_results[i]->IsBound() )
	 return( TRUE );
   }

   return( FALSE );
}

Cltn<StubBinding *> * Stub::EntryBindings( Map *plugin )
{
   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
      if( plugin_bindings.CurrentItem()->plugin == plugin )
	 return( &(plugin_bindings.CurrentItem()->entry_bindings) );
   }
   return 0;
}

Cltn<StubBinding *> * Stub::ExitBindings( Map *plugin )
{
   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
      if( plugin_bindings.CurrentItem()->plugin == plugin )
	 return( &(plugin_bindings.CurrentItem()->exit_bindings) );
   }
   return 0;
}

void Stub::RemoveEntryBinding( Start *plugin_start, Map *plugin )
{
   Cltn<StubBinding *> *entry_bindings = EntryBindings( plugin );
   StubBinding *sb;

   for( entry_bindings->First(); !entry_bindings->IsDone(); entry_bindings->Next() ) {
      sb = entry_bindings->CurrentItem();
      if( sb->path_endpoint == plugin_start ) {
	 sb->path_endpoint = NULL;
	 break;
      }
   }
}

void Stub::RemoveExitBinding( Result *plugin_result, Map *plugin )
{
   Cltn<StubBinding *> *exit_bindings;
   StubBinding *sb;

   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
      if( plugin_bindings.CurrentItem()->plugin == plugin ) {
	 exit_bindings = &(plugin_bindings.CurrentItem()->exit_bindings);
	 break;
      }
   }

   for( exit_bindings->First(); !exit_bindings->IsDone(); exit_bindings->Next() ) {
      sb = exit_bindings->CurrentItem();
      if( sb->path_endpoint == plugin_result ) {
	 sb->path_endpoint = NULL;
	 break;
      }
   }
}

Empty * Stub::PathContinued( Hyperedge *io_point, Map *submap )
{
   Cltn<StubBinding *> *io_bindings;
   Node *boundary_node = NULL;
   bool path_end = ((io_point->EdgeType() == RESULT) ? TRUE : FALSE );

   io_bindings = path_end ? ExitBindings( submap ) : EntryBindings( submap );

   for( io_bindings->First(); !io_bindings->IsDone(); io_bindings->Next() ) {
      if( io_bindings->CurrentItem()->path_endpoint == io_point ) {
	 boundary_node = io_bindings->CurrentItem()->boundary_node;
	 break;
      }
   }
   if( boundary_node != NULL ) {
      if( path_end )
	 return( (Empty * )boundary_node->NextEdge() );
      else
	 return( (Empty * )boundary_node->PreviousEdge() ); 
   } else
      return NULL;
}

bool Stub::ConnectPath( Hyperedge *trigger_edge, flag execute )
{

   if( execute ) {
      ((WaitingPlace *)trigger_edge)->ConnectPath( this, execute );
      return ( TRUE );  // not used
   }
   else {
      edge_type type = trigger_edge->EdgeType();

      if( !((type == WAIT) || (type == TIMER) || (type == START)) )
	 return( FALSE );
      else
	 return( TRUE );
   }
}

bool Stub::DisconnectPath( Hyperedge *trigger_edge, flag execute )
{
   if( execute ) {
      ((WaitingPlace *)trigger_edge)->DisconnectPath( this, execute );
      return ( TRUE );  // not used
   }
   else {
      edge_type type = trigger_edge->EdgeType();

      if( !((type == WAIT) || (type == TIMER) || (type == START)) )
	 return( FALSE );
      else
	 return( TRUE );
   }
}

void Stub::EdgeSelected()
{
   InstallConditions( condition_manager );
}

void Stub::GeneratePostScriptDescription( FILE *ps_file )
{
   ContinuityBinding *cb;
   bool conditions_empty = condition_manager.ConditionsEmpty(),
      has_description = HasDescription();

   fprintf( ps_file, "%d rom (%s%s Stub - ) S\n", PrintManager::text_font_size, ( shared ? "Shared " : "" ), ((stype == STATIC) ? "Static" : "Dynamic" ) );
   fprintf( ps_file, "[/Dest /%s", PrintManager::PrintPostScriptText( parent_map->MapLabel(), FALSE ) ); // add pdf mark midline
   fprintf( ps_file, "Stub%s /DEST pdfmark\n", PrintManager::PrintPostScriptText( stub_label, FALSE ) );
   fprintf( ps_file, "%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( stub_label ) );
   if( has_description ) GeneratePSElementDescription( ps_file );
   if( !conditions_empty ) condition_manager.GeneratePostScriptDescription( ps_file );

   if( PrintManager::TextNonempty( selection_policy ) ) {
      fprintf( ps_file, "%d rom\n1 IN ID (Selection Policy ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID\n" );
      PrintManager::PrintSequence( selection_policy );
      fprintf( ps_file, "OD\n" );
   }
   
   if( continuity_bindings.Size() != 0 ) {
      fprintf( ps_file, "%d rom\n1 IN ID (Enforce Bindings ) P OD 2 IN ID %d bol\n\n",
	       PrintManager::text_font_size, PrintManager::text_font_size );
      for( continuity_bindings.First(); !continuity_bindings.IsDone(); continuity_bindings.Next() ) {
	 cb = continuity_bindings.CurrentItem();
	 fprintf( ps_file, "(IN%d <-> OUT%d ) P\n", source->in( cb->input_node )+1, target->in( cb->output_node )+1 );
      }
      fprintf( ps_file, "OD\n" );
   }
   
   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() )
      plugin_bindings.CurrentItem()->GeneratePostScript( ps_file );

   fprintf( ps_file, "L\n" );
}

void Stub::ShowInputPaths()
{
   Hyperedge *prev_edge;
   int path_identifier = 1;
   
   for( source->First(); !source->IsDone(); source->Next() ) {
      prev_edge = source->CurrentItem()->PreviousEdge();
      if( prev_edge )
	 ((Empty *)prev_edge)->PathNumber( path_identifier++ );
   }
}

void Stub::ShowOutputPaths()
{
   Hyperedge *next_edge;
   int path_identifier = -1;
   
   for( target->First(); !target->IsDone(); target->Next() ) {
      next_edge = target->CurrentItem()->NextEdge();
      if( next_edge )
	 ((Empty *)next_edge)->PathNumber( path_identifier-- );
   }
}

void Stub::SaveXMLDetails( FILE *fp )
{
   char buffer[300];
   
   sprintf( buffer, "<stub shared=\"%s\" type=\"%s\" ", ( shared ? "yes" : "no" ), ( stype == STATIC ? "static" : "dynamic" ) );
   PrintXMLText( fp, buffer );
   if( selection_policy )
      fprintf( fp, "selection-policy=\"%s\" ", PrintDescription( selection_policy ) );
   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );

   PrintNewXMLBlock( fp, "stub-entry-list" );

   for( source->First(); !source->IsDone(); source->Next() )
   {
      sprintf( buffer, "stub-entry hyperedge-id=\"h%d\" stub-entry-id=\"si%d\" /", source->CurrentItem()->PreviousEdge()->GetNumber(),
	       source->CurrentItem()->GetNumber() );
      PrintXML( fp, buffer );
   }
   
   PrintEndXMLBlock( fp, "stub-entry-list" );

   PrintNewXMLBlock( fp, "stub-exit-list" );

   for( target->First(); !target->IsDone(); target->Next() )
   {
      sprintf( buffer, "stub-exit hyperedge-id=\"h%d\" stub-exit-id=\"so%d\" /", target->CurrentItem()->NextEdge()->GetNumber(),
	       target->CurrentItem()->GetNumber() );
      PrintXML( fp, buffer );
   }

   ServiceRequest::SaveRequests( &service_requests, fp );
   PrintEndXMLBlock( fp, "stub-exit-list" );
   
   condition_manager.SaveXML( fp, "precondition", "postcondition" );

   
#if 0
   // this formal-to-value stuff is as of yet unimplemented, so it cannot be saved
   PrintNewXMLBlock( fp, "instance-values" );
   for( ; ; )
   {
      sprintf( buffer, "formal-to-value formal-param=\"%s\" value=\"%s\" /", , );
      PrintXML( fp, buffer );
   }
   PrintEndXMLBlock( fp, "instance-values" );
#endif
   
   PrintEndXMLBlock( fp, "stub" );
}


void Stub::SaveCSMXMLDetails( FILE *fp )
{
/*
   char buffer[300];
   if( submaps.Size() == 0 )
	   fprintf( fp, " sub=\" \" ");
   else{
	   fprintf( fp, "sub=\" ");
       for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
       if( !XmlManager::MapExportable( submaps.CurrentItem() ) )
	      continue; // skip over maps not contained in an export list
       sprintf( buffer, "m%d ",submaps.CurrentItem()->GetNumber());
	   fprintf(fp, buffer );
	   }
	   } 
   fprintf( fp, " \" " );
   if( selection_policy )
      fprintf( fp, "selection-cond=\"%s\" ", PrintDescription( selection_policy ) );
   fprintf( fp, ">\n" );
   LinebreakXML( fp );
   IndentNewXMLBlock( fp );

   //PrintNewXMLBlock( fp, "stub-entry-list" );

   for( source->First(); !source->IsDone(); source->Next() )
   {
      sprintf( buffer, "InBinding id=\"si%d\" in=\"h%d\" /", source->CurrentItem()->GetNumber(), source->CurrentItem()->PreviousEdge()->GetNumber());
      PrintXML( fp, buffer );
   }
   
   //PrintEndXMLBlock( fp, "stub-entry-list" );

   //PrintNewXMLBlock( fp, "stub-exit-list" );

   for( target->First(); !target->IsDone(); target->Next() )
   {
      sprintf( buffer, "OutBinding id=\"so%d\" out=\"h%d\" /", target->CurrentItem()->GetNumber(), target->CurrentItem()->NextEdge()->GetNumber());
      PrintXML( fp, buffer );
   }

   ServiceRequest::SaveRequests( &service_requests, fp );
   //PrintEndXMLBlock( fp, "stub-exit-list" );
   
   condition_manager.SaveXML( fp, "precondition", "postcondition" );

   
#if 0
   // this formal-to-value stuff is as of yet unimplemented, so it cannot be saved
   PrintNewXMLBlock( fp, "instance-values" );
   for( ; ; )
   {
      sprintf( buffer, "formal-to-value formal-param=\"%s\" value=\"%s\" /", , );
      PrintXML( fp, buffer );
   }
   PrintEndXMLBlock( fp, "instance-values" );
#endif
   PrintEndXMLBlock( fp, "stub" );
  */
   char buffer[300];
   StubBinding *sb;
   PluginBinding *pb;
   if( submaps.Size() == 0 ){
     //sprintf( buffer, "<Refinement parent=\"h%d\" ", hyperedge_number );
	 //PrintXML(fp, buffer);
	   return;
   }

   for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
      if( !XmlManager::MapExportable( submaps.CurrentItem() ) )
	 continue; // skip over maps not contained in an export list
      sprintf( buffer, "<Refinement parent=\"h%d\" sub=\"m%d\"> ",hyperedge_number, submaps.CurrentItem()->GetNumber());
      PrintXMLText( fp, buffer );
      LinebreakXML( fp );

      pb = SubmapBinding( submaps.CurrentItem() );
	       
      //if( pb->logical_condition )
	 //fprintf( fp, " branch-condition=\"%s\"", PrintDescription( pb->logical_condition ) );
      //fprintf( fp, " probability=\"%f\" >\n", pb->probability );
      IndentNewXMLBlock( fp );
             
      if( pb->entry_bindings.Size() > 0 ) {
	 //PrintNewXMLBlock( fp, "in-connection-list" );
	 for( pb->entry_bindings.First(); !pb->entry_bindings.IsDone(); pb->entry_bindings.Next() ) {
	    if( pb->entry_bindings.CurrentItem()->path_endpoint ) {
	       sb = pb->entry_bindings.CurrentItem();
           sprintf( buffer, "<InBinding id=\"si%d\" start=\"h%d\" " , sb->boundary_node->GetNumber(), sb->path_endpoint->GetNumber());
	       PrintXMLText( fp, buffer );
          for( source->First(); !source->IsDone(); source->Next() )
           {
			   if((source->CurrentItem()->GetNumber())==(sb->boundary_node->GetNumber())){
                sprintf( buffer,"in=\"h%d\"/> ", source->CurrentItem()->PreviousEdge()->GetNumber());
				fprintf( fp,buffer );
				//PrintXMLText(fp, buffer);
			   }
           }
		   
           LinebreakXML( fp );
	    }
	 }
	 //PrintEndXMLBlock( fp, "in-connection-list" );
      }

      if( pb->exit_bindings.Size() > 0 ) {
	 //PrintNewXMLBlock( fp, "out-connection-list" );
	 for( pb->exit_bindings.First(); !pb->exit_bindings.IsDone(); pb->exit_bindings.Next() ) {
	    if( pb->exit_bindings.CurrentItem()->path_endpoint ) {
	       sb = pb->exit_bindings.CurrentItem();
	       sprintf( buffer, "<OutBinding id=\"so%d\" end=\"h%d\" ",sb->boundary_node->GetNumber(), sb->path_endpoint->GetNumber());
	       PrintXMLText( fp, buffer );
          for( target->First(); !target->IsDone(); target->Next() )
           {
			   if((sb->boundary_node->GetNumber())==(target->CurrentItem()->GetNumber()) ){
                  sprintf( buffer, "out=\"h%d\"/>", target->CurrentItem()->NextEdge()->GetNumber());
                  fprintf( fp, buffer );
			   }
           }
          LinebreakXML( fp );
	    }
	 }
	 //PrintEndXMLBlock( fp, "out-connection-list" );
      }
     PrintEndXMLBlock( fp, "Refinement" );
	 LinebreakXML( fp );
   } // end for submaps 
}


void Stub::OutputPerformanceData( std::ofstream &pf )
{
  PluginBinding *pb;

  if( submaps.Size() < 2 ) return;

   pf << "\nStub\t\"" << stub_label << "\"\n";

   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
     pb = plugin_bindings.CurrentItem();
     pf << "\"" << pb->plugin->MapLabel() << "\"\n";
     pf << pb->probability << std::endl;
   }
}

PluginBinding * Stub::SubmapBinding( Map *plugin )
{
   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
      if( plugin_bindings.CurrentItem()->plugin == plugin )
	 return( plugin_bindings.CurrentItem() );
   }

   return( NULL ); // if no bindings were found
}

void Stub::SaveBindings( FILE *fp )
{
   char buffer[300];
   StubBinding *sb;
   PluginBinding *pb;
  
   if( submaps.Size() == 0 ) return;

   for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
      if( !XmlManager::MapExportable( submaps.CurrentItem() ) )
	 continue; // skip over maps not contained in an export list
      sprintf( buffer, "<plugin-binding parent-map-id=\"m%d\" submap-id=\"m%d\" stub-id=\"h%d\"",
	       parent_map->GetNumber(), submaps.CurrentItem()->GetNumber(), hyperedge_number );
      PrintXMLText( fp, buffer );

      pb = SubmapBinding( submaps.CurrentItem() );
	       
      if( pb->logical_condition )
	 fprintf( fp, " branch-condition=\"%s\"", PrintDescription( pb->logical_condition ) );
      fprintf( fp, " probability=\"%f\" >\n", pb->probability );
      IndentNewXMLBlock( fp );
      
      if( pb->entry_bindings.Size() > 0 ) {
	 PrintNewXMLBlock( fp, "in-connection-list" );
	 for( pb->entry_bindings.First(); !pb->entry_bindings.IsDone(); pb->entry_bindings.Next() ) {
	    if( pb->entry_bindings.CurrentItem()->path_endpoint ) {
	       sb = pb->entry_bindings.CurrentItem();
	       sprintf( buffer, "in-connection hyperedge-id=\"h%d\" stub-entry-id=\"si%d\" /", sb->path_endpoint->GetNumber(),
			sb->boundary_node->GetNumber() );
	       PrintXML( fp, buffer );
	    }
	 }
	 PrintEndXMLBlock( fp, "in-connection-list" );
      }

      if( pb->exit_bindings.Size() > 0 ) {
	 PrintNewXMLBlock( fp, "out-connection-list" );
	 for( pb->exit_bindings.First(); !pb->exit_bindings.IsDone(); pb->exit_bindings.Next() ) {
	    if( pb->exit_bindings.CurrentItem()->path_endpoint ) {
	       sb = pb->exit_bindings.CurrentItem();
	       sprintf( buffer, "out-connection hyperedge-id=\"h%d\" stub-exit-id=\"so%d\" /", sb->path_endpoint->GetNumber(),
			sb->boundary_node->GetNumber() );
	       PrintXML( fp, buffer );
	    }
	 }
	 PrintEndXMLBlock( fp, "out-connection-list" );
      }
      PrintEndXMLBlock( fp, "plugin-binding" );
   } // end for submaps 
}

void Stub::SaveContinuityBindings( FILE *fp )
{
   ContinuityBinding *cb;
   char buffer[100];

   if( continuity_bindings.Size() == 0 )
      return;

   sprintf( buffer, "enforce-bindings stub-id=\"h%d\" ", hyperedge_number );
   PrintNewXMLBlock( fp, buffer );
   
   for( continuity_bindings.First(); !continuity_bindings.IsDone(); continuity_bindings.Next() ) {
      cb = continuity_bindings.CurrentItem();
      sprintf( buffer, "path-binding stub-entry-id=\"si%d\" stub-exit-id=\"so%d\" /",
	       cb->input_node->PreviousEdge()->GetNumber(), cb->output_node->NextEdge()->GetNumber() );
      PrintXML( fp, buffer );
   }
   
   PrintEndXMLBlock( fp, "enforce-bindings" );
}

void Stub::AddContinuityBinding( Hyperedge *stub_input, Hyperedge *stub_output )
{
   continuity_bindings.Add( new ContinuityBinding( stub_input->TargetSet()->GetFirst(), stub_output->SourceSet()->GetFirst() ) );
}

bool Stub::ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir )
{
   ((StubFigure *)figure)->ReplaceDependentPath( new_path, old_path ); // replace the old path in the dependent path list
   return FALSE;  // as end of path from either direction is reached
}

Path * Stub::InputPath()
{
   return( source->GetFirst()->PreviousEdge()->GetFigure()->GetPath() );
}

void Stub::ResetGenerationState()
{
   traversal_context = 0;

   while( !traversal_bindings.is_empty() )
      traversal_bindings.Detach();
}

Hyperedge * Stub::GenerateMSC( bool first_pass )
{
   MSCGenerator *msc= MSCGenerator::Instance();

   if( submaps.Size() == 0 )
      return( target->GetFirst()->NextEdge() ); // as stub has only one output simply skip over

   if( generation_pass != first_pass ) {
      traversal_context = 0;
      generation_pass = first_pass;
   }

   msc->DecomposeStub( this ); // add this stub to top of msc generator stack
   return( traversal_bindings.Get( ++traversal_context )->SubmapPathStart( msc->PreviousEmpty()->PrimaryTarget() ) );
}

Hyperedge * Stub::ScanScenarios( scan_type type )
{
//   Map *current_plugin;
   Map *sole_plugin = NULL;
   GenerationState *gs = NULL;
   int submap_count, target_count, i;
   char buffer[200];
   MSCGenerator *msc= MSCGenerator::Instance();

   submap_count = submaps.Size();
   target_count = target->Size();

   if( tracing_id != ScenarioList::ScenarioTracingId() ) {
     while( !scenario_submaps.is_empty() )
       scenario_submaps.Detach();
     tracing_id = ScenarioList::ScenarioTracingId();
   }
   
   if( submap_count == 0 ) {
      if( target_count == 1 )
	 return( target->GetFirst()->NextEdge() ); // as stub has only one output simply skip over
      else {
	 sprintf( buffer, "Stub \"%s\" in Map \"%s\" does not have any plugin maps defined\nand has %d outputs.",
		  stub_label, parent_map->MapLabel(), target_count );
	 ErrorMessage( buffer, TRUE );
	 msc->AbortGeneration();
	 return( NULL ); // stop this branch of scanning as an error resulted
      }
   }
   else if( submap_count == 1 ) {
      sole_plugin = submaps.GetFirst();
      //sole_plugin->setScanningParentStub(this); 
      ScanPluginMap( sole_plugin , type );
      return( NULL );
   }
   else { // multiple submaps
      if( msc->ScenarioTracing() ) {
	 for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
	    if( PathVariableList::EvaluateLogicalExpression( SubmapBinding( submaps.CurrentItem() )->ReferencedCondition() ) == TRUE ) {
	       if( sole_plugin == NULL )
		  sole_plugin = submaps.CurrentItem();
	       else { // two submaps evaluated to true
		  sprintf( buffer, "The dynamic stub \"%s\" in Map \"%s\"\n has multiple submaps whose logical selection conditions evaluated to true.",
			   stub_label, parent_map->MapLabel(), submap_count );
		  ErrorMessage( buffer, TRUE );
		  msc->AbortGeneration();
		  return( NULL );
	       }
	    }
	 }

	 if( sole_plugin == NULL ) {
	    sprintf( buffer, "The dynamic stub \"%s\" in Map \"%s\"\nhas %d submaps and none of their logical selection conditions evaluated to true.",
		     stub_label, parent_map->MapLabel(), submap_count );
	    ErrorMessage( buffer, TRUE );
	    msc->AbortGeneration();
	    return( NULL );
	 }

	 if( msc->HighlightActive() )
	   scenario_submaps.AddUnique( sole_plugin );
	 //sole_plugin->setScanningParentStub(this); 
	 ScanPluginMap( sole_plugin, type );
	 return( NULL );
      }
      else { // scan all submaps, no scenario tracing
	 if( submap_count > 1 )
	    gs = new GenerationState;

	 for( i = 1; i <= submap_count; i++ ) {
	    ScanPluginMap( submaps.Get( i ), type );
	    if( msc->GenerationAborted() )
	       break; // stop recursive scanning
	    if( i > 1 )
	       msc->ResetGenerationState( gs );
	 }

	 if( submap_count > 1 )
	    delete gs;
	 return( NULL ); // stop this branch of scanning as it is complete as paths in plugins will either end in the plugin or exit
	 // the stub through a bound end point and continue along the original path meaning they have already been scanned at this point
      }
   }
}


Hyperedge * Stub::HighlightScenario( )
{
//   Map *current_plugin;
   Map *sole_plugin = NULL;
//   GenerationState *gs = NULL;
   int submap_count, target_count;
// int i;
   char buffer[200];
   MSCGenerator *msc= MSCGenerator::Instance();

   submap_count = submaps.Size();
   target_count = target->Size();

   if( tracing_id != ScenarioList::ScenarioTracingId() ) {
     while( !scenario_submaps.is_empty() )
       scenario_submaps.Detach();
     tracing_id = ScenarioList::ScenarioTracingId();
   }
   
   if( submap_count == 0 ) {
      if( target_count == 1 )
	 return( target->GetFirst()->NextEdge() ); // as stub has only one output simply skip over
      else {
	 sprintf( buffer, "Stub \"%s\" in Map \"%s\" does not have any plugin maps defined\nand has %d outputs.",
		  stub_label, parent_map->MapLabel(), target_count );
	 ErrorMessage( buffer, TRUE );
	 msc->AbortGeneration();
	 return( NULL ); // stop this branch of scanning as an error resulted
      }
   }
   else if( submap_count == 1 ) {
      sole_plugin = submaps.GetFirst();
      ScanPluginMap( sole_plugin , SCENARIO_TRACE );
      return( NULL );
   }
   else { // multiple submaps
      for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
         if( PathVariableList::EvaluateLogicalExpression( SubmapBinding( submaps.CurrentItem() )->ReferencedCondition() ) == TRUE ) {
            if( sole_plugin == NULL )
               sole_plugin = submaps.CurrentItem();
	    else { // two submaps evaluated to true
	       sprintf( buffer, "The dynamic stub \"%s\" in Map \"%s\"\n has multiple submaps whose logical selection conditions evaluated to true.",
			   stub_label, parent_map->MapLabel(), submap_count );
	       ErrorMessage( buffer, TRUE );
	       msc->AbortGeneration();
	       return( NULL );
	    }
	 }
      }

      if( sole_plugin == NULL ) {
	 sprintf( buffer, "The dynamic stub \"%s\" in Map \"%s\"\nhas %d submaps and none of their logical selection conditions evaluated to true.",
	     stub_label, parent_map->MapLabel(), submap_count );
	 ErrorMessage( buffer, TRUE );
	 msc->AbortGeneration();
	 return( NULL );
      }

      if( msc->HighlightActive() )
         scenario_submaps.AddUnique( sole_plugin );
	 //sole_plugin->setScanningParentStub(this); 
      ScanPluginMap( sole_plugin, SCENARIO_TRACE );
      return( NULL );
   }
}


void Stub::ScanPluginMap( Map *submap, scan_type type )
{
   Hyperedge *submap_start, *current_edge;
   PluginBinding *plugin_binding;
   char buffer[200];
   MSCGenerator *msc= MSCGenerator::Instance();

   if( msc->SubmapRecursionExists( submap ) ) {
      sprintf( buffer, "The dynamic stub \"%s\" in Map \"%s\"\nhas a plugin \"%s\" which is a parent map of the current map.",
	       stub_label, parent_map->MapLabel(), submap->MapLabel() );
      ErrorMessage( buffer, TRUE );
      msc->AbortGeneration();
   }
   else if( (submap_start = SubmapBinding( submap )->SubmapPathStart( msc->PreviousEmpty()->PrimaryTarget() ) ) == NULL ) {
      sprintf( buffer, "Stub \"%s\" in Map \"%s\" has incomplete input bindings.",
	       stub_label, parent_map->MapLabel() );
      ErrorMessage( buffer, TRUE );
      msc->AbortGeneration();
   }
   else { // if no error conditions exist, scan submap
      plugin_binding = SubmapBinding( submap );
      if( msc->ScenarioTracing() ) {
	 if( traversal_bindings.Size() == 0 )
	    msc->RegisterPathElement( this );
      }     
      traversal_bindings.Add( plugin_binding );
      msc->DecomposeStub( this ); // add this stub to top of msc generator stack

      current_edge = submap_start;
      submap->setScanningParentStub(this);
      current_submap = submap;
      while( current_edge ) { // scan along the starting path of the sole plugin that evaluates to true
         if( type == SCENARIO_TRACE ) {
	    current_edge = current_edge->HighlightScenario( );
	 }
         else {
	    current_edge = current_edge->ScanScenarios( type );
	 }
      }  
      if( !msc->ScenarioTracing() )
	 traversal_bindings.detachr();
   }
}


void Stub::DeleteGenerationData()
{
   while( !traversal_bindings.is_empty() )
      traversal_bindings.Detach();
}

Start * Stub::SubmapPathStart( Empty *empty )
{  // this procedure assumes only a single submap, i.e. a static stub
   Node *input_node;

   if( submaps.Size() == 0 )
      return( NULL );
    
   input_node = empty->PrimaryTarget();

   Cltn<StubBinding *> &entry_bindings = plugin_bindings.GetFirst()->entry_bindings;

   for( entry_bindings.First(); !entry_bindings.IsDone(); entry_bindings.Next() ) {
      if( entry_bindings.CurrentItem()->boundary_node == input_node ) {
	 return( (Start *)entry_bindings.CurrentItem()->path_endpoint );
	 break;
      }
   }

   // if this point has been reached bindings may not have been made, check if there is only one start point in plugin map

#ifdef MSC_DEBUG	       
   std::cout << "Bindings not found - path start - stub: " << stub_label << std::endl << std::flush;
#endif

   Start *sole_start = NULL;
   Cltn<Hyperedge *> *edges = submaps.GetFirst()->MapHypergraph()->Hyperedges();

   for( edges->First(); !edges->IsDone(); edges->Next() ) {
      if( edges->CurrentItem()->EdgeType() == START ) {
	 if( sole_start )
	    return( NULL ); // stop traversal as at least two start points exist 
	 else
	    sole_start =  (Start *)edges->CurrentItem();
      }
   }

   return( sole_start );
}

Start * Stub::SubmapPathStart( Empty *empty, Map *submap )
{
   Node *input_node = empty->PrimaryTarget();
   Cltn<StubBinding *> *entry_bindings = EntryBindings( submap );
   StubBinding *sb;

   for( entry_bindings->First(); !entry_bindings->IsDone(); entry_bindings->Next() ) {
      sb = entry_bindings->CurrentItem();
      if( sb->boundary_node == input_node )
	 return( (Start *)sb->path_endpoint );
   }

   return( NULL );
}

Result * Stub::SubmapPathEnd( Empty *empty, Map *submap )
{
   Node *output_node = empty->SourceSet()->GetFirst();
   Cltn<StubBinding *> *exit_bindings = ExitBindings( submap );
   StubBinding *sb;

   for( exit_bindings->First(); !exit_bindings->IsDone(); exit_bindings->Next() ) {
      sb = exit_bindings->CurrentItem();
      if( sb->boundary_node == output_node )
	 return( (Result *)sb->path_endpoint );
   }

   return( NULL );
}

Result * Stub::SubmapPathEnd( Empty *empty )
{ // this procedure assumes only a single submap, i.e. a static stub
   Node *output_node;

   if( submaps.Size() == 0 )
      return( NULL );
   
   output_node = empty->SourceSet()->GetFirst();

   Cltn<StubBinding *> &exit_bindings = plugin_bindings.GetFirst()->exit_bindings;

   for( exit_bindings.First(); !exit_bindings.IsDone(); exit_bindings.Next() ) {
      if( exit_bindings.CurrentItem()->boundary_node == output_node )
	 return( (Result *)exit_bindings.CurrentItem()->path_endpoint );
   }

   return( NULL );
}

Hyperedge * Stub::StubExitPath( Result *result, Map *submap )
{
   Cltn<StubBinding *> *exit_bindings;

   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
      if( plugin_bindings.CurrentItem()->plugin == submap ) {
	 exit_bindings = &(plugin_bindings.CurrentItem()->exit_bindings);
	 break;
      }
   }
   
   for( exit_bindings->First(); !exit_bindings->IsDone(); exit_bindings->Next() ) {
      if( exit_bindings->CurrentItem()->path_endpoint == result )
	 return( exit_bindings->CurrentItem()->boundary_node->NextEdge() ); // return the corresponding stub output point
   }

   // if this point has been reached bindings may not have been made, check if there is only one path end in plugin map
   // and that stub has only one output point

#ifdef MSC_DEBUG	       
   std::cout << "Bindings not found - result: " << result->HyperedgeName() << " stub exit path - stub: " << stub_label << std::endl << std::flush;
#endif

   Result *sole_end = NULL;
   Cltn<Hyperedge *> *edges = submaps.GetFirst()->MapHypergraph()->Hyperedges();

   if( target->Size() > 1 )
      return( NULL ); // stop traversal as stub has more than one output

   for( edges->First(); !edges->IsDone(); edges->Next() ) {
      if( edges->CurrentItem()->EdgeType() == RESULT ) {
	 if( sole_end )
	    return( NULL ); // stop traversal as at least two path ends exist 
	 else
	    sole_end = (Result *)edges->CurrentItem();
      }
   }

   return( sole_end ? target->GetFirst()->NextEdge() : NULL );
}

bool Stub::ContainsSubmap( Map *map )
{
   for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
      if( submaps.CurrentItem() == map )
	 return TRUE;
   }

   return FALSE;
}

bool Stub::BoundToEndpoint( Hyperedge *endpoint, Map *submap )
{
   Cltn<StubBinding *> *bindings;
   PluginBinding *pb;

   for( plugin_bindings.First(); !plugin_bindings.IsDone(); plugin_bindings.Next() ) {
      pb = plugin_bindings.CurrentItem();
      if( pb->plugin == submap )
	 bindings = ((endpoint->EdgeType() == START) ? &(pb->entry_bindings) : &(pb->exit_bindings) );
   }

   for( bindings->First(); !bindings->IsDone(); bindings->Next() ) {
      if( bindings->CurrentItem()->path_endpoint == endpoint )
	 return TRUE;
   }

   return FALSE;
}

bool Stub::InputBound( Map *submap, Empty *empty )
{
   Node *input_node = empty->PrimaryTarget();
   Cltn<StubBinding *> *entry_bindings = EntryBindings( submap );

   for( entry_bindings->First(); !entry_bindings->IsDone(); entry_bindings->Next() ) {
      if( entry_bindings->CurrentItem()->boundary_node == input_node )
	 return( (entry_bindings->CurrentItem()->path_endpoint != NULL) ? TRUE : FALSE );
   }
   return FALSE;
}

Hyperedge * Stub::TraverseScenario( )
{
//   Map *current_plugin;
   Map *sole_plugin = NULL;
// GenerationState *gs = NULL;
   int submap_count, target_count;
// int i;
   char buffer[200];
   SCENARIOGenerator *sce= SCENARIOGenerator::Instance();

   sce->setCurrentStub(NULL);
   sce->setCurrentStub(this);
   

   submap_count = submaps.Size();
   target_count = target->Size();

   if( tracing_id != ScenarioList::ScenarioTracingId() ) {
     while( !scenario_submaps.is_empty() )
       scenario_submaps.Detach();
     tracing_id = ScenarioList::ScenarioTracingId();
   }
   
   if( submap_count == 0 ) {
      if( target_count == 1 )
	 return( target->GetFirst()->NextEdge() ); // as stub has only one output simply skip over
      else {
	 sprintf( buffer, "Stub \"%s\" in Map \"%s\" does not have any plugin maps defined\nand has %d outputs.",
		  stub_label, parent_map->MapLabel(), target_count );
	 ErrorMessage( buffer, TRUE );
	 sce->AbortGeneration();
	 return( NULL ); // stop this branch of scanning as an error resulted
      }
   }
   else if( submap_count == 1 ) {
      sole_plugin = submaps.GetFirst();
      //sole_plugin->setScanningParentStub(this); 
      TraversePluginMap( sole_plugin ); //tempoaraily added, need to be modified he 
      return( NULL );
   }
   else { // multiple submaps
      for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
         if( PathVariableList::EvaluateLogicalExpression( SubmapBinding( submaps.CurrentItem() )->ReferencedCondition() ) == TRUE ) {
	     if( sole_plugin == NULL )
                sole_plugin = submaps.CurrentItem();
	     else { // two submaps evaluated to true
	 	sprintf( buffer, "The dynamic stub \"%s\" in Map \"%s\"\n has multiple submaps whose logical selection conditions evaluated to true.",
			   stub_label, parent_map->MapLabel(), submap_count );
		ErrorMessage( buffer, TRUE );
		sce->AbortGeneration();
		return( NULL );
	     }
	  }
      }
      
      
      if( sole_plugin == NULL ) {
          sprintf( buffer, "The dynamic stub \"%s\" in Map \"%s\"\nhas %d submaps and none of their logical selection conditions evaluated to true.",
             stub_label, parent_map->MapLabel(), submap_count );
          ErrorMessage( buffer, TRUE );
          sce->AbortGeneration();
          return( NULL );
      }
      /**************  SCUI ******************/
      
      char sole_exp[LABEL_SIZE*2+1];
      const char * condition_label = NULL;
      PluginBinding *pb;
      
      pb = SubmapBinding( sole_plugin ); 
      strncpy( sole_exp, PrintDescription( pb->logical_condition ) , LABEL_SIZE*2 );
	    
      condition_label = sole_plugin->MapLabel();
      
      Path_data * selection = new Conditions( hyperedge_number, condition_label, sole_exp );
      sce->AddPathTraversal(selection);
      /*  sce->AddPathList(selection); */

      /*********** END SCUI *********************/

         
      // if( msc->HighlightActive() )
      //   scenario_submaps.AddUnique( sole_plugin );
      //sole_plugin->setScanningParentStub(this);
      TraversePluginMap( sole_plugin );
      return( NULL );
   }

      
      
}

void Stub::TraversePluginMap( Map *submap )
{
   Hyperedge *submap_start, *current_edge;
   PluginBinding *plugin_binding;
   char buffer[200];
   SCENARIOGenerator *sce= SCENARIOGenerator::Instance();

   if( sce->SubmapRecursionExists( submap ) ) {
      sprintf( buffer, "The dynamic stub \"%s\" in Map \"%s\"\nhas a plugin \"%s\" which is a parent map of the current map.",
               stub_label, parent_map->MapLabel(), submap->MapLabel() );
      ErrorMessage( buffer, TRUE );
      sce->AbortGeneration();
   }
   else if( (submap_start = SubmapBinding( submap )->SubmapPathStart( sce->PreviousEmpty()->PrimaryTarget() ) ) == NULL ) {
      sprintf( buffer, "Stub \"%s\" in Map \"%s\" has incomplete input bindings.",
               stub_label, parent_map->MapLabel() );
      ErrorMessage( buffer, TRUE );
      sce->AbortGeneration();
   }
   else { // if no error conditions exist, scan submap
      plugin_binding = SubmapBinding( submap );
     /* if( msc->ScenarioTracing() ) {
         if( traversal_bindings.Size() == 0 )
            msc->RegisterPathElement( this );
      }  */
      traversal_bindings.Add( plugin_binding );
     // msc->DecomposeStub( this ); // add this stub to top of msc generator stack

      current_edge = submap_start;
      submap->setScanningParentStub(this);
      current_submap = submap;
      while( current_edge ) // scan along the starting path of the sole plugin that evaluates to true
         current_edge = current_edge->TraverseScenario( );

      //if( !msc->ScenarioTracing() )
      //  traversal_bindings.detachr();
   }
}
     

void Stub::SaveDXLDetails( FILE *fp )
{
   fprintf( fp, "\"%s\", \"%s\", \"", HyperedgeName(), ( stype == STATIC ? "static" : "dynamic" ) );
   for( submaps.First(); !submaps.IsDone(); submaps.Next() ) {
       fprintf( fp, "m%d;", submaps.CurrentItem()->GetNumber() );
   }   
   fprintf( fp, "\" )");
}
