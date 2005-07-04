/***********************************************************
 *
 * File:			empty.cc
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "empty.h"
#include "resp_ref.h"
#include "synchronization.h"
#include "or_fork.h"
#include "connect.h"
#include "hyperedge.h"
#include "or_join.h"
#include "start.h"
#include "stub.h"
#include "timer.h"
#include "wait.h"
#include "hypergraph.h"
#include "map.h"
#include <iostream>
#include "result.h"
#include "point_figure.h"
#include "path.h"
#include "synch_figure.h"
#include "stub_figure.h"
#include "timestamp.h"
#include "goal_tag.h"
#include "loop.h"
#include "msc_generator.h"
#include "print_mgr.h"
#include "scenario.h"
#include "node.h"
#include "scenario_generator.h"
#include "component.h"
#include "component_mgr.h"


extern Label *current_label;
extern int showing_labels;

extern void DrawScreen();
extern void InstallConditions( ConditionManager &conditions );
extern void ResetConditions();

static char characteristic_names[][20]= {

   "failure-point",
   "shared",
   "direction-arrow"
};

Empty::Empty() : Hyperedge()
{ 
   figure = new PointFigure( this, EMPTY_FIG );
   label = current_label;
   label->IncrementCount();
   condition_manager.ConditionType( EMPTY_POINT );
   display_label = FALSE;
   characteristics = NONE;
   path_number = 0;
   highlighted = FALSE;
   highlight_id = 0;
}

Empty::Empty( int empty_id, float x, float y, Label *plabel, const char *desc, int designation, bool show_label ) : Hyperedge()
{
   load_number = empty_id;
   
   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = empty_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;    
   //End of the modification by Bo Jiang, April, 2005
   
   figure = new PointFigure( this, EMPTY_FIG );
   figure->SetActualPosition( x ,y );
   label = plabel;
   if( label ) label->IncrementCount();
   if( desc != NULL ) unique_desc = strdup( desc );
   characteristics = designation;
   display_label = show_label;
   condition_manager.ConditionType( EMPTY_POINT );
   path_number = 0;
   highlighted = FALSE;
   highlight_id = 0;
}

Empty::~Empty()
{
   if( label ) label->DecrementCount();
   label = NULL;

   if( figure->IsSelected() )  // remove conditions from lists if edge deleted
      ResetConditions();
}

Label *Empty::PathLabel()
{
   return( label );
}

void Empty::SetLabel( Label *new_label )
{
   if( label ) label->DecrementCount();
   new_label->IncrementCount();
   label = new_label;
}

bool Empty::ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir )
{
   if( figure->GetPath() == old_path ) {
      figure->SetPath( new_path );
      if( new_label ) this->SetLabel( new_label );
      return TRUE;
   }
   else
      return FALSE;
}

void Empty::EdgeSelected()
{
   InstallConditions( condition_manager );
}

bool Empty::Perform( transformation trans, execution_flag execute )
{
   bool rc;
   Stub *stub = NULL;
   
   if( execute ) { // check if this empty is the input or output of a stub

      if( target->GetFirst()->NextEdge()->EdgeType() == STUB )
	 stub = (Stub *)(target->GetFirst()->NextEdge());
      else if( source->GetFirst()->PreviousEdge()->EdgeType() == STUB )
	 stub = (Stub *)(source->GetFirst()->PreviousEdge());
   }
   
   switch( trans ){
  
   case ADD_RESPONSIBILITY:
      rc = AddResponsibility( execute );
      break;
   case ADD_WAIT:
      rc = AddWait( execute );
      break;
   case ADD_STUB:
      rc = AddStub( execute );
      break;
   case ADD_TIMER:
      rc = AddTimer( execute );
      break;
   case ADD_OR_FORK:
      rc = AddOrFork( execute );
      break;
   case ADD_AND_FORK:
      rc = AddAndFork( execute );
      break;
   case ADD_LOOP:
      rc = AddLoop( execute );
      break;
   case CUT:
      rc = Cut( execute );
      break;
   case DECOMPOSE_FROM_JOIN:
      rc = DecomposeFromJoin( execute );
      break;
   case DECOMPOSE_FROM_STUB:
      rc = DecomposeFromStub( execute );
      break;
   case DELETE_POINT:
      rc = DeletePoint( execute );
      break;
   case ADD_POINT:
      rc = AddPoint( execute );
      break;
   case DELETE_BRANCH:
      rc = DeleteBranch( execute );
      break;
   case EDIT_LABEL:
      rc = EditPathLabel( execute );
      break;
   case ENABLE_PATH_LABEL:
      rc = EnablePathLabel( execute );
      break;
   case DISABLE_PATH_LABEL:
      rc = DisablePathLabel( execute );
      break;
   case ADD_FAILURE_POINT:
      rc = AddFailurePoint( execute );
      break;
   case REMOVE_FAILURE_POINT:
      rc = RemoveFailurePoint( execute );
      break;
   case ADD_SHARED_RESPONSIBILITY:
      rc = AddSharedResponsibility( execute );
      break;
   case REMOVE_SHARED_RESPONSIBILITY:
      rc = RemoveSharedResponsibility( execute );
      break;
   case ADD_TIMESTAMP:
      rc = AddTimestamp( execute );
      break;
   case ADD_GOAL_TAG:
      rc = AddGoalTag( execute );
      break;
   case ADD_DIRECTION_ARROW:
      rc = AddDirectionArrow( execute );
      break;
   case REMOVE_DIRECTION_ARROW:
      rc = RemoveDirectionArrow( execute );
      break;
   case FOLLOW_PATH_SUBMAP:
      rc = FollowPathInSubmap( execute );
      break;
   }

   if( execute && (stub != NULL) )
      stub->ShowInputOutputPaths();
   
   return( rc );
}

void Empty::AddSimple( Hyperedge *edge, bool new_left, bool new_right )
{ 
   bool empty_left = FALSE , empty_right = FALSE;

   if( !new_left ) {
      if( source->GetFirst()->PreviousEdge()->EdgeType() == EMPTY )
	 empty_left = TRUE;
   }

   if( !new_right ) {
      if( target->GetFirst()->NextEdge()->EdgeType() == EMPTY )
	 empty_right = TRUE;
   }

   if( empty_left && empty_right ){	// no extra empties needed
  
      edge->AttachSource( source->GetFirst() );
      edge->AttachTarget( target->GetFirst() );
    
      display_manager->Add( edge, this );  
   }
   else if( empty_left ){	// one empty node needed to the right
  
      Empty *new_empty = new Empty;
      Node *new_node = new Node( B );

      edge->AttachSource( source->GetFirst() );
      edge->AttachTarget( new_node );
      new_empty->AttachSource( new_node );
      new_empty->AttachTarget( target->GetFirst() );
  
      display_manager->Add( edge, this );  
      display_manager->AddBR( new_empty, this );
   }
   else if( empty_right ){	// one empty node needed to the left
  
      Empty *new_empty = new Empty;
      Node *new_node = new Node( A );

      edge->AttachSource( new_node );
      edge->AttachTarget( target->GetFirst() );
      new_empty->AttachSource( source->GetFirst() );
      new_empty->AttachTarget( new_node );

      display_manager->AddBL( new_empty, this );  // order reversed after changes
      display_manager->Add( edge, this );  	   // made with and forks -01/10/97
   }
   else {				// two empty nodes needed

      Empty *empty1 = new Empty;
      Empty *empty2 = new Empty;
      Node *node1 = new Node( A );
      Node *node2 = new Node( B );
 
      empty1->AttachSource( source->GetFirst() );
      empty1->AttachTarget( node1 ); 
      edge->AttachSource( node1 );
      edge->AttachTarget( node2 );
      empty2->AttachSource( node2 );
      empty2->AttachTarget( target->GetFirst() );

      display_manager->AddBL( empty1, this ); 
      display_manager->AddBR( empty2, this );
      display_manager->Add( edge, this );
   }

   // transfer characteristics from this empty about to be deleted to the empty
   // to the left of the new edge

   Empty *left_empty = (Empty *)edge->FirstInput();
   left_empty->characteristics = characteristics;
   left_empty->display_label = display_label;
   left_empty->CopyConditions( condition_manager );
 
   parent_map->MapHypergraph()->PurgeEdge( this );
   
   display_manager->SetActive( 0 );
   display_manager->Update();
}
    
bool Empty::AddResponsibility( execution_flag execute )
{
   ResponsibilityReference *new_ref;
   
   if( execute ){

      new_ref = new ResponsibilityReference;
      new_ref->SetInstall( this );

      return ( TRUE );  // not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}

bool Empty::AddAndFork( execution_flag execute )
{ 
   if( execute ){
      
      Synchronization *synch = new Synchronization( FORK );
      AddSimple( synch );
      synch->ExtendFork();
      return ( TRUE );  		// not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}
 
bool Empty::AddGoalTag( execution_flag execute )
{ 
   if( execute ){
      
      GoalTag *gt = new GoalTag();
      AddSimple( gt );
      return ( TRUE );  		// not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}
        
bool Empty::AddOrFork( execution_flag execute )
{ 
   if( execute ){
  
      OrFork *or_fork = new OrFork;
      AddSimple( or_fork );
      or_fork->ExtendFork();
      return ( TRUE ); 		 // not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}

Synchronization * Empty::AddAndJoin()
{
   current_label = label;  // set appropriate path label before creating and join
   Synchronization *synch = new Synchronization( JOIN );
   AddSimple( synch );
   return( synch );
}

OrJoin * Empty::AddOrJoin()
{
   current_label = label;  // set appropriate path label before creating or join
   OrJoin *or_join = new OrJoin;
   AddSimple( or_join );
   return( or_join );
}

bool Empty::DeleteBranch( execution_flag execute )
{
   Hyperedge *end_edge, *start_edge, *prev_edge;
   bool synch;
   edge_type end_type, prev_type;
   bool joined = FALSE,
      and_fork_end = FALSE;

   // delete a branch of either an and or an or fork
 
   if( execute ){
  
      end_edge = figure->GetPath()->GetPathEnd();
      start_edge = figure->GetPath()->GetPathStart();
      end_type = end_edge->EdgeType();

      if( (end_type == OR_JOIN) || (end_type == SYNCHRONIZATION) || (end_type == STUB) )
	 joined = TRUE;

      synch = ( ( start_edge->EdgeType() == SYNCHRONIZATION ) ? TRUE : FALSE );
    
      Node *front_node = source->GetFirst();
      Node *end_node;

      if( synch ) {
    
	 Synchronization *synch = (Synchronization *)front_node->PreviousEdge();
	 synch->DetachTarget( front_node );
	 display_manager->DeleteStartNull( this );
      
	 if( joined ) {
      
	    Path* branch_path = figure->GetPath();
	    Cltn<Node *> *ajs = end_edge->SourceSet();
	    for( ajs->First(); !ajs->IsDone(); ajs->Next() ){
	       if( ajs->CurrentItem()->PreviousEdge()->GetFigure()->GetPath() == branch_path ){
		  end_node = ajs->CurrentItem();
		  break;
	       }
	    }
    
	    end_edge->DetachSource( end_node );
        
	    if( end_type == SYNCHRONIZATION ) {

	       display_manager->DeleteEndNull( end_node->PreviousEdge() );
	       
	       if( ((Synchronization *)end_edge)->PathCount() == 1 )
		  ((Synchronization *)end_edge)->Delete();
	       else {
		  if( showing_labels )
		     parent_map->EditLabel( end_edge, "Enter new label for And Join path" );
	       }
	    }
	    else if (end_type == OR_JOIN ) {		// end = or join
        
	       display_manager->DeleteEndNull( end_node->PreviousEdge() );
	       if( end_edge->SourceSet()->Size() == 1 )
		  ((OrJoin *)end_edge)->Delete();
	       else {
		  if( showing_labels )
		     parent_map->EditLabel( end_edge, "Enter new label for Or Join path" );
	       }
	    }
	    else {  // end = stub

	       ((StubFigure *)end_edge->GetFigure())->GetCurves()->Delete( figure->GetPath() );
	       figure->GetPath()->PurgeFigure( end_edge->GetFigure() );
	    }
    
	 }
	 else {
	    end_node = end_edge->SourceSet()->GetFirst();
	    parent_map->MapHypergraph()->PurgeEdge( end_edge );     
	 }
    
	 if( synch->PathCount() == 1 )
	    synch->Delete();
	 else {
	    if( showing_labels ) 
	       parent_map->EditLabel( synch, "Enter new Label for And Fork" ); // prompt use for new and fork label
	 }
    
      }
      else { 	// or fork
    
	 Hyperedge *or_fork = front_node->PreviousEdge();
	 or_fork->DetachTarget( front_node );
	 display_manager->DeleteStartNull( this );

	 if( joined ) {

	    Path* branch_path = figure->GetPath();
	    Cltn<Node *> *ojs = end_edge->SourceSet();
	    for( ojs->First(); !ojs->IsDone(); ojs->Next() ){
	       if( ojs->CurrentItem()->PreviousEdge()->GetFigure()->GetPath() == branch_path ){
		  end_node = ojs->CurrentItem();
		  break;
	       }
	    }
    
	    end_edge->DetachSource( end_node );
      
	    if( end_type == OR_JOIN ) {
      
	       display_manager->DeleteEndNull( end_node->PreviousEdge() );
	       if( end_edge->SourceSet()->Size() == 1 )
		  ((OrJoin *)end_edge)->Delete(); 
	       else {
		  if( showing_labels )
		     parent_map->EditLabel( end_edge, "Enter new label for Or Join path" );
	       }
	    }
	    else if( end_type == SYNCHRONIZATION ) {		// end is and join

	       display_manager->DeleteEndNull( end_node->PreviousEdge() );
	       if( ((Synchronization *)end_edge)->PathCount() == 1 )
		  ((Synchronization *)end_edge)->Delete();  
	       else {
		  if( showing_labels )
		     parent_map->EditLabel( end_edge, "Enter new label for And Join path" );
	       }
	    }
	    else {  // end is a stub
	       
	       ((StubFigure *)end_edge->GetFigure())->GetCurves()->Delete( figure->GetPath() );
	       figure->GetPath()->PurgeFigure( end_edge->GetFigure() );
	    }
    
	 }
	 else {
	    end_node = end_edge->SourceSet()->GetFirst();
	    parent_map->MapHypergraph()->PurgeEdge( end_edge );     
	 }
  
	 if( or_fork->TargetSet()->Size() == 1 )
	    ((OrFork *)or_fork)->Delete();
	 else {
	    if( showing_labels )
	       parent_map->EditLabel( or_fork, "Enter new label for Or Fork" );  // prompt user for new or fork label
	 }
      }	// ENDIF - SYNCHRONIZATION
    
      parent_map->MapHypergraph()->PurgeSegmentFrom( front_node, end_node );
 
      return( TRUE );
   }
   else {	// execute false - validate transformation
  
      prev_edge = source->GetFirst()->PreviousEdge();
      prev_type = prev_edge->EdgeType();
      end_edge = figure->GetPath()->GetPathEnd();
      end_type = end_edge->EdgeType();

      if( (end_type == OR_FORK) || (end_type == STUB) )
	 return( FALSE );

      if( end_type == SYNCHRONIZATION ) {
	 if( ((Synchronization *)end_edge)->InputCount() == 1 ) // check if synchronization has only one input
	    and_fork_end = TRUE;
      }
      
      if( prev_type == OR_FORK  && !and_fork_end )
	 return( TRUE );
      
      if( prev_type == SYNCHRONIZATION ){
	 if( ((Synchronization *)prev_edge)->OutputCount() > 1 && !and_fork_end )
	    return( TRUE );
      }

      return( FALSE );

   }
  
}
    
bool Empty::AddStub( execution_flag execute )
{
   bool new_left = FALSE, new_right = FALSE;
   Node *new_node, *sn, *tn;
   Empty *new_empty;
   Hyperedge *prev_edge, *next_edge;
   Stub *stub;
   
   if( execute ){

      stub = new Stub;
      sn = source->GetFirst();
      tn = PrimaryTarget();
      prev_edge = sn->PreviousEdge();
      next_edge = tn->NextEdge();
      
      if( prev_edge->EdgeType() == STUB ) { // add extra empty hyperedge to the left so that two empties will lie between the stubs
	 
	 new_empty = new Empty;
	 new_node = new Node( A );
	 new_empty->AttachTarget( new_node );
	 new_empty->AttachSource( sn );
	 this->ReplaceSource( sn, new_node );
	 display_manager->AddBetween( new_empty, prev_edge, this );
	 new_empty->PathNumber( path_number );
	 new_left = TRUE;
      }
      else if( prev_edge->EdgeType() == EMPTY ) { // if there is a stub two edges over to the left set flag for the creation of second empty
	 if( prev_edge->FirstInput()->EdgeType() == STUB )
	    new_left = TRUE;
      }
      
      if( next_edge->EdgeType() == STUB ) { // add extra empty hyperedge to the right so that two empties will lie between the stubs

	 new_empty = new Empty;
	 new_node = new Node( B );
	 new_empty->AttachSource( new_node );
	 new_empty->AttachTarget( tn );
	 this->ReplaceTarget( tn, new_node );
	 display_manager->AddBetween( new_empty, this, next_edge );
	 new_empty->PathNumber( path_number );
	 new_right = TRUE;
      }
      else if( next_edge->EdgeType() == EMPTY ) { // if there is a stub two edges over to the right set flag for the creation of second empty
	 if( next_edge->FirstOutput()->EdgeType() == STUB )
	    new_right = TRUE;
      }
      
      AddSimple( stub, new_left, new_right );
      stub->Install();
      return ( TRUE );  // not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}
  
bool Empty::AddTimer( execution_flag execute )
{ 
   if( execute ){
      Timer *timer = new Timer( this );
      if (timer == NULL) {} // Makes GCC happy... timer is now used!
      return ( TRUE );  // not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}

bool Empty::AddTimestamp( execution_flag execute )
{ 
   if( execute ){
      Timestamp *timestamp = new Timestamp;
      AddSimple( timestamp );
      return ( TRUE );  // not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}

bool Empty::AddWait( execution_flag execute )
{ 
   if( execute ){
      Wait *wait = new Wait;
      AddSimple( wait );
      display_manager->CreateWaitSynch( wait );
      return ( TRUE );  // not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}

bool Empty::AddLoop( execution_flag execute )
{
   if( execute ){
      Loop *loop = new Loop;
      AddSimple( loop );
      loop->Create();
      return ( TRUE );  // not used
   }
   else
      return( !HasTargetColour(D) );	//  change needed later
}

bool Empty::ConnectPath( Hyperedge *trigger_edge, execution_flag execute )
{ 
   if( execute ) {
      ((WaitingPlace *)trigger_edge)->ConnectPath( this, execute );
      return ( TRUE );  // not used
   }
   else {
      edge_type type = trigger_edge->EdgeType();

      if( !((type == WAIT) || (type == TIMER) || (type == START)) )
	  return( FALSE );

      if( figure->GetPath() == trigger_edge->GetFigure()->GetPath() )
	 return( FALSE );       
            
      return( TRUE );
   }

}

bool Empty::DisconnectPath( Hyperedge *trigger_edge, execution_flag execute )
{ 
   if( execute ) {
      ((WaitingPlace *)trigger_edge)->DisconnectPath( this, execute );
      return ( TRUE );  // not used
   }
   else {
      edge_type type = trigger_edge->EdgeType();

      if( !((type == WAIT) || (type == TIMER) || (type == START)) )
	  return( FALSE );

      if( figure->GetPath() == trigger_edge->GetFigure()->GetPath() )
	 return( FALSE );       
            
      return( TRUE );
   }

}

bool Empty::Cut( execution_flag execute )
{ 
   // cut transformation for empty
  
   if( execute ){
    
      Node *node1 = new Node( A );
      Node *node2 = new Node( B );
      Empty *front_empty = new Empty;
      Empty *end_empty = new Empty;
      Result *result = new Result( FALSE );  // cancel default installation of
      Start *start = new Start( FALSE );     // start and result as path starts and ends
      
      front_empty->AttachSource( source->GetFirst() );
      front_empty->AttachTarget( node1 );
      result->AttachSource( node1 );
      start->AttachTarget( node2 );
      end_empty->AttachSource( node2 );
      end_empty->AttachTarget( target->GetFirst() );
      
      display_manager->AddBL( front_empty, this );
      display_manager->AddBR( end_empty, this );
      display_manager->AddBetween( result, front_empty, this );
      display_manager->AddBetween( start, this, end_empty );   
      
      parent_map->MapHypergraph()->PurgeEdge( this );
      display_manager->SplitPaths( result, start );
      display_manager->SetPath( result->GetFigure()->GetPath() );
      DrawScreen();
      if( showing_labels )
	 parent_map->EditLabel( start, "Enter unique label for second path" );
      
      return ( TRUE );  // not used
    
   }
   else {	//  execute false, validate transformation
  
      if( HasTargetColour( D ) )
	 return( FALSE );

      return( TRUE );	 
      
   }
}


bool Empty::AndCompose( Hyperedge *edge, execution_flag execute )
{ 
   // and compose a path including self to another UCM path
  
   if( execute )
      ((Result *)edge)->AndCompose( this, execute );
   return ( TRUE );  // not used
}


bool Empty::OrCompose( Hyperedge *edge, execution_flag execute )
{ 
   // or compose a path including self to another UCM path
  
   if( execute )
      ((Result *)edge)->OrCompose( this, execute );
   return ( TRUE );  // not used
}

bool Empty::DecomposeFromJoin( execution_flag execute )
{ 
   edge_type type;
   Hyperedge *next_edge = this->PrimaryTarget()->NextEdge();
   
   if( execute )
   {
      DecomposeFrom( (MultipathEdge *)next_edge );
      return ( TRUE );  // not used
   }
   else
   {
      type = next_edge->EdgeType();
      
      if( type == OR_JOIN )
	 return( TRUE );

      if( type == SYNCHRONIZATION ){
	 if( ((Synchronization *)next_edge)->InputCount() > 1 )
	    return( TRUE );
      }

      return( FALSE );
   }      
}

bool Empty::DecomposeFromStub( execution_flag execute )
{ 
   Hyperedge *next_edge = this->PrimaryTarget()->NextEdge();
   Hyperedge *previous_edge = source->GetFirst()->PreviousEdge();

   if( execute ){

      if( next_edge->EdgeType() == STUB ) {

	 Result *new_result = new Result;
  
	 display_manager->AddAfter( new_result, this );
	 display_manager->DisconnectStubJoin( this, next_edge, new_result );

	 Node *front_node = this->PrimaryTarget();
	 
	 next_edge->DetachSource( front_node );
	 new_result->AttachSource( front_node );
	 
	 display_manager->SetPath( figure->GetPath() );
	 display_manager->Update();
      }
      else {   // previous edge is a stub

	 Start *new_start = new Start;

	 display_manager->AddBL( new_start, this );
	 display_manager->DisconnectStubFork( this, previous_edge, new_start );

	 Node *front_node = source->GetFirst();
	 previous_edge->DetachTarget( front_node );
	 new_start->AttachTarget( front_node );

	 display_manager->SetPath( figure->GetPath() );
	 display_manager->Update();

      }
      path_number = 0;
      return ( TRUE );  // not used
   }
   else {

      if( next_edge->EdgeType() == STUB )
	 return( (next_edge->SourceSet()->Size() > 1) ? TRUE : FALSE );
      else if( previous_edge->EdgeType() == STUB )
	 return( (previous_edge->TargetSet()->Size() > 1) ? TRUE : FALSE );     
      else
	 return( FALSE );
   }      
}

void Empty::DecomposeFrom(  MultipathEdge *mp_edge  )
{ 
   edge_type type = mp_edge->EdgeType();

   // decompose the path this empty belongs to from the edge it is attached to 
   // changed to accomodate and joins as well - 01/07/97
  
   Result *new_result = new Result;
  
   display_manager->AddAfter( new_result, this );

   Node *front_node = this->PrimaryTarget();
   
   mp_edge->DetachSource( front_node );
   new_result->AttachSource( front_node );

   display_manager->DisconnectJoin( this, new_result );
  
   if( mp_edge->PathCount() == 1 )
      mp_edge->Delete();
   else
   { // path count > 1

      if( showing_labels ) {
	    
	 // prompt user for new label of join path
	 if( type == OR_JOIN )
	    parent_map->EditLabel( mp_edge, "Enter new label for Or Join output path" );
	 if( type == SYNCHRONIZATION )
	    parent_map->EditLabel( mp_edge, "Enter new label for And Join output path" );
      }
   }
  
   display_manager->SetPath( figure->GetPath() );
   current_label = label;  // rest current label to that of this path
   display_manager->Update();  
}

bool Empty::DeletePoint( execution_flag execute )
{
   static Hyperedge *prev_edge, *next_edge;
   edge_type prev_type, next_type;
   
   if( execute ){
  
      Node *deleted_node;
      Empty *transfer_empty;
    
      if( prev_edge->EdgeType() == EMPTY ) {
	 transfer_empty = (Empty *)prev_edge;
	 deleted_node = source->GetFirst();
	 prev_edge->ReplaceTarget( deleted_node, target->GetFirst() );
      }
      else {
	 transfer_empty = (Empty *)next_edge;
	 deleted_node = target->GetFirst();
	 next_edge->ReplaceSource( deleted_node, source->GetFirst() );
      }

      // transfer characteristics from this empty about to be deleted to the empty
      // to the left of the new edge if it exists and to the right if it doesn't
      transfer_empty->characteristics = characteristics;
      transfer_empty->display_label = display_label;
      transfer_empty->CopyConditions( condition_manager );
      figure->GetPath()->PathChanged();

      parent_map->MapHypergraph()->PurgeEdge( this );
      parent_map->MapHypergraph()->PurgeNode( deleted_node );
      // display_manager->Update();
      return( TRUE ); // not used
    
   }
   else {

      prev_edge = source->GetFirst()->PreviousEdge();
      next_edge = target->GetFirst()->NextEdge();
      prev_type = prev_edge->EdgeType();
      next_type = next_edge->EdgeType();
      
      // ensure two empty points between two stubs
      if( prev_type == STUB && next_type == EMPTY ) {
	 if( next_edge->FirstOutput()->EdgeType() == STUB )
	    return( FALSE );
      }

      if( prev_type == EMPTY && next_type == STUB ) {
	 if( prev_edge->FirstInput()->EdgeType() == STUB )
	    return( FALSE );
      }
      
      // valid if at least one empty to the left or right and if this empty does not trigger a wait or a timer
      if( ( prev_type == EMPTY || next_type == EMPTY ) && !HasTargetColour(D) ){
	 return( TRUE );
      } 
      else
	 return( FALSE );
   }
}

bool Empty::DeleteHyperedge()
{
   edge_type prev_type;

   // determine if this empty is the first on a branch of a fork
   prev_type = source->GetFirst()->PreviousEdge()->EdgeType();

   if( (prev_type == OR_FORK) || (prev_type == SYNCHRONIZATION) ) {
      if( this->DeleteBranch( FALSE ) == TRUE ) {
	 this->DeleteBranch( TRUE );
	 return TRUE;
      }
   }

   // determine if this empty is part of a sequence of two or more empties
   if( this->DeletePoint( FALSE ) == TRUE ) {
      this->DeletePoint( TRUE );
      return TRUE;
   }

   return FALSE;
}

bool Empty::AddPoint( execution_flag execute )
{
   if( execute ){
      
      Empty* new_empty = new Empty;
      Node *node1 = new Node( B );
      Node *tnode = target->GetFirst();

      if( tnode->NextEdge()->EdgeType() == STUB ) {
	 new_empty->PathNumber( path_number );
	 path_number = 0;
      }

      new_empty->AttachSource( node1 );
      new_empty->AttachTarget( tnode );

      display_manager->AddBR( new_empty, this );

      target->Remove( tnode );
      AttachTarget( node1 );

      return( TRUE ); // not used
   }
   else
      return( TRUE );
}

bool Empty::EditPathLabel( execution_flag execute )
{
   if( execute )
      return( parent_map->EditLabel( this, "Enter new label for this path" ) );
   else
      return( (showing_labels || display_label) ? TRUE : FALSE );
}

bool Empty::EnablePathLabel( execution_flag execute )
{
   if( execute ) {
      display_label = TRUE;
      return( parent_map->EditLabel( this, "Enter new label for this path" ) );
   }
   else
      return( (!display_label) && (!showing_labels) );
}

bool Empty::DisablePathLabel( execution_flag execute )
{
   if( execute ) {
      display_label = FALSE;
      return( TRUE );
   }
   else
      return( (display_label) && (!showing_labels) );

}

bool Empty::AddFailurePoint( execution_flag execute )
{
   if( execute ) {
      characteristics = FAILURE_POINT;
      return( TRUE );
   }
   else
      return( characteristics == NONE  && source->GetFirst()->PreviousEdge()->EdgeType() != ABORT );
}

bool Empty::RemoveFailurePoint( execution_flag execute )
{
   if( execute ) {
      characteristics = NONE;
      return( TRUE );
   }
   else
      return( characteristics == FAILURE_POINT );
}

bool Empty::AddSharedResponsibility( execution_flag execute )
{
   if( execute ) {
      characteristics = SHARED_RESPONSIBILITY;
      return( TRUE );
   }
   else
      return( characteristics == NONE && source->GetFirst()->PreviousEdge()->EdgeType() != ABORT );
}

bool Empty::RemoveSharedResponsibility( execution_flag execute )
{
   if( execute ) {
      characteristics = NONE;
      return( TRUE );
   }
   else
      return( characteristics == SHARED_RESPONSIBILITY );
}

bool Empty::AddDirectionArrow( execution_flag execute )
{
   if( execute ) {
      characteristics = DIRECTION_ARROW;
      return( TRUE );
   }
   else
      return( characteristics == NONE && source->GetFirst()->PreviousEdge()->EdgeType() != ABORT );
}

bool Empty::RemoveDirectionArrow( execution_flag execute )
{
   if( execute ) {
      characteristics = NONE;
      return( TRUE );
   }
   else
      return( characteristics == DIRECTION_ARROW );
}

int Empty::DoubleClickAction()
{
   if( FollowPathInSubmap( FALSE ) == TRUE ) 
      FollowPathInSubmap( TRUE );
   else {
      if( showing_labels || display_label )
	 return( parent_map->EditLabel( this, "Enter new label for this path" ) );
   }
   return( UNMODIFIED );
}

bool Empty::FollowPathInSubmap( execution_flag execute )
{
   static Hyperedge *stub_edge;
   Hyperedge *next_edge, *previous_edge;

   if( execute ) { 
      ((Stub *)stub_edge)->ViewPlugin( this );
      return( UNMODIFIED );
   } else {
      next_edge = this->PrimaryTarget()->NextEdge();
      previous_edge = source->GetFirst()->PreviousEdge();

      if( next_edge->EdgeType() == STUB )
	 stub_edge = next_edge;
      else if( previous_edge->EdgeType() == STUB )
	 stub_edge = previous_edge;
      else
	 return( FALSE );

      return( ((Stub *)stub_edge)->HasViewableSubmap() );
   }
}

void Empty::ReorderPaths()
{
   Hyperedge *previous_edge = source->GetFirst()->PreviousEdge(),
      *next_edge = this->PrimaryTarget()->NextEdge();
   HyperedgeFigure *prev_figure = NULL,
      *next_figure = NULL;
   
   if( previous_edge ) prev_figure = previous_edge->GetFigure();
   if( prev_figure ) prev_figure->OrderPaths();
   if( next_edge ) next_figure = next_edge->GetFigure();
   if( next_figure ) next_figure->OrderPaths();
}

void Empty::SaveXMLDetails( FILE *fp )
{
   PrintXMLText( fp, "<empty-segment" );
   fprintf( fp, " path-label-id=\"pl%d\"", label->GetNumber() );
   if( characteristics != NONE )
      fprintf( fp, " characteristics=\"%s\"", characteristic_names[characteristics-2] );
   if( display_label == TRUE )
      fprintf( fp, " show-label=\"yes\"" );
   fprintf( fp, " >\n" );
   IndentNewXMLBlock( fp );
   condition_manager.SaveXML( fp, "precondition", "postcondition" );
   PrintEndXMLBlock( fp, "empty-segment" );
}

void Empty::SaveCSMXMLDetails( FILE *fp )
{

   PrintXMLText( fp, "<empty-segment" );
   fprintf( fp, " path-label-id=\"pl%d\"", label->GetNumber() );
   if( characteristics != NONE )
      fprintf( fp, " characteristics=\"%s\"", characteristic_names[characteristics-2] );
   if( display_label == TRUE )
      fprintf( fp, " show-label=\"yes\"" );
   fprintf( fp, " >\n" );
   IndentNewXMLBlock( fp );
   condition_manager.SaveXML( fp, "precondition", "postcondition" );
   PrintEndXMLBlock( fp, "empty-segment" );
}



Hyperedge * Empty::GenerateMSC( bool first_pass )
{
#ifdef MSC_DEBUG	       
   cout << "Empty::GenerateMSC: " << hyperedge_number << endl << flush;
#endif

   MSCGenerator::Instance()->PreviousEmpty( this );
   TriggerAsynchronousConnection();   
   return( PrimaryTarget()->NextEdge() );
}

Hyperedge * Empty::ScanScenarios( scan_type type )
{
   int tracing_id;
   bool multiple_pass = FALSE;

#ifdef MSC_DEBUG	       
   cout << "Empty::ScanScenarios: " << hyperedge_number << endl << flush;
#endif
   MSCGenerator::Instance()->PreviousEmpty( this );
   if( type == SCENARIO_TRACE ) {
      if( (tracing_id = ScenarioList::ScenarioTracingId()) == highlight_id )
	 multiple_pass = TRUE;
      figure->GetPath()->HighlightPath( multiple_pass ? -1*tracing_id : tracing_id );
      highlight_id = tracing_id;
   }
   TriggerAsynchronousConnection(); 
   PrimaryTarget()->setVisited(TRUE);
   return( PrimaryTarget()->NextEdge() );
}


Hyperedge * Empty::HighlightScenario( )
{
   int tracing_id;
   bool multiple_pass = FALSE;

#ifdef MSC_DEBUG	       
   cout << "Empty::ScanScenarios: " << hyperedge_number << endl << flush;
#endif
   MSCGenerator::Instance()->PreviousEmpty( this );
   if( (tracing_id = ScenarioList::ScenarioTracingId()) == highlight_id )
      multiple_pass = TRUE;
   figure->GetPath()->HighlightPath( multiple_pass ? -1*tracing_id : tracing_id );
   highlight_id = tracing_id;
   TriggerAsynchronousConnection(); 
   PrimaryTarget()->setVisited(TRUE);
   return( PrimaryTarget()->NextEdge() );
}

void Empty::TriggerAsynchronousConnection()
{
   if( HasTargetColour( D ) )
      ((WaitingPlace *)(ExceptionPath()->NextEdge()->FirstOutput()))->TriggerWaitingPlace();
}

void Empty::GeneratePostScriptDescription( FILE *ps_file )
{
   bool conditions_empty = condition_manager.ConditionsEmpty(),
      has_description = HasDescription();

   if( conditions_empty &&  !has_description ) return;

   fprintf( ps_file, "%d rom (Empty Point - ) P\n", PrintManager::text_font_size );
   if( has_description ) GeneratePSElementDescription( ps_file );
   if( !conditions_empty ) condition_manager.GeneratePostScriptDescription( ps_file );
}

void Empty::CopyConditions( ConditionManager &cm )
{
   condition_manager.AddConditions( cm );
}

Hyperedge *  Empty::TraverseScenario()
{
   SCENARIOGenerator::Instance()->PreviousEmpty( this );
   TriggerAsynchronousConnection(); 
   PrimaryTarget()->setVisited(TRUE);
   return( PrimaryTarget()->NextEdge() );
}

void Empty::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
