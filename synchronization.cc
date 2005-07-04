/***********************************************************
 *
 * File:			synchronization.cc
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:        Feb 2003
 *                              Shuhua Cui
 *
 ***********************************************************/

#include "synchronization.h"
#include "synch_figure.h"

#include "empty.h"
#include "result.h"
#include "hypergraph.h"
#include "path.h"
#include "msc_generator.h"
#include "component_ref.h"
#include "scenario.h"
#include "start.h"
#include "utilities.h"
#include "scenario_generator.h"
#include "path_data.h"
#include "action.h"

Synchronization *Synchronization::previous_join;

Synchronization::Synchronization( synch_type type ) : MultipathEdge()
{
   stype = type;
   figure = new SynchronizationFigure( this );
   scenarioGenerated = FALSE;
}

Synchronization::Synchronization( int synch_id, float x, float y, const char *desc, int orientation ) : MultipathEdge()
{
   load_number = synch_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file
   hyperedge_number = synch_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;    
   //End of the modification by Bo Jiang, April, 2005

   figure = new SynchronizationFigure( this );
   figure->SetPosition( x, y );
   if( desc != NULL ) unique_desc = strdup( desc );
   ((SynchronizationFigure *)figure)->Orientation( orientation );
    scenarioGenerated = FALSE;
}

bool Synchronization::Perform( transformation trans, execution_flag execute )
{
   switch( trans ) {

   case ADD_BRANCH:
      return( AddBranch( execute ) );
      break;

   case ROTATE:
      
      if( execute )
	 ((SynchronizationFigure *)figure)->ChangeOrientation();
      else
	 return( TRUE );
      break;
   }
   return FALSE;
}

bool Synchronization::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute )
{

   switch( trans ) {
   
   case ADD_INCOMING_SYNCHRONIZATION:
      return( AddIncomingSynch( edge, execute ) );
      break;

   }
   return FALSE;
}

void Synchronization::AttachSource( Node *new_node )
{
   Hyperedge::AttachSource( new_node );
   this->RecheckSynchronizationType();
}
 
void Synchronization::DetachSource( Node *node )
{
   Hyperedge::DetachSource( node );
   this->RecheckSynchronizationType();
}

void Synchronization::AttachTarget( Node *new_node )
{
   Hyperedge::AttachTarget( new_node );
   this->RecheckSynchronizationType();
}

void Synchronization::DetachTarget( Node *node )
{
   Hyperedge::DetachTarget( node );
   this->RecheckSynchronizationType();
}

void Synchronization::RecheckSynchronizationType()
{
   if( source->Size() == 1 )
      stype = FORK;
   else if( target->Size() == 1 )
      stype = JOIN;
   else
      stype = MULTIPATH;
}

int Synchronization::PathCount()
{

   int inputs = source->Size();
   int outputs = target->Size();
   int count = Max( inputs, outputs );
   return( count );

}

bool Synchronization::AddIncomingSynch( Hyperedge *edge, flag execute )
{
   if( execute ){
      Node *node = new Node( A );
      Node *front_node = edge->SourceSet()->GetFirst();
      Empty *prev_edge = (Empty *)(front_node->PreviousEdge());
  
      AttachSource( node );
      prev_edge->DetachTarget( front_node );
      prev_edge->AttachTarget( node );
  
      prev_edge->GetFigure()->GetPath()->SetPathEnd( this );
      display_manager->AddAndJoinBranch( this, prev_edge );

      parent_map->MapHypergraph()->PurgeNode( front_node );
      parent_map->MapHypergraph()->PurgeEdge( edge );
  
      prev_edge->GetFigure()->GetPath()->PathChanged();
      return( TRUE );  // not used
      
   }
   else {  // validate transformation

      if( edge->EdgeType() != RESULT )
	 return( INVALID );
    
      if( source->Size() > 1 ) // disable as AndJoin transformation should be used
	 return( INVALID );
      
      return( AVAILABLE );    
   }
}

void Synchronization::ConnectForkPath( Hyperedge *edge )
{
   Node *node = new Node( A );
   Node *front_node = edge->SourceSet()->GetFirst();
   Empty *prev_edge = (Empty *)(front_node->PreviousEdge());
  
   AttachSource( node );
   prev_edge->DetachTarget( front_node );
   prev_edge->AttachTarget( node );
  
   prev_edge->GetFigure()->GetPath()->SetPathEnd( this );

   display_manager->CreateAndJoin( this, prev_edge );
  
   parent_map->MapHypergraph()->PurgeNode( front_node );
   parent_map->MapHypergraph()->PurgeEdge( edge );
  
   prev_edge->GetFigure()->GetPath()->PathChanged();
}

void Synchronization::ExtendFork()
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
   new_empty->GetFigure()->GetPath()->SetPathStart( this );	// set path start to and fork
    
   display_manager->AddFirstToRight( new_empty, this );
   display_manager->AddRight( new_result, new_empty );
  
   display_manager->CreateAndFork( this, new_empty );

   new_result->GetFigure()->SetSelected();
   display_manager->SetActive( new_result->GetFigure() );
}

bool Synchronization::AddBranch( flag execute )
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
      new_empty->GetFigure()->GetPath()->SetPathStart( this );	// set path start to and fork
    
      display_manager->AddAndForkBranch( this, new_empty, new_result );
  
      figure->ResetSelected();
      new_result->GetFigure()->SetSelected();
      display_manager->SetActive( new_result->GetFigure() );
      return( TRUE );
   }
   else
      return( TRUE );
}

void Synchronization::Delete()
{
   // delete and fork and merge curves of the incoming
   // and the remaining outgoing path
   Node *front_node = source->GetFirst();
   Node *end_node = target->GetFirst();
   Hyperedge *prev_edge = front_node->PreviousEdge();
   Hyperedge *next_edge = end_node->NextEdge();
   char remaining_label[20];

   // store text label of remaining incoming or outgoing path
   if( stype == FORK )
      strcpy( remaining_label, next_edge->PathLabel()->TextLabel() );
   else
      strcpy( remaining_label, prev_edge->PathLabel()->TextLabel() );
   
   display_manager->SetPath( figure->GetPath() );
   if( stype == FORK ) {
      display_manager->DeleteStartNull( next_edge ); // delete start null of remaining path leaving fork 
      display_manager->DeleteEndNull( prev_edge );   // delete end null of path entering fork
      display_manager->JoinPaths( prev_edge, next_edge ); 
   }
   else {
      display_manager->DeleteEndNull( prev_edge );   // delete end null of remaining path entering join
      display_manager->DeleteStartNull( next_edge ); // delete start null of path leaving join
      display_manager->JoinPaths( prev_edge, next_edge ); 
   }      

   current_label = this->PathLabel();  // reset label so empty is created with label of incoming path
   Empty *new_empty = new Empty;
   Node *node1 = new Node( B );
   Node *node2 = new Node( A );
   new_empty->AttachSource( node1 );
   new_empty->AttachTarget( node2 );

   next_edge->ReplaceSource( end_node, node2 );
   prev_edge->ReplaceTarget( front_node, node1 );
     
   display_manager->AddAtPosition( new_empty, this, next_edge );

   parent_map->MapHypergraph()->PurgeSegmentFrom( front_node, end_node );
   display_manager->SetActive( NULL );

   // reset label contents to that of the remaining label
   new_empty->PathLabel()->TextLabel( remaining_label );   
}

Hyperedge * Synchronization::ScanScenarios( scan_type type )
{
   MSCGenerator *msc = MSCGenerator::Instance();
   Synchronization *previous_fork = NULL;
   int current_branch, branch_count;
   char buffer[200];
   Hyperedge * current_edge = NULL;
//   Node * current_node = NULL;
   // assume nested and forks and scan each successive branch when the corresponding join is reached
   // the recursive nature of the scanning will cause all path combinations to be explored
   if( source->Size() > 1 ) { // synchronization is an and join or multipath synchronization
      if( !allVisited()) {
          msc->CurrentParallelBranch( &previous_fork, current_branch );
          if(previous_fork != NULL) {
              branch_count = previous_fork->TargetSet()->Size();
              if( current_branch < branch_count ) { // more branches of the previous fork need to be scanned
	          msc->IncrementParallelBranch();
		  current_edge = previous_fork->TargetSet()->Get( current_branch+1 )->NextEdge();
		  if( current_branch == (branch_count - 1) )
		      msc->EndParallelTraversal();
	          return current_edge; 
              }
          }     
          else {
             int n = msc->getScenarioStartIndex();
             if( n < msc->CurrentScenario()->StartingPoints()->Size()) {
                 msc->IncrementScenarioStartIndex();
                 return( msc->CurrentScenario()->StartingPoints()->Get( ++n ) );
             }
	     else { 
	         sprintf( buffer, "Both paths did not synchronize at the and join in Map \"%s\".",
		          parent_map->MapLabel() );
	         ErrorMessage( buffer, TRUE );
	         msc->AbortGeneration();
	         return( NULL );
	     }
          }
       }
       else {
          //input_branch_index = 0;
          if( target->Size() > 1 ) // synchronization is multipath
	      msc->AddParallelToStack( this ); // push this fork onto stack for next parallel scanning
	  ResetAllVisited();
          return( target->GetFirst()->NextEdge() ); // start scan down first output branch
       }

   }
   else { // synchronization is simply an and fork
      msc->AddParallelToStack( this ); // push this fork onto stack
      return( target->GetFirst()->NextEdge() ); // start scan down first output branch
   }
   return 0;
}

Hyperedge * Synchronization::HighlightScenario( )
{
   MSCGenerator *msc = MSCGenerator::Instance();
   Synchronization *previous_fork = NULL;
   int current_branch, branch_count;
   char buffer[200];
   Hyperedge * current_edge = NULL;
//   Node * current_node = NULL;
   // assume nested and forks and scan each successive branch when the corresponding join is reached
   // the recursive nature of the scanning will cause all path combinations to be explored
   if( source->Size() > 1 ) { // synchronization is an and join or multipath synchronization
      if( !allVisited()) {
          msc->CurrentParallelBranch( &previous_fork, current_branch );
          if(previous_fork != NULL) {
              branch_count = previous_fork->TargetSet()->Size();
              if( current_branch < branch_count ) { // more branches of the previous fork need to be scanned
	          msc->IncrementParallelBranch();
		  current_edge = previous_fork->TargetSet()->Get( current_branch+1 )->NextEdge();
		  if( current_branch == (branch_count - 1) )
		      msc->EndParallelTraversal();
	          return current_edge; 
              }
          }     
          else {
             int n = msc->getScenarioStartIndex();
             if( n < msc->CurrentScenario()->StartingPoints()->Size()) {
                 msc->IncrementScenarioStartIndex();
                 return( msc->CurrentScenario()->StartingPoints()->Get( ++n ) );
             }
	     else { 
	         sprintf( buffer, "Both paths did not synchronize at the and join in Map \"%s\".",
		          parent_map->MapLabel() );
	         ErrorMessage( buffer, TRUE );
	         msc->AbortGeneration();
	         return( NULL );
	     }
          }
       }
       else {
          //input_branch_index = 0;
          if( target->Size() > 1 ) // synchronization is multipath
	      msc->AddParallelToStack( this ); // push this fork onto stack for next parallel scanning
	  ResetAllVisited();
          return( target->GetFirst()->NextEdge() ); // start scan down first output branch
       }

   }
   else { // synchronization is simply an and fork
      msc->AddParallelToStack( this ); // push this fork onto stack
      return( target->GetFirst()->NextEdge() ); // start scan down first output branch
   }
   return 0;
}


Hyperedge * Synchronization::GenerateMSC( bool first_pass )
{
   Hyperedge *current_edge, *join = NULL;
   MSCGenerator *msc = MSCGenerator::Instance();
   ComponentReference *pc, *bound_component;
   int decomposition_level;
   bool continue_loop, first_branch = TRUE;
   Stub **decomposition_stack;

   if( (bound_component = figure->GetContainingComponent()) == NULL )
      bound_component = msc->StubComponent(); // default to enclosing component of parent stub

   msc->ComponentCrossing( bound_component, first_pass );

   decomposition_level = msc->CurrentDecompositionStack( &decomposition_stack );

   if( target->Size() > 1 ) { // synchronization is a multi-output and fork or multi-path synchronization
      // output multiple msc streams using the par construct
      if( !first_pass ) msc->StartInlineConstruct( PARALLEL );
      pc = msc->PreviousComponent();

      for( target->GetFirst(); !target->IsDone(); target->Next() ) {
	 current_edge = target->CurrentItem()->NextEdge();
	 msc->PreviousComponent( pc ); // reset the previous component
	 continue_loop = TRUE;
	 if( !first_branch )   // at the start of each branch, reset stack, output par construct separator during generation pass
	    msc->NewInlineBranch( PARALLEL, decomposition_stack, decomposition_level, first_pass );
	 do {
	    current_edge = current_edge->GenerateMSC( first_pass );
	    if( current_edge ) {
	       if( current_edge->IsAndJoin() ) {
		  if( current_edge != previous_join ) { // avoid stopping loop for the same join twice in a row
		     join = current_edge;
		     continue_loop = FALSE;                // thereby requiring new MSC input message
		  }
	       }
	       else if( msc->WaitingSynchronization() )
		  continue_loop = FALSE;
	    }
	 } while( current_edge && continue_loop );
	 first_branch = FALSE;
      }

      if( !first_pass ) msc->EndInlineConstruct( PARALLEL );
      msc->StartNewPath();
      if( join != NULL ) previous_join = (Synchronization *)join;
      if( decomposition_level > 0 ) free( decomposition_stack );
      if( msc->WaitingSynchronization() ) {
	 msc->SetWaitingSynchronization( FALSE );
	 return( current_edge ); // return output path of synchronized timer or wait point
      } else
	 return( join ); // return the corresponding join to this fork
   }
   else // synchronization is a single output and join
      return( target->GetFirst()->NextEdge() );
}

bool Synchronization::ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir )
{
   if( sdir == FORWARD_SEARCH )
      new_path->GetEndFigure()->SetPath( new_path ); // set path in synch null figure ending path
   else // BACKWARD_SEARCH
      new_path->GetStartFigure()->SetPath( new_path ); // set path in synch null figure starting path

   return FALSE;  // as end of path from either direction is reached
}

void Synchronization::SaveXMLDetails( FILE *fp )
{
   char buffer[300];

   sprintf( buffer, "synchronization cardinality-source=\"%d\" cardinality-target=\"%d\" orientation=\"%d\"", source->Size(), target->Size(),
	    ((SynchronizationFigure *)figure)->Orientation() );
   PrintNewXMLBlock( fp, buffer );   
   PrintEndXMLBlock( fp, "synchronization" );
}


void Synchronization::SaveCSMXMLDetails( FILE *fp )
{
	
   char buffer[300];

   sprintf( buffer, "synchronization cardinality-source=\"%d\" cardinality-target=\"%d\" orientation=\"%d\"", source->Size(), target->Size(),
	    ((SynchronizationFigure *)figure)->Orientation() );
   PrintNewXMLBlock( fp, buffer );   
   PrintEndXMLBlock( fp, "synchronization" );
   
}


bool Synchronization::allVisited()
{
   for( source->GetFirst(); !source->IsDone(); source->Next() ) {
       if( source->CurrentItem()->getVisited() != TRUE ) {
           return FALSE;
       }
   }
   return TRUE;
}

void Synchronization::ResetAllVisited()
{
   Node * current_node = NULL;
   for( source->First(); !source->IsDone(); source->Next() ) {
       current_node = source->CurrentItem();
       current_node->setVisited( FALSE ); 
   }
}

Hyperedge * Synchronization::TraverseScenario( )
{
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Synchronization *previous_fork = NULL;
   int current_branch, branch_count;
   char buffer[200];
   bool incompleteSearch = true;
   Hyperedge * current_edge = NULL;
   Path_data * current_path_data = sce->getCurrentPathData( );
   ComponentReference * component = NULL;
   
   if( scenarioGenerated == FALSE) {  // It is the first time to traverse the synchronization point,
                                // ensure only 1 unique Action item corresponding to each synchronization point
      waiting_place = new Action( hyperedge_number, "waiting_place", Waiting_place );      
      if( (component = figure->GetEnclosingComponent() ) != NULL ) {
         waiting_place->SetComponentName( component->ReferencedComponent()->GetLabel() );
	 waiting_place->SetComponentId( component->ReferencedComponent()->GetComponentNumber() );
	 waiting_place->SetComponentRole( component->ComponentRole() );
	 waiting_place->SetComponentReferenceId( component->ComponentReferenceNumber() );	
      }
      sce->AddPathList(waiting_place);
      scenarioGenerated = TRUE;
   }
   // assume nested and forks and scan each successive branch when the corresponding join is reached
   // the recursive nature of the scanning will cause all path combinations to be explored   
   if( source->Size() > 1 ) { // synchronization is an and join or multipath synchronization  
      if( !allVisited()) {    // Remainning branches of the synchronization need to be traversed first.
          sce->AddPathTraversal(waiting_place);
	  sce->RegisterSynchElements(this);
	  RegisterWaitingElement( current_path_data );
          sce->CurrentParallelBranch( &previous_fork, current_branch );
          if(previous_fork != NULL) {
              branch_count = previous_fork->TargetSet()->Size();
              if( current_branch < branch_count ) { // more branches of the previous fork need to be scanned
	          sce->IncrementParallelBranch();
		  current_edge = previous_fork->TargetSet()->Get( current_branch+1 )->NextEdge();
		  if( current_branch == (branch_count - 1) )  //All branches of previous fork have been scaned, remove the fork from stack
		      sce->EndParallelTraversal();
		  return current_edge; 
              }
          }     
          else { //if no more fork on parallel stack, then search the next starting point
             int startIndex = sce->getScenarioStartIndex();
	     if( startIndex < sce->CurrentScenario()->StartingPoints()->Size()) {
                 sce->IncrementScenarioStartIndex();
                 return( sce->CurrentScenario()->StartingPoints()->Get( ++startIndex) );
             }
	     else { 
	         sprintf( buffer, "Both paths did not synchronize at the and join in Map \"%s\".",
		          parent_map->MapLabel() );
	         ErrorMessage( buffer, TRUE );
	         sce->AbortGeneration();
	         return( NULL );
	     }
         }
      }
      else { // All previous path of the synchronization have been traversed, collapse the graph,
             // then traverse output branch
          if( target->Size() > 1 ) // synchronization is multipath
	      sce->AddParallelToStack( this,  waiting_place ); // push this fork onto stack for next parallel scanning
           
	   RegisterWaitingElement( current_path_data );
	   sce->AddPathTraversal(waiting_place);
	   while( incompleteSearch == true ) {
	      incompleteSearch = SearchRoot( waiting_element, waiting_place );
	   }	   
	   ResetTraverseState( );	   	   	   
	   return( target->GetFirst()->NextEdge() ); // start scan down first output branch
       }

   }
   else { // synchronization is simply an and fork
      sce->AddParallelToStack( this, sce->getCurrentPathData() ); // push this fork onto stack, and remember the parent of fork
      scenarioGenerated = FALSE; 
      return( target->GetFirst()->NextEdge() ); // start scan down first output branch
   }
   return 0;
}

// This method sets the state of the synchronization point to its original state, so that it has no effect
// on next traversal 
void Synchronization::ResetTraverseState( )
{
   ResetAllVisited();
   scenarioGenerated = FALSE;	   
   waiting_element.Empty();
}

void Synchronization:: RegisterWaitingElement(Path_data * current_path_data ) 
{
   waiting_element.Add( current_path_data );  
}


/*
bool Synchronization::SearchRoot( ) 
{
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Path_data *  current_element;
   Path_data * new_root = NULL;
   Path_data * common_item = NULL;
   Path_data * des = NULL;
   int scanCount;
   int MaxCount = 100;
   bool incompleteSearch = false;
   
   //backtrack along each branch of the synchronization point, increment each elements's scanCount by 1
   //the root element must have been scanned multiple times 
   for( waiting_element.First(); !waiting_element.IsDone(); waiting_element.Next() ) {
      current_element = waiting_element.CurrentItem();
      while( current_element != NULL ) {
         current_element->IncrementScanCount();
	 current_element = Advance( current_element );
      }
   }
   
   // find the first common element with the least number of scanCount, due to the unwell_nestedness of maps,
   // the scanCount of the root may be less than the length of waiting_elements. 
   for( waiting_element.First(); !waiting_element.IsDone(); waiting_element.Next() ) {
      current_element = waiting_element.CurrentItem();
      while( current_element != NULL ) {
         scanCount = current_element->GetScanCount();
	 if( scanCount > 1 && scanCount < MaxCount ) {
	     MaxCount = scanCount;
	     common_item = current_element;
	 }
	 current_element = Advance( current_element );          	  
      }
   }
   
   if( MaxCount == 100 ) { // No common elements exist, the parallel block starts from scenario starting points
      waiting_place->SetRoot( new_root );
      des = waiting_place;
   }
   else {
      current_element = FindDataWithCount( MaxCount );
      if( MaxCount < waiting_element.Size() ) { 
         // unwell_nested map, partially collapse the branches to form a inner parallel block, 
	 // add one additional Action as the ending point of the inner parallel block 
	 des = AddSynchronizationPoint( current_element );
	 incompleteSearch = true; 
      }
      else { //well_nested map
         des = waiting_place;
	 waiting_place->SetRoot( current_element );
      }
   }
   
   ResetReferenceCount();
   Collapse( des, incompleteSearch ); 
   return  incompleteSearch;   
}
*/

// This method scans down each branch of the synchronization point, sets the destination attribute of each 
// path_data element to be the parameter des, so all elements with the same destination are in the same
// parallel block
void Synchronization::SetDestination( Path_data * des ) 
{
   Path_data * current_element = NULL;
   Cltn<Path_data *> * previous_path;
      
   previous_path = des->PreviousPath(); 		   
   for( previous_path->First(); !previous_path->IsDone(); previous_path->Next() ) {
      current_element = previous_path->CurrentItem();
      while( current_element != des->GetRoot() ) {
         current_element->RegisterDestination( des );
	 current_element = Advance( current_element );
      }
   }   
}

void Synchronization::Collapse( Path_data * des, bool incompleteSearch ) 
{
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Path_data * current_element = NULL;
   bool isRoot = false;
   Path_data * new_root = NULL;		   
   Cltn<Path_data *> * previous_path;
   Path_data * root = NULL;
      
   previous_path = des->PreviousPath(); 
   SetDestination( des );
   
   // If the parallel block starts from scenario starting points, add 1 additional Action element as the start of
   // the parallel, add this new_root to list pathtraversal 
   if(des->GetRoot() == NULL ) {  
      new_root = new Action(0, "Addition", Fork);      
      sce->ReplacePathTraversal(new_root, des);
      sce->AddPathList( new_root );
   }
   root = des->GetRoot();
   
   // If the parallel is only a partial collapse of a larger parallel block, move root element's next path elements 
   // which are not not synchronized to destination des to the waiting place
   if( incompleteSearch == true ) {     
      MoveToWaitingPlace( root, des );
   }
   
   // scan down each branch of the synchronization's previous path, if the current element is not the root of a 
   // parallel, shift all its next path elements whose destination is not des to waiting_place.  
   for( previous_path->First(); !previous_path->IsDone(); previous_path->Next() ) {
      current_element = previous_path->CurrentItem();
      while( current_element != des->GetRoot() ) {
         sce->UpdateParallelStack( current_element, waiting_place );
         if( !isRoot ) {
	    MoveToWaitingPlace( current_element, des );           
	 }	
     	 if( current_element->PreviousPath() != NULL ) {
	    if( current_element->PreviousPath()->Size() <= 1 ) {     
               current_element = current_element->Previous();
	       isRoot = false;
            }
            else {
               current_element = current_element->GetRoot(); 
	       isRoot = true; 
            } 
	 }
	 else {
	    break;
	 }
      }
   } 
   AddForkAfter( root, des );
}


// Add an additional root element before the parallel block, so that future operation of the root element will
// treat the parallel as an entity
void Synchronization::AddForkAfter( Path_data * root, Path_data * des )
{
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Cltn<Path_data *> deleteList;
   Path_data * current_element = NULL;
   Cltn<Path_data *> * rootNextList = root->NextPath();
   Path_data * new_root = new Action(0, "Addition", Fork);
   sce->AddPathList( new_root );
   
   for( rootNextList->First(); !rootNextList->IsDone(); rootNextList->Next() ) {
      current_element = rootNextList->CurrentItem();
      if( current_element->GetDestination() == des ) {
         deleteList.Add(current_element);
      }
   }

   for( deleteList.First(); !deleteList.IsDone(); deleteList.Next() ) {
      current_element = deleteList.CurrentItem();
      if( rootNextList->Includes( current_element ) ) {
         root->RemoveFromNext(current_element);
         new_root->AddNext(current_element);
      }
   }
   root->AddNext( new_root );
   des->SetRoot( new_root );
}

void Synchronization::MoveToWaitingPlace( Path_data * root, Path_data * des )
{
   Cltn<Path_data *> * rootNextList = root->NextPath();
   Cltn<Path_data *> deleteList;
   Path_data * current_element = NULL;
   
   if( rootNextList->Includes( des ) ) {
      for( rootNextList->First(); !rootNextList->IsDone(); rootNextList->Next() ) {
         current_element = rootNextList->CurrentItem();
         if( current_element != des ) {
            deleteList.Add(current_element);
         }
      }
   }
   else {
      for( rootNextList->First(); !rootNextList->IsDone(); rootNextList->Next() ) {
         current_element = rootNextList->CurrentItem();
         if( current_element->GetDestination() != des ) {
            deleteList.Add(current_element);
         }
      }
   }
   
   for( deleteList.First(); !deleteList.IsDone(); deleteList.Next() ) {
      current_element = deleteList.CurrentItem();
      if( rootNextList->Includes( current_element ) ) {
         root->RemoveFromNext(current_element);
         waiting_place->AddNext(current_element);
      }
   }
}

void  Synchronization::ResetScenarioGenerated(bool value)
{
   scenarioGenerated = value;
}


Path_data * Synchronization::FindDataWithCount( int count )
{
   int referenceCount;
   Path_data * current_element = NULL;
   
   for( waiting_element.First(); !waiting_element.IsDone(); waiting_element.Next() ) {
      current_element = waiting_element.CurrentItem();
      while( current_element != NULL ) {
         referenceCount = current_element->GetScanCount();
         if( referenceCount == count ) {
	    return current_element;	   
	 }
	 current_element = Advance( current_element );
      }
   }
   return NULL;	
}  


// In unwell_nested situations, there may be the case that multiple paths synchronized at a synchronization
// point, but only some of the pathes are from one fork, then we need to group the pathes from the and fork 
// first. we can do this by appending a new Path_data item after these pathes, and set its next pointer pointing
// to the synchronization element. In this method, parameter root represents the fork element, if one of the 
// waiting_element leads to the root, then we add it to deleteList, and split its connection with waiting_place.
// Finally, we delete each element of the deleteList from the waiting_element, and add the new element to 
// waiting_element.
Path_data *  Synchronization::AddSynchronizationPoint( Path_data * root )
{
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Path_data * temp = NULL;
   Path_data * current_element = NULL;
   Cltn<Path_data *> deleteList;

   Path_data * des = new Action( 0, "waiting_place", Waiting_place);
   sce->AddPathList( des );
         
   for( waiting_element.First(); !waiting_element.IsDone(); waiting_element.Next() ) {
      current_element = waiting_element.CurrentItem();
      temp = current_element;
      while( temp != NULL ) {
         if( temp == root ) {
	    deleteList.Add( current_element );
	    current_element->RemoveFromNext( waiting_place );
	    current_element->AddNext( des );
	    break;	   
	 }
	 temp = Advance( temp );
      }
   }
   
   for( deleteList.First(); !deleteList.IsDone(); deleteList.Next() ) {
      current_element = deleteList.CurrentItem();
      if( waiting_element.Includes( current_element ) ) {
         waiting_element.Remove( current_element );
      }
   }
   des->AddNext( waiting_place );
   RegisterWaitingElement( des );
   des->SetRoot( root );
     
   return des;
}

// This method sets the scanCount of each elements along the parallel branches to its original value 0.
void Synchronization::ResetReferenceCount( )
{
   Path_data * current_element;
   
   for( waiting_element.First(); !waiting_element.IsDone(); waiting_element.Next() ) {
      current_element = waiting_element.CurrentItem();
      while( current_element != NULL ) {
         current_element->ResetScanCount();
	 current_element = Advance( current_element );
      }
   }
}

// This method makes the current_data go backward one step, if current_data's previous pointer point to multiple
// path_data elements, then according to the nature that collapse is from inside out, the previous pointer is 
// pointing to a parallel block, so it skips over to the root of the parallel block; if the pointer has only one 
// element, it simply returns that element,; otherwise it return NULL 
Path_data * Synchronization::Advance(Path_data * current_data )
{
   Path_data * next_data = NULL;
   
   if( current_data->PreviousPath() != NULL ) {
      if( current_data->PreviousPath()->Size() == 1 ) {
	 next_data = current_data->Previous();
      }
      else {
         if( current_data->PreviousPath()->Size() > 1 ) {
            next_data = current_data->GetRoot();
	 }
	 else {
	    next_data = NULL;
	 }
      }
   }  
   return next_data;
}

void Synchronization::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
