/***********************************************************
 *
 * File:			or_join.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "or_join.h"
#include "or_figure.h"
#include "empty.h"
#include "connect.h"
#include "start.h"
#include "result.h"
#include "hypergraph.h"
#include "path.h"
#include "map.h"
#include "msc_generator.h"

extern void DrawScreen();
extern int showing_labels;

int OrJoin::number_joins = 1;

ElementContext::~ElementContext()
{
   if( stub_hierarchy != NULL ) delete [] stub_hierarchy;
}

bool ElementContext::SameContext( int *hierarchy, int level  )
{
   if( level != decomposition_level )
      return FALSE;
   else {
      for( int i = 0; i < decomposition_level; i++ ) {
	 if( hierarchy[i] != stub_hierarchy[i] )
	    return FALSE;
      }
   }
   return TRUE;
}

OrJoin::OrJoin() : MultipathEdge()
{
   figure = new OrFigure( this );
}

OrJoin::OrJoin( int join_id, float x, float y, const char *desc, int orientation ) : MultipathEdge()
{
   load_number = join_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = join_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;  
   //End of the modification by Bo Jiang, April, 2005
   
   figure = new OrFigure( this );
   figure->SetPosition( x, y );
   if( desc != NULL ) unique_desc = strdup( desc );
   ((OrFigure *)figure)->Orientation( orientation );
}

bool OrJoin::Perform( transformation trans, execution_flag execute )
{
   if( trans == ROTATE ) {
      if( execute )
	 ((OrFigure *)figure)->ChangeOrientation();
      else
	 return( TRUE );
   }   
   return FALSE;
}

int OrJoin::PathCount()
{
   return( source->Size() );
}

void OrJoin::ConnectForkPath( Hyperedge *edge )
{
   Node *node = new Node( A );
   Node *front_node = edge->SourceSet()->GetFirst();
   Hyperedge *prev_edge = front_node->PreviousEdge();
  
   AttachSource( node );
   prev_edge->DetachTarget( front_node );
   prev_edge->AttachTarget( node );
  
   edge->GetFigure()->GetPath()->SetPathEnd( this );
  
   display_manager->CreateOrJoin( this, (Empty *)prev_edge );

   prev_edge->GetFigure()->GetPath()->PathChanged();

   parent_map->MapHypergraph()->PurgeNode( front_node );
   parent_map->MapHypergraph()->PurgeEdge( edge );
}

void OrJoin::Delete()
{
   // reset current label so that empty will be created with proper label
   current_label = source->GetFirst()->PreviousEdge()->PathLabel();

   Node *front_node = source->GetFirst();
   Node *end_node = target->GetFirst();
   Hyperedge *prev_edge = front_node->PreviousEdge();
   Hyperedge *next_edge = end_node->NextEdge(); 

   display_manager->SetPath( figure->GetPath() );
   display_manager->DeleteEndNull( prev_edge ); // delete end null of path entering join
   display_manager->JoinPaths( prev_edge, this ); 

  // change path stored in null at start of path at and fork
   figure->GetPath()->GetStartFigure()->SetPath( figure->GetPath() );

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
}

bool OrJoin::ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir )
{
   if( figure->GetPath() == old_path ) // backward search or forward search begining at fork itself
      figure->SetPath( new_path ); // set path for or fork figure itself
   else // search is forward from input path into fork
      new_path->GetEndFigure()->SetPath( new_path ); // set path in or null figure ending path

   return FALSE;  // as end of path from either direction is reached
}

Hyperedge * OrJoin::ScanScenarios( scan_type type )
{
   int map_level, *map_context = NULL;
   char title[150];
//   Hyperedge *current_edge;
   bool context_found = FALSE;
   ElementContext *current_context;
   MSCGenerator *msc = MSCGenerator::Instance();

   // determine if this context of this fork object has been reached before, indicating an implicit loop, count such traversals
   msc->MapContext( &map_context, map_level );

   for( context_history.First(); !context_history.IsDone(); context_history.Next() ) {
      current_context = context_history.CurrentItem();
      if( current_context->SameContext( map_context, map_level ) ) {
	 context_found = TRUE;
	 current_context->IncrementTraversalCount();
	 if( current_context->TraversalCount() == 100 ) { // stop traversal as possibly endless loop has been detected
	    sprintf( title, "Possibly Endless Path Loop Detected in Map %s", parent_map->MapLabel() );
	    fl_show_alert( title, "The same context of an Or Join has been reached 100 times.", 
			   " Please use the path control data model to control path looping.", 0 );
	    msc->AbortGeneration();
	    return( NULL );
	 }
	 break;
      }
   }

   if( !context_found ) {
      if( context_history.Size() == 0 )
    	 msc->RegisterPathElement( this );
      context_history.Add( new ElementContext( map_context, map_level ) ); // add this join to bottom of stack
   }
   else
      if( map_context != NULL ) delete [] map_context;

   return( PrimaryTarget()->NextEdge() );
}

Hyperedge * OrJoin::HighlightScenario( )
{
   int map_level, *map_context = NULL;
   char title[150];
//   Hyperedge *current_edge;
   bool context_found = FALSE;
   ElementContext *current_context;
   MSCGenerator *msc = MSCGenerator::Instance();

   // determine if this context of this fork object has been reached before, indicating an implicit loop, count such traversals
   msc->MapContext( &map_context, map_level );

   for( context_history.First(); !context_history.IsDone(); context_history.Next() ) {
      current_context = context_history.CurrentItem();
      if( current_context->SameContext( map_context, map_level ) ) {
	 context_found = TRUE;
	 current_context->IncrementTraversalCount();
	 if( current_context->TraversalCount() == 100 ) { // stop traversal as possibly endless loop has been detected
	    sprintf( title, "Possibly Endless Path Loop Detected in Map %s", parent_map->MapLabel() );
	    fl_show_alert( title, "The same context of an Or Join has been reached 100 times.", 
			   " Please use the path control data model to control path looping.", 0 );
	    msc->AbortGeneration();
	    return( NULL );
	 }
	 break;
      }
   }

   if( !context_found ) {
      if( context_history.Size() == 0 )
    	 msc->RegisterPathElement( this );
      context_history.Add( new ElementContext( map_context, map_level ) ); // add this join to bottom of stack
   }
   else
      if( map_context != NULL ) delete [] map_context;

   return( PrimaryTarget()->NextEdge() );
}

void OrJoin::DeleteGenerationData()
{
   while( !context_history.is_empty() )
      delete context_history.Detach();
}

void OrJoin::SaveXMLDetails( FILE *fp )
{
   char buffer[300];

   sprintf( buffer, "join orientation=\"%d\" /", ((OrFigure *)figure)->Orientation() );
   PrintXML( fp, buffer );
}

void OrJoin::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}

void OrJoin::SaveCSMXMLDetails( FILE *fp )
{
	
   char buffer[300];

   sprintf( buffer, "join orientation=\"%d\" /", ((OrFigure *)figure)->Orientation() );
   PrintXML( fp, buffer );

   
}


