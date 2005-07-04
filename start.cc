/***********************************************************
 *
 * File:			start.cc
 * Author:			Andrew Miga
 * Created:			June 1996
 
 * Modification history:	July 1996
                                Modified Feb 2003
				Shuhua Cui
 ***********************************************************/

#include "start.h"
#include "connect.h"
#include "stub.h"
#include "hypergraph.h"
#include "point_figure.h"
#include "map.h"
#include "forms.h"
#include "print_mgr.h"
#include "msc_generator.h"
#include "scenario.h"
#include "utilities.h"
#include "variable.h"
#include <iostream>
#include <stdlib.h>
#include "scenario_generator.h"
#include "action.h"
#include "path_data.h"
#include "component_ref.h"
#include "resourceacquire.h"
#include "resourcerelease.h"

extern void EditArrivalProcess( Start * );
extern void ListParents( Map *map, Hyperedge *submap_endpoint = NULL );
extern int map_deletion;

int Start::number_starts = 1;

static char process_names[][15] = {

   "Exponential",
   "Deterministic",
   "Uniform",
   "Erlang",
   "Expert"
};

Start::Start( bool install ) : WaitingPlace()
{
   figure = new PointFigure( this, START_FIG );
   if( install ) figure->GetPath()->SetPathStart( this );
   identifier[0] = 0;
   identifier[19] = 0;
   expert = NULL;
   bound = FALSE;
   centered = TRUE;
   arrival = UNDEFINED;
   stream = OPEN_ARRIVAL;
   population_size = 0;
   annotations_complete = TRUE;
}

Start::Start( int start_id, const char *name, float x, float y, const char *desc, const char *logical_expression, arrival_type atype,
	      stream_type stype, int population, float values[], const char *ed, const char *alignment ) : WaitingPlace()
{
   load_number = start_id;
   
   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = start_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;  
   //End of the modification by Bo Jiang, April, 2005   
   
   figure = new PointFigure( this, START_FIG );
   figure->SetPosition( x, y );
   identifier[19] = 0;
   strncpy( identifier, name, 19 );
   if( desc != NULL ) unique_desc = strdup( desc );
   logical_condition = ((logical_expression != NULL) ? strdup( logical_expression ) : NULL );
   arrival = atype;
   stream = stype;
   population_size = population;
   input[0] = values[0];
   input[1] = values[1];
   expert = ((ed != NULL) ? strdup( ed ) : NULL);
   bound = FALSE;
   centered = TRUE;
   annotations_complete = TRUE;
   if( alignment != NULL ) {
      if( strequal( alignment, "flush" ) )
	 centered = FALSE;
   }
}

Start::~Start()
{
   Cltn<Stub *> *parent_stubs;
   Scenario *cs;

   if( map_deletion ) return;  // if map is being deleted don't bother
   
   parent_stubs = parent_map->ParentStubs();

   for( parent_stubs->First(); !parent_stubs->IsDone(); parent_stubs->Next() )
      parent_stubs->CurrentItem()->RemoveEntryBinding( this, parent_map );

   while( !start_scenarios.is_empty() ) {
      cs = start_scenarios.Detach();
      cs->RemoveStartingPoint( this );
   }
}

Label *Start::PathLabel()
{
   return( target->GetFirst()->NextEdge()->PathLabel() );
}

bool Start::Perform( transformation trans, execution_flag execute )
{
   switch( trans ) {

   case FOLLOW_PATH_PARENT_MAP:
      return( FollowPathInParentMap( execute ) );
      break;
   case DELETE_PATH:
      return( DeletePath( execute ) );
      break;
   case LIST_BOUND_PARENT_MAPS:
      return( ListBoundParentMaps( execute ) );
      break;
   case EDIT_PATH_INITIATION:
      return( EditPathInitiation( execute ) );
      break;
   case EDIT_START_NAME:
      return( EditWaitingName( execute ) );
      break;
   case EDIT_LOGICAL_CONDITION:
      return( EditGuardCondition( execute ) );
      break;
   case CENTER_LABEL:
      return( CenterLabel( execute ) );
      break;
   case FLUSH_LABEL:
      return( FlushLabel( execute ) );
      break;
   //case GENERATE_MSC:
   //   return( CreateMSC( execute ) );
    //  break;
   case SET_SCENARIO_START:
      return( SetAsScenarioStart( execute ) );
      break;
   case LIST_SCENARIOS:
      return( ListScenarios( execute ) );
      break;
   }      
   return FALSE;
}

bool Start::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute )
{

   switch( trans ){
  
   case MERGE:
      return( Merge( edge, execute ) );
      break;
   case CONNECT:
      return( ConnectPath( edge, execute ) );
      break;
   case DISCONNECT:
      return( DisconnectPath( edge, execute ) );
      break;
   }
   return FALSE;
}

int Start::DoubleClickAction()
{
   if( FollowPathInParentMap( FALSE ) )
      return( FollowPathInParentMap( TRUE ) );
   else
      return( EditWaitingName( TRUE ) );
}

bool Start::FollowPathInParentMap( execution_flag execute )
{
   static Empty *bound_empty;

   if( execute ){
      parent_map->HighlightSubmapEntryPoint( this );
      display_manager->ContinuePathInParent( bound_empty, START );
      return( UNMODIFIED );    
   } else // execute false, validate only
      return( ((bound_empty = display_manager->PathContinuedInParent( this )) != NULL) ? TRUE : FALSE );
}

bool Start::CenterLabel( execution_flag execute )
{
   if( execute ) {
      centered = TRUE;
      return( TRUE );
   }
   else
      return( centered == FALSE );
}

bool Start::FlushLabel( execution_flag execute )
{
   if( execute ) {
      centered = FALSE;
      return( TRUE );
   }
   else
      return( centered == TRUE );
}

bool Start::Merge( Hyperedge *edge, execution_flag execute )
{

   edge_type type = edge->EdgeType();
   
   if( execute ){
  
      switch( type ){
	 
      case RESULT:
	 MergeWithResult( edge );
	 break;
      case STUB:
	 MergeWithStub( edge );
	 break;
      }
      return( TRUE ); // not used
   }
   else {

      if( !((type == RESULT) || (type == STUB)) )
	 return( INVALID );

      if( figure->GetPath() == edge->GetFigure()->GetPath() )
	 return( UNAVAILABLE );       

      if( HasSourceColour( C ) ) // check if this start is triggered by a result
	 return( UNAVAILABLE );       
 
      // check if it is a result which already triggers another wait
      if(type == RESULT) {
	 if( edge->HasTargetColour( D ) )
	    return( UNAVAILABLE );       
      }

      // search for edge on any path starting from this start and disable if found
      //if( this->SamePath( this, edge ) )  // disabled temporarily due to endless loops
      // return( FALSE );
      
      return( AVAILABLE );
   }

}

bool Start::ConnectPath( Hyperedge *trigger_edge, execution_flag execute )
{
   if( !execute ) {
      // search for edge on any path starting from this start and disable if found
      //if( this->SamePath( this, trigger_edge ) )
      // return( FALSE );
   }

   return( WaitingPlace::ConnectPath( trigger_edge, execute ) );
}

bool Start::DeletePath( execution_flag execute )
{
   if( execute )
      return( Hyperedge::DeletePath() );
   else { // search for joining of separate paths and disable in such a case
      TransformationManager::Instance()->CurrentGraph()->ResetVisited(); // reset visited flags of all hyperedges in hypergraph
      Hyperedge::joined_paths = FALSE;
      target->GetFirst()->NextEdge()->Search( FORWARD_SEARCH );
      return( Hyperedge::joined_paths ? FALSE : TRUE );
      //return( ( target->GetFirst()->NextEdge()->Search( FORWARD_SEARCH ) == JOINED_PATHS ) ? FALSE : TRUE ); // call recursive Search method to search for other Starts
   }
}    

bool Start::DeleteHyperedge()
{ // delete path if applicable
   TransformationManager::Instance()->CurrentGraph()->ResetVisited(); // reset visited flags of all hyperedges in hypergraph
   Hyperedge::joined_paths = FALSE;
   target->GetFirst()->NextEdge()->Search( FORWARD_SEARCH );
   if( Hyperedge::joined_paths )
      return FALSE;
   return( Hyperedge::DeletePath() );
}

bool Start::CreateMSC( execution_flag execute )
{
   if( execute ){
      if( QuestionDialog( "Confirm Generation of All Path Combinations",
			  "This operation is recommended only for small designs to avoid combinatorial explosion. ",
			  "Large designs require MSC generation from scenario definitions to eliminate invalid paths.",
			  "Yes, Generate All Paths", "No, Cancel Operation" ) )
	 MSCGenerator::Instance()->GenerateAllMSCPaths( this );
      return( UNMODIFIED );
   }	
   else	
      return( TRUE );
}


bool Start::EditPathInitiation( execution_flag execute )
{
   if( execute )
      EditArrivalProcess( this );

   return( TRUE );
}

bool Start::ListBoundParentMaps( execution_flag execute )
{
   if( execute ) {
      ListParents( parent_map, this );
      return( TRUE );
   } else
      return( parent_map->HasBoundParents( this ) );
}

bool Start::SetAsScenarioStart( execution_flag execute )
{
   if( execute ) {
      ScenarioList::ViewScenarioList( this );
      return( TRUE );
   } else
      return( ScenarioList::ScenariosDefined() );
}

bool Start::ListScenarios( execution_flag execute )
{
   if( execute ) {
      ScenarioList::ViewStartScenarios( this );
      return( UNMODIFIED );
   } else
      return( start_scenarios.Size() > 0 );
}

void Start::MergeWithStub( Hyperedge *stub )
{
   Node *join_node = target->GetFirst();

//   Hyperedge *next_edge = join_node->NextEdge();
   stub->AttachTarget( join_node );
   display_manager->CreateStubJoin( this, stub );
   parent_map->MapHypergraph()->PurgeEdge( this );
}

void Start::MergeWithResult( Hyperedge *result )
{
   Hypergraph *graph = TransformationManager::Instance()->CurrentGraph();

   Node *join_node = new Node( A );
   Hyperedge *prev_edge = result->FirstInput();
   Hyperedge *next_edge = target->GetFirst()->NextEdge();
  
   prev_edge->DetachTarget( result->SourceSet()->GetFirst() );
   prev_edge->AttachTarget( join_node );
   next_edge->DetachSource( target->GetFirst() );
   next_edge->AttachSource( join_node );
  

   graph->PurgeNode( result->SourceSet()->GetFirst() );	// delete unneeded edges
   graph->PurgeEdge( result );				// start, result
   graph->PurgeNode( target->GetFirst() );
   graph->PurgeEdge( this );
  
   display_manager->JoinPaths( prev_edge, next_edge ); 
}

Hyperedge * Start::ScanScenarios( scan_type type )
{
   return( EvaluateGuardCondition() );
}

Hyperedge * Start::HighlightScenario( )
{
   return( EvaluateGuardCondition() );
}

void Start::VerifyAnnotations()
{
   // verify that this start point has either an arrival process defined or is a start point of a plugin
   if( parent_map->IsPlugIn() ) return;

   if( strequal( identifier, "" ) ) {
      annotations_complete = FALSE;
      parent_map->HighlightParentStubs();
      return;
   }

   annotations_complete = TRUE;
   
   if( arrival != UNDEFINED ) { // check for valid arrival process
      switch( arrival ) {
      case EXPONENTIAL:
      case DETERMINISTIC:
	 if( input[0] != 0 ) return;
	 break;
      case UNIFORM:
      case ERLANG:
	 if( input[0] != 0 || input[1] != 0 ) return;
	 break;
      case EXPERT:
	 if( expert ) {
	    if( !strequal( expert, "" ) ) return;
	 }
	 break;
      default:		// UNDEFINED
	 break;
      }
   }
   
   annotations_complete = FALSE;  // set flag to false if all tests failed
   parent_map->HighlightParentStubs();
}

void Start::SaveXMLDetails( FILE *fp )
{
   char buffer[300], closed_stream[50];

   switch( arrival ) {
   case EXPONENTIAL:
      sprintf( buffer, "start arrival=\"exponential\" mean=\"%f\"", input[0] );
      break;
   case DETERMINISTIC:
      sprintf( buffer, "start arrival=\"deterministic\" value=\"%f\"", input[0] );
      break;
   case UNIFORM:
      sprintf( buffer, "start arrival=\"uniform\" low=\"%f\" high=\"%f\"", input[0], input[1] );
      break;
   case ERLANG:
      sprintf( buffer, "start arrival=\"erlang\" kernel=\"%f\" mean=\"%f\"", input[1], input[0] ); 
      break;
   case EXPERT:
      sprintf( buffer, "start arrival=\"expert\" expert-distribution=\"%s\"", expert ? PrintDescription( expert ) : "" );
      break;
   case UNDEFINED:
      sprintf( buffer, "start" );
      break;
   }

   strcat( buffer, " stream-type=\"" );
   if( stream == OPEN_ARRIVAL )
      strcat( buffer, "open\"" );
   else { // stream == CLOSED_ARRIVAL
      sprintf( closed_stream, "closed\" population-size=\"%d\"", population_size );      
      strcat( buffer, closed_stream );
   }

   if( logical_condition != NULL ) {
      strcat( buffer, " logical-condition=\"" );
      strcat( buffer, PrintDescription( logical_condition ) );
      strcat( buffer, "\"" );
   }
   strcat( buffer, " label-alignment=\"" );
   strcat( buffer, centered ? "centered" : "flush" );
   strcat( buffer, "\" " );
   
   PrintNewXMLBlock( fp, buffer );

   condition_manager.SaveXML( fp, "precondition", "triggering-event" );

   PrintEndXMLBlock( fp, "start" );
}


void Start::SaveCSMXMLDetails( FILE *fp )
{
   char buffer[300], closed_stream[50];
   if( stream == OPEN_ARRIVAL ){
   switch( arrival ) {
   case EXPONENTIAL:
      sprintf( buffer, "<OpenWorkload arrival=\"exponential\" mean=\"%f\"", input[0] );
      break;
   case DETERMINISTIC:
      sprintf( buffer, "<OpenWorkload arrival=\"deterministic\" value=\"%f\"", input[0] );
      break;
   case UNIFORM:
      sprintf( buffer, "<OpenWorkload arrival=\"uniform\" low=\"%f\" high=\"%f\"", input[0], input[1] );
      break;
   case ERLANG:
      sprintf( buffer, "<OpenWorkload arrival=\"erlang\" kernel=\"%f\" mean=\"%f\"", input[1], input[0] ); 
      break;
   case EXPERT:
      sprintf( buffer, "<OpenWorkload arrival=\"expert\" expert-distribution=\"%s\"", expert ? PrintDescription( expert ) : "" );
      break;
   case UNDEFINED:
      sprintf( buffer, "<OpenWorkload" );
      break;
   }
   PrintXMLText(fp, buffer);
   fprintf(fp, " id=\"w%d\" ", hyperedge_number);
   fprintf( fp, "/>" );
   LinebreakXML( fp );
	//PrintEndXMLBlock( fp, "OpenWorkload" );
   }

   else {
   switch( arrival ) {
   case EXPONENTIAL:
      sprintf( buffer, "<CloseWorkload arrival=\"exponential\" mean=\"%f\"", input[0] );
      break;
   case DETERMINISTIC:
      sprintf( buffer, "<CloseWorkload arrival=\"deterministic\" value=\"%f\"", input[0] );
      break;
   case UNIFORM:
      sprintf( buffer, "<CloseWorkload arrival=\"uniform\" low=\"%f\" high=\"%f\"", input[0], input[1] );
      break;
   case ERLANG:
      sprintf( buffer, "<CloseWorkload arrival=\"erlang\" kernel=\"%f\" mean=\"%f\"", input[1], input[0] ); 
      break;
   case EXPERT:
      sprintf( buffer, "<CloseWorkload arrival=\"expert\" expert-distribution=\"%s\"", expert ? PrintDescription( expert ) : "" );
      break;
   case UNDEFINED:
      sprintf( buffer, "<CloseWorkload" );
      break;
   }
   sprintf( closed_stream, " id=\"w%d\" population-size=\"%d\" ",hyperedge_number, population_size );      
   strcat( buffer, closed_stream );
   PrintXMLText(fp, buffer);
   fprintf( fp, "/>" );
   LinebreakXML( fp );
    //PrintNewXMLBlock( fp, buffer );
	//PrintEndXMLBlock( fp, "CloseWorkload" );
   }
   /*
   strcat( buffer, " stream-type=\"" );
   if( stream == OPEN_ARRIVAL )
      strcat( buffer, "open\"" );
   else { // stream == CLOSED_ARRIVAL
      sprintf( closed_stream, "closed\" population-size=\"%d\"", population_size );      
      strcat( buffer, closed_stream );
   }

   if( logical_condition != NULL ) {
      strcat( buffer, " logical-condition=\"" );
      strcat( buffer, PrintDescription( logical_condition ) );
      strcat( buffer, "\"" );
   }
   strcat( buffer, " label-alignment=\"" );
   strcat( buffer, centered ? "centered" : "flush" );
   strcat( buffer, "\" " );
   */
  // PrintNewXMLBlock( fp, buffer );
  
   //condition_manager.SaveXML( fp, "precondition", "triggering-event" );

  // PrintEndXMLBlock( fp, "OpenWorkload" );
 
}

void Start::AddStartResourceAcquire (Component* comp )
{

	Resourceacquire *RA = new Resourceacquire();
    RA->SetAcquiredComponent(comp);
	Node *tnode=target -> GetFirst();
	Node *node1= new Node(B);
	RA-> AttachSource(node1);
	RA-> AttachTarget(tnode);
	target->Remove(tnode);
	AttachTarget(node1);

}

void Start::AddStartResourceRelease (Component* comp, ComponentReference* comp_ref )
{

	Resourcerelease *RR = new Resourcerelease();
    RR->SetReleasedComponent(comp);
	RR->SetReleasedComponentRef(comp_ref);
    Node *tnode=target -> GetFirst();
	Node *node1= new Node(B);
	RR-> AttachSource(node1);
	RR-> AttachTarget(tnode);
	target->Remove(tnode);
	AttachTarget(node1);
	
}

void Start::AddCSMResourceAcquire ( )
{

	Component* comp = NULL;
	Component* comp_parent = NULL;
    ComponentReference* compref_parent= NULL;
    ComponentReference* curr_comp=NULL;
	Cltn<ComponentReference* > curr_comp_parent;
	Cltn<ComponentReference* > prev_comp_parent;
	     
	    
	// check if the current start point resides in a component
    ComponentReference* comp_ref = this->GetFigure()->GetContainingComponent();
	if(!comp_ref)
	  return;
   
	comp = comp_ref->ReferencedComponent();
    //find out the parent components for the current component, and store them in the collection curr_comp_parent
    curr_comp_parent.Add(comp_ref);	 //add current component to the stack bottom 
	prev_comp_parent.AddB(comp_ref); // add current component to the front
	curr_comp=comp_ref;
    while(curr_comp)
    {
        compref_parent=curr_comp->GetParent();
        if(compref_parent)
		{
            curr_comp_parent.Add(compref_parent); // add each element in front of the collection
            prev_comp_parent.AddB(compref_parent);
		}
		curr_comp=compref_parent;
	}
			 
          
	  
	  //acquire all the parents and current components
	  
	if(comp)
	{
	   for( prev_comp_parent.First(); !prev_comp_parent.IsDone(); prev_comp_parent.Next())
	   {
           compref_parent=prev_comp_parent.CurrentItem();
	       comp_parent= compref_parent->ReferencedComponent();
	       if(comp_parent && compref_parent)
			  AddStartResourceRelease(comp_parent,compref_parent); 
		         
       }

       
       for( curr_comp_parent.First(); !curr_comp_parent.IsDone(); curr_comp_parent.Next())
	   {
           compref_parent=curr_comp_parent.CurrentItem();
	       comp_parent= compref_parent->ReferencedComponent();
	       if(comp_parent)
			  AddStartResourceAcquire(comp_parent); 
		          
       }
	        
    }	    
 

}//end function



bool Start::HasTextualAnnotations()
{
   return( HasConditions() || HasDescription() || ( arrival != UNDEFINED && PrintManager::include_performance ) );
}

void Start::GeneratePostScriptDescription( FILE *ps_file )
{
   bool has_description = HasDescription(),
      has_conditions = HasConditions();
   char *guard_condition;

   if( !has_description && !has_conditions && (logical_condition == NULL) && ( arrival == UNDEFINED || !PrintManager::include_performance ) )
      return;

   fprintf( ps_file, "%d rom (Start Point - ) S\n%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size,
	    PrintManager::PrintPostScriptText( identifier ));
   if( has_description ) GeneratePSElementDescription( ps_file );

   if( PrintManager::include_scenarios ) {
      if( (guard_condition = BooleanVariable::ReferenceToName( logical_condition )) != NULL ) {
	 fprintf( ps_file, "%d rom 1 IN ID (Guard Condition ) P OD\n2 IN ID\n", PrintManager::text_font_size );
	 fprintf( ps_file, "%d bol (%s ) P OD\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( guard_condition ) );
	 free( guard_condition );
      }
   }

   if( has_conditions ) condition_manager.GeneratePostScriptDescription( ps_file );

   if( arrival == UNDEFINED || !PrintManager::include_performance ) return;

   fprintf( ps_file, "%d rom\n1 IN ID (Interarrival-time distribution - ) S \n%d bol (%s ) P OD\n2 IN ID\n",
	    PrintManager::text_font_size, PrintManager::text_font_size, process_names[arrival] );

   switch( arrival ) {
   case EXPONENTIAL:
      fprintf( ps_file, "%d rom (Mean - ) S %d bol (%f ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, input[0] );
      break;
   case DETERMINISTIC:
      fprintf( ps_file, "%d rom (Value - ) S %d bol (%f ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, input[0] );
      break;
   case UNIFORM:
      fprintf( ps_file, "%d rom (Low - ) S %d bol (%f ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, input[0] );
      fprintf( ps_file, "%d rom (High - ) S %d bol (%f ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, input[1] );
      break;
   case ERLANG:
      fprintf( ps_file, "%d rom (Kernel - ) S %d bol (%f ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, input[0] );
      fprintf( ps_file, "%d rom (Mean - ) S %d bol (%f ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, input[1] );
      break;
   case EXPERT:
      fprintf( ps_file, "%d rom (Expert distribution - ) S %d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size, 
	       (expert ? expert : "") );
      break;
   default:		// UNDEFINED
      break;
   }   
   fprintf( ps_file, "L OD\n" );
}

void Start::OutputPerformanceData( std::ofstream &pf )
{
   if( arrival == UNDEFINED ) return;

   pf << "Start\t\"" << identifier << "\"\n";
   pf << "Arrival Process\t" << process_names[arrival] << std::endl;

   switch( arrival ) {
   case EXPONENTIAL:
      pf << "Mean\t" << input[0] << std::endl;
      break;
   case DETERMINISTIC:
      pf << "Value\t" << input[0] << std::endl;
      break;
   case UNIFORM:
      pf << "Low\t" << input[0] << std::endl;
      pf << "High\t" << input[1] << std::endl;
      break;
   case ERLANG:
      pf << "Kernel\t" << input[0] << std::endl;
      pf << "Mean\t" << input[1] << std::endl;
      break;
   case EXPERT:
      pf << "Expert\t" << (expert ? expert : "") << std::endl;
      break;
   default:		// UNDEFINED
      break;
   }   
}

void Start::ExpertInput( const char *new_process )
{
   if( expert )
      free( expert );
   expert = strdup( new_process );
}

bool Start::SamePath( Hyperedge *first, Hyperedge *second )
{
   Cltn<Node *> *tset;
   Hyperedge *next_edge;

   static int depth = 0;

   depth++;
   std::cout << "\nSame Path level: " << depth << "\tfirst:" << (void *)first << first->EdgeName()
	<< "\tsecond:" << (void *)second << second->EdgeName() << std::flush;
   if( depth > 100 ) {
      getchar();
      exit(1);
   }
   
   // search through the hyperedge chain from first and attempt to find second
   // if found return true signifying that both edges are on the same path

   tset = first->TargetSet();

   for( tset->First(); !tset->IsDone(); tset->Next() ) {
      next_edge = tset->CurrentItem()->NextEdge();
      if( next_edge->EdgeType() == CONNECT )
	 return( FALSE );
      if( next_edge == second ) {
	 depth--;
	 return( TRUE );
      }
      if( this->SamePath( next_edge, second ) == TRUE ) {
	 depth--;
	 return( TRUE );
      }
      
   }
   
   depth--;
   return( FALSE );
   
}


Hyperedge * Start::TraverseScenario( )
{
   if( EvaluateCondition()) {
      SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
      int hyperedge_number = this->GetNumber();
      const char * name = this->HyperedgeName();
      Path_data  * action;
      ComponentReference * component = NULL;
//      Stub * parent_stub = NULL;
//      PluginBinding * binding = NULL;
      Cltn<StubBinding *> entryBinding;

      Stub* currentStub;
      int   stubSearchLevel;

      currentStub = sce->getCurrentStub();
      stubSearchLevel = 1;
      
      if( !IsConnectStart( ) ) {	          
         action = new Action(hyperedge_number, name, start);
      }
      else {
         action = new Action(hyperedge_number, name, Connect_Start);
      }
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

      sce->AddPathTraversal( action );
      sce->AddPathList( action );        
      return ( target->GetFirst()->NextEdge() );
   }
   return NULL;
}


bool Start::IsConnectStart( ) 
{
   bool isConnectStart;  
   Stub * parent_stub = NULL;
   PluginBinding * binding = NULL;
   Cltn<StubBinding *> entryBinding;
   
   if( this->SourceSet()->is_empty() ) {
      isConnectStart = false;
   }
   else {
      isConnectStart = true;
   }
   if( isConnectStart == false ) {
      parent_stub = parent_map->getScanningParentStub();
      if( parent_stub ) {
         binding = parent_stub->SubmapBinding( parent_map );
         entryBinding = binding->entry_bindings;
         for(  entryBinding.First(); !entryBinding.IsDone(); entryBinding.Next() ) {
            if( entryBinding.CurrentItem()->path_endpoint == this ) {
  	       return true;
	    }
         }
      }
   }
   return  isConnectStart;
}	   
	   
	   
	   
void Start::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
