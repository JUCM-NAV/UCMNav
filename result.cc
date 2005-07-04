/***********************************************************
 *
 * File:			result.cc
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:	July 1996
 *                              Mar 2003 
 *                              Shuhua Cui
 *
 ***********************************************************/

#include "result.h"
#include "empty.h"
#include "synchronization.h"
#include "or_join.h"
#include "hypergraph.h"
#include "point_figure.h"
#include "path.h"
#include "waiting_place.h"
#include "print_mgr.h"
#include "msc_generator.h"
#include "component_ref.h"
#include "stub.h"
#include "connect.h"
#include "loop.h"
#include <iostream>
#include "scenario.h"
#include "hyperedge.h"
#include "start.h"
#include "scenario_generator.h"
#include "action.h"
#include "wait.h"
#include "resourceacquire.h"
#include "resourcerelease.h"

extern void DrawScreen();
extern void InstallConditions( ConditionManager &conditions );
extern void ListParents( Map *map, Hyperedge *submap_endpoint = NULL );
extern void ResetConditions();
extern int showing_labels;
extern int map_deletion;

int Result::number_results = 1;

Result::Result( bool install) : Hyperedge()
{
   figure = new PointFigure( this, RESULT_FIG  );
   if( install ) figure->GetPath()->SetPathEnd( this );
   condition_manager.ConditionType( PATH_END );
   identifier[0] = 0;
   identifier[19] = 0;
   bound = FALSE;
   centered = TRUE;
   highlighted = FALSE;
}

Result::Result( int result_id, const char *name, float x, float y, const char *desc, const char *alignment ) : Hyperedge()
{
   load_number = result_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = result_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;    
   //End of the modification by Bo Jiang, April, 2005   
   
   figure = new PointFigure( this, RESULT_FIG  );
   figure->SetPosition( x, y );
   strncpy( identifier, name, 19 );
   identifier[19] = 0;
   if( desc != NULL ) unique_desc = strdup( desc );
   condition_manager.ConditionType( PATH_END );
   bound = FALSE;
   centered = TRUE;
   highlighted = FALSE;
   if( alignment != NULL ) {
      if( strequal( alignment, "flush" ) )
	 centered = FALSE;
   }
}

Result::~Result()
{
   if( figure->IsSelected() )  // remove conditions from lists if edge deleted
      ResetConditions();

   if( map_deletion ) return;  // if map is being deleted don't bother deregistering
   
   if( this->HasTargetColour( D ) ) { // break triggering connection if one exists
      Node *node_d = this->TargetOfColour( D );
      Node *node_c = node_d->NextEdge()->TargetSet()->GetFirst();
      node_c->NextEdge()->DetachSource( node_c ); // detach triggered node of start or wait point
      parent_map->MapHypergraph()->PurgeSegmentFrom( node_d, node_c );
   }

   Cltn<Stub *> *parent_stubs = parent_map->ParentStubs();

   for( parent_stubs->First(); !parent_stubs->IsDone(); parent_stubs->Next() )
      parent_stubs->CurrentItem()->RemoveExitBinding( this, parent_map );
}

bool Result::Perform( transformation trans, execution_flag execute )
{
   switch( trans ){

   case EDIT_END_NAME:
      return( EditPathEndName( execute ) );
      break;
   case FOLLOW_PATH_PARENT_MAP:
      return( FollowPathInParentMap( execute ) );
      break;
   case LIST_BOUND_PARENT_MAPS:
      return( ListBoundParentMaps( execute ) );
      break;
   case CENTER_LABEL:
      return( CenterLabel( execute ) );
      break;
   case FLUSH_LABEL:
      return( FlushLabel( execute ) );
      break;
   }
   return FALSE;
}

bool Result::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute )
{

   switch( trans ){
  
   case AND_COMPOSE:
      return( AndCompose( edge, execute ) );
      break;
   case OR_COMPOSE:
      return( OrCompose( edge, execute ) );
      break;
   case STUB_COMPOSE:
      return( StubCompose( edge, execute ) );
      break;
   }
   return FALSE;
}

int Result::DoubleClickAction()
{
   if( FollowPathInParentMap( FALSE ) )
      return( FollowPathInParentMap( TRUE ) );
   else
      return( EditPathEndName( TRUE ) );
}

bool Result::EditPathEndName( execution_flag execute )
{
   const char *name;

   if( execute ){
      name = fl_show_input( "Enter the Path End Name", identifier );
      if( name ) {
	 strncpy( identifier, name, 19 );
	 return( MODIFIED );
      } else
	 return( UNMODIFIED );
   }
   else
      return( TRUE );
}

bool Result::CenterLabel( execution_flag execute )
{
   if( execute ) {
      centered = TRUE;
      return( TRUE );
   }
   else
      return( centered == FALSE );
}

bool Result::FlushLabel( execution_flag execute )
{
   if( execute ) {
      centered = FALSE;
      return( TRUE );
   }
   else
      return( centered == TRUE );
}

bool Result::ConnectPath( Hyperedge *trigger_edge, execution_flag  execute )
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

bool Result::DisconnectPath( Hyperedge *trigger_edge, execution_flag  execute )
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

bool Result::AndCompose( Hyperedge *edge, execution_flag  execute )
{

   edge_type type = edge->EdgeType();
  
   if( execute ){  
      if( type == EMPTY )
	 (((Empty *)edge)->AddAndJoin())->ConnectForkPath( this );
      else if( type == RESULT ) { // redirect message to empty preceding result
	(((Empty *)(edge->FirstInput()))->AddAndJoin())->ConnectForkPath( this );
	edge->GetFigure()->ResetSelected();
      }
      else {		// edge = AND_JOIN
    	 Node *new_nodea = new Node( A );
	 Hyperedge *prev_edge = source->GetFirst()->PreviousEdge();
	 prev_edge->DetachTarget( source->GetFirst() );
	 prev_edge->AttachTarget( new_nodea );
	 edge->AttachSource( new_nodea );
	 display_manager->AddAndJoinBranch( (Synchronization *)edge, (Empty *)prev_edge );
	 parent_map->MapHypergraph()->PurgeNode( source->GetFirst() );
	 parent_map->MapHypergraph()->PurgeEdge( this );
      }
      return( TRUE );  // not used
   }  
   else {    // execute false, validate

      if(! ((type == EMPTY) || (type == SYNCHRONIZATION) || (type == RESULT)) )
	 return( INVALID );

      if( type == SYNCHRONIZATION ) {
	 if( ((Synchronization *)edge)->SynchronizationType() != JOIN )
	    return( INVALID );       
      }

      if( figure->GetPath() == edge->GetFigure()->GetPath() )
	 return( UNAVAILABLE );       
      
      if( !ValidComposition() )
	 return( UNAVAILABLE );
      
      return( AVAILABLE );
   }
}

bool Result::OrCompose( Hyperedge *edge, execution_flag  execute )
{
  edge_type type = edge->EdgeType();
  
   if( execute ){
      if( type == EMPTY )
	 (((Empty *)edge)->AddOrJoin())->ConnectForkPath( this );
      else if( type == RESULT ) { // redirect message to empty preceding result
	 (((Empty *)(edge->FirstInput()))->AddOrJoin())->ConnectForkPath( this );
	 edge->GetFigure()->ResetSelected();
      }
      else {
      	 Node *new_nodea = new Node( A );
	 Hyperedge *prev_edge = source->GetFirst()->PreviousEdge();
	 prev_edge->DetachTarget( source->GetFirst() );
	 prev_edge->AttachTarget( new_nodea );
	 edge->AttachSource( new_nodea );
	 display_manager->AddOrJoinBranch( (OrJoin *)edge, (Empty *)prev_edge );
	 figure->GetPath()->SetPathEnd( edge );
	 parent_map->MapHypergraph()->PurgeNode( source->GetFirst() );
	 parent_map->MapHypergraph()->PurgeEdge( this );
	 
	 // prompt user for new label for or join
	 if( showing_labels ) {
	    DrawScreen();
	    parent_map->EditLabel( edge, "Enter new label for or join output path" );
	 }
      }
      return( TRUE );  // not used
   }  
   else {    // execute false, validate

      if(! ((type == EMPTY) || (type == OR_JOIN) || (type == RESULT)) )
	 return( INVALID );
  
      if( figure->GetPath() == edge->GetFigure()->GetPath() )
	 return( UNAVAILABLE );       
      
      if( !ValidComposition() )
	 return( UNAVAILABLE );
      
      return( AVAILABLE );
   }
}
 
bool Result::StubCompose( Hyperedge *edge, execution_flag  execute )
{
  
   if( execute ){
      Node *new_nodea = new Node( A );
      Hyperedge *prev_edge = source->GetFirst()->PreviousEdge();
      prev_edge->DetachTarget( source->GetFirst() );
      prev_edge->AttachTarget( new_nodea );
      edge->AttachSource( new_nodea );
      display_manager->CreateStubJoin( this, edge );
      parent_map->MapHypergraph()->PurgeNode( source->GetFirst() );
      parent_map->MapHypergraph()->PurgeEdge( this );
      return( TRUE );
    
   }
   else { // execute false, validate only

      if( edge->EdgeType() != STUB )
	 return( INVALID );

      if( figure->GetPath() == edge->FirstOutput()->GetFigure()->GetPath() )
	 return( UNAVAILABLE );       

      if( !ValidComposition() )
	 return( UNAVAILABLE );
      
      return( AVAILABLE );
   }
}

bool Result::FollowPathInParentMap( execution_flag execute )
{
   static Empty *bound_empty;

   if( execute ){
      parent_map->HighlightSubmapExitPoint( this );
      display_manager->ContinuePathInParent( bound_empty, RESULT );
      return( UNMODIFIED );    
   } else // execute false, validate only
      return( ((bound_empty = display_manager->PathContinuedInParent( this )) != NULL) ? TRUE : FALSE );
}

bool Result::ListBoundParentMaps( execution_flag execute )
{
   if( execute ) {
      ListParents( parent_map, this );
      return( TRUE );
   } else
      return( parent_map->HasBoundParents( this ) );
}

bool Result::ValidComposition()
{
   Hyperedge *path_start = figure->GetPath()->GetPathStart();
   if( path_start->EdgeType() == LOOP ) {
      if( ((Loop *)path_start)->LoopOutputPath() == figure->GetPath() )
	 return( FALSE );
   }
   for( target->First(); !target->IsDone(); target->Next() ){
      if( target->CurrentItem()->NextEdge() != 0 )
	 return( FALSE );
   }

   return( TRUE );
}
    
void Result::EdgeSelected()
{
   InstallConditions( condition_manager );
}

void Result::PathEndName( char *new_name )
{
   strncpy( identifier, new_name, 19 );
}

Hyperedge * Result::ScanScenarios( scan_type type )
{
//   Hyperedge *triggered_edge;
   Hyperedge *return_value = NULL;
   MSCGenerator *msc = MSCGenerator::Instance();
   Hyperedge *next_edge;

   if( this->HasTargetColour( D ) ) { // check if this result triggers a start, timer or wait
      next_edge = this->TargetOfColour( D )->NextEdge()->FirstOutput();
      if( type != ADVANCE_SCAN )
	 return( next_edge );
      else {
	 if( next_edge->EdgeType() == START )
	    return( next_edge );
	 else {
	    if( next_edge == msc->SynchronizedElement() )
	       msc->SetParallelSynch();
	    return( NULL );
	 }
     }
   }

   if( msc->InsideStub() )
      return_value = msc->GetUpperLevelStub()->StubExitPath( this, parent_map );

   if( return_value == NULL ) // determine if path is an and fork path, other branches may need to be scanned
      return_value = msc->NextParallelBranch();

   if( return_value == NULL ){ // end of scenario is reached, increment count or generate actual scenario
      if( type == MSC_FIRST_PASS )
	 msc->IncrementScenarioCount();
      else if( type == MSC_GENERATION_PASS )
	 msc->GenerateScenario();
   }

   return( return_value );  
}


Hyperedge * Result::HighlightScenario( )
{
//   Hyperedge *triggered_edge;
   Hyperedge *return_value = NULL;
   MSCGenerator *msc = MSCGenerator::Instance();
   Hyperedge *next_edge;
   Stub * parent_stub;
   
   if( this->HasTargetColour( D ) ) { // check if this result triggers a start, timer or wait
      next_edge = this->TargetOfColour( D )->NextEdge()->FirstOutput();
      return( next_edge );
   }

   parent_map = this->ParentMap();
   parent_stub = parent_map->getScanningParentStub();

   if( parent_stub ) {
      return_value =  parent_stub->StubExitPath( this, parent_map );
   }

   if( return_value == NULL ) // determine if path is an and fork path, other branches may need to be scanned
      return_value = msc->NextParallelBranch();

   return( return_value ); 
}


Hyperedge * Result::GenerateMSC( bool first_pass )
{
//   Hyperedge *triggered_edge
   Hyperedge *return_value = NULL; // default is to return null as end of path has been reached
   MSCGenerator *msc = MSCGenerator::Instance();
   ComponentReference *component = NULL;

   if( this->HasTargetColour( D ) ) // check if this result triggers a start, timer or wait
      return( this->TargetOfColour( D )->NextEdge() );
   
   // determine if this path end is inside a stub and therefore the path should continue in the upper level
   if( msc->InsideStub() )
      return_value = msc->GetUpperLevelStub()->StubExitPath( this, parent_map );
   
   if( return_value == NULL ) { // send message to environment if applicable

      if( (component = figure->GetContainingComponent()) == NULL ) {
	 if( first_pass )
	    msc->EnvironmentReferenced();
      }

      if( msc->NewComponentRole( component ) ) {
	 if( first_pass && component )
	    msc->AddComponent( component );
	 if( !first_pass )
	    msc->CreateMessage( component, this );
	 if( component )
	    msc->PreviousComponent( component );
      }
      else { // this result is in same component as previous relevant element
	 if( !first_pass )
	    msc->CreateCondition( HyperedgeName() );
      }
   }

   return( return_value );   
}

void Result::SaveXMLDetails( FILE *fp )
{
   char buffer[50];

   sprintf( buffer, "end-point label-alignment=\"%s\" ",  centered ? "centered" : "flush" );
   PrintNewXMLBlock( fp, buffer );
   condition_manager.SaveXML( fp, "postcondition", "resulting-event" );
   PrintEndXMLBlock( fp, "end-point" );
}


void Result::SaveCSMXMLDetails( FILE *fp )
{
	
   char buffer[50];

   sprintf( buffer, "end label-alignment=\"%s\" ",  centered ? "centered" : "flush" );
   PrintNewXMLBlock( fp, buffer );
   condition_manager.SaveXML( fp, "postcondition", "resulting-event" );
   PrintEndXMLBlock( fp, "end" );
   
}

void Result::AddEndResourceAcquire (Component* comp )
{

	Resourceacquire *RA = new Resourceacquire();
    RA->SetAcquiredComponent(comp);
	Node *pnode=source -> GetFirst();
	Node *node1= new Node(A);
	RA-> AttachSource(pnode);
	RA-> AttachTarget(node1);
	source->Remove(pnode);
	AttachSource(node1);

}

void Result::AddEndResourceRelease (Component* comp,ComponentReference* comp_ref )
{

	Resourcerelease *RR = new Resourcerelease();
    RR->SetReleasedComponent(comp);
	RR->SetReleasedComponentRef(comp_ref);
    Node *pnode=source -> GetFirst();
	Node *node1= new Node(A);
	RR-> AttachSource(pnode);
	RR-> AttachTarget(node1);
	source->Remove(pnode);
	AttachSource(node1);
	
}

void Result::AddCSMEndResourceRelease ( )
{

	Component* comp = NULL;
	Component* comp_parent = NULL;
    ComponentReference* compref_parent= NULL;
    ComponentReference* curr_comp=NULL;
	Cltn<ComponentReference* > curr_comp_parent;
	//Cltn<ComponentReference* > prev_comp_parent;
	     
	    
	// check if the current end point resides in a component
    ComponentReference* comp_ref = this->GetFigure()->GetContainingComponent();
	if(!comp_ref)
	  return;
   
	comp = comp_ref->ReferencedComponent();
    //find out the parent components for the current component, and store them in the collection curr_comp_parent
    curr_comp_parent.Add(comp_ref);	 //add current component to the stack bottom 
	//prev_comp_parent.AddB(comp_ref); // add current component to the front
	curr_comp=comp_ref;
    while(curr_comp)
    {
        compref_parent=curr_comp->GetParent();
        if(compref_parent)
		{
            curr_comp_parent.Add(compref_parent); // add each element in front of the collection
            //prev_comp_parent.AddB(compref_parent);
		}
		curr_comp=compref_parent;
	}
			 
          
	  
	  //acquire all the parents and current components
	  
	if(comp)
	{
	
	   for( curr_comp_parent.First(); !curr_comp_parent.IsDone(); curr_comp_parent.Next())
	   {
           compref_parent=curr_comp_parent.CurrentItem();
	       comp_parent= compref_parent->ReferencedComponent();
	       if(comp_parent && compref_parent)
			  AddEndResourceRelease(comp_parent,compref_parent); 
		         
       }

       
    }	    
 

}//end function


void Result::GeneratePostScriptDescription( FILE *ps_file )
{
   bool conditions_empty = condition_manager.ConditionsEmpty(),
      has_description = HasDescription();

   if( conditions_empty &&  !has_description ) return;

   fprintf( ps_file, "%d rom (End Point - ) S\n%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size,
	    PrintManager::PrintPostScriptText( identifier ) );
   if( has_description ) GeneratePSElementDescription( ps_file );
   if( !conditions_empty ) condition_manager.GeneratePostScriptDescription( ps_file );
}

Hyperedge * Result::TraverseScenario( )
{
//   Hyperedge *triggered_edge;
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Hyperedge *next_edge = NULL;
   Stub * parent_stub;
   int hyperedge_number;
   const char * name;
   ComponentReference * component = NULL;
   Path_data *  action;
   hyperedge_number = this->GetNumber();
   name = this->HyperedgeName();
   Stub* currentStub;
   int   stubSearchLevel;
   
      
   if( this->HasTargetColour( D ) ) { // check if this result triggers a start, timer or wait
      action = new Action( hyperedge_number, name, Trigger_End );
      sce->AddPathTraversal( action );
      sce->AddPathList( action );
      next_edge = this->TargetOfColour( D )->NextEdge()->FirstOutput();
   }
   else {      
      parent_map = this->ParentMap();
      parent_stub = parent_map->getScanningParentStub();
      if( parent_stub ) {
         next_edge =  parent_stub->StubExitPath( this, parent_map );
	 if(next_edge != NULL ) {
	    action = new Action( hyperedge_number, name, Connect_End );
	    sce->AddPathTraversal( action );
	    sce->AddPathList( action );
	 }
      }
   }
   if( next_edge == NULL ) {// determine if path is an and fork path, other branches may need to be scanned
      action = new Action(hyperedge_number, name, End_Point);
      sce->AddPathTraversal( action );
      sce->AddPathList( action ); 
      next_edge = sce->NextParallelBranch();
   } 
   
   currentStub = sce->getCurrentStub();
   stubSearchLevel = 1;
     

   if( (component = figure->GetEnclosingComponent() ) != NULL ) {
         action->SetComponentName( component->ReferencedComponent()->GetLabel() );
	 action->SetComponentId( component->ReferencedComponent()->GetComponentNumber() );
	 action->SetComponentRole( component->ComponentRole() );
	 action->SetComponentReferenceId( component->ComponentReferenceNumber() );	
   }

   else {
      	 Map* parentMap;
	 Cltn <Stub *> *parentStubs;
	 Stub* stubPtr;
	 HyperedgeFigure *stubFigure;
	 Hyperedge *currHyperedgePtr;
	 int found = 0;
	 
	 currHyperedgePtr = this;
	 
	 while ( !found ) {
	 
	 	if ( (parentMap = currHyperedgePtr->ParentMap()) == NULL ) 
	 	{
	 		std::cout << "FATAL ERROR, Can't get parent MAP of current hype edge\n";
			break;
		};
		 
		parentStubs = parentMap->ParentStubs();
		  
	        if ( parentStubs == NULL)
	 	{
	 		std::cout << "DEGUG INFO: Can't get parent STUBS of map\n";
			break;
		};
		  
	 	for ( parentStubs->First(); !parentStubs->IsDone(); parentStubs->Next())
		{
		        std::cout << "CUI: the stub cltn size is: " << parentStubs->Size() << std::endl;

			std::cout <<"SearchLevel is" << stubSearchLevel<< std::endl;

		        stubPtr = parentStubs->CurrentItem();
			std::cout << stubPtr->StubName() << std::endl; 

			if (stubSearchLevel == 0) { 
			       found = 1; 
			       break; 
			}

	 		if ( stubPtr != currentStub && parentStubs->Size() > 1 )
			       continue;
			else {
			       found = 1;
			       stubSearchLevel = 0;
			       break;
			}
                }
		
		if ( !found )
	        {
	 		std::cout << "DEGUG INFO: Can't get stub ptr of the stub collection\n";
			found = 1;
			break;
		} 
	        
		if ( (stubFigure = stubPtr->GetFigure()) == NULL)
		{
		
			std::cout << "FATAL ERROR: Can't get stubFigure of stub\n";
			break;
		} 
	        
		if ( (component = stubFigure->GetContainingComponent()) == NULL )
         	{
	 		std::cout << "DEBUG INFO: GetContainingComponent found noting\n";
			currHyperedgePtr = stubPtr;
			found = 0;
	 	}
		else {
	        	action->SetComponentName( component->ReferencedComponent()->GetLabel() );
	 		action->SetComponentId( component->ReferencedComponent()->GetComponentNumber() );
	 		action->SetComponentRole( component->ComponentRole() );
	                action->SetComponentReferenceId( component->ComponentReferenceNumber() );			
			found = 1;
		}
	 }  //end while loop

    }
      
   return( next_edge ); 
}

void Result::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
