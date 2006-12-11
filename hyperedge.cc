/***********************************************************
 *
 * File:			hyperedge.cc
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "hyperedge.h"
#include "hypergraph.h"
#include "waiting_place.h"
#include "empty.h"
#include "figure.h"
#include "xml_generator.h"
#include "print_mgr.h"
#include "msc_generator.h"
#include "component_ref.h"
#include "utilities.h"
#include <iostream>
#include "scenario_generator.h"
#include "action.h"
#include "resourcerelease.h"
#include "resourceacquire.h"
#include "resp_ref.h"
#include "dummy.h"

extern void SetCurrentPath( Path *new_path );
extern void UpdateResponsibilityList();
extern void ResetConditions();
extern void SetPromptUserForSave();

int Hyperedge::number_hyperedges = 0;
Node *Hyperedge::search_node;
bool Hyperedge::joined_paths;

extern Label *current_label;

static char hyperedge_names[][17] = {

   "Connect",
   "Abort",
   "Empty Point",
   "Or Fork",
   "Or Join",
   "Responsibility",
   "Result",
   "Start",
   "Stub",
   "Synchronization",
   "Timer",
   "Wait",
   "Timestamp Point",
   "Goal Tag",
   "Loop"
};

Hyperedge::Hyperedge()
{ 
   source = new Cltn<Node *>; 
   target = new Cltn<Node *>;
   parent_map = display_manager->CurrentMap();
   parent_map->MapHypergraph()->RegisterEdge( this );
 
   hyperedge_number = number_hyperedges++;
   unique_desc = NULL;
   ResetVisited();
}

Hyperedge::~Hyperedge()
{ 
   if( figure ) delete figure;
   if( unique_desc ) free( unique_desc );

   delete source;
   delete target;
   
   figure = NULL;
   source = NULL;
   target = NULL;
   hyperedge_number = 0;   
}

const char * Hyperedge::EdgeName()
{
   return( hyperedge_names[EdgeType()] );
}

Label *Hyperedge::PathLabel()
{
   return( source->GetFirst()->PreviousEdge()->PathLabel() );
}

void Hyperedge::SetLabel( Label *new_label ) {}

void Hyperedge::AttachSource( Node *new_node )
{
   if ( !source->Includes( new_node ) )
      source->Add( new_node );
   else
      std::cerr << "Source node to add already included.\n";

   new_node->AttachNext( this );
}

void Hyperedge::AttachTarget( Node *new_node )
{ 
   if ( !target->Includes( new_node ) )
      target->Add( new_node );
   else
      std::cerr << "Target node to add already included.\n";
 
   new_node->AttachPrevious( this );
}

void Hyperedge::DetachSource( Node *node )
{ 
   if ( source->Includes(node) ){
      node->DetachNext();
      source->Remove( node );
   }
   else
      std::cerr << "Source node to remove doesn't exist.\n";  
}

void Hyperedge::DetachTarget( Node *node )
{ 
   if ( target->Includes(node) ){
      node->DetachPrevious();
      target->Remove( node );
   }
   else
      std::cerr << "Target node to remove doesn't exist.\n";  
}
 
bool Hyperedge::HasSourceColour( nodeColour colour )
{ 
   for ( source->First(); !source->IsDone(); source->Next() ){ 
      if ( source->CurrentItem()->GetColour() == colour )
	 return TRUE;
   }
  
   return FALSE;
}

bool Hyperedge::HasTargetColour( nodeColour colour )
{ 
   for ( target->First(); !target->IsDone(); target->Next() ){ 
      if ( target->CurrentItem()->GetColour() == colour )
	 return TRUE;
   }
  
   return FALSE;
}

Node * Hyperedge::SourceOfColour( nodeColour colour )
{ 
   for ( source->First(); !source->IsDone(); source->Next() ){ 
      if ( source->CurrentItem()->GetColour() == colour )
	 return( source->CurrentItem() );
   }
   return 0;
} 
 
Node * Hyperedge::TargetOfColour( nodeColour colour )
{ 
   for ( target->First(); !target->IsDone(); target->Next() ){ 
      if ( target->CurrentItem()->GetColour() == colour )
	 return( target->CurrentItem() );
   }
   return 0;
} 

Node * Hyperedge::PrimarySource()
{
   for ( source->First(); !source->IsDone(); source->Next() ){ 
      if ( source->CurrentItem()->GetColour() != C )
	 return( source->CurrentItem() );
   }
   return 0;
}

Node * Hyperedge::PrimaryTarget()
{
   for ( target->First(); !target->IsDone(); target->Next() ){ 
      if ( target->CurrentItem()->GetColour() != D )
	 return( target->CurrentItem() );
   }
   return 0;
}

Node * Hyperedge::ExceptionPath()
{
   for ( target->First(); !target->IsDone(); target->Next() ){ 
      if ( target->CurrentItem()->GetColour() == D )
	 return( target->CurrentItem() );
   }

   return( NULL ); // no exception path exists
}

bool Hyperedge::DeletePath()
{  
   if( QuestionDialog( "Confirm Path Deletion", "Do you wish to delete this path ?", "", "Delete Path", "Cancel Operation" ) == TRUE ) {
      this->DeletePathSection();
      parent_map->MapHypergraph()->EmptyAllTrash();
      display_manager->SetPath( NULL );
      UpdateResponsibilityList();
      return TRUE;
   }
   else
      return FALSE;
}

void Hyperedge::DeletePathSection()
{ 
#if defined( TRACE ) 
   Trace trace( __FILE__, __LINE__, "void Hyperedge::DeletePathSection()" ); 
#endif /* TRACE */ 

   // delete the path containing this edge. Connected or unrelated paths
   // are not affected.
   Hypergraph *graph = parent_map->MapHypergraph();
   Node *node;
   Path *path;
   bool quit_loop = FALSE;

   target->First();
   
   while( !quit_loop ) {

      if( target->IsDone() )
	 break;

      node = target->CurrentItem();
      if( !graph->NodeTrash()->Includes( node ) ){
	 if( node->GetColour() == D ){ 
	    if( node->NextEdge() != 0 ) {
	       if( node->NextEdge()->EdgeType() == CONNECTION ) {  // delete the node, connect and C node and return as this edge has no more target nodes
		  ((WaitingPlace *)(node->NextEdge()->FirstOutput()))->DisconnectPath( this, EXECUTE );
		  quit_loop = TRUE;
	       }
	       else { // type == ABORT continue with path deletion
		  display_manager->DeleteTimeoutPath( (Timer *)node->PreviousEdge() );
		  graph->TrashNode( node ); 
		  node->NextEdge()->DeletePathSection();  
	       }
	    }
	 }
	 else {
	    graph->TrashNode( node ); 
	    node->NextEdge()->DeletePathSection();  
	 }
      }

      if( !quit_loop )
	 target->Next();
   }

   if( EdgeType() == LOOP ) {
      quit_loop = FALSE;
      source->First();
   
      while( !quit_loop ) {

	 if( source->IsDone() )
	    break;

	 node = source->CurrentItem();
	 if( !graph->NodeTrash()->Includes( node ) ){
	    if( node->GetColour() == C ){ 
	       if( node->PreviousEdge() != 0 ) {
		  if( node->PreviousEdge()->EdgeType() == CONNECTION ) {  // delete node, connect and C node and return as this edge has no more source nodes
		     ((WaitingPlace *)(node->PreviousEdge()->FirstInput()))->DisconnectPath( this, EXECUTE );
		     quit_loop = TRUE;
		  }
		  else { // type == ABORT continue with path deletion
		     display_manager->DeleteTimeoutPath( (Timer *)node->PreviousEdge() );
		     graph->TrashNode( node ); 
		     node->PreviousEdge()->DeletePathSection();  
		  }
	       }
	    }
	    else {
	       graph->TrashNode( node ); 
	       node->PreviousEdge()->DeletePathSection();  
	    }
	 }

	 if( !quit_loop )
	    source->Next();
      }
   }

   if( graph->TrashEdge( this ) ) {
      if( figure ) {
	 if( (path = figure->GetPath()) != NULL )
	    path->Destroy(); // mark all paths for deletion
      }
   }
} 

int Hyperedge::Search( search_direction dir )
{
   Node *node;

   if( visited == TRUE ) // if this edge has already been searched stop this branch of the search
      return( QUIT );
   
   SetVisited();  // set this edge as being visited

   if( dir == FORWARD_SEARCH || dir == BIDIRECTIONAL ) {
      for( target->First(); !target->IsDone(); target->Next() ){
	 node = target->CurrentItem();
	 if( node->GetColour() != D && node->GetColour() != C ) {
	    Hyperedge::search_node = node;
	    if( node->NextEdge()->Search( FORWARD_SEARCH ) == JOINED_PATHS ) return( JOINED_PATHS ); // if another Start was found connected stop search
	 }
      }
   }
   
   if( dir == BACKWARD_SEARCH || dir == BIDIRECTIONAL )  {
      for( source->First(); !source->IsDone(); source->Next() ){
	 node = source->CurrentItem();
	 if( node->GetColour() != D && node->GetColour() != C ) {
	    Hyperedge::search_node = node;
	    if( node->PreviousEdge()->Search( BACKWARD_SEARCH ) == JOINED_PATHS ) return( JOINED_PATHS ); // if another Start was found connected stop search
	 }
      }
   }  
      return( QUIT );
}

void Hyperedge::Transform( transformation trans )
{ 
   execution_flag execute = TRUE;
   if( Perform( trans, execute ) == MODIFIED )
      SetPromptUserForSave();
}

bool Hyperedge::Validate( transformation trans )
{ 
   execution_flag execute = FALSE;
   return( Perform( trans, execute ) );
}

void Hyperedge::TransformDouble( transformation trans, Hyperedge *edge )
{ 
   execution_flag execute = TRUE;
   if( PerformDouble( trans, edge, execute ) == MODIFIED )
      SetPromptUserForSave();
}

bool Hyperedge::ValidateDouble( transformation trans, Hyperedge *edge )
{ 
   execution_flag execute = FALSE;
   return( PerformDouble( trans, edge, execute ) );
}

//  saves common hyperedge data and calls edge's specific SaveXMLDetails() routine
void Hyperedge::SaveXML( FILE *fp )
{
   char buffer[100];
   float t_fx, t_fy;

   sprintf( buffer, "<hyperedge hyperedge-id=\"h%d\" ", hyperedge_number );
   PrintXMLText( fp, buffer );
   if( this->HasName() )
      fprintf( fp, "hyperedge-name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
   if( figure ) {
      figure->GetPosition( t_fx, t_fy );
      fprintf( fp, "fx=\"%f\" fy=\"%f\" ", t_fx/VIRTUAL_SCALING, t_fy/VIRTUAL_SCALING );
   }
   if( PrintManager::TextNonempty( unique_desc ) )
      fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );

   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );
    
   // save hyperedge's specific details
   SaveXMLDetails( fp );
   
   PrintEndXMLBlock( fp, "hyperedge" );
   LinebreakXML( fp );

}

void Hyperedge::SaveCSMXML( FILE *fp )
{
   char buffer[100];
   //float t_fx, t_fy;
  
   if ( EdgeType() == START)
   {
		sprintf( buffer, "<Start id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
        // Removed. Start has no name in CSM.
		//		if( this->HasName() )
		//		    fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
		//		else
		//            fprintf( fp, "name=\" \" ");
		//if( PrintManager::TextNonempty( unique_desc ) )
		//fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );
		if( target->Size() > 0 ){
           for( target->First(); !target->IsDone(); target->Next() )
           {
	          sprintf( buffer, "target= \"h%d\" ", target->CurrentItem()->NextEdge()->GetNumber() );
	          fprintf( fp, buffer );
	       }
		}
	    fprintf( fp, ">\n" );
		IndentNewXMLBlock( fp );
		SaveCSMXMLDetails( fp );
		PrintEndXMLBlock( fp, "Start" );
		LinebreakXML( fp );
   }

	
if ( EdgeType() == EMPTY)
   {
		
		sprintf( buffer, "<Sequence id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
        // Removed. Sequence has no name in CSM.
		// if( this->HasName() )
		//     fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
        // else
        //     fprintf( fp, "name=\" \" ");
		//if( PrintManager::TextNonempty( unique_desc ) )
		//fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );
		SaveCSMXMLConnections(fp);
		fprintf( fp, "/>\n" );
        
		//IndentNewXMLBlock( fp );
		//SaveCSMXMLDetails( fp );
		//PrintEndXMLBlock( fp, "Sequence" );
		LinebreakXML( fp );
		//SaveCSMResourceRelease( fp );
   }

if ( EdgeType() == DUMMY)
   {
		
		sprintf( buffer, "<Step id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
		fprintf( fp, "name=\" Dummy Step\" ");
		SaveCSMXMLConnections(fp);
		fprintf( fp, "/>\n" );
        
		//IndentNewXMLBlock( fp );
		//SaveCSMXMLDetails( fp );
		//PrintEndXMLBlock( fp, "Sequence" );
		LinebreakXML( fp );
		//SaveCSMResourceRelease( fp );
   }


if ( EdgeType() == RESOURCEACQUIRE)
  {
	sprintf ( buffer, "<ResourceAcquire id=\"h%d\" acquire=\"c%d\" ", hyperedge_number, this->GetAcquiredComponent()->GetComponentNumber() );
	PrintXMLText (fp, buffer);
    //SaveCSMXMLConnections(fp);
   //output sucessor and predecessor for responsibility
   if( source->Size() > 0 ){
           for( source->First(); !source->IsDone(); source->Next() )
           {
	          sprintf( buffer, "predecessor= \"h%d\" ", source->CurrentItem()->PreviousEdge()->GetNumber() );
	          fprintf( fp, buffer );
	       }
		}
   if( target->Size() > 0 ){
           for( target->First(); !target->IsDone(); target->Next() )
           {
	          sprintf( buffer, "successor= \"h%d\" ", target->CurrentItem()->NextEdge()->GetNumber() );
	          fprintf( fp, buffer );
	       }
		}
  
  	fprintf (fp, "/>\n" );
	LinebreakXML (fp );

  }

 

if ( EdgeType() == RESPONSIBILITY_REF)
   {
		sprintf( buffer, "<Step id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
		//SaveCSMXMLConnections(fp);
		//fprintf( fp, ">\n" );
        //IndentNewXMLBlock( fp );
		SaveCSMXMLDetails( fp );
		//PrintEndXMLBlock( fp, "Step" );
		LinebreakXML( fp );
        
    }

if ( EdgeType() == RESOURCERELEASE)
  {
	sprintf ( buffer, "<ResourceRelease id=\"h%d\" release=\"c%d\" ", hyperedge_number, this->GetReleasedComponent()->GetComponentNumber() );
	PrintXMLText (fp, buffer);
    //SaveCSMXMLConnections(fp);
   //output sucessor and predecessor for responsibility
   if( source->Size() > 0 ){
           for( source->First(); !source->IsDone(); source->Next() )
           {
	          sprintf( buffer, "predecessor= \"h%d\" ", source->CurrentItem()->PreviousEdge()->GetNumber() );
	          fprintf( fp, buffer );
	       }
		}
   if( target->Size() > 0 ){
           for( target->First(); !target->IsDone(); target->Next() )
           {
	          sprintf( buffer, "successor= \"h%d\" ", target->CurrentItem()->NextEdge()->GetNumber() );
	          fprintf( fp, buffer );
	       }
		}
  
  	fprintf (fp, "/>\n" );
	LinebreakXML (fp );

  }


 if ( EdgeType() == SYNCHRONIZATION)
   {
	   if( source->Size() == 1 ){
	    sprintf( buffer, "<Fork id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
		if( this->HasName() )
			fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
		else
			fprintf (fp, "name=\" \" ");
		//if( PrintManager::TextNonempty( unique_desc ) )
		//fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );
		SaveCSMXMLMulPathConnections(fp);
		LinebreakXML( fp );
	   }
	   else if( target->Size() == 1 ){
        sprintf( buffer, "<Join id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
		if( this->HasName() )
			fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
		else
			fprintf (fp, "name=\" \" ");
		//if( PrintManager::TextNonempty( unique_desc ) )
		//fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );
		//fprintf( fp, ">\n" );
        //IndentNewXMLBlock( fp );
		SaveCSMXMLMulPathConnections(fp);
		//SaveCSMXMLDetails( fp );
		//PrintEndXMLBlock( fp, "Join" );
		LinebreakXML( fp );
	   }
   }
     if ( EdgeType() == OR_JOIN)
      {
		sprintf( buffer, "<Merge id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
		if( this->HasName() )
			fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
		else
			fprintf( fp, "name=\" \" ");
		//if( PrintManager::TextNonempty( unique_desc ) )
		//fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );
		//SaveCSMXMLConnections(fp);
		//fprintf( fp, ">\n" );
        
		//IndentNewXMLBlock( fp );
        SaveCSMXMLMulPathConnections(fp);
		//SaveCSMXMLDetails( fp );
		//PrintEndXMLBlock( fp, "Merge" );
		LinebreakXML( fp );
    }
    if ( EdgeType() == OR_FORK)
    {
		sprintf( buffer, "<Branch id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
		if( this->HasName() )
		    fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
		else
			fprintf( fp, "name=\" \" ");
		//if( PrintManager::TextNonempty( unique_desc ) )
		//	fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );
		//SaveCSMXMLConnections(fp);
		//fprintf( fp, ">\n" );
        //IndentNewXMLBlock( fp );
        SaveCSMXMLMulPathConnections(fp);
		//SaveCSMXMLDetails( fp );
		//PrintEndXMLBlock( fp, "Branch" );
		LinebreakXML( fp );
   }

   if ( EdgeType() == RESULT)
	 {
		sprintf( buffer, "<End id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
        // Removed. End has no name in CSM.
		// if( this->HasName() )
		//     fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
        // else
        //     fprintf( fp, "name=\" \" ");
		//if( PrintManager::TextNonempty( unique_desc ) )
		//fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );
        if( source->Size() > 0 ){
           for( source->First(); !source->IsDone(); source->Next() )
           {
	          sprintf( buffer, "source= \"h%d\" ", source->CurrentItem()->PreviousEdge()->GetNumber() );
	          fprintf( fp, buffer );
	       }
		}
		fprintf( fp, "/>\n" );
		//IndentNewXMLBlock( fp );
		//SaveCSMXMLDetails( fp );
		//PrintEndXMLBlock( fp, "End" );
		LinebreakXML( fp );
     }


   if ( EdgeType() == STUB)
    {
		sprintf( buffer, "<Step id=\"h%d\" ", hyperedge_number );
		PrintXMLText( fp, buffer );
		if( this->HasName() )
		    fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
		else
			fprintf( fp, "name=\" \" ");
		//SaveCSMXMLConnections(fp);
		//fprintf( fp, ">\n" );
        //IndentNewXMLBlock( fp );
        SaveCSMXMLMulPathConnections(fp);
		SaveCSMXMLDetails( fp );
		//PrintEndXMLBlock( fp, "Refinement" );
		LinebreakXML( fp );
   }
   
/*
   sprintf( buffer, "<PathConnection id=\"h%d\" ", hyperedge_number );
   PrintXMLText( fp, buffer );
   if( this->HasName() )
    fprintf( fp, "name=\"%s\" ", PrintDescription( this->HyperedgeName() ) );
   //if( figure ) {
     // figure->GetPosition( t_fx, t_fy );
     // fprintf( fp, "fx=\"%f\" fy=\"%f\" ", t_fx/VIRTUAL_SCALING, t_fy/VIRTUAL_SCALING );
  // }
   if( PrintManager::TextNonempty( unique_desc ) )
      fprintf( fp, "description=\"%s\" ", PrintDescription( unique_desc ) );

   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );
    
   // save hyperedge's specific details
   SaveXMLDetails( fp );
   
   PrintEndXMLBlock( fp, "PathConnection" );
   LinebreakXML( fp );
*/
}

void Hyperedge::SaveXMLConnections( FILE *fp )
{
   char buffer[100];
   
   if( target->Size() > 0 )
   {
      sprintf( buffer, "hyperedge-connection source-hyperedge=\"h%d\" ", hyperedge_number );
      PrintNewXMLBlock( fp, buffer );

      for( target->First(); !target->IsDone(); target->Next() )
      {
	 sprintf( buffer, "hyperedge-ref hyperedge-id=\"h%d\" /", target->CurrentItem()->NextEdge()->GetNumber() );
	 PrintXML( fp, buffer );
      }
      
      PrintEndXMLBlock( fp, "hyperedge-connection" );
   }
}

void Hyperedge::SaveCSMXMLConnections( FILE *fp )
{
   char buffer[100];
   if( source->Size() > 0 )
   {  
      for( source->First(); !source->IsDone(); source->Next() )
      {
	    sprintf( buffer, "source= \"h%d\" ", source->CurrentItem()->PreviousEdge()->GetNumber() );
	    fprintf( fp, buffer );
      }
   }       
   
   if( target->Size() > 0 )
   {  
      for( target->First(); !target->IsDone(); target->Next() )
      {
	    sprintf( buffer, "target= \"h%d\" ", target->CurrentItem()->NextEdge()->GetNumber() );
	    fprintf( fp, buffer );
      }
          
   }
   
}

void Hyperedge::SaveCSMXMLMulPathConnections( FILE *fp )
{
   char buffer[100];
   
   if( source->Size() == 1 )
   {      	  
      for( source->First(); !source->IsDone(); source->Next() )
        {
	    sprintf( buffer, "source= \"h%d\" ", source->CurrentItem()->PreviousEdge()->GetNumber() );
	    fprintf( fp, buffer );
        //LinebreakXML( fp );
        }
      sprintf(buffer, "target=\" " );
      fprintf( fp, buffer );
	  
      for( target->First(); !target->IsDone(); target->Next() )
      {
        sprintf( buffer,"h%d ",target->CurrentItem()->NextEdge()->GetNumber() );
		fprintf( fp,buffer );
        //LinebreakXML( fp );
      }
      fprintf( fp, " \"/> " );
      LinebreakXML( fp );
     
   }

   if( target->Size() == 1 && source->Size() !=1 ) {

      sprintf(buffer, "source =\" " );
      fprintf( fp, buffer );
      for( source->First(); !source->IsDone(); source->Next() )
      {
	    sprintf( buffer, "h%d ", source->CurrentItem()->PreviousEdge()->GetNumber() );
	    fprintf( fp, buffer );
      }
       fprintf( fp, " \" ");
     for( target->First(); !target->IsDone(); target->Next() )
        {
	      sprintf( buffer, "target= \"h%d\" ", target->CurrentItem()->NextEdge()->GetNumber() );
	      fprintf( fp, buffer );
        //LinebreakXML( fp );
        }
      fprintf( fp, " /> " );
      LinebreakXML( fp );
     
   }
}

bool Hyperedge::Normalize( )
{

    if(EdgeType()== RESPONSIBILITY_REF ||EdgeType()== RESOURCEACQUIRE ||EdgeType()== RESOURCERELEASE )
	{
	  if(source->GetFirst()->PreviousEdge()->EdgeType()==RESPONSIBILITY_REF ||source->GetFirst()->PreviousEdge()->EdgeType()== RESOURCEACQUIRE ||source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCERELEASE)
	  {
        Empty * new_empty= new Empty;
	    Node *node1= new Node(A);
	    Node *pnode=source->GetFirst();
        new_empty->AttachSource(pnode);
	    new_empty->AttachTarget(node1);
	    source->Remove(pnode);
	    AttachSource(node1);
	  }
	  
	  if(target->GetFirst()->NextEdge()->EdgeType()==RESPONSIBILITY_REF||source->GetFirst()->PreviousEdge()->EdgeType()== RESOURCEACQUIRE ||source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCERELEASE)
	  {
	    Empty * new_empty= new Empty;
	    Node *node1= new Node(B);
	    Node *nnode=target->GetFirst();
        new_empty->AttachSource(node1);
	    new_empty->AttachTarget(nnode);
	    target->Remove(nnode);
	    AttachTarget(node1);
	  }

	}

    
	if(EdgeType()==EMPTY)
    {
       if(source->GetFirst()->PreviousEdge()->EdgeType() == EMPTY)
	   {
	      Node *front_node = source->GetFirst();
          front_node->PreviousEdge()->ReplaceTarget( front_node, target->GetFirst() );
		  source->Remove(front_node);
          //Hypergraph *graph = parent_map->MapHypergraph();
          //graph->PurgeNode( front_node );
          //graph->PurgeEdge( this );
		  return true;  
       
	   }
      
      //else if(target->GetFirst()->NextEdge()->EdgeType()!=RESPONSIBILITY_REF && target->GetFirst()->NextEdge()->EdgeType()!= RESOURCEACQUIRE && target->GetFirst()->NextEdge()->EdgeType()!=RESOURCERELEASE && target->GetFirst()->NextEdge()->EdgeType()!= STUB) 
	   else if (target->GetFirst()->NextEdge()->EdgeType() == EMPTY)
	   {

	      Node *front_node = target->GetFirst();
          front_node->NextEdge()->ReplaceSource( front_node, source->GetFirst() );
		  target->Remove(front_node);
          //Hypergraph *graph = parent_map->MapHypergraph();
          //graph->PurgeNode( front_node );
          //graph->PurgeEdge( this );
		  return true;  
	   }
	 
	   else //add or replace this empty point with a dummy resp
	   {
	     if((source->GetFirst()->PreviousEdge()->EdgeType()!=RESPONSIBILITY_REF && source->GetFirst()->PreviousEdge()->EdgeType()!= RESOURCEACQUIRE && source->GetFirst()->PreviousEdge()->EdgeType()!=RESOURCERELEASE && source->GetFirst()->PreviousEdge()->EdgeType()!= STUB) && 
	     (target->GetFirst()->NextEdge()->EdgeType()!=RESPONSIBILITY_REF && target->GetFirst()->NextEdge()->EdgeType()!= RESOURCEACQUIRE && target->GetFirst()->NextEdge()->EdgeType()!=RESOURCERELEASE && target->GetFirst()->NextEdge()->EdgeType()!= STUB ))
	    {
	     
         
		 Dummy * new_ref = new Dummy;
		 
		 Node *node1= new Node(B);
		 Node *node2= new Node(A);
		 
         new_ref->AttachSource(node1);
	     new_ref->AttachTarget(node2);
	     
		 Node *front_node = source->GetFirst();
		 Node *end_node = target->GetFirst();
		 Hyperedge * prev_edge = front_node->PreviousEdge();
		 Hyperedge * next_edge = end_node->NextEdge();
		 
		 next_edge ->ReplaceSource(end_node,node2);
		 prev_edge ->ReplaceTarget(front_node,node1);
		 
		 return true; 
        }
       
       if((source->GetFirst()->PreviousEdge()->EdgeType()!=RESPONSIBILITY_REF && source->GetFirst()->PreviousEdge()->EdgeType()!= RESOURCEACQUIRE && source->GetFirst()->PreviousEdge()->EdgeType()!=RESOURCERELEASE && source->GetFirst()->PreviousEdge()->EdgeType()!= STUB) && 
	     (target->GetFirst()->NextEdge()->EdgeType()==RESPONSIBILITY_REF || target->GetFirst()->NextEdge()->EdgeType()== RESOURCEACQUIRE || target->GetFirst()->NextEdge()->EdgeType()==RESOURCERELEASE || target->GetFirst()->NextEdge()->EdgeType()== STUB ))
	   {
     //if(source->GetFirst()->PreviousEdge()->EdgeType()==RESPONSIBILITY_REF ||source->GetFirst()->PreviousEdge()->EdgeType()== RESOURCEACQUIRE ||source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCERELEASE)
	  //{
        Dummy * new_dummy= new Dummy;
	    Node *node1= new Node(A);
	    Node *pnode=source->GetFirst();
        new_dummy->AttachSource(pnode);
	    new_dummy->AttachTarget(node1);
	    source->Remove(pnode);
	    AttachSource(node1);
	  }
      
	  if((source->GetFirst()->PreviousEdge()->EdgeType()==RESPONSIBILITY_REF || source->GetFirst()->PreviousEdge()->EdgeType()== RESOURCEACQUIRE || source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCERELEASE || source->GetFirst()->PreviousEdge()->EdgeType()== STUB) && 
	     (target->GetFirst()->NextEdge()->EdgeType()!=RESPONSIBILITY_REF && target->GetFirst()->NextEdge()->EdgeType()!= RESOURCEACQUIRE && target->GetFirst()->NextEdge()->EdgeType()!=RESOURCERELEASE && target->GetFirst()->NextEdge()->EdgeType()!= STUB ))
	   {
	  //if(target->GetFirst()->NextEdge()->EdgeType()==RESPONSIBILITY_REF||source->GetFirst()->PreviousEdge()->EdgeType()== RESOURCEACQUIRE ||source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCERELEASE)
	  //{
	    Dummy * new_dummy= new Dummy;
	    Node *node1= new Node(B);
	    Node *nnode=target->GetFirst();
        new_dummy->AttachSource(node1);
	    new_dummy->AttachTarget(nnode);
	    target->Remove(nnode);
	    AttachTarget(node1);
	  }
	 
	 }
	   
      
	}
	
  
	
	return false;
}

void Hyperedge::AddResourceAcquire (Component* comp )
{
	Resourceacquire *RA = new Resourceacquire();
	RA->SetAcquiredComponent(comp);
	Node *pnode=source -> GetFirst();
	Node *node2= new Node(A);
	RA-> AttachSource(pnode);
	RA-> AttachTarget(node2);
	source->Remove(pnode);
	AttachSource(node2);

}


void Hyperedge::AddResourceRelease (Component* comp, ComponentReference* comp_ref )
{
	Resourcerelease * RR = new Resourcerelease();
	RR->SetReleasedComponent(comp);
	RR->SetReleasedComponentRef(comp_ref);
    Node *pnode=source -> GetFirst();
	Node *node2= new Node(A);
	RR-> AttachSource(pnode);
	RR-> AttachTarget(node2);
	source->Remove(pnode);
	AttachSource(node2);

}


void Hyperedge::AddCSMResourceAcquire ( )
{
	bool sameComp= false;
    Component* comp = NULL;
	Component* comp_prev = NULL;
    Component* comp_parent = NULL;
    ComponentReference* compref_parent= NULL;
    ComponentReference* curr_comp=NULL;
    Cltn<ComponentReference* > curr_comp_parent;
    Cltn<ComponentReference* > prev_comp_parent;
   
    ComponentReference* comp_ref = this->GetFigure()->GetContainingComponent();
	if(!comp_ref)
	  return;

	// check if the current responsibility resides in a component
   	comp = comp_ref->ReferencedComponent();
	
	if(source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCEACQUIRE)
		return;
    
	if(source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCERELEASE)
	{
	  ComponentReference* comp_ref_prev= source->GetFirst()->PreviousEdge()->GetReleasedComponentRef();
      if(comp_ref_prev)
		{
           comp_prev = comp_ref_prev->ReferencedComponent();
                			  
		  //check if current edge and previous edge are in the same component-reference
		  if(comp_ref_prev==comp_ref)
			    sameComp=true;
		  else //they are in different component, find the difference
		  {
             //find out the parent components for the current component, and store them in the collection curr_comp_parent
         	 curr_comp_parent.AddB(comp_ref); //add current compoent to the stack bottom 
             curr_comp=comp_ref;
	         while(curr_comp)
		     {
		        compref_parent=curr_comp->GetParent();
		        if(compref_parent)
                curr_comp_parent.AddB(compref_parent); // add each element in front of the collection
                curr_comp=compref_parent;
		      }
                     
		      //find out the parent component for the previous component
			  prev_comp_parent.AddB(comp_ref_prev);
			  curr_comp=comp_ref_prev;
	          while(curr_comp)
		      {
			    compref_parent=curr_comp->GetParent();
			    if(compref_parent)
                prev_comp_parent.AddB(compref_parent); // add each element in front of the collection
                curr_comp=compref_parent;
		      }
				  
		   }
				 								
	    }
        else //previous edge is not inside any component
		{
		  //find out the parent components for the current component, and store them in the collection curr_comp_parent
          curr_comp_parent.AddB(comp_ref); //add current compoent to the stack bottom 
		  curr_comp=comp_ref;
	      while(curr_comp)
		  {
		      compref_parent=curr_comp->GetParent();
		      if(compref_parent)
              curr_comp_parent.AddB(compref_parent); // add each element in front of the collection
              curr_comp=compref_parent;
		  }
		}
		 
            
	  //acquire all the parents and current components
	  if(!sameComp)
       {
	      if(comp)
	       {
	           //if the path is incoming edge, then just ignore the difference and go on.
               for( curr_comp_parent.First(); !curr_comp_parent.IsDone(); curr_comp_parent.Next())
	            {
                    compref_parent=curr_comp_parent.CurrentItem();
					bool include= prev_comp_parent.Includes(compref_parent);
                    comp_parent= compref_parent->ReferencedComponent();
		                          
			        if(!include && comp_parent)
					{
			           AddResourceAcquire(comp_parent);
					   //AddEmpty(this);
					   //deleteEdge =Normalize();
				    }
			    }
           }
       }
    return;
  }

	//Check the previous edge
    if( source->Size() > 0 )
	{
        for( source->First(); !source->IsDone(); source->Next() )
        {
			//Find if the previous edge resides in one or more components
	         ComponentReference* comp_ref_prev=(source->CurrentItem()->PreviousEdge()->GetFigure())->GetContainingComponent();
		     if(comp_ref_prev)
			 {
                  comp_prev = comp_ref_prev->ReferencedComponent();
                  			  
				 //check if current edge and previous edge are in the same component-reference
				 if(comp_ref_prev==comp_ref)
				    sameComp=true;
			     else //they are in different component, find the difference
				 {
                    //find out the parent components for the current component, and store them in the collection curr_comp_parent
                     
					 curr_comp_parent.AddB(comp_ref); //add current compoent to the stack bottom 
                     curr_comp=comp_ref;
	                 while(curr_comp)
		             {
			            compref_parent=curr_comp->GetParent();
			            if(compref_parent)
                        curr_comp_parent.AddB(compref_parent); // add each element in front of the collection
                        curr_comp=compref_parent;
		             }
                     
					 //find out the parent component for the previous component
					 prev_comp_parent.AddB(comp_ref_prev);
					 curr_comp=comp_ref_prev;
	                 while(curr_comp)
		             {
			            compref_parent=curr_comp->GetParent();
			            if(compref_parent)
                        prev_comp_parent.AddB(compref_parent); // add each element in front of the collection
                        curr_comp=compref_parent;
		             }
				  
				 }
				 								
	         }
            
			 else //previous edge is not inside any component
			 {
			    //find out the parent components for the current component, and store them in the collection curr_comp_parent
                curr_comp_parent.AddB(comp_ref); //add current compoent to the stack bottom 
				curr_comp=comp_ref;
	            while(curr_comp)
		        {
			        compref_parent=curr_comp->GetParent();
			        if(compref_parent)
                    curr_comp_parent.AddB(compref_parent); // add each element in front of the collection
                    curr_comp=compref_parent;
		        }
			 }
			 
            
	  //acquire all the parents and current components
	  
	   if(!sameComp)
       {
	      if(comp)
	       {
	           //if the path is incoming edge, then just ignore the difference and go on.
               for( curr_comp_parent.First(); !curr_comp_parent.IsDone(); curr_comp_parent.Next())
	            {
                    compref_parent=curr_comp_parent.CurrentItem();
					bool include= prev_comp_parent.Includes(compref_parent);
                    comp_parent= compref_parent->ReferencedComponent();
		                          
			        if(!include && comp_parent)
					{
			           AddResourceAcquire(comp_parent);
					   //AddEmpty(this);
					   //deleteEdge =Normalize();
				    }
			    }
           }
       }
	        
    }// end loop
	    
  } //end if
 
}//end function


void Hyperedge::AddCSMResourceRelease()
{
	bool sameComp= false;
    Component* comp_parent = NULL;
    ComponentReference* compref_parent= NULL;
    ComponentReference* curr_comp=NULL;
    Cltn<ComponentReference* > curr_comp_parent;
    Cltn<ComponentReference* > prev_comp_parent;
    
	ComponentReference* comp_ref = this->GetFigure()->GetContainingComponent();

    if(source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCEACQUIRE)
	  return;
	if(source->GetFirst()->PreviousEdge()->EdgeType()==RESOURCERELEASE)
	  return;

	
	//Check the previous edge
    if( source->Size() > 0 )
	{
        for( source->First(); !source->IsDone(); source->Next() )
        {
			//Find if the previous edge resides in one or more components
	         ComponentReference* comp_ref_prev=(source->CurrentItem()->PreviousEdge()->GetFigure())->GetContainingComponent();
		     if(comp_ref_prev)
			 {
                 //check if current component and previous component are the same or not
                 if(comp_ref)
			     {
                    if(comp_ref == comp_ref_prev)
						sameComp= true;
			     }

				 // if current component and previous component not in the same component
				 if(!sameComp)
				 {
				    prev_comp_parent.Add(comp_ref_prev);//add the previour current component in front
					//comp_prev = comp_ref_prev->ReferencedComponent();
                    curr_comp=comp_ref_prev;
	                while(curr_comp)
				    {
			            compref_parent=curr_comp->GetParent();
			            if(compref_parent)
                        prev_comp_parent.Add(compref_parent); // add each element in front of the collection
                        curr_comp=compref_parent;
		            }
				

                    if(comp_ref)
	                {
                      curr_comp_parent.Add(comp_ref); 
		               //find out the parent components for the current component, and store them in the collection curr_comp_parent
                      curr_comp=comp_ref;
	                  while(curr_comp)
		               {
			               compref_parent=curr_comp->GetParent();
			               if(compref_parent)
                           curr_comp_parent.Add(compref_parent); // add each element in front of the collection
                           curr_comp=compref_parent;
		               }
					   
		
                 	}//end if
				
				 }//end if
		     				
	         } //end if
	    
  
          //release the acquired previous components
          if(!sameComp && comp_ref_prev)
          {
	         for( prev_comp_parent.First(); !prev_comp_parent.IsDone(); prev_comp_parent.Next())
	         {
               compref_parent=prev_comp_parent.CurrentItem();
			   bool include=curr_comp_parent.Includes(compref_parent);
			 
			   if(!include)
			   {
                  comp_parent= compref_parent->ReferencedComponent();
		          if(comp_parent && compref_parent)
				  {
		 		     AddResourceRelease(comp_parent,compref_parent);
					 //AddEmpty(this);
					 //deleteEdge = Normalize();
				  }
             
		       }
	         }
	      }
      } // end for 
 
  }//end if
  
}//end function

void Hyperedge::SaveDXL( FILE *fp )
{
   char buffer[100];
   float t_fx, t_fy;

  if (EdgeType()==RESPONSIBILITY_REF){
     sprintf( buffer, "respRef( \"h%d\"", hyperedge_number );
     PrintXMLText( fp, buffer );
     if( figure ) {
       figure->GetPosition( t_fx, t_fy );
       if(figure->GetEnclosingComponent())
          fprintf( fp, ", %d, %d, \"cr%d\", ", (int)((t_fx/VIRTUAL_SCALING)*WIDTH), (int)((t_fy/VIRTUAL_SCALING)*HEIGHT), figure->GetEnclosingComponent()->ComponentReferenceNumber() );
       else
          fprintf( fp, ", %d, %d, \"\", ", (int)((t_fx/VIRTUAL_SCALING)*WIDTH), (int)((t_fy/VIRTUAL_SCALING)*HEIGHT) );
     }
     SaveDXLDetails( fp );
     LinebreakXML( fp );
  }
  if (EdgeType()==STUB){
     sprintf( buffer, "stub( \"h%d\"", hyperedge_number );
     PrintXMLText( fp, buffer );
     if( figure ) {
        figure->GetPosition( t_fx, t_fy );
        fprintf( fp, ", %d, %d, ", (int)((t_fx/VIRTUAL_SCALING)*WIDTH), (int)((t_fy/VIRTUAL_SCALING)*HEIGHT) );
     }    
    SaveDXLDetails( fp );
    LinebreakXML( fp );
  }   
}

void Hyperedge::GeneratePostScriptDescription( FILE *ps_file )
{
   if( !PrintManager::TextNonempty( unique_desc ) ) return;
   fprintf( ps_file, "%d rom (%s ) P\n", PrintManager::text_font_size, EdgeName() );   
   GeneratePSElementDescription( ps_file );
}

void Hyperedge::GeneratePSElementDescription( FILE *ps_file )
{
   if( !PrintManager::TextNonempty( unique_desc ) ) return;
   fprintf( ps_file, "1 IN ID %d rom (Description ) P OD\n", PrintManager::text_font_size );
   fprintf( ps_file, "2 IN ID\n" );
   PrintManager::PrintDescription( unique_desc );
   fprintf( ps_file, "OD\n" );
}

bool Hyperedge::HasDescription()
{
   return( PrintManager::TextNonempty( unique_desc ) );
}

void Hyperedge::EdgeSelected()
{
   ResetConditions();
}

// default behaviour

bool Hyperedge::Perform( transformation trans, execution_flag execute ){ return 0; }
bool Hyperedge::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ){ return 0; }

#ifdef DEBUG
void Hyperedge::Output()
{

   cout << "\n\nThis edge is a " << EdgeName() << " Its address is: " << this << endl;

   if( source->Size() != 0 ) {
      
      if( source->Size() == 1 )
	 cout << "Its source node and previous edge is : " << endl;
      else
	 cout << "Its source nodes and previous edges are : " << endl;
      
      for( source->First(); !source->IsDone(); source->Next() ) {
	 cout << "Node: " << source->CurrentItem() << " Previous Edge: "
	      << source->CurrentItem()->PreviousEdge()->EdgeName()
	      << " address: " << source->CurrentItem()->PreviousEdge() << endl;
      }
   }

   if( target->Size() != 0 ) {

      if( target->Size() == 1 )
	 cout << "Its target node and next edge is : " << endl;
      else
	 cout << "Its target nodes and next edges are : " << endl;
      
      for( target->First(); !target->IsDone(); target->Next() ) {
	 cout << "Node: " << target->CurrentItem() << " Next Edge: "
	      << target->CurrentItem()->NextEdge()->EdgeName()
	      << " address: " << target->CurrentItem()->NextEdge() << endl;
      }
   }

}
#endif

void Hyperedge::Description( const char *new_desc )
{
   if( new_desc != NULL ) {
      if( unique_desc != NULL ) {
	 free( unique_desc );
	 unique_desc = NULL;
      }
      if( strlen( new_desc ) > 0 )
	 unique_desc = strdup( new_desc );
   }
}

bool Hyperedge::HasName()
{
   return( FALSE );
}

const char * Hyperedge::HyperedgeName()
{
   return( NULL );
}

void Hyperedge::DeleteSimpleEdge()
{ // delete hyperedge and replace it with an empty
   display_manager->SetPath( figure->GetPath() );
   Empty *new_empty = new Empty;
   Node *node1 = new Node( B );
   Node *node2 = new Node( A );
   new_empty->AttachSource( node1 );
   new_empty->AttachTarget( node2 );
    
   Node *front_node = source->GetFirst();
   Node *end_node = target->GetFirst();
   Hyperedge *prev_edge = front_node->PreviousEdge();
   Hyperedge *next_edge = end_node->NextEdge(); 

   next_edge->ReplaceSource( end_node, node2 );
   prev_edge->ReplaceTarget( front_node, node1 );

   display_manager->Add( new_empty , this );
        
   parent_map->MapHypergraph()->PurgeSegmentFrom( front_node, end_node );
}

int Hyperedge::DeleteElement()
{ // delete hyperedge without replacing it with an empty
   Hypergraph *graph = parent_map->MapHypergraph();

   Node *front_node = source->GetFirst();
   front_node->PreviousEdge()->ReplaceTarget( front_node, target->GetFirst() );
   graph->PurgeNode( front_node );
   graph->PurgeEdge( this );
   return TRUE;
}

void Hyperedge::DeleteEdge()
{
   parent_map->MapHypergraph()->PurgeEdge( this );
}

bool Hyperedge::ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir )
{  // default method for non-empty, non multipath hyperedges
   if( figure->GetPath() == old_path ) {
      figure->SetPath( new_path );
      return TRUE;
   }
   else
      return FALSE;
}

Path * Hyperedge::InputPath()
{
   return( figure->GetPath() );
}

Hyperedge * Hyperedge::GenerateMSC( bool first_pass )
{
   return( PrimaryTarget()->NextEdge() );
}

Hyperedge * Hyperedge::HighlightScenario( )
{
   return( PrimaryTarget()->NextEdge() );
}


Hyperedge * Hyperedge::ScanScenarios( scan_type type )
{
   return( PrimaryTarget()->NextEdge() );
}

Hyperedge *  Hyperedge::TraverseScenario()
{
   return( PrimaryTarget()->NextEdge() );
}

//The following eght methods are related to the scenario generation of class Synchronization and wait_synch
bool Hyperedge::SearchRoot( Cltn<Path_data *> & waiting_element, Path_data * waiting_place ) 
{
   Path_data *  current_element;
   Path_data * new_root = NULL;
   Path_data * common_item = NULL;
   Path_data * des = NULL;
   int scanCount;
   int MaxCount = 100;
   bool incompleteSearch = false;
   
   SCENARIOGenerator::Instance();

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
      current_element = FindDataWithCount( MaxCount, waiting_element );
      if( MaxCount < waiting_element.Size() ) { 
         // unwell_nested map, partially collapse the branches to form a inner parallel block, 
	 // add one additional Action as the ending point of the inner parallel block 
	 des = AddSynchronizationPoint( current_element, waiting_element, waiting_place );
	 incompleteSearch = true; 
      }
      else { //well_nested map
         des = waiting_place;
	 waiting_place->SetRoot( current_element );
      }
   }
   
   ResetReferenceCount( waiting_element );
   Collapse( des, incompleteSearch, waiting_place ); 
   return  incompleteSearch;   
}

// This method scans down each branch of the synchronization point, sets the destination attribute of each 
// path_data element to be the parameter des, so all elements with the same destination are in the same
// parallel block
void Hyperedge::SetDestination( Path_data * des ) 
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

void Hyperedge::Collapse( Path_data * des, bool incompleteSearch, Path_data * waiting_place ) 
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
      MoveToWaitingPlace( root, des, waiting_place );
   }
   
   // scan down each branch of the synchronization's previous path, if the current element is not the root of a 
   // parallel, shift all its next path elements whose destination is not des to waiting_place.  
   for( previous_path->First(); !previous_path->IsDone(); previous_path->Next() ) {
      current_element = previous_path->CurrentItem();
      while( current_element != des->GetRoot() ) {
         sce->UpdateParallelStack( current_element, waiting_place );
         if( !isRoot ) {
	    MoveToWaitingPlace( current_element, des, waiting_place );           
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
void Hyperedge::AddForkAfter( Path_data * root, Path_data * des )
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

void Hyperedge::MoveToWaitingPlace( Path_data * root, Path_data * des, Path_data * waiting_place )
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

Path_data * Hyperedge::FindDataWithCount( int count, Cltn<Path_data *> & waiting_element )
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
Path_data *  Hyperedge::AddSynchronizationPoint( Path_data * root,  Cltn<Path_data *> & waiting_element, Path_data * waiting_place )
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
void Hyperedge::ResetReferenceCount(  Cltn<Path_data *> & waiting_element )
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
Path_data * Hyperedge::Advance(Path_data * current_data )
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


