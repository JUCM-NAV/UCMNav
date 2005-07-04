/***********************************************************
 *
 * File:			timer.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:        June 1999
 *                              Added timeout paths
 *                              Feb 2003
 *                              Shuhua Cui
 *
 ***********************************************************/

#include "timer.h"
#include "point_figure.h"
#include "connect.h"
#include "empty.h"
#include "result.h"
#include "start.h"
#include "hypergraph.h"
#include "msc_generator.h"
#include "component_ref.h"
#include "print_mgr.h"
#include "utilities.h"
#include "variable.h"
#include "xml_mgr.h"
#include <iostream>
#include "scenario_generator.h"
#include "action.h"
#include "component_ref.h"

extern void InstallConditions( ConditionManager &conditions );
extern void ResetConditions();

extern bool design_deletion;

Cltn<Timer *> Timer::timer_list;

Timer::Timer( Empty *empty ) : WaitSynch()
{
   condition_manager.ConditionType( TIMER_POINT );
   figure = new PointFigure( this, TIMER_FIG );
   identifier[19] = 0;
   wait_type[0] = 0;
   wait_type[DISPLAYED_LABEL] = 0;
   install_empty = empty;
   timeout_variable = NULL;
   EditTimerCharacteristics( TRUE );
}

Timer::Timer( int timer_id, const char *name, float x, float y, const char *desc, BooleanVariable *tv,
	      const char *type, const char *logical_expression ) : WaitSynch()
{
   load_number = timer_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file
   hyperedge_number = timer_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;    
   //End of the modification by Bo Jiang, April, 2005

   condition_manager.ConditionType( TIMER_POINT );
   figure = new PointFigure( this, TIMER_FIG );
   figure->SetPosition( x, y );
   identifier[19] = 0;
   strncpy( identifier, name, 19 );
   if( desc != NULL ) unique_desc = strdup( desc );
   timeout_variable = tv;
   if( timeout_variable != NULL ) timeout_variable->SetAsTimeoutVariable();
   wait_type[0] = 0;
   wait_type[DISPLAYED_LABEL] = 0;
   if( type != NULL ) strncpy( wait_type, type, DISPLAYED_LABEL );
   logical_condition = ((logical_expression != NULL) ? strdup( logical_expression ) : NULL );
   timer_list.Add( this );
   install_empty = NULL;
}

Timer::~Timer()
{
   char variable_name[DISPLAYED_LABEL+10];
   
   if( design_deletion == FALSE ) {
      if( timeout_variable != NULL ) {
	 if( timeout_variable->ReferenceCount() == 0 )
	    delete timeout_variable;
	 else {
	    sprintf( variable_name, "%s_DELETED", identifier );
	    timeout_variable->BooleanName( variable_name );
	    timeout_variable->AllowTimerVariableDeletion();
	 } 
      }
   }
   
   timer_list.Delete( this );
}

bool Timer::Perform( transformation trans, execution_flag execute )
{
   switch( trans ){
  
   case EDIT_TIMER_CHARACTERISTICS:
      return( EditTimerCharacteristics( execute ) );
      break;
   case EDIT_LOGICAL_CONDITION:
      return( EditGuardCondition( execute ) );
      break;
   case DELETE_TIMER:
      return( DeleteTimer( execute ) );
      break;
   case ADD_TIMEOUT_PATH:
      return( AddTimeoutPath( execute ) );
      break;
   case DELETE_TIMEOUT_PATH:
      return( DeleteTimeoutPath( execute ) );
      break;
   }

   std::cout << "Warning: problem in Timer::Perform... should not get here!";
   return( FALSE );  // DA: Added August 2004
}

bool Timer::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute )
{

   switch( trans ){
  
   case CONNECT:
      return( ConnectPath( edge, execute ) );
      break;
   case DISCONNECT:
      return( DisconnectPath( edge, execute ) );
      break;
   }

   std::cout << "Warning: problem in Timer::PerformDouble... should not get here!";
   return( FALSE );  // DA: Added August 2004
}

bool Timer::DeleteTimer( flag execute )
{
   if( execute ){
      display_manager->DeleteWaitSynch( this );
      DeleteWaiting();
      return( TRUE );  // not used
   }
   else // disable if timeout path exists
      return( HasTargetColour( D ) ? FALSE : TRUE );
}

bool Timer::DeleteHyperedge()
{
   if( HasTargetColour( D ) )
      return FALSE;

   this->BreakConnections();
   this->DeleteElement();
   return TRUE;
}

bool Timer::AddTimeoutPath( flag execute )
{
   if( execute ){

      display_manager->CreateNewSegment();

      Node *node1 = new Node( D );  
      Node *node2 = new Node( B );  
      Node *node3 = new Node( A );  
      Abort *abort = new Abort;
      Empty *new_empty = new Empty;
      Result *new_result = new Result;

      this->AttachTarget( node1 );
      abort->AttachSource( node1 );
      abort->AttachTarget( node2 );
      new_empty->AttachSource( node2 );   
      new_empty->AttachTarget( node3 );
      new_result->AttachSource( node3 );

      display_manager->AddTimeoutPath( this, new_empty );

      figure->ResetSelected();
      new_result->GetFigure()->SetSelected();
      display_manager->SetActive( new_result->GetFigure() );

      return( TRUE );
   }
   else // disable if timeout path already exists
      return( HasTargetColour( D ) ? FALSE : TRUE );
}

bool Timer::DeleteTimeoutPath( flag execute )
{
   Node *front_node, *end_node;
   static Hyperedge *end_edge;
   Hypergraph *current_graph;
   
   front_node = ExceptionPath();

   if( execute ){

      current_graph = TransformationManager::Instance()->CurrentGraph();
      display_manager->DeleteTimeoutPath( this );
      this->DetachTarget( front_node );
      end_node = end_edge->SourceSet()->GetFirst();
      current_graph->PurgeEdge( end_edge );
      current_graph->PurgeSegmentFrom( front_node, end_node );
      return ( TRUE ); // DA: Added August 2004
   }
   else {

      if( !HasTargetColour( D ) ) // disable if no timeout path exists
	 return( FALSE );

      // determine end type of timeout path end, find path from empty after abort hyperedge   
      end_edge = front_node->NextEdge()->FirstOutput()->GetFigure()->GetPath()->GetPathEnd();

      if( end_edge->EdgeType() != RESULT )
	 return( FALSE ); // disable if timeout path is connected

      return( TRUE );
   }
   return ( TRUE ); // DA: Added August 2004 
}

bool Timer::EditTimerCharacteristics( execution_flag execute )
{
   char *name, *type, *entered_name, *entered_type;
   char variable_name[DISPLAYED_LABEL+10], timer_name[50], buffer[200];

   if( execute ) {
      entered_name = identifier;
      entered_type = wait_type;
      forever {
	 if( TwoStringDialog( "Edit Timer Characteristics", "Timer Name", "Type of Timer",
			      entered_name, entered_type, &name, &type ) == TRUE ) {

	    XmlManager::RemoveWhitespace( timer_name, name );
	    if( TimerNameValid( timer_name, identifier )) {
	       strncpy( identifier, timer_name, 19 );
	       strncpy( wait_type, type, DISPLAYED_LABEL );
	       sprintf( variable_name, "%s_timeout", identifier );
	       if( install_empty != NULL ) { // complete timer install operation
		  install_empty->InstallSimpleElement( this );
		  display_manager->CreateWaitSynch( this );
		  timer_list.Add( this );
		  timeout_variable = new BooleanVariable( variable_name, TRUE );
		  install_empty = NULL;
	       }
	       else
		  timeout_variable->BooleanName( variable_name );
	       return MODIFIED;
	    }
	    else { // name was invalid
	       if( strequal( name, "" ) ){ // name is empty
		  fl_set_resource( "flAlert.title", "Error: Empty Timer Name Entered" );
		  strcpy( buffer, "An empty timer name has been entered." );
	       } else { // name already exists
		  fl_set_resource( "flAlert.title", "Error: Duplicate Timer Name Entered" );
		  sprintf( buffer, "The entered name \"%s\" already exists for a timer.", timer_name );
	       }
	       fl_show_alert( buffer, "Please enter a unique nonempty name.", "", 0 );
	       entered_name = timer_name;
	       entered_type = type;
	    }
	 }
	 else { // cancel button was pressed
	    if( install_empty != NULL ) // cancel timer install operation
	       this->DeleteEdge();
	    return UNMODIFIED;
	 }
      }
   }
   else
      return TRUE;
}

bool Timer::TimerNameValid( const char *new_name, const char *old_name )
{
   if( strequal( new_name, "" ) )
      return FALSE;

   if( old_name != NULL ) {
      if( strequal( new_name, old_name ) )
	 return TRUE;
   }

   for( timer_list.First(); !timer_list.IsDone(); timer_list.Next() ) {
      if( strequal( timer_list.CurrentItem()->HyperedgeName(), new_name ) )
	 return FALSE;
   }

   return TRUE;
}

void Timer::ValidateTimerList()
{
   Timer *timer;
   bool list_invalid = FALSE;
   char variable_name[DISPLAYED_LABEL+10];

   for( timer_list.First(); !timer_list.IsDone(); timer_list.Next() ) {
      if( timer_list.CurrentItem()->timeout_variable == NULL ) {
	 list_invalid = TRUE;
	 break;
      }
   }

   if( !list_invalid ) return; // all timers have timeout variables

   for( int i = 1; i <= timer_list.Size(); i++ ) {
      timer = timer_list.Get( i );
      timer->VerifyUniqueTimerName();
      if( timer->timeout_variable == NULL ) {
	 sprintf( variable_name, "%s_timeout", timer->identifier );
	 timer->timeout_variable = new BooleanVariable( variable_name, TRUE );
      }
   }
}

void Timer::VerifyUniqueTimerName()
{
   static int max_id = 0;
   bool duplicate_name;
   char new_name[20], old_name[20];
   Timer *ct;
   int extension = 0;

   if( strequal( identifier, "" ) ) { // name is empty, give timern where n = 1,2,3...
      forever {
	 sprintf( new_name, "timer%d", ++max_id );
	 if( TimerNameValid( new_name ) ) {
	    strcpy( identifier, new_name );
	    return;
	 }
      }
   }
   else {

      duplicate_name = FALSE;

      for( timer_list.First(); !timer_list.IsDone(); timer_list.Next() ) {
	 ct = timer_list.CurrentItem();
	 if( ct != this ) {
	    if( strequal( ct->identifier, identifier ) ) {
	       duplicate_name = TRUE;
	       break;
	    }
	 }
      }

      if( !duplicate_name ) return; // name was unique

      strcpy( old_name, identifier );
      if( strlen( old_name ) > 17 )
	 old_name[17] = 0;
      sprintf( new_name, "%s-%d", old_name, extension++ );

      forever {

	 for( timer_list.First(); !timer_list.IsDone(); timer_list.Next() ) {
	    ct = timer_list.CurrentItem();
	    if( ct != this ) {
	       if( strequal( ct->identifier, new_name ) )
		  duplicate_name = TRUE;
	    }
	 }
	 
	 if( duplicate_name )
	    sprintf( new_name, "%s-%d", old_name, extension++ );
	 else {
	    strcpy( identifier, new_name );
	    return;
	 }
      }
   }
}

void Timer::TimerPositionChanged()
{
   for ( target->First(); !target->IsDone(); target->Next() ) {
      if( target->CurrentItem()->GetColour() == D )
	 target->CurrentItem()->NextEdge()->FirstOutput()->GetFigure()->GetPath()->PathChanged();
      else
	 target->CurrentItem()->NextEdge()->GetFigure()->GetPath()->PathChanged(); 
   }
}

void Timer::DisconnectTimeoutPath()
{
   if( HasTargetColour( D ) ) // delete start figure of timeout path
      display_manager->DeleteTimeoutPath( this );
}

void Timer::SaveXMLDetails( FILE *fp )
{
   char buffer[100];

   sprintf( buffer, "<waiting-place timer=\"yes\" timeout-variable=\"bv%d\" ", timeout_variable->BooleanId() );
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

bool Timer::HasTextualAnnotations()
{
   return( HasConditions() || HasDescription() || PrintManager::TextNonempty( wait_type ) );
}

Hyperedge * Timer::ScanScenarios( scan_type type )
{
   Hyperedge *current_edge;
   Hyperedge *next_scan;
   GenerationState gs;
   MSCGenerator *msc = MSCGenerator::Instance();
   Node *timeout_node;
   char buffer[200];

   if( msc->ScenarioTracing() ) {
      // if this timer is triggered and the timeout variable is false it should be interpreted as a synchronization
      if( this->HasSourceColour( C ) && (timeout_variable->Value() == FALSE) ) { 
	 if( !InputsSynchronized() ) {
	    if( (next_scan = msc->NextScanningPath( this )) != NULL ) { // determine next and fork branch or scenario start point if they exist
	       StoreSynchType( (next_scan->EdgeType() == START) ? FALSE : TRUE );
	       return( next_scan );
	    }
	    else { // there are no further paths to scan
	       sprintf( buffer, "Both paths did not synchronize at the Timer \"%s\" in Map \"%s\".",
			identifier, parent_map->MapLabel() );
	       ErrorMessage( buffer, TRUE );
	       msc->AbortGeneration();
	       return( NULL );	    
	    }
	 }
      }

      //  evaluate logical condition (if it exists) to determine if scanning should be halted or continued down the path after this wait
      if( EvaluateGuardCondition() == NULL )
	 return( NULL );
      
      if( timeout_variable->IsNotInitialized() ) { // value is undefined
	 sprintf( buffer, "The timeout variable \"%s\" for the Timer \"%s\" in Map \"%s\" was not initialized.",
		  timeout_variable->BooleanName(), identifier, parent_map->MapLabel() );
	 ErrorMessage( buffer, TRUE );
	 msc->AbortGeneration();
	 return( NULL );	    
      }
      else {
	 if( timeout_variable->Value() == FALSE )
	    current_edge = PrimaryTarget()->NextEdge();
	 else if( timeout_variable->Value() == TRUE ) {
	    timeout_node = this->ExceptionPath();
	    if( timeout_node != NULL )
	       current_edge = this->ExceptionPath()->NextEdge();
	    else
	       current_edge = NULL;
	 }
      }

      if( current_edge == NULL ) {
	 sprintf( buffer, "No timeout path was defined for the Timer \"%s\" in Map \"%s\".",
		  identifier, parent_map->MapLabel() );
	 ErrorMessage( buffer, TRUE );
	 msc->AbortGeneration();
	 return( NULL );
      } else
	 return( current_edge );

   }
   else {
      if( !this->HasSourceColour( C ) ) // return output path immediately if no trigger path exists
	 return( PrimaryTarget()->NextEdge() );
   
      // follow the normal output path
      current_edge = PrimaryTarget()->NextEdge();
      normal_path = TRUE;
      while( current_edge ) // scan along the normal output path
	 current_edge = current_edge->ScanScenarios( type );

      msc->ResetGenerationState( &gs );

      // follow the timeout path
      timeout_node = this->ExceptionPath();
      if( timeout_node != NULL )
	 current_edge = timeout_node->NextEdge();
      else
	 current_edge = PrimaryTarget()->NextEdge(); // set default 'timeout' path to be the normal output path
      normal_path = FALSE;

      while( current_edge ) // scan along the timeout path if it exists
	 current_edge = current_edge->ScanScenarios( type );
   
      return( NULL );
   }
}


Hyperedge * Timer::HighlightScenario( )
{
   Hyperedge *current_edge;
   Hyperedge *next_scan;
   //GenerationState gs;
   MSCGenerator *msc = MSCGenerator::Instance();
   Node *timeout_node;
   char buffer[200];

   // if this timer is triggered and the timeout variable is false it should be interpreted as a synchronization
   if( this->HasSourceColour( C ) && (timeout_variable->Value() == FALSE) ) { 
      if( !InputsSynchronized() ) {
	  next_scan = msc->NextParallelBranch();
	  return next_scan;
      }
      else {
	  current_edge = PrimaryTarget()->NextEdge();
	  return current_edge;
      }
   }
      //  evaluate logical condition (if it exists) to determine if scanning should be halted or continued down the path after this wait
   if( EvaluateGuardCondition() == NULL )
       return( NULL );
      
   if( timeout_variable->IsNotInitialized() ) { // value is undefined
       sprintf( buffer, "The timeout variable \"%s\" for the Timer \"%s\" in Map \"%s\" was not initialized.",
           timeout_variable->BooleanName(), identifier, parent_map->MapLabel() );
       ErrorMessage( buffer, TRUE );
       msc->AbortGeneration();
       return( NULL );	    
   }
   else {
      if( timeout_variable->Value() == FALSE )
	   current_edge = PrimaryTarget()->NextEdge();
      else if( timeout_variable->Value() == TRUE ) {
	  timeout_node = this->ExceptionPath();
	  if( timeout_node != NULL )
	     current_edge = this->ExceptionPath()->NextEdge();
	  else
	     current_edge = NULL;
      }
   }

   if( current_edge == NULL ) {
       sprintf( buffer, "No timeout path was defined for the Timer \"%s\" in Map \"%s\".",
		  identifier, parent_map->MapLabel() );
       ErrorMessage( buffer, TRUE );
       msc->AbortGeneration();
       return( NULL );
   } else
       return( current_edge );
}

Hyperedge * Timer::GenerateMSC( bool first_pass )
{
   MSCGenerator *msc = MSCGenerator::Instance();
   Node *timeout_node;
   Hyperedge *timeout_path, *previous_edge, *current_edge;
   Hyperedge *next_scan, *return_value;
   ComponentReference *timer_component = NULL;
   char buffer[200];

   if( (timer_component = figure->GetContainingComponent()) == NULL )
      timer_component = msc->StubComponent();

   msc->ComponentCrossing( timer_component, first_pass );

   if( msc->ScenarioTracing() ) {
      // if this timer is triggered and the timeout variable is false it should be interpreted as a synchronization
      if( this->HasSourceColour( C ) && (timeout_variable->Value() == FALSE) ) { 
	 if( !InputsSynchronized() ) {
	    if( ForkSynchronization() == TRUE )
	       return_value = NULL; // allow Synchronization MSC method to do its job
	    else if( (next_scan = msc->NextScanningPath( NULL )) != NULL ) { // determine scenario start point if it exists
	       return_value = next_scan;
	       msc->ScenarioStartPoint( (Start *)next_scan, first_pass );
	    }
	    else { // there are no further paths to scan
	       sprintf( buffer, "Both paths did not synchronize at the Timer \"%s\" in Map \"%s\".",
			identifier, parent_map->MapLabel() );
	       ErrorMessage( buffer, TRUE );
	       msc->AbortGeneration();
	       return( NULL );	    
	    }
	    if( !first_pass ) {
	       msc->CreateAction( identifier );
	       msc->StartTimer( identifier );
	    }
	    return( return_value );	 
	 }

	 if( ForkSynchronization() == TRUE ) // if flag was set signifying that synchronization came from an and fork branch
	    msc->SetWaitingSynchronization( TRUE ); // notify previous fork to stop here instead of searching for an and join

      }
      if( timeout_variable->Value() == FALSE ) {
	 current_edge = PrimaryTarget()->NextEdge();
	 if( !first_pass ) msc->HaltTimer( identifier );
      }
      else { 
	 current_edge = this->ExceptionPath()->NextEdge();
	 if( !first_pass ) {
	    msc->CreateAction( identifier );
	    msc->StartTimer( identifier );
	    msc->Timeout( identifier );
	 }
      }
      return( current_edge );
   }
   else {

      if( !this->HasSourceColour( C ) ) // return output path immediately if no trigger path exists
	 return( PrimaryTarget()->NextEdge() );

      if( normal_path ) { // start timer and stop with event from environment, follow normal output path
	 if( !first_pass ) {
	    msc->CreateAction( identifier );
	    msc->StartTimer( identifier );
	 }
	 // find name of start point which triggers this timer
	 previous_edge = this->SourceOfColour( C )->PreviousEdge();
	 while( previous_edge->EdgeType() != START )
	    previous_edge = previous_edge->FirstInput();

	 if( !first_pass ) msc->StopTimer( identifier, timer_component, previous_edge );
	 return( PrimaryTarget()->NextEdge() );
      }
      else { // start timer, output timeout event, follow timeout path if it exists
	 if( !first_pass ) {
	    msc->CreateAction( identifier );
	    msc->StartTimer( identifier );
	    msc->Timeout( identifier );
	 }
	 timeout_node = this->ExceptionPath();
	 if( timeout_node != NULL )
	    timeout_path = timeout_node->NextEdge();
	 else
	    timeout_path = PrimaryTarget()->NextEdge(); // set default 'timeout' path to be the normal output path
	 return( timeout_path );
      }
   }
}

Hyperedge * Timer::TraverseScenario( )
{
   Hyperedge *current_edge;
   Hyperedge *next_scan;
   GenerationState gs;
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Node *timeout_node;
   char buffer[200];
   Path_data * time_out;
   ComponentReference * component = NULL;
   Path_data * trigger = NULL;
   
   // if this timer is triggered and the timeout variable is false it should be interpreted as a synchronization
   if( this->HasSourceColour( C ) && (timeout_variable->Value() == FALSE) ) { 
      if( !InputsSynchronization( trigger ) ) {
         timer_element.Add( trigger );
	 next_scan = sce->NextParallelBranch();
	 return next_scan;
      }
      else {
         timer_element.Add( trigger );
         timer_reset = new Action( hyperedge_number, identifier, Timer_Reset );
	 if( (component = figure->GetEnclosingComponent() ) != NULL ) {
            timer_reset->SetComponentName( component->ReferencedComponent()->GetLabel() );
	    timer_reset->SetComponentId( component->ReferencedComponent()->GetComponentNumber() );
	    timer_reset->SetComponentRole( component->ComponentRole() );
	    timer_reset->SetComponentReferenceId( component->ComponentReferenceNumber() );	    
         }

	 for( timer_element.First(); !timer_element.IsDone(); timer_element.Next() ) {
	      timer_element.CurrentItem()->AddNext( timer_reset );
	 }
	 sce->ResetCurrentPathData( timer_reset );
	 SearchRoot( timer_element, timer_reset );
	 Collapse( timer_element, timer_reset );	  
	 timer_element.Empty();
         current_edge = PrimaryTarget()->NextEdge();
	 return current_edge;
      }	 	    
   }
   // evaluate logical condition (if it exists) to determine if scanning should be halted or continued down the path after this wait
   if( EvaluateGuardCondition() == NULL )
      return( NULL );
      
   if( timeout_variable->IsNotInitialized() ) { // value is undefined
      sprintf( buffer, "The timeout variable \"%s\" for the Timer \"%s\" in Map \"%s\" was not initialized.",
		  timeout_variable->BooleanName(), identifier, parent_map->MapLabel() );
      ErrorMessage( buffer, TRUE );
      sce->AbortGeneration();
      return( NULL );	    
   }
   else {
      if( timeout_variable->Value() == FALSE ) {
          current_edge = PrimaryTarget()->NextEdge();
      }
      else {
         timer_set = new Action(hyperedge_number, identifier, Timer_Set);
	 // DA, August 2004: Added to include component info in timers (set and timeout)
	 if( (component = figure->GetEnclosingComponent() ) != NULL ) {
            timer_set->SetComponentName( component->ReferencedComponent()->GetLabel() );
	    timer_set->SetComponentId( component->ReferencedComponent()->GetComponentNumber() );
	    timer_set->SetComponentRole( component->ComponentRole() );
	    timer_set->SetComponentReferenceId( component->ComponentReferenceNumber() );	    
         }
	 
         sce->AddPathTraversal( timer_set );
         time_out = new Action( hyperedge_number, identifier, Time_Out );
	 // DA, August 2004: Added to include component info in timers (set and timeout)
	 if( (component = figure->GetEnclosingComponent() ) != NULL ) {
            time_out->SetComponentName( component->ReferencedComponent()->GetLabel() );
	    time_out->SetComponentId( component->ReferencedComponent()->GetComponentNumber() );
	    time_out->SetComponentRole( component->ComponentRole() );
	    time_out->SetComponentReferenceId( component->ComponentReferenceNumber() );	    
         }
	 
	 sce->AddPathTraversal( time_out );
	 timeout_node = this->ExceptionPath();
	 if( timeout_node != NULL )
            current_edge = this->ExceptionPath()->NextEdge();
         else
            current_edge = NULL;
      }    
   }
     
   if( current_edge == NULL ) {
       sprintf( buffer, "No timeout path was defined for the Timer \"%s\" in Map \"%s\".",
		  identifier, parent_map->MapLabel() );
       ErrorMessage( buffer, TRUE );
       sce->AbortGeneration();
       return( NULL );
   } else
       return( current_edge );
}

Path_data * Timer::CreateAction()
{
   Path_data * p_data;
   
   p_data = new Action(hyperedge_number, identifier, Timer_Set );
   return p_data;
}

void Timer::EmptyPathData()
{
   timer_element.Empty();
}

void Timer::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
