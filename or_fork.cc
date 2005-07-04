
/***********************************************************
 *
 * File:			or_fork.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:	October 1996 - Implemented
 *
 ***********************************************************/

#include "or_fork.h"
#include "or_figure.h"
#include "empty.h"
#include "result.h"
#include "map.h"
#include "hypergraph.h"
#include "print_mgr.h"
#include "msc_generator.h"
#include "component_ref.h"
#include "variable.h"
#include "utilities.h"
#include "scenario.h"
#include <iostream>
#include <stdlib.h>

#include "path_data.h"
#include "conditions.h"
#include "scenario_generator.h"

extern Presentation *ppr;
extern int showing_labels;

extern void EditOrForkSpecification( OrFork *fork );

int OrFork::number_forks = 1;

char * ExitPoint::LogicalCondition()
{
   return( (logical_condition != NULL) ? BooleanVariable::ReferenceToName( logical_condition ) : NULL );
}

void ExitPoint::LogicalCondition( const char *new_condition )
{
   logical_condition = BooleanVariable::NameToReference( new_condition, logical_condition );
}

void ExitPoint::BranchChoice( char *new_choice )
{
   if( branch_choice != NULL ) { // free old string if necessary
      free( branch_choice );
      branch_choice = NULL;
   }
   if( PrintManager::TextNonempty( new_choice ) )
      branch_choice = new_choice;
   else { // if text contains only whitespace characters
      if( new_choice != NULL )
	 free( new_choice );
   }
}

OrFork::OrFork() : MultipathEdge()
{
   figure = new OrFigure( this );
   annotations_complete = TRUE;
   previous_selection = NULL;
   identifier[0] = 0;
   identifier[19] = 0;
   generation_pass = FALSE;
}

OrFork::OrFork( int fork_id, const char *name, float x, float y, const char *desc, int orientation ) : MultipathEdge()
{
   load_number = fork_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = fork_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;  
   //End of the modification by Bo Jiang, April, 2005   
   
   figure = new OrFigure( this );
   figure->SetPosition( x, y );
   if( desc != NULL ) unique_desc = strdup( desc );
   ((OrFigure *)figure)->Orientation( orientation );
   annotations_complete = TRUE;
   strncpy( identifier, name, 19 );
   identifier[19] = 0;
   previous_selection = NULL;
   generation_pass = FALSE;
}

bool OrFork::Perform( transformation trans, execution_flag execute )
{

   switch( trans ) {
   
   case ADD_BRANCH:      
      return( AddBranch( execute ) );
      break;

   case EDIT_FORK_SPECIFICATION:
      return( EditForkSpecification( execute ) );
      break;
      
   case ROTATE:
      
      if( execute )
	 ((OrFigure *)figure)->ChangeOrientation();
      else
	 return( TRUE );
      break;

   }
   return FALSE;
}

int OrFork::PathCount()
{
  return( target->Size() );
}

void OrFork::AttachTarget( Node *new_node )
{
   Hyperedge::AttachTarget( new_node );
   
   char *selection = ((previous_selection) ? previous_selection : NULL);
   exit_points.Add( new ExitPoint( new_node, selection ) );
}

void OrFork::DetachTarget( Node *node )
{
   // remove exit point
   ExitPoint *ep;

   for( exit_points.First(); !exit_points.IsDone(); exit_points.Next() ) {
      ep = exit_points.CurrentItem();
      if( ep->ExitNode() == node ) {
	 if( ep->BranchChoice() ) {  // save characteristic for next target
	    if( previous_selection ) free( previous_selection );
	    previous_selection = ep->BranchChoice();
	 }
	 delete ep;
	 exit_points.RemoveCurrentItem();
	 break;
      }
   }   
   
   Hyperedge::DetachTarget( node );
}

void OrFork::AddBranchingCharacteristic( Hyperedge *next_edge, const char *condition, const char *characteristic, float branch_prob )
{
   ExitPoint *ep;
   Node *target_node = next_edge->SourceSet()->GetFirst();
   
   for( exit_points.First(); !exit_points.IsDone(); exit_points.Next() ) {
      if( exit_points.CurrentItem()->ExitNode() == target_node ) {
	 ep = exit_points.CurrentItem();
	 break;
      }	 
   }
   
   if( PrintManager::TextNonempty( condition ) )
      ep->LoadLogicalCondition( strdup( condition ) );
   if( PrintManager::TextNonempty( characteristic ) )
      ep->LoadBranchChoice( strdup( characteristic ) );
   ep->Probability( branch_prob );
}

void OrFork::ExtendFork()
{   
   display_manager->CreateNewSegment();
        
   Node *node1 = new Node( B );  
   Node *node2 = new Node( A );
   Empty *new_empty = new Empty;
   Result *new_result = new Result;
    
   new_empty->AttachSource( node1 );   
   new_empty->AttachTarget( node2 );
   this->AttachTarget( node1 );
   new_result->AttachSource( node2 );
   new_empty->GetFigure()->GetPath()->SetPathStart( this );	// set path start to or fork

   display_manager->AddFirstToRight( new_empty, this );
   display_manager->AddRight( new_result, new_empty );
   display_manager->Update();

   display_manager->CreateOrFork( this, new_empty );

   new_result->GetFigure()->SetSelected();
   display_manager->SetActive( new_result->GetFigure() );
}

bool OrFork::AddBranch( flag execute )
{
   if( execute ){
      display_manager->CreateNewSegment();
        
      Node *node1 = new Node( B );  
      Node *node2 = new Node( A );
      Empty *new_empty = new Empty;
      Result *new_result = new Result;
    
      new_empty->AttachSource( node1 );   
      new_empty->AttachTarget( node2 );
      this->AttachTarget( node1 );
      new_result->AttachSource( node2 );
      new_empty->GetFigure()->GetPath()->SetPathStart( this );	// set path start to or fork

      display_manager->AddOrForkBranch( this, new_empty );

      figure->ResetSelected();
      new_result->GetFigure()->SetSelected();
      display_manager->SetActive( new_result->GetFigure() );

      return( TRUE );
   }
   else
      return( TRUE );

}

void OrFork::Delete()
{
   char remaining_label[20];
   // delete or fork and merge curves of the incoming and 
   // the remaining outgoing path

   Node *front_node = source->GetFirst();
   Node *end_node = target->GetFirst();
   Hyperedge *prev_edge = front_node->PreviousEdge();
   Hyperedge *next_edge = end_node->NextEdge(); 

   // store text label of remaining outgoing path
   strcpy( remaining_label, next_edge->PathLabel()->TextLabel() );

   display_manager->SetPath( figure->GetPath() );
   display_manager->DeleteStartNull( next_edge ); // delete start null of path
   display_manager->JoinPaths( this, next_edge ); // leaving fork

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

    // reset label contents to that of the remaining label
   new_empty->PathLabel()->TextLabel( remaining_label );  
}

void OrFork::SaveXMLDetails( FILE *fp )
{
   char buffer[100];

   sprintf( buffer, "fork orientation=\"%d\" /",  ((OrFigure *)figure)->Orientation() );
   PrintXML( fp, buffer );
}


void OrFork::SaveCSMXMLDetails( FILE *fp )
{
	   char buffer[100];

   sprintf( buffer, "fork orientation=\"%d\" /",  ((OrFigure *)figure)->Orientation() );
   PrintXML( fp, buffer );
  
}


void OrFork::OutputPerformanceData( std::ofstream &pf )
{
   ExitPoint *ep;
   const char *label;
   int i = 1;

   pf << "\nOr Fork\t\"" << identifier << "\"";

   PositionOrderOutputs();

   for( target->First(); !target->IsDone(); target->Next() ) {
      ep = this->BranchSpecification( target->CurrentItem() );
      label = target->CurrentItem()->NextEdge()->PathLabel()->TextLabel();
      if( strequal( label, "" ) || strequal( label, "-" ) )
	 pf << "\n\"Branch " << i << "\"\n";
      else
	 pf << "\n\"" << label << "\"\n";
      pf << ep->Probability();
      i++;
   }

}

Hyperedge * OrFork::ScanScenarios( scan_type type )
{
   Hyperedge *current_edge;
   Node *current_node, *sole_branch = NULL;
   GenerationState gs;
   char buffer[200];
   bool path_followed = FALSE;
   MSCGenerator *msc = MSCGenerator::Instance();

   if( msc->ScenarioTracing() ) {
      for( target->First(); !target->IsDone(); target->Next() ) {
	 if( PathVariableList::EvaluateLogicalExpression( BranchSpecification( target->CurrentItem() )->ReferencedCondition() ) == TRUE ) {
	    if( sole_branch == NULL )
	       sole_branch = target->CurrentItem();
	    else { // two branches evaluated to true
	       sprintf( buffer, "Multiple logical selection conditions of the branches of the Or Fork\n\"%s\" in Map \"%s\" evaluated to true.\n",
			identifier, parent_map->MapLabel() );
	       ErrorMessage( buffer, TRUE );
	       msc->AbortGeneration();
	       return( NULL );
	    }
	 }
      }

      if( sole_branch == NULL ) {
	 sprintf( buffer, "None of the logical selection conditions of any of the branches of the Or Fork\n\"%s\" in Map \"%s\" evaluate to true.\n",
		  identifier, parent_map->MapLabel() );
	 ErrorMessage( buffer, TRUE );
	 msc->AbortGeneration();
	 return( NULL );
      }

      current_edge = sole_branch->NextEdge();
      if( traversal_outputs.Size() == 0 )
	 msc->RegisterPathElement( this );
      traversal_outputs.Add( current_edge );
      return( current_edge );
   }
   else { // recursively scan all branches
      for( int i = 1; i <= target->Size(); i++ ) {

	 current_node = target->Get( i );
	 current_edge = current_node->NextEdge();

	 if( path_followed == FALSE ) {
	    traversal_outputs.Add( current_edge );
	    path_followed = TRUE;
	 } else {
	    msc->ResetGenerationState( &gs );
	    traversal_outputs.ReplaceEnd( current_edge );
	 }

	 while( current_edge ) // scan along each branch
	    current_edge = current_edge->ScanScenarios( type );

	 if( msc->GenerationAborted() )
	    break; // stop recursive scanning
      }

      traversal_outputs.Detach(); // remove this output flag from list as all output paths have been scanned
      return( NULL ); // stop this branch of scanning as it is complete
   }
}

Hyperedge * OrFork::HighlightScenario( )
{
   Hyperedge *current_edge;
//   Node *current_node;
   Node *sole_branch = NULL;
   GenerationState gs;
   char buffer[200];
//   bool path_followed = FALSE;
   MSCGenerator *msc = MSCGenerator::Instance();

     for( target->First(); !target->IsDone(); target->Next() ) {
	 if( PathVariableList::EvaluateLogicalExpression( BranchSpecification( target->CurrentItem() )->ReferencedCondition() ) == TRUE ) {
	    if( sole_branch == NULL )
	       sole_branch = target->CurrentItem();
	    else { // two branches evaluated to true
	       sprintf( buffer, "Multiple logical selection conditions of the branches of the Or Fork\n\"%s\" in Map \"%s\" evaluated to true.\n",
			identifier, parent_map->MapLabel() );
	       ErrorMessage( buffer, TRUE );
	       msc->AbortGeneration();
	       return( NULL );
	    }
	 }
      }

      if( sole_branch == NULL ) {
	 sprintf( buffer, "None of the logical selection conditions of any of the branches of the Or Fork\n\"%s\" in Map \"%s\" evaluate to true.\n",
		  identifier, parent_map->MapLabel() );
	 ErrorMessage( buffer, TRUE );
	 msc->AbortGeneration();
	 return( NULL );
      }

      current_edge = sole_branch->NextEdge();
      if( traversal_outputs.Size() == 0 )
	 msc->RegisterPathElement( this );
      traversal_outputs.Add( current_edge );
      return( current_edge );
}


Hyperedge * OrFork::GenerateMSC( bool first_pass )
{
   Hyperedge *current_branch;
   ComponentReference *bound_component = NULL;
   MSCGenerator *msc = MSCGenerator::Instance();

   if( generation_pass != first_pass ) { 
      traversal_context = 0;
      generation_pass = first_pass;
   }

   if( strequal( identifier, "user_fork" ) )   if( strequal( identifier, "user_fork" ) )
     current_branch = NULL; // TEMPORARY

   if( (bound_component = figure->GetContainingComponent()) == NULL )
      bound_component = msc->StubComponent(); // default to enclosing component of parent stub

   msc->ComponentCrossing( bound_component, first_pass );

   current_branch = traversal_outputs.Get( ++traversal_context );

   if( !first_pass ) 
      msc->CreateCondition( current_branch->PathLabel()->TextLabel() );

   return( current_branch ); // simply follow the path of the current scan
}

void OrFork::DeleteGenerationData()
{
   while( !traversal_outputs.is_empty() )
      traversal_outputs.Detach();
}

bool OrFork::HasTextualAnnotations()
{
   return( HasDescription() || PrintManager::include_performance );
}

void OrFork::PositionOrderOutputs()
{
   // order output nodes in graphical order, top to bottom for horizontal orientations
   // and left to right for vertical orientations
   Node *outputs[20], *ordered_outputs[20];
   int num_outputs, i = 0, j, k = 0, min_index;
   float x, y, min_coord, *oc, prev_oc = 0;
   figure_orientation orientation;
   bool position_ordered = TRUE;

   orientation = ((OrFigure *)figure)->Orientation();
   if( (orientation == FORWARD_FIG) || (orientation == BACKWARD_FIG) ) // determine ordering coordinate
      oc = &y; // for horizontal forks order on y positions
   else
      oc = &x; // for vertical forks order on x positions

   // determine if reordering on position is necessary
   for( target->First(); !target->IsDone(); target->Next() ) {
      target->CurrentItem()->NextEdge()->GetFigure()->GetPosition( x, y );
      if( prev_oc > *oc ) {
	 position_ordered = FALSE;
	 break;
      }
      prev_oc = *oc;
   }

   if( position_ordered ) return;

   for( target->First(); !target->IsDone(); target->Next() )
      outputs[i++] = target->CurrentItem();

   num_outputs = i;

   for( i = 0; i < num_outputs; i++ ) {
      min_coord = 10.0;
      for( j = 0; j < num_outputs; j++ ) {
	 if( outputs[j] ) {
	    outputs[j]->NextEdge()->GetFigure()->GetPosition( x, y );
	    if( *oc < min_coord ) {
	       min_index = j;
	       min_coord = *oc;
	    }
	 }
      }
      ordered_outputs[k++] = outputs[min_index];
      outputs[min_index] = NULL; // remove the branch from list
   }

   k = 0;
   for( target->First(); !target->IsDone(); target->Next() )
      target->SetCurrentItem( ordered_outputs[k++] );
}

void OrFork::GeneratePostScriptDescription( FILE *ps_file )
{
   ExitPoint *ep;
   char *logical_condition;
   int i = 1;
   bool has_description = HasDescription();
   
   if( !( has_description || PrintManager::include_performance || PrintManager::include_scenarios ) ) return;

   fprintf( ps_file, "%d rom (Or Fork - ) S\n%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size,
	    PrintManager::PrintPostScriptText( identifier ));
   if( has_description ) GeneratePSElementDescription( ps_file );

   if( !(PrintManager::include_performance || PrintManager::include_scenarios) ) return;   
   
   PositionOrderOutputs();

   fprintf( ps_file, "%d rom\n", PrintManager::text_font_size );

   for( target->First(); !target->IsDone(); target->Next() ) {
      ep = this->BranchSpecification( target->CurrentItem() );
      fprintf( ps_file, "(Output Path %d ) P\n", i++ );
      if( (logical_condition = ep->LogicalCondition()) != NULL ) {
	 fprintf( ps_file, "1 IN ID (Selection Condition ) P OD\n2 IN ID\n" );
	 fprintf( ps_file, "(%s ) P OD\n", PrintManager::PrintPostScriptText( logical_condition ) );
	 free( logical_condition );
      } 
      if( PrintManager::TextNonempty( ep->BranchChoice() ) ) {
	 fprintf( ps_file, "1 IN ID (Branch Selection ) P OD\n2 IN ID\n" );
	 fprintf( ps_file, "(%s ) P OD\n", PrintManager::PrintPostScriptText( ep->BranchChoice() ) );
      }
      if( ep->Probability() != 0.0 ) {
	 fprintf( ps_file, "1 IN ID (Probability ) P OD\n2 IN ID\n" );
	 fprintf( ps_file, "(%f ) P L OD\n", ep->Probability() );
      }
   }
}

ExitPoint * OrFork::BranchSpecification( Node *output_node )
{
   for( exit_points.First(); !exit_points.IsDone(); exit_points.Next() ) {
      if( exit_points.CurrentItem()->ExitNode() == output_node )
	 return( exit_points.CurrentItem() );
   }
   return 0;
}

void OrFork::VerifyAnnotations()
{
   ExitPoint *ep;

   annotations_complete = TRUE;

   if( strequal( identifier, "" ) ) {
      annotations_complete = FALSE;
      parent_map->HighlightParentStubs();
      return;
   }

   for( exit_points.First(); !exit_points.IsDone(); exit_points.Next() ) {
      ep = exit_points.CurrentItem();
      if( ep->BranchChoice() == NULL && ep->Probability() == 0 ) {
	 annotations_complete = FALSE;
	 break;
      }
   }

   if( annotations_complete == FALSE )
      parent_map->HighlightParentStubs();
}

void OrFork::SavePathBranchingSpec( FILE *fp )
{
   ExitPoint *ep;
   bool characteristics_exist = FALSE;
   char buffer[2000];
   
   // save branch selection characteristics if they are defined

   for( exit_points.First(); !exit_points.IsDone(); exit_points.Next() ) {
      ep = exit_points.CurrentItem();
      if( ep->Probability() != 0 ) {
	 characteristics_exist = TRUE;
	 break;
      }
      else if( ep->BranchChoice() != NULL ) {
	 characteristics_exist = TRUE;
	 break;
      }
      else if( ep->ReferencedCondition() != NULL ) {
	 characteristics_exist = TRUE;
	 break;
      }
   }
   
   if( !characteristics_exist )
      return;

   sprintf( buffer, "path-branching-spec fork-id=\"h%d\" ", hyperedge_number );
   PrintNewXMLBlock( fp, buffer );

   for( exit_points.First(); !exit_points.IsDone(); exit_points.Next() ) {
      ep = exit_points.CurrentItem();
      sprintf( buffer, "<path-branching-characteristic characteristic=\"%s\" empty-segment=\"h%d\" probability=\"%f\"" ,
	       (ep->BranchChoice() ? PrintDescription( ep->BranchChoice() ) : "" ), ep->ExitNode()->NextEdge()->GetNumber(), ep->Probability() );
      PrintXMLText( fp, buffer );
      if( ep->ReferencedCondition() )
	 fprintf( fp, " branch-condition=\"%s\"", PrintDescription( ep->ReferencedCondition() ) );
      fprintf( fp, " />\n" );      
   }

   PrintEndXMLBlock( fp, "path-branching-spec" );
}

bool OrFork::EditForkSpecification( execution_flag execute )
{
   if( execute ){
      EditOrForkSpecification( this );
      return( UNMODIFIED );
   }
   else
      return( TRUE );
}

bool OrFork::ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir )
{
   if( figure->GetPath() == old_path ) // forward search or backward search begining at fork itself
      figure->SetPath( new_path ); // set path for or fork figure itself
   else // search is backward from output path into fork
      new_path->GetStartFigure()->SetPath( new_path ); // set path in or null figure starting path

   return FALSE;  // as end of path from either direction is reached
}

Hyperedge * OrFork::TraverseScenario()
{
   Hyperedge *current_edge;
//   Node *current_node;
   Node *sole_branch = NULL;
   GenerationState gs;
   char buffer[200];
//   bool path_followed = FALSE;
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   char sole_exp[LABEL_SIZE*2+1];
   sole_exp[LABEL_SIZE*2] = 0;
   const char * condition_label = NULL;
   
   for( target->First(); !target->IsDone(); target->Next() ) {
     if( PathVariableList::EvaluateLogicalExpression( BranchSpecification( target->CurrentItem() )->ReferencedCondition() ) == TRUE ) {
	 strncpy( sole_exp, BranchSpecification( target->CurrentItem() )->ReferencedCondition(), LABEL_SIZE*2 );
	 if( sole_branch == NULL )
	    sole_branch = target->CurrentItem();
	 else { // two branches evaluated to true
	    sprintf( buffer, "Multiple logical selection conditions of the branches of the Or Fork\n\"%s\" in Map \"%s\" evaluated to true.\n",
		identifier, parent_map->MapLabel() );
	    ErrorMessage( buffer, TRUE );
	    sce->AbortGeneration();
	    return( NULL );
	 }
      }
   }
   
   if( sole_branch == NULL ) {
      sprintf( buffer, "None of the logical selection conditions of any of the branches of the Or Fork\n\"%s\" in Map \"%s\" evaluate to true.\n",
		  identifier, parent_map->MapLabel() );
      ErrorMessage( buffer, TRUE );
      sce->AbortGeneration();
      return( NULL );
   }
   current_edge = sole_branch->NextEdge();
   
   condition_label = ((Empty *) current_edge)->PathLabel()->TextLabel();
   Path_data * selection = new Conditions( hyperedge_number, condition_label, sole_exp );
   sce->AddPathTraversal(selection);
   sce->AddPathList(selection);
   return( current_edge );
  
}
void OrFork::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
