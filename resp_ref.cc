/***********************************************************
 *
 * File:			resp_ref.cc
 * Author:			Andrew Miga
 * Created:			April 1999
 *	
 * Modification history:	Feb 2003 
 *                              Shuhua Cui
 *
 ***********************************************************/

#include "resp_ref.h"
#include "resp_mgr.h"
#include "empty.h"
#include "hypergraph.h"
#include "resp_figure.h"
#include "map.h"
#include "print_mgr.h"
#include "display.h"
#include "msc_generator.h"
#include "component_ref.h"
#include "service.h"
#include "devices.h"
#include <iostream>
#include "scenario_generator.h"
#include "action.h"
#include "path_data.h"
#include "stub.h"
#include "component_mgr.h"

class Stub;


extern DisplayManager *display_manager;

extern void EditResponsibilityDesc( ResponsibilityReference *new_responsibility, bool new_ref = FALSE );
extern void UpdateResponsibilityList();
extern void ResetConditions();


ResponsibilityReference::ResponsibilityReference() : Hyperedge()
{
   figure = new ResponsibilityFigure( this );
   parent_map->Responsibilities()->Add( this );  
   responsibility = NULL;
   arrow_position = ARROW_RIGHT;
   EditResponsibilityDesc( this, TRUE );
}

ResponsibilityReference::ResponsibilityReference( int resp_id, float x, float y, const char *direction,
						  int parent_id, const char *position ) : Hyperedge()
{
   figure = new ResponsibilityFigure( this );
   figure->SetPosition( x, y );
   ((ResponsibilityFigure *)figure)->Direction( this->Direction( direction ) );
   parent_map->Responsibilities()->Add( this );  
   load_number = resp_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = resp_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;    
   //End of the modification by Bo Jiang, April, 2005

   responsibility = ResponsibilityManager::Instance()->FindResponsibility( ((parent_id != NA) ? parent_id : resp_id) );
   responsibility->IncrementCount();
   arrow_position = position ? DynamicArrow::ArrowPosition( position ) : ARROW_RIGHT;
}

ResponsibilityReference::~ResponsibilityReference()
{
   parent_map->Responsibilities()->Delete( this );
   if( responsibility ) responsibility->DecrementCount();
   if( figure->IsSelected() )  // remove conditions from lists if edge deleted
      ResetConditions();
}

const char * ResponsibilityReference::HyperedgeName()
{
   return( (responsibility) ? responsibility->Name() : "" );
}

bool ResponsibilityReference::Perform( transformation trans, execution_flag execute )
{
   if( trans == DELETE_RESPONSIBILITY )      
      return( DeleteResponsibility( execute ) );
   if( trans == EDIT_RESPONSIBILITY )      
      return( EditResponsibility( execute ) );
   return FALSE;
}

bool ResponsibilityReference::DeleteResponsibility( execution_flag execute )
{
   if ( execute ) {
      display_manager->SetPath( figure->GetPath() );
      current_label = this->PathLabel();
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
      UpdateResponsibilityList();
      return( TRUE );  // not used
   }
   else
      return ( TRUE );
}
  
bool ResponsibilityReference::EditResponsibility( execution_flag execute )
{
   if( execute ) {
      EditResponsibilityDesc( this );
      return( TRUE );
   }
   else
      return( TRUE );
}

void ResponsibilityReference::ParentResponsibility( Responsibility *new_parent )
{
   new_parent->IncrementCount();
   if( responsibility ) responsibility->DecrementCount();
   responsibility = new_parent;
}

void ResponsibilityReference::Install()
{
   empty->InstallSimpleElement( this );
   ResponsibilityManager::Instance()->AddResponsibility( responsibility );
}

void ResponsibilityReference::VerifyAnnotations()
{
   if( !responsibility->VerifyAnnotations() )
      parent_map->HighlightParentStubs();
}

Hyperedge * ResponsibilityReference::ScanScenarios( scan_type type )
{
   responsibility->ExecuteVariableOperations();
   return( target->GetFirst()->NextEdge() );
}

Hyperedge * ResponsibilityReference::HighlightScenario( )
{
   responsibility->ExecuteVariableOperations();
   return( target->GetFirst()->NextEdge() );
}

Hyperedge * ResponsibilityReference::GenerateMSC( bool first_pass )
{
   ComponentReference *bound_component;
   MSCGenerator *msc = MSCGenerator::Instance();

      responsibility->ExecuteVariableOperations();

   if( (bound_component = figure->GetContainingComponent()) == NULL )
       bound_component = msc->StubComponent() ; // default to enclosing component of parent stub
       
     msc->ComponentCrossing( bound_component, first_pass );

  if( !first_pass )
      msc->CreateAction( responsibility->Name() );

   return( target->GetFirst()->NextEdge() );
}

void ResponsibilityReference::SaveXMLDetails( FILE *fp )
{
   char buffer[150];
   
   sprintf( buffer, "responsibility-ref resp-id=\"r%d\" direction=\"%s\"", responsibility->ResponsibilityNumber(),
	    this->Direction( ((ResponsibilityFigure *)figure)->Direction() ) );
   if( responsibility->HasDynarrow() ) {
      strcat( buffer, " arrow-position=\"" );
      strcat( buffer, DynamicArrow::Position( arrow_position ) );
      strcat( buffer, "\"" );
   }
   strcat( buffer, " /" );
   PrintXML( fp, buffer );
}

void ResponsibilityReference::SaveCSMXMLDetails( FILE *fp )
{
	
   char buffer[150];

   sprintf( buffer, "name=\"%s\" ", responsibility->Name());      
   fprintf( fp, buffer );
   if(responsibility->Description() !=NULL)
   {
      sprintf(buffer,"description=\"%s\" ", responsibility->Description());
      fprintf (fp, buffer);
   }
   else
   {
	  sprintf(buffer, "description=\" \"");
	  fprintf (fp,buffer);
    } 

   ComponentReference* comp_ref = this->GetFigure()->GetContainingComponent();
   Component* comp = NULL;
   // if there is a component reference
   if( comp_ref ) {
    comp = comp_ref->ReferencedComponent();
    // if there is a component actually referenced
    if( comp )
       sprintf(buffer, " component=\"c%d\" ", comp->GetComponentNumber() );         // "comp id" 
       //sprintf(buffer, " component=\"c%d\" ", comp_ref->ComponentReferenceNumber() ); // "comp ref id" )
    // else there is no component actually referenced
    else // TO BE FIXED (display component attributes window upon component creation)
       sprintf(buffer, " component=\"c%d\" ", comp_ref->ComponentReferenceNumber() ); // "comp ref id" )
    
   fprintf( fp, buffer );
   }

   // output hostDemand for responsibilities
   DeviceDirectory *dd = DeviceDirectory::Instance();
   ServiceRequest *sr = NULL;
   Cltn<ServiceRequest *> *service_requests = responsibility->ServiceRequests();
   if( service_requests->Size() == 0 ) 
        sprintf(buffer," hostDemand=\"0\" ");                       // "0 service time",
   else {
     for( service_requests->First(); !service_requests->IsDone(); service_requests->Next() ){
        sr = service_requests->CurrentItem();
        if( ( sr->DeviceId() == PROCESSOR_DEVICE ) || ( dd->DeviceType( sr->DeviceId() ) == PROCESSOR ) )
           sprintf(buffer,"hostDemand=\"%s\" ", PrintDescription( sr->Amount() ) );                     // "service time"
      }
    }
   fprintf( fp, buffer );

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
  
  fprintf( fp, "/>\n" );
}

void ResponsibilityReference::SaveDXLDetails( FILE *fp )
{
  fprintf( fp, "\"r%d\", \"%s\"", responsibility->ResponsibilityNumber(), responsibility->Name() );
  if( PrintManager::TextNonempty( this->Description() ) ) 
          fprintf(fp, ", \"%s\" )", removeNewlineinString( this->Description() ) );
   else
          fprintf( fp, ", \"\" )");   
}

void ResponsibilityReference::OutputPerformanceData( std::ofstream &pf )
{
  ServiceRequest *sr;
  DeviceDirectory *dd = DeviceDirectory::Instance();
  Cltn<ServiceRequest *> *service_requests = responsibility->ServiceRequests();

  if( service_requests->Size() == 0 ) return;

  pf << "\nResponsibility\t\"" << responsibility->Name() << "\"\n";

  for( service_requests->First(); !service_requests->IsDone(); service_requests->Next() ){
    sr = service_requests->CurrentItem();
    if( sr->DeviceId() == PROCESSOR_DEVICE )
       pf << "\"unbound processor\"\n";
    else
       pf << "\"" << dd->DeviceName( sr->DeviceId() ) << "\"\n";
    pf << sr->Amount() << std::endl;
  }

}

void ResponsibilityReference::GeneratePostScriptDescription( FILE *ps_file )
{
   if( responsibility->Visited() ) return;
   responsibility->SetVisited();

   responsibility->GeneratePostScriptDescription( ps_file );
}

const char *ResponsibilityReference::Direction( etResponsibility_direction direction )
{
   switch( direction ) {
   case RESP_UP:
      return "UP";
   case RESP_DOWN:
      return "DOWN";
   case RESP_LEFT:
      return "LEFT";
   case RESP_RIGHT:
      return "RIGHT";
   }
   return "";
}

etResponsibility_direction ResponsibilityReference::Direction( const char *direction )
{
   if( strequal( direction, "UP" ) )
      return( RESP_UP );
   else if( strequal( direction, "DOWN" ) )
      return( RESP_DOWN );
   else if( strequal( direction, "LEFT" ) )
      return( RESP_LEFT );
   else if( strequal( direction, "RIGHT" ) )
      return( RESP_RIGHT );
   return RESP_RIGHT;
}

Hyperedge * ResponsibilityReference::TraverseScenario( )
{
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   int hyperedge_number = this->GetNumber();
   Stub* currentStub;
   int   stubSearchLevel;
     
   const char * name = responsibility->Name();
   ComponentReference * component = NULL;
      
   Path_data * action = new Action(hyperedge_number, name, Resp);
   
   currentStub = sce->getCurrentStub();
   stubSearchLevel = 1;
     
   if( (component = figure->GetEnclosingComponent() ) != NULL ) 
   {     
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
   
   
   sce->AddPathTraversal(action); //note
   sce->AddPathList(action); 
   responsibility->ExecuteVariableOperations();
   return( target->GetFirst()->NextEdge() );
}

