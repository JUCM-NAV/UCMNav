/***********************************************************
 *
 * File:			waiting_place.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:	Implemented connect/disconnect
 *				January 1997
 *
 ***********************************************************/

#include "waiting_place.h"
#include "connect.h"
#include "empty.h"
#include "hypergraph.h"
#include "print_mgr.h"
#include "map.h"
#include "variable.h"
#include "msc_generator.h"
#include "utilities.h"
#include <iostream>
#include "scenario_generator.h"

extern void InstallConditions( ConditionManager &conditions );
extern void ResetConditions();
extern const char * EditLogicalCondition( const char *condition );

extern int map_deletion;

int WaitingPlace::number_waits = 1;

WaitingPlace::WaitingPlace() : Hyperedge()
{
   condition_manager.ConditionType( PATH_START );
   identifier[0] = 0;
   identifier[19] = 0;
   logical_condition = NULL;
}

WaitingPlace::~WaitingPlace()
{
   this->BreakConnections();
   if( logical_condition != NULL ) free( logical_condition );
   if( figure->IsSelected() )  // remove conditions from lists if edge deleted
      ResetConditions();
}

void WaitingPlace::EdgeSelected()
{
   InstallConditions( condition_manager );
}

void WaitingPlace::DeleteWaiting()
{ 
   Empty *new_empty = new Empty;
   Node *node1 = new Node( B );
   Node *node2 = new Node( A );
   new_empty->AttachSource( node1 );
   new_empty->AttachTarget( node2 );

   Node *front_node = this->PrimarySource();
   Node *end_node = target->GetFirst();
  
   Hyperedge *prev_edge = front_node->PreviousEdge();
   Hyperedge *next_edge = end_node->NextEdge(); 
  
   this->BreakConnections();
   
   next_edge->ReplaceSource( end_node, node2 );
   prev_edge->ReplaceTarget( front_node, node1 );
  
   display_manager->Add( new_empty, this );
   parent_map->MapHypergraph()->PurgeSegmentFrom( front_node, end_node );
}

void WaitingPlace::BreakConnections()
{
   // check if waiting place is triggered by an empty or a result and break
   // the connection if one is found

   if( map_deletion ) return;  // if hypergraph is being deleted don't bother
   
   if( this->HasSourceColour(C) ) {  
      Node *node_c = this->SourceOfColour(C); // find source connection node     
      Node *node_d = node_c->PreviousEdge()->SourceSet()->GetFirst(); // determine source node of connect
      node_d->PreviousEdge()->DetachTarget( node_d ); // detach D target node of triggering edge  
      this->DetachSource( node_c );      
      parent_map->MapHypergraph()->PurgeSegmentFrom( node_d, node_c );
   }
}

bool WaitingPlace::ConnectPath( Hyperedge *trigger_edge, execution_flag execute )
{ 
#if defined( TRACE ) 
   Trace trace( __FILE__, __LINE__, "bool WaitingPlace::ConnectPath( Hyperedge *trigger_edge, execution_flag execute )" ); 
#endif /* TRACE */ 


   if( execute ){
  
      Node *nodec = new Node( C );
      Node *noded = new Node( D );
      Connect *connect = new Connect;
    
      connect->AttachSource( noded );
      connect->AttachTarget( nodec );
      AttachSource( nodec );
      trigger_edge->AttachTarget( noded );
    
      display_manager->Connect( trigger_edge, this );
      return( TRUE );  // not used
   }
   else {	// validate transformation
  
      // invalidate transformation if another empty or result is already connected as a trigger
      if( this->HasSourceColour( C ) )
	 return( UNAVAILABLE );

      edge_type type = trigger_edge->EdgeType();
      if( !((type == RESULT) || (type == EMPTY)) )
	 return( INVALID );

      if( figure->GetPath() == trigger_edge->GetFigure()->GetPath() )
	 return( UNAVAILABLE );

       // invalidate transformation if the empty or result is already connected	
      if( trigger_edge->HasTargetColour( D ) )
	 return( UNAVAILABLE );
          
      return( AVAILABLE );
   }
}
 
bool WaitingPlace::DisconnectPath( Hyperedge *trigger_edge, execution_flag execute )
{ 
#if defined( TRACE ) 
   Trace trace( __FILE__, __LINE__, "bool WaitingPlace::DisconnectPath( Hyperedge *trigger_edge, execution_flag execute )" ); 
#endif /* TRACE */ 

   if( execute ){  
      Node *front_node, *end_node;
      for( source->First(); !source->IsDone(); source->Next() ){
	 if( source->CurrentItem()->PreviousEdge() != 0 ){
	    if( source->CurrentItem()->PreviousEdge()->FirstInput() == trigger_edge ){
	       end_node = source->CurrentItem();
	       front_node = end_node->PreviousEdge()->SourceSet()->GetFirst();
	       trigger_edge->DetachTarget( front_node );
	       DetachSource( end_node );
	       display_manager->Disconnect( trigger_edge, this );
	       parent_map->MapHypergraph()->PurgeSegmentFrom( front_node, end_node );
	       return( TRUE );
	    }
	 }
      }
      
      return ( FALSE );  // DA: Added August 2004
   }
   else {	// validate transformation, must be already connected
  
      edge_type type = trigger_edge->EdgeType();
      if( !((type == RESULT) || (type == EMPTY)) )
	 return( INVALID );
    
      for( source->First(); !source->IsDone(); source->Next() ){
	 if( source->CurrentItem()->PreviousEdge() != 0 ){
	    if( source->CurrentItem()->PreviousEdge()->FirstInput() == trigger_edge )
	       return( AVAILABLE );
	 }
      }
      return( UNAVAILABLE );
   }
}  

bool WaitingPlace::EditWaitingName( execution_flag execute )
{
   const char *name;
   char prompt[30];
   
   if( execute ){
      sprintf( prompt, "Enter the %s Name", EdgeName() );
      name = fl_show_input( prompt, identifier );
      if( name ) {
	 strncpy( identifier, name, 19 );
	 return( MODIFIED );
      } else
	 return( UNMODIFIED );
   }
   else
      return( TRUE );
}

bool WaitingPlace::EditGuardCondition( execution_flag execute )
{
   const char *edited_condition;
   char *expanded_condition;
   int rc;

   if( execute ){
      expanded_condition = ((logical_condition != NULL) ? BooleanVariable::ReferenceToName( logical_condition ) : NULL);
      if( (edited_condition = EditLogicalCondition( expanded_condition )) != NULL ) {
	 logical_condition = BooleanVariable::NameToReference( edited_condition, logical_condition );
	 rc = MODIFIED;
      }
      else
	 rc = UNMODIFIED;
      if( expanded_condition != NULL ) free( expanded_condition );
      return( rc );
   }
   else
      return( TRUE );
}

Hyperedge * WaitingPlace::EvaluateGuardCondition()
{
   char buffer[200];
   MSCGenerator *msc = MSCGenerator::Instance();

   //  evaluate logical condition (if it exists) to determine if scanning should be halted
   if( logical_condition != NULL ) {
      if( PathVariableList::EvaluateLogicalExpression( logical_condition ) == FALSE ) {
	 sprintf( buffer, "The guard condition of the %s point \"%s\" in Map \"%s\" evaluated to false.",
		  EdgeName(), identifier, parent_map->MapLabel() );
	 ErrorMessage( buffer, TRUE );
	 msc->AbortGeneration();
	 return( NULL );
      }
   }
   return( target->GetFirst()->NextEdge() ); // continue scanning down the path after this wait
}

void WaitingPlace::WaitingName( char *new_name )
{
   strncpy( identifier, new_name, 19 );
}

bool WaitingPlace::EvaluateCondition()
{
   char buffer[200];
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();

   //  evaluate logical condition (if it exists) to determine if scanning should be halted
   if( logical_condition != NULL ) {
      if( PathVariableList::EvaluateLogicalExpression( logical_condition ) == FALSE ) {
	 sprintf( buffer, "The guard condition of the %s point \"%s\" in Map \"%s\" evaluated to false.",
		  EdgeName(), identifier, parent_map->MapLabel() );
	 ErrorMessage( buffer, TRUE );
	 sce->AbortGeneration();
	 return( false);
      }
   }
   return true;  //( target->GetFirst()->NextEdge() ); // continue scanning down the path after this wait
}

