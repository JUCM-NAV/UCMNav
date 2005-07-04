/***********************************************************
 *
 * File:			loop.cc
 * Author:			Andrew Miga
 * Created:			April 2000
 *
 * Modification history:
 ***********************************************************/

#include "loop.h"
#include "loop_figure.h"
#include "start.h"
#include "empty.h"
#include "result.h"
#include "node.h"
#include "hypergraph.h"
#include "print_mgr.h"
#include "msc_generator.h"
#include "component_ref.h"
#include "utilities.h"

Loop::Loop() : Hyperedge() 
{
   figure = new LoopFigure( this );

   identifier[0] = 0;
   identifier[19] = 0;
   strcpy( loop_count, "1" );
   loop_count[19] = 0;
}

Loop::Loop( int loop_id, const char *name, float x, float y, const char *desc, const char *orientation, const char *count ) : Hyperedge()
{
   load_number = loop_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = loop_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;
   //End of the modification by Bo Jiang, April, 2005   
   
   figure = new LoopFigure( this, orientation );
   figure->SetPosition( x, y );
   if( desc != NULL ) unique_desc = strdup( desc );
   identifier[0] = 0;
   identifier[19] = 0;
   strncpy( identifier, name, 19 );
   strcpy( loop_count, ((count != NULL) ? count : "1" ) );
   loop_count[19] = 0;
}

bool Loop::Perform( transformation trans, execution_flag execute )
{
   switch( trans ) {
   
   case EDIT_LOOP_CHARACTERISTICS:
      return( EditLoopCharacteristics( execute ) );
      break;
      
   case ROTATE:
      
      if( execute )
	 ((LoopFigure *)figure)->ChangeOrientation();
      return( TRUE );
      break;

   case DELETE_LOOP:      
      return( DeleteLoop( execute ) );
      break;
   }
   return FALSE;
}

void Loop::Create()
{
   Node *node1, *node2, *node3, *node4;
   Empty *out_empty, *mid_empty, *in_empty;
   Path *loop_path;

   // create loop of three empty points
   display_manager->CreateNewSegment();     

   node1 = new Node( B );  
   node2 = new Node( A );
   node3 = new Node( A );
   node4 = new Node( A );

   out_empty = new Empty;
   mid_empty = new Empty;
   in_empty = new Empty;
  
   this->AttachTarget( node1 );
   out_empty->AttachSource( node1 );   
   out_empty->AttachTarget( node2 );
   mid_empty->AttachSource( node2 );
   mid_empty->AttachTarget( node3 );
   in_empty->AttachSource( node3 );
   in_empty->AttachTarget( node4 );
   this->AttachSource( node4 );

   loop_path = out_empty->GetFigure()->GetPath();
   loop_path->SetPathStart( this ); // set path start to loop
   loop_path->SetPathEnd( this ); // set path end to loop
   mid_empty->AddDirectionArrow( TRUE );

   display_manager->CreateLoop( this, in_empty, out_empty );
}

bool Loop::EditLoopCharacteristics( execution_flag execute )
{
   char *name, *count;

   if( execute ) {
      if( TwoStringDialog( "Edit Loop Characteristics", "Loop Name", "Loop Count",
			   identifier, loop_count, &name, &count ) == TRUE ) {
	 strncpy( identifier, name, 19 );
	 strncpy( loop_count, count, 19 );
	 return MODIFIED;
      }
      else
	 return UNMODIFIED;
   }
   else
      return TRUE;
}

bool Loop::DeleteHyperedge()
{
   if( DeleteLoop( FALSE ) == TRUE )
      return( DeleteLoop( TRUE ) );
   else
      return FALSE;  
}

bool Loop::DeleteLoop( execution_flag execute )
{
   Hyperedge *scan_edge;
   edge_type etype;
   Node *front_node, *end_node, *node1, *node2;
   Hyperedge *prev_edge, *next_edge;
   Empty *new_empty;

   if( execute ){
      front_node = source->GetFirst();
      end_node = target->GetFirst();
      prev_edge = front_node->PreviousEdge();
      next_edge = end_node->NextEdge();

      display_manager->DeleteStartNull( next_edge ); // delete start null of main path output
      display_manager->DeleteEndNull( prev_edge );   // delete end null of main path input
      display_manager->DeleteStartNull( target->Get( 2 )->NextEdge() ); // delete start null of loop output
      display_manager->DeleteEndNull( source->Get( 2 )->PreviousEdge() );   // delete end null of loop input
      display_manager->JoinPaths( prev_edge, next_edge ); 

      new_empty = new Empty;
      node1 = new Node( B );
      node2 = new Node( A );
      new_empty->AttachSource( node1 );
      new_empty->AttachTarget( node2 );

      next_edge->ReplaceSource( end_node, node2 );
      prev_edge->ReplaceTarget( front_node, node1 );
     
      display_manager->AddAtPosition( new_empty, this, next_edge );

      parent_map->MapHypergraph()->PurgeLoop( front_node, end_node );
      display_manager->SetActive( NULL );

      return( TRUE );
   }
   else {  // allow deletion of loop only if it's input and output paths are empty

      scan_edge = source->Get( 2 )->PreviousEdge(); // get the source edge of the loop input
      etype = scan_edge->EdgeType();

      while( etype != START && etype != LOOP ) {
	 if( etype != EMPTY )
	    return( FALSE ); // disable delete if a non empty element is found
	 scan_edge = scan_edge->FirstInput();
	 etype = scan_edge->EdgeType();
      }
      if( etype == LOOP ) // if there is an empty path from input to output it can be deleted
	 return( (scan_edge == this) ? TRUE : FALSE );

      scan_edge = target->Get(2)->NextEdge(); // get the target edge of the loop output
      etype = scan_edge->EdgeType();

      while( etype != RESULT ) {
	 if( etype != EMPTY )
	    return( FALSE ); // disable delete if a non empty element is found
	 scan_edge = scan_edge->FirstOutput();
	 etype = scan_edge->EdgeType();
      }

      return( TRUE ); // as nothing was found on the loop inputs or outputs it can be deleted
   }
}

Hyperedge * Loop::GenerateMSC( bool first_pass )
{
   MSCGenerator *msc = MSCGenerator::Instance();

   // use the pointer of the input hyperedge to determine if the incoming path is the main path or the loop input
   if( LoopInput( msc->PreviousEmpty() ) == MAIN_IN ) {
      msc->ComponentCrossing( figure->GetContainingComponent(), first_pass );
      if( !first_pass ) msc->StartInlineConstruct( LOOP_SECTION );
      return( target->Get( 2 )->NextEdge() ); // return first point of loop output
   } else { // LOOP_IN
      if( !first_pass ) msc->EndInlineConstruct( LOOP_SECTION );
      return( target->GetFirst()->NextEdge() ); // return output of main path
   }
}

Hyperedge * Loop::ScanScenarios( scan_type type )
{
   // assume loop is connected and no paths terminate or fork inside loop
   MSCGenerator *msc = MSCGenerator::Instance();

   // use the pointer of the input hyperedge to determine if the incoming path is the main path or the loop input
   if( LoopInput( msc->PreviousEmpty() ) == MAIN_IN )
      return( target->Get( 2 )->NextEdge() ); // return first point of loop output
   else // LOOP_IN
      return( target->GetFirst()->NextEdge() ); // return output of main path
}

bool Loop::ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir )
{
   if( sdir == FORWARD_SEARCH )
      new_path->GetEndFigure()->SetPath( new_path ); // set path in loop null figure ending path
   else // BACKWARD_SEARCH
      new_path->GetStartFigure()->SetPath( new_path ); // set path in loop null figure starting path

   return FALSE;  // as end of path from either direction is reached
}

void Loop::SaveXMLDetails( FILE *fp )
{
   char buffer[300];

   sprintf( buffer, "<loop orientation=\"%s\"", ((LoopFigure *)figure)->Orientation() );
   PrintXMLText( fp, buffer );
   if( PrintManager::TextNonempty( loop_count ) )
      fprintf( fp, " exit-condition=\"%s\"", PrintDescription( loop_count ) );
   fprintf( fp, " />\n" );
}

void Loop::OutputPerformanceData( std::ofstream &pf )
{
  pf << "Loop\t\"" << identifier << "\"\n";
  pf << "loop count\t" << loop_count << std::endl;
}

bool Loop::HasTextualAnnotations()
{
   return( HasDescription() || PrintManager::TextNonempty( loop_count ) );
}

void Loop::GeneratePostScriptDescription( FILE *ps_file )
{
   bool has_description = HasDescription(),
      has_condition = PrintManager::TextNonempty( loop_count );

   if(!( has_description || has_condition )) return;

   fprintf( ps_file, "%d rom (Or Fork - ) S\n%d bol (%s ) P\n", PrintManager::text_font_size, 
	    PrintManager::text_font_size, PrintManager::PrintPostScriptText( identifier ));
   if( has_description ) GeneratePSElementDescription( ps_file );
   if( has_condition ) {
      fprintf( ps_file, "1 IN ID %d rom (Loop Count ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID\n" );
      PrintManager::PrintDescription( loop_count );
      fprintf( ps_file, "OD\n" );
   }
}


int Loop::LoopInput( Hyperedge *previous_edge )
{ // return index depending on whether previous edge is part of main path input or loop input
   return( (source->in( previous_edge->PrimaryTarget() ) == 0) ? MAIN_IN : LOOP_IN );
}

Path * Loop::LoopOutputPath()
{
   return( target->Get( 2 )->NextEdge()->GetFigure()->GetPath() );
}

int Loop::Search( search_direction dir )
{
   if( dir == FORWARD_SEARCH || dir == BIDIRECTIONAL ) {
      if( source->in( Hyperedge::SearchNode() ) == 0 ) {
	 if( target->GetFirst()->NextEdge()->Search( FORWARD_SEARCH ) == JOINED_PATHS ) // search is on main path, continue search on main output, ignore loop
	    return( JOINED_PATHS );
      }
      else
	 return( JOINED_PATHS ); // search began on loop input path, disable path deletion 
   }

   if( dir == BACKWARD_SEARCH || dir == BIDIRECTIONAL )  {
      if( source->GetFirst()->PreviousEdge()->Search( BACKWARD_SEARCH ) == JOINED_PATHS ) // search came in on main output, continue down main input
	 return( JOINED_PATHS );
   }
   return QUIT;
}
void Loop::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
