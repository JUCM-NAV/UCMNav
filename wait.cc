/***********************************************************
 *
 * File:			wait.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:        Feb 2003
 *                              Shuhua Cui
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "wait.h"
#include "start.h"
#include "point_figure.h"
#include "synchronization.h"
#include "connect.h"
#include "msc_generator.h"
#include "print_mgr.h"
#include "variable.h"
#include "map.h"
#include "scenario.h"
#include "utilities.h"
#include <iostream>
#include "scenario_generator.h"
#include "action.h"
#include "path_data.h"
#include "component_ref.h"

extern void InstallConditions( ConditionManager &conditions );
extern void ResetConditions();
extern FD_Main *pfdmMain;

Wait::Wait() : WaitSynch()
{
   condition_manager.ConditionType( WAIT_POINT );
   figure = new PointFigure( this, WAIT_FIG );
   identifier[19] = 0;
   wait_type[0] = 0;
   wait_type[DISPLAYED_LABEL] = 0;
}

Wait::Wait( int wait_id, const char *name, float x, float y, const char *desc, const char *type, const char *logical_expression ) : WaitSynch()
{
   load_number = wait_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file
   hyperedge_number = wait_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;    
   //End of the modification by Bo Jiang, April, 2005

   condition_manager.ConditionType( WAIT_POINT );
   figure = new PointFigure( this, WAIT_FIG );
   figure->SetPosition( x, y );
   identifier[19] = 0;
   strncpy( identifier, name, 19 );
   if( desc != NULL ) unique_desc = strdup( desc );
   wait_type[0] = 0;
   wait_type[DISPLAYED_LABEL] = 0;
   if( type != NULL ) strncpy( wait_type, type, DISPLAYED_LABEL );
   logical_condition = ((logical_expression != NULL) ? strdup( logical_expression ) : NULL );
}

bool Wait::Perform( transformation trans, execution_flag execute )
{
   switch( trans ){
   case EDIT_WAIT_CHARACTERISTICS:
      return( EditWaitCharacteristics( execute ) );
      break;
   case EDIT_LOGICAL_CONDITION:
      return( EditGuardCondition( execute ) );
      break;
   case DELETE_WAIT:
      return( DeleteWait( execute ) );
      break;
   }
   return FALSE;
}

bool Wait::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute )
{
   switch( trans ){ 
   case CONNECT:
      return( ConnectPath( edge, execute ) );
      break;
   case DISCONNECT:
      return( DisconnectPath( edge, execute ) );
      break;
   }
   return FALSE;
}

bool Wait::DeleteWait( flag execute )
{
   if( execute ){
      display_manager->DeleteWaitSynch( this );
      DeleteWaiting();
      return( TRUE );
   }
   else
      return( TRUE );
}

bool Wait::DeleteHyperedge()
{
   this->BreakConnections();
   this->DeleteElement();
   return TRUE;
}

void Wait::WaitPositionChanged()
{
   if( target->Size() > 0 )
      FirstOutput()->GetFigure()->GetPath()->PathChanged(); 
}

bool Wait::EditWaitCharacteristics( execution_flag execute )
{
   char *name, *type;

   if( execute ) {
      if( TwoStringDialog( "Edit Wait Characteristics", "Wait Name", "Type of Wait",
			   identifier, wait_type, &name, &type ) == TRUE ) {
	 strncpy( identifier, name, 19 );
	 strncpy( wait_type, type, DISPLAYED_LABEL );
	 return MODIFIED;
      }
      else
	 return UNMODIFIED;
   }
   else
      return TRUE;
}

void Wait::SaveXMLDetails( FILE *fp )
{
   char buffer[100];

   sprintf( buffer, "<waiting-place timer=\"no\" " );
   PrintXMLText( fp, buffer );
   if( PrintManager::TextNonempty( wait_type ) )
      fprintf( fp, "wait-type=\"%s\" ", PrintDescription( wait_type ) );
   if( logical_condition != NULL )
      fprintf( fp, "logical-condition=\"%s\" ", PrintDescription( logical_condition ) );
   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );
   condition_manager.SaveXML( fp, "precondition", "triggering-event" );
   PrintEndXMLBlock( fp, "waiting-place" );
}

bool Wait::HasTextualAnnotations()
{
   return( HasConditions() || HasDescription() || PrintManager::TextNonempty( wait_type ) );
}

Hyperedge * Wait::HighlightScenario( )
{
   Hyperedge *next_scan = NULL;
//   char buffer[200];
   MSCGenerator *msc = MSCGenerator::Instance();

   if( this->HasSourceColour( C ) ) {  // if this wait is triggered it should be interpreted as a synchronization
      if( !InputsSynchronized() ) {
	   next_scan = msc->NextParallelBranch();
	   return next_scan;
       }
       else {
            next_scan = PrimaryTarget()->NextEdge();
	    return next_scan;
       }
   }

   //  evaluate logical condition (if it exists) to determine if scanning should be halted or continued down the path after this wait
   return( EvaluateGuardCondition() );
}

Hyperedge * Wait::ScanScenarios( scan_type type )
{
   Hyperedge *next_scan = NULL;
   char buffer[200];
   MSCGenerator *msc = MSCGenerator::Instance();

   if( this->HasSourceColour( C ) ) {  // if this wait is triggered it should be interpreted as a synchronization
      if( !InputsSynchronized() ) {
	 if( (next_scan = msc->NextScanningPath( this )) != NULL ) { // determine next and fork branch or scenario start point if they exist
	    StoreSynchType( (next_scan->EdgeType() == START) ? FALSE : TRUE );
	    return( next_scan );
	 }
	 else { // there are no further paths to scan
	    sprintf( buffer, "Both paths did not synchronize at the Wait point \"%s\" in Map \"%s\".",
		     identifier, parent_map->MapLabel() );
	    ErrorMessage( buffer, TRUE );
	    msc->AbortGeneration();
	    return( NULL );
	 }
      }
   }

   //  evaluate logical condition (if it exists) to determine if scanning should be halted or continued down the path after this wait
   return( EvaluateGuardCondition() );
}


Hyperedge * Wait::GenerateMSC( bool first_pass )
{
   Hyperedge *next_scan = NULL;
   ComponentReference *wait_component = NULL;
   MSCGenerator *msc = MSCGenerator::Instance();
   char buffer[200];

   if( (wait_component = figure->GetContainingComponent()) == NULL )
      wait_component = msc->StubComponent();

   msc->ComponentCrossing( wait_component, first_pass );

   if( this->HasSourceColour( C ) ) {  // if this wait is triggered it should be interpreted as a synchronization
      if( !InputsSynchronized() ) {
	 if( ForkSynchronization() == TRUE )
	    return( NULL ); // allow Synchronization MSC method to do its job
	 else if( (next_scan = msc->NextScanningPath( NULL )) != NULL ) { // determine next scenario start point if it exists
	    msc->ScenarioStartPoint( (Start *)next_scan, first_pass );
	    return( next_scan );
	 }
	 else { // there are no further paths to scan
	    sprintf( buffer, "Both paths did not synchronize at the Wait point \"%s\" in Map \"%s\".",
		     identifier, parent_map->MapLabel() );
	    ErrorMessage( buffer, TRUE );
	    msc->AbortGeneration();
	    return( NULL );	    
	 }
      }

      if( ForkSynchronization() == TRUE ) // if flag was set signifying that synchronization came from an and fork branch
	 msc->SetWaitingSynchronization( TRUE ); // notify previous fork to stop here instead of searching for an and join

   }
   return( target->GetFirst()->NextEdge() ); // continue scanning down the path after this wait
}


Hyperedge * Wait::TraverseScenario( )
{
   Hyperedge *next_scan = NULL;
//   char buffer[200];
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
//   Path_data * current_element = NULL;
   Path_data * trigger = NULL;
//   Path_data * current_path_data = sce->getCurrentPathData( );
   ComponentReference * component = NULL;
   
          
   if( this->HasSourceColour( C ) ) {  // if this wait is triggered it should be interpreted as a synchronization
      if( !InputsSynchronization( trigger ) ) {
	  waiting_element.Add( trigger );
	  next_scan = sce->NextParallelBranch();	  
	  return next_scan;
       }
       else {
          waiting_element.Add( trigger );
          next_scan = PrimaryTarget()->NextEdge();
	  waiting_leave = new Action(hyperedge_number, identifier, WP_Leave);
	  if( (component = figure->GetEnclosingComponent() ) != NULL ) {
             waiting_leave->SetComponentName( component->ReferencedComponent()->GetLabel() );
	     waiting_leave->SetComponentId( component->ReferencedComponent()->GetComponentNumber() );
	     waiting_leave->SetComponentRole( component->ComponentRole() );
	     waiting_leave->SetComponentReferenceId( component->ComponentReferenceNumber() );	     
          }

	  for( waiting_element.First(); !waiting_element.IsDone(); waiting_element.Next() ) {
	      waiting_element.CurrentItem()->AddNext(waiting_leave);
	  }
	  sce->ResetCurrentPathData( waiting_leave );
	  SearchRoot( waiting_element, waiting_leave );
	  Collapse( waiting_element, waiting_leave );
	  waiting_element.Empty();	  
          return next_scan;
       }
   }
   //  evaluate logical condition (if it exists) to determine if scanning should be halted or continued down the path after this wait
   return( EvaluateGuardCondition() );
}


Path_data * Wait::CreateAction()
{
   Path_data * p_data;
   p_data = new Action(hyperedge_number, identifier, WP_Enter);
   return p_data;
}

void Wait::EmptyPathData()
{
   waiting_element.Empty();
}


void Wait::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
