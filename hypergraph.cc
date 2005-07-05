/***********************************************************
 *
 * File:			hypergraph.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "hypergraph.h"
#include "stub.h"
#include "timer.h"
#include <iostream>
#include <stdio.h>

Hypergraph::Hypergraph()
{ 
   trans_manager = TransformationManager::Instance();
   node_pool = new Cltn<Node *>;
   edge_pool = new Cltn<Hyperedge *>;

   for( int i = 0; i < 2; i++ ) {
      node_trash[i] = new Cltn<Node *>;
      edge_trash[i] = new Cltn<Hyperedge *>;
   }
}

Hypergraph::~Hypergraph()
{ 
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == TIMER )
	 ((Timer *)(edge_pool->CurrentItem()))->DisconnectTimeoutPath();
   }

   while( !node_pool->is_empty())
      delete node_pool->Detach();
   delete node_pool;
   node_pool = NULL;

   while( !edge_pool->is_empty())
      delete edge_pool->Detach();
   delete edge_pool;
   edge_pool = NULL;

   for( int i = 0; i < 2; i++ ) {
      while( !node_trash[i]->is_empty())
	 delete node_trash[i]->Detach();
      delete node_trash[i];  

      while( !edge_trash[i]->is_empty())
	 delete edge_trash[i]->Detach();
      delete edge_trash[i];
   }  
}

void Hypergraph::EmptyAllTrash( int index )
{ 
   while( !edge_trash[index]->is_empty())
      delete edge_trash[index]->Detach();

   while( !node_trash[index]->is_empty())
      delete node_trash[index]->Detach();
}

void Hypergraph::PurgeEdge( Hyperedge *edge )
{ 
   if( edge_pool->Includes( edge ) ){
      edge_pool->Remove( edge );
      delete edge;
   }
   else
      std::cerr << "Error: edge already removed.\n";  
}

void Hypergraph::PurgeNode( Node *node )
{   
   if( node_pool->Includes( node ) ){
      node_pool->Remove( node );
      delete node;
   }
   else
      std::cerr << "Error: node already removed.\n";  
}

void Hypergraph::PurgeGraph()
{ 
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() )
      PurgeEdge( edge_pool->CurrentItem() );

   for( node_pool->First(); !node_pool->IsDone(); node_pool->Next() )
      PurgeNode( node_pool->CurrentItem() );
}

void Hypergraph::PurgeSegmentFrom( Node *start_node, Node *end_node )
{ 
   PurgeSubSegment( start_node, end_node );
   EmptyAllTrash( 1 );
}

void Hypergraph::PurgeSubSegment( Node *start_node, Node *end_node )
{
   // remove all the nodes and hyperedges from the pools from start to end nodes
  
   if( start_node == end_node )
      PurgeNode( end_node );
   else {
  
      Hyperedge *current_edge;
      current_edge = start_node->NextEdge();

      if( current_edge != 0 ){
    
	 Cltn<Node *> *tset = current_edge->TargetSet();
	 for( tset->First(); !tset->IsDone(); tset->Next() )
	    PurgeSubSegment( tset->CurrentItem(), end_node );
	 TrashEdge( current_edge, 1 );

	 Cltn<Node *> *sset = current_edge->SourceSet();
	 for( sset->First(); !sset->IsDone(); sset->Next() )
	    TrashNode( sset->CurrentItem(), 1 );
      }
   }

}

void Hypergraph::PurgeLoop( Node *input_node, Node *output_node )
{ // deletes an empty loop where loop output or input may or may not be joined
   Hyperedge *current_edge, *loop = input_node->NextEdge();
   Node *current_node;

   TrashNode( output_node, 1 );
   TrashNode( input_node, 1 );
   TrashEdge( loop, 1 );

   // follow loop output path
   current_node = loop->TargetSet()->Get( 2 );
   while( !node_trash[1]->Includes( current_node ) ) {
      TrashNode( current_node, 1 );
      current_edge = current_node->NextEdge();
      if( edge_trash[1]->Includes( current_edge ) )
	 break;
      TrashEdge( current_edge, 1 );
      if( current_edge->TargetSet()->Size() == 0 )
	 break; // result has been reached
      current_node = current_edge->TargetSet()->GetFirst();
      if( current_node->GetColour() == D )
	 break; // path triggering connection has been reached
   }

   // follow loop input path
   current_node = loop->SourceSet()->Get( 2 );
   while( !node_trash[1]->Includes( current_node ) ) {
      TrashNode( current_node, 1 );
      current_edge = current_node->PreviousEdge();
      if( edge_trash[1]->Includes( current_edge ) )
	 break;
      TrashEdge( current_edge, 1 );
      if( current_edge->SourceSet()->Size() == 0 )
	 break; // result has been reached
      current_node = current_edge->SourceSet()->GetFirst();
      if( current_node->GetColour() == C )
	 break; // path triggering connection has been reached
   }   

   EmptyAllTrash( 1 );
}

void Hypergraph::RegisterEdge( Hyperedge *edge )
{ 

   if( edge_pool->Includes( edge ) )
      std::cerr << "Error: Edge already registered.\n";
   else
      edge_pool->Add( edge );
}

void Hypergraph::RegisterNode( Node *node )
{ 

   if( node_pool->Includes( node ) )
      std::cerr << "Error: Node already registered.\n";
   else
      node_pool->Add( node );
}

bool Hypergraph::TrashEdge( Hyperedge *edge, int index )
{ 
   if( !edge_trash[index]->Includes( edge ) ){
      edge_trash[index]->Add( edge );
      if( edge_pool->Includes( edge ) )
	 edge_pool->Remove( edge );
      return TRUE;
   } else
      return FALSE;
}
    
void Hypergraph::TrashNode( Node *node, int index )
{ 
   if( !node_trash[index]->Includes( node ) ){
      node_trash[index]->Add( node );
      if( node_pool->Includes( node ) )
	 node_pool->Remove( node );
   }
}

void Hypergraph::ResetVisited()
{
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() )
      edge_pool->CurrentItem()->ResetVisited();
}

void Hypergraph::SaveXML( FILE *fp )
{
   Hyperedge *loop_inputs[100];
   int i, number_loops = 0;

   // reorder edge pool list to place empty hyperedges which are loop inputs of loop hyperedges
   // at end of list. This ensures that the loop input path is always the second source node
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == LOOP )
	 loop_inputs[number_loops++] = edge_pool->CurrentItem()->SourceSet()->Get( 2 )->PreviousEdge(); // find the empty point entering the loop input
   }

   for( i = 0; i < number_loops; i++ ) {
      edge_pool->Delete( loop_inputs[i] ); // remove from wherever it is in the list
      edge_pool->Add( loop_inputs[i] );    // add at end of list
   }

   // save non-stub hyperedges first, then stubs to avoid forward references
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() != STUB )
	 edge_pool->CurrentItem()->SaveXML( fp );
   }

   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB )
	 edge_pool->CurrentItem()->SaveXML( fp );
   }

   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() )
      edge_pool->CurrentItem()->SaveXMLConnections( fp );

   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() )
	 edge_pool->CurrentItem()->SavePathBranchingSpec( fp );

   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB )
	 ((Stub *)(edge_pool->CurrentItem()))->SaveContinuityBindings( fp );
   }
}

void Hypergraph::SaveCSMXML( FILE *fp )
{
   Hyperedge *loop_inputs[100];
   int i, number_loops = 0;
   Hyperedge * hyperedge_pool [1000];
   Hyperedge * end_pool [1000];
   int j,number_resp =0, number_end=0;
   
  // Cltn<Hyperedge *> *new_edge_pool= Hyperedges();

   // reorder edge pool list to place empty hyperedges which are loop inputs of loop hyperedges
   // at end of list. This ensures that the loop input path is always the second source node
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == LOOP )
	 loop_inputs[number_loops++] = edge_pool->CurrentItem()->SourceSet()->Get( 2 )->PreviousEdge(); // find the empty point entering the loop input
   }

   for( i = 0; i < number_loops; i++ ) {
      edge_pool->Delete( loop_inputs[i] ); // remove from wherever it is in the list
      edge_pool->Add( loop_inputs[i] );    // add at end of list
   }
   
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == START || edge_pool->CurrentItem()->EdgeType() == RESULT ||edge_pool->CurrentItem()->EdgeType() == RESPONSIBILITY_REF||edge_pool->CurrentItem()->EdgeType() == EMPTY 
	  || edge_pool->CurrentItem()->EdgeType() == SYNCHRONIZATION ||edge_pool->CurrentItem()->EdgeType() == OR_FORK||edge_pool->CurrentItem()->EdgeType() == OR_JOIN)
         hyperedge_pool[number_resp ++] = edge_pool->CurrentItem();
    }

   // add resource acquire and release elements when necessary
   for( j=0; j< number_resp; j++)
   {
      if (hyperedge_pool[j]->EdgeType() ==START)
	      hyperedge_pool[j]->AddCSMResourceAcquire();
	  else
	  {    
        hyperedge_pool[j]->AddCSMResourceRelease();
		hyperedge_pool[j]->AddCSMResourceAcquire();
	  }

      if (hyperedge_pool[j]->EdgeType() ==RESULT)
	  {
   	   //  hyperedge_pool[j]->AddCSMResourceAcquire();
		 hyperedge_pool[j]->AddCSMEndResourceRelease();
	  }
	}

// record where resource acquirement elements should be added
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB )
         end_pool[number_end ++] = edge_pool->CurrentItem();
   }
    for(j=0; j<number_end; j++)
	{
	     end_pool[j]->AddCSMResourceRelease();
		 end_pool[j]->AddCSMResourceAcquire();
    }
	
/*
   // save non-stub hyperedges first, then stubs to avoid forward references
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() != STUB )
	  {   
		  new_pool[number_edge ++]=edge_pool->CurrentItem();
      }
		
  }  
   for( j=0; j<number_edge; j++)
   {	  
		  bool deleteEdge=false;
		  deleteEdge= new_pool[j]->Normalize( );
          if(deleteEdge)
          { 
			Hyperedge* edge= new_pool[j];
	        PurgeEdge( edge);
		  }
       
   }   
*/

   // save non-stub hyperedges first, then stubs to avoid forward references
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() != STUB )
	      edge_pool->CurrentItem()->SaveCSMXML( fp );
   }
 
    
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB )
	      edge_pool->CurrentItem()->SaveCSMXML( fp );
   }


   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB )
	 ((Stub *)(edge_pool->CurrentItem()))->SaveContinuityBindings( fp );
   }
   
}

void Hypergraph::SaveDXL( FILE *fp )     //added by Bo Jiang for graph info exporting in DXL, August, 2004
{
   Hyperedge *loop_inputs[100];
   int i, number_loops = 0;

   // reorder edge pool list to place empty hyperedges which are loop inputs of loop hyperedges
   // at end of list. This ensures that the loop input path is always the second source node
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == LOOP ){
	 loop_inputs[number_loops++] = edge_pool->CurrentItem()->SourceSet()->Get( 2 )->PreviousEdge(); // find the empty point entering the loop input
     }
   }

   for( i = 0; i < number_loops; i++ ) {
      edge_pool->Delete( loop_inputs[i] ); // remove from wherever it is in the list
      edge_pool->Add( loop_inputs[i] );    // add at end of list
   }

   // save non-stub hyperedges first, then stubs to avoid forward references
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType()== RESPONSIBILITY_REF ){
	 edge_pool->CurrentItem()->SaveDXL( fp );
      } 
   }

   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB ){
	 edge_pool->CurrentItem()->SaveDXL( fp );
     } 
   }

}

Responsibility * Hypergraph::FindResponsibility( int resp_id )
{
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->LoadIdentifier() == resp_id )
	 return( (Responsibility *)(edge_pool->CurrentItem()) );
   }
   return 0;
}

void Hypergraph::ConnectHyperedges( int source_id, int target_id )
{
   Hyperedge *source_hyperedge, *target_hyperedge;
   edge_type source_type, target_type;
   nodeColour colour;
   Node *new_node;
   
   // find source and target hyperedges
   source_hyperedge = FindHyperedge( source_id );
   target_hyperedge = FindHyperedge( target_id );
   
   // determine edge types of source and target hyperedges and thus proper node colour
   source_type = source_hyperedge->EdgeType();
   target_type = target_hyperedge->EdgeType();

   if( source_type == CONNECTION )
      colour = C;
   else if( target_type == CONNECTION || target_type == ABORT )
      colour = D;
   else if( source_type != EMPTY )
      colour = B;
   else
      colour = A;
   
   // create node, install as target of source hyperedge and as source of target hyperedge
   new_node = new Node( colour );
   source_hyperedge->AttachTarget( new_node );
   target_hyperedge->AttachSource( new_node );
}

Hyperedge * Hypergraph::FindHyperedge( int hyperedge_id )
{
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->LoadIdentifier() == hyperedge_id )
	 return( edge_pool->CurrentItem() );
   }
   return 0;
}

void Hypergraph::SavePluginBindings( FILE *fp )
{
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB )
	 ((Stub *)edge_pool->CurrentItem())->SaveBindings( fp );
   }
}

#ifdef DEBUG
void Hypergraph::Output()
{
   for ( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() )
      edge_pool->CurrentItem()->Output();
}
#endif

