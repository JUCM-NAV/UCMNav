/***********************************************************
 *
 * File:			msc_generator.cc
 * Author:			Andrew Miga
 * Created:			November 1999
 *
 * Modification history:
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "msc_generator.h"
#include "scenario.h"
#include "defines.h"
#include "component_ref.h"
#include "hyperedge.h"
#include "synchronization.h"
#include "print_mgr.h"
#include "start.h"
#include "stub.h"
#include "utilities.h"
#include "variable.h"
#include "display.h"
#include "wait_synch.h"
#include "wait.h"
#include <sys/stat.h>
#include "collection.h"

extern DisplayManager *display_manager;

MSCGenerator * MSCGenerator::SoleMSCGenerator = NULL;

#define OUTPUT_BYTE_LIMIT 50000000
#define VALUE(v) ((v == TRUE) ?  "T" : "F")

static char inline_structures[3][5] = { "par", "alt", "loop" };

static FD_MSCGenerationStatus *pfdmgs = NULL;

GenerationState::GenerationState()
{
   MSCGenerator *msc = MSCGenerator::Instance();

   parallel_depth = msc->CurrentParallelStack( parallel_stack );
   decomposition_level = msc->CurrentDecompositionStack( &decomposition_stack );
}

GenerationState::~GenerationState()
{
   if( decomposition_level > 0 ) free( decomposition_stack );
}

MSCGenerator::MSCGenerator()
{
   int i;
   
   decomposition_level = 0;
   memset( space_string, ' ', 70 );
   msc_basename[LABEL_SIZE] = 0;
   nesting_levels = 0;
   generation_id = 0;
   waiting_synchronization = FALSE;
   highlight_active = FALSE;

   for( i = 0; i < MAX_NESTING_LEVELS; i++ )
      fork_history[i].decomposition_stack = NULL;
   ClearParallelStack();

   for( i = 0; i < MAXIMUM_HIERARCHY_LEVELS; i++ )
      decomposition_stack[i] = NULL;
}

void MSCGenerator::ResetGenerationState( GenerationState *gs )
{
   ResetParallelStack( gs->parallel_stack, gs->parallel_depth );
   ResetDecompositionStack( gs->decomposition_stack, gs->decomposition_level );
}

bool MSCGenerator::UpdateElapsedTime()
{
   char elapsed[50];
   time_t elapsed_time, hours;
   static time_t previous_elapsed = 0;
   
   elapsed_time = time( NULL ) - start_time;
   if( elapsed_time != previous_elapsed ) {

      if( fl_check_forms() == pfdmgs->CancelButton ) {
	 abort_generation = TRUE;
	 return FALSE;;
      }

      if( elapsed_time < 60 )
	 sprintf( elapsed, "%d s", elapsed_time );
      else if( elapsed_time < 3600 )
	 sprintf( elapsed, "%d m %d s", elapsed_time/60, elapsed_time%60 );
      else {
	 hours = elapsed_time/3600;
	 elapsed_time -= hours*3600;
	 sprintf( elapsed, "%d h %d m %d s", hours, elapsed_time/60, elapsed_time%60 ); 
      }
      fl_set_object_label( pfdmgs->Time, elapsed );
      previous_elapsed = elapsed_time;
      return TRUE;
   } else
      return FALSE;
}

void MSCGenerator::IncrementScenarioCount()
{ 
   char count[10];

   scenario_count++; generation_id++;

   if( scenario_tracing == TRUE ) return;
   if( UpdateElapsedTime() ) {
      sprintf( count, "%d", scenario_count );
      fl_set_object_label( pfdmgs->Count, count );
   }

#ifdef MSC_DEBUG	       
   std::cout << "\n\nnew scenario started scenario count: " << scenario_count << " generation id: " 
	<< generation_id << std::endl;
#endif
}

MSCGenerator * MSCGenerator::Instance()
{
   if( SoleMSCGenerator == NULL )      
      SoleMSCGenerator = new MSCGenerator();
   
   return( SoleMSCGenerator );
}

void MSCGenerator::ClearParallelStack()
{
   for( int i = 0; i < MAX_NESTING_LEVELS; i++ ) {
      fork_history[i].fork = NULL;
      fork_history[i].current_branch = 0;
      if( fork_history[i].decomposition_stack != NULL )
	 free( fork_history[i].decomposition_stack );
      fork_history[i].decomposition_stack = NULL;
      fork_history[i].decomposition_level = 0;
   }

}

void MSCGenerator::GenerateAllMSCPaths( Start *start )
{
   char count[10];
   const char *basename;
   FL_OBJECT *returned;
   bool basename_invalid;

   // execute recursive algorithm to determine all possible scenarios by
   // taking all possible or fork branch combinations and all plugins of dynamic stubs
   scenario_tracing = FALSE;
   abort_generation = FALSE;
   ClearErrorLog();

   if( pfdmgs == NULL )
      pfdmgs = create_form_MSCGenerationStatus();

   scenario_start_point = start;
   scenario_start_component = start->GetFigure()->GetContainingComponent();
   previous_component = scenario_start_component;
   scenario_start = TRUE; // flag signifying need for initial environment message
   fl_set_object_label( pfdmgs->Count, "" );
   fl_set_object_label( pfdmgs->Time, "" );
   fl_set_object_label( pfdmgs->CurrentScenario, "" );
   fl_set_object_label( pfdmgs->BytesWritten, "" );
   fl_set_object_lcol( pfdmgs->GenerateButton, FL_INACTIVE );
   fl_set_object_lcol( pfdmgs->CurrentNumberLabel, FL_INACTIVE );
   fl_set_input( pfdmgs->ScenarioBasename, start->HyperedgeName() );

   time( &start_time );
   fl_show_form( pfdmgs->MSCGenerationStatus, FL_PLACE_CENTER, FL_TRANSIENT, "Generating All Possible MSC Scenarios" );

   ScanScenarios( MSC_FIRST_PASS );

   sprintf( count, "%d", scenario_count ); // output final count
   fl_set_object_label( pfdmgs->Count, count );

   if( abort_generation ) {
      fl_hide_form( pfdmgs->MSCGenerationStatus );
      TerminateTraversal();
      return;
   }

   fl_set_object_lcol( pfdmgs->GenerateButton, FL_BLACK );

   do {
      do {
	 returned = fl_do_forms();
      } while( returned != pfdmgs->GenerateButton && returned != pfdmgs->CancelButton );

      if( returned == pfdmgs->CancelButton ) {
	 fl_hide_form( pfdmgs->MSCGenerationStatus );
	 return;
      }

      basename = fl_get_input( pfdmgs->ScenarioBasename );
      if( PrintManager::TextNonempty( basename ) )
	 basename_invalid = FALSE;
      else {
	 fl_set_resource( "flAlert.title", "Error: Empty Scenario Basename Detected" );
	 fl_show_alert( "A valid prefix is needed for all file names.", "Please enter a nonempty prefix." , "", 0 );
	 basename_invalid = TRUE;
      }
   } while( basename_invalid );

   fl_set_object_lcol( pfdmgs->CurrentNumberLabel, FL_BLACK );
   fl_set_object_label( pfdmgs->Time, "" );
   strncpy( msc_basename, MscName( basename ), LABEL_SIZE );   
   time( &start_time );
   bytes_written = 0;

   ScanScenarios( MSC_GENERATION_PASS );
   fl_hide_form( pfdmgs->MSCGenerationStatus );
   TerminateTraversal();
}

void MSCGenerator::GenerateScenarioMSC( Scenario *scenario )
{
   char buffer[200];

   // evaluate logical conditions at or fork branches and plugin bindings
   // based on variable initializations specified by the scenario
   // to trace the defined path for the scenario
   
   ClearErrorLog();
   if( !scenario->HasStartingPoints() ) {
      sprintf( buffer, "Scenario \"%s\" does not have a start point specified. MSC generation aborted", scenario->Name() );
      ErrorMessage( buffer, TRUE );
      return;
   }

   BooleanVariable::ScenarioInitialize( scenario->VariableInitializations() );
   scenario_tracing = TRUE;
   abort_generation = FALSE;
   current_scenario = scenario;
   scenario_start_index = 0;
   strncpy( msc_basename, MscName( scenario->Name() ), LABEL_SIZE );   
   ScanScenarios( MSC_GENERATION_PASS );   
   TerminateTraversal();
   ValidatePostconditionConsistency();
}

void MSCGenerator::HighlightScenario( Scenario *scenario )
{
   BooleanVariable::ScenarioInitialize( scenario->VariableInitializations() );
   scenario_tracing = TRUE;
   highlight_active = TRUE;
   abort_generation = FALSE;
   current_scenario = scenario;
   scenario_start_index = 0;
   ClearErrorLog();
   ScanScenarios( SCENARIO_TRACE );
   TerminateTraversal();
   ValidatePostconditionConsistency();
   highlight_active = FALSE;
}

void MSCGenerator::ValidatePostconditionConsistency()
{
   bool inconsistency_found = FALSE;
   Cltn<VariableInit *> *postconditions = current_scenario->ScenarioPostconditions();
   VariableInit *pc;
   char buffer[200];

   if( abort_generation == TRUE ) return; // don't check consistency for incomplete traversals

   for( postconditions->First(); !postconditions->IsDone(); postconditions->Next() ) {
      pc = postconditions->CurrentItem();
      if( pc->Variable()->IsNotInitialized() || pc->Variable()->Value() != pc->Value() ) {
	 if( !inconsistency_found ) {
	    sprintf( buffer, "Postcondition Inconsistencies for Scenario \"%s\"", current_scenario->Name() );
	    ErrorMessage( buffer, TRUE );
	    inconsistency_found = TRUE;
	 }
	 sprintf( buffer, "variable \"%s\" postcondition: %s actual value: %s", pc->Variable()->BooleanName(),
		  VALUE(pc->Value()), ((pc->Variable()->IsNotInitialized()) ? "uninitialized" : VALUE(pc->Variable()->Value()) ));
	 ErrorMessage( buffer );
      }
   }

   if( inconsistency_found )
      ErrorMessage( "<------------------------------------------>" );
}

void MSCGenerator::ScanScenarios( scan_type type )
{
   Hyperedge *current_edge, * current_element;

   Synchronization::ResetHistory();
   ClearParallelStack();
   current_scan = type;
   scenario_count = 0;
   decomposition_level = 0;
   environment_reference = FALSE;
   char buffer[200];
   edge_type etype;
   
   if( scenario_tracing ) {
      while( scenario_start_index < current_scenario->StartingPoints()->Size() ) { // scan all unscanned scenario starting points
	 scenario_start_point = current_scenario->StartingPoints()->Get( ++scenario_start_index );
	 current_edge = scenario_start_point;
	 while( current_edge ) {
	    if( type == SCENARIO_TRACE ) {
	       current_edge = current_edge->HighlightScenario( );
	    }
	    else {
	       current_edge = current_edge->ScanScenarios( type );
	    }
	 }
         if( abort_generation == TRUE )
      	    break;
      }
      for( path_elements.First(); ! path_elements.IsDone();  path_elements.Next() ) {
          current_element = path_elements.CurrentItem();
	  etype = current_element->EdgeType();
	  if( etype == WAIT ||  etype == TIMER) {
	      if ((( WaitSynch * )current_element)->getWaitContexts()->Size() != 0 ) {
	           sprintf( buffer, "Both paths did not synchronize at the Wait point \"%s\".",
		    current_element->HyperedgeName());
	           ErrorMessage( buffer, TRUE );
	           AbortGeneration();
		   break;
	      }
	  }	  
      }	            
   }
   else {
      current_edge = scenario_start_point;
      while( current_edge )
	 current_edge = current_edge->ScanScenarios( type );
   }
}

const char * MSCGenerator::ComponentRole( ComponentReference *ref )
{
   static char role[LABEL_SIZE+DISPLAYED_LABEL+10];

   if( PrintManager::TextNonempty( ref->ComponentRole() ) ) {
      sprintf( role, "%s[%s]", ref->ReferencedComponent()->GetLabel(), ref->ComponentRole() );
      return( MscName( role ) );
   }
   else
      return( MscName( ref->ReferencedComponent()->GetLabel() ) );
}

void MSCGenerator::ScenarioStartPoint( Start *start, bool first_pass )
{ 
   scenario_start = TRUE; // flag signifying need for initial environment message
   scenario_start_point = start;
   if( (scenario_start_component = scenario_start_point->GetFigure()->GetContainingComponent()) != NULL ) {
     if( first_pass ) {
	 AddComponent( scenario_start_component );
	 scenario_start_component->IncrementTraversalCount(); // TEST LINE
     }
   } else
      environment_reference = TRUE;

   previous_component = scenario_start_component;
}

void MSCGenerator::GenerateScenario()
{ // generate MSC statements for Telelogic MSC PR ( Phrasal Representation )
   char msc_name[LABEL_SIZE+5], filename[LABEL_SIZE+10], count[10], written[20];
  // ComponentReference *start_component;

   if( scenario_tracing ) {
      if( scenario_start_index < current_scenario->StartingPoints()->Size() ) // determine if unscanned scenario starting points exist
	 return; // don't generate scenario at this point
   }

   if( (scenario_tracing == TRUE) && (scenario_count == 0) ) // disable numbering for first ( possibly only ) defined scenario
      strcpy( msc_name, msc_basename );
   else
      sprintf( msc_name, "%s_%d", msc_basename, ++scenario_count );
   sprintf( filename, "%s.msc", msc_name );
   std::ofstream msc_file( filename );
   msc = &msc_file;
   message_count = 0; // total number of messages
   unnamed_count = 0; // number of unnamed messages
   generation_id++; // increment generation identifier

   msc_file << "mscdocument " << msc_name << ";\n"; // output document name
   msc_file << "msc " << msc_name << ";\n\n"; // output name

   if( scenario_tracing == TRUE ) {
      Cltn<VariableInit *> *vinit = current_scenario->VariableInitializations();
      VariableInit *ci;
      time_t ct;

      msc_file << "/* UCM Design: " << display_manager->DesignName() << std::endl;
      if( display_manager->DesignDescription() != NULL )
	 msc_file << display_manager->DesignDescription() << std::endl;
      time(&ct);
      msc_file << ctime( &ct );
      msc_file << "scenario: " << current_scenario->Name() << std::endl;
      msc_file << "scenario group: " << current_scenario->Group()->GroupName() << std::endl;
      if( current_scenario->Description() != NULL )
	 msc_file << "scenario description: " << current_scenario->Description() << std::endl;
      msc_file << "variable initializations:" << std::endl;
      for( vinit->First(); !vinit->IsDone(); vinit->Next() ) {
	 ci = vinit->CurrentItem();
	 msc_file << ci->Variable()->BooleanName() << " = " << ((ci->Value() == TRUE) ? "T" : "F") << std::endl;
      } 
      msc_file << "*/\n\n";
   }

   if( scenario_tracing ) {
      BooleanVariable::ScenarioInitialize( current_scenario->VariableInitializations() ); // reset variables as responsibilities may have changed them
      scenario_start_index = 0;
   }
   TraverseScenario( TRUE ); // first pass, determine components, component traversal counts

   // output component declarations
   if( environment_reference )
      msc_file << "Environment: instance;\n"; // output declaration of environment component
   for( component_traversals.First(); !component_traversals.IsDone(); component_traversals.Next() )
      msc_file << ComponentRole( component_traversals.CurrentItem() ) << ": instance;\n";
   msc_file << std::endl;

   if( scenario_tracing == TRUE ) {
      msc_file << "text \'scenario: " << current_scenario->Name();
      if( current_scenario->Description() != NULL )
	 msc_file << " description: " << current_scenario->Description();
      msc_file << "\';\n\n";
   }

   if( scenario_tracing ) {
      BooleanVariable::ScenarioInitialize( current_scenario->VariableInitializations() ); // reset variables as responsibilities may have changed them
      scenario_start_index = 0;
   }
   TraverseScenario(); // second pass, generate rest of messages

   // terminate all remaining components
   for( component_traversals.First(); !component_traversals.IsDone(); component_traversals.Next() )
      msc_file << ComponentRole( component_traversals.CurrentItem() ) << ": endinstance;\n";

   if( environment_reference )
      msc_file << "Environment: endinstance;\n"; // output endinstance statement for environment object
   msc_file << "endmsc;\n";
   msc_file.close();
   component_traversals.Empty();

   if( scenario_tracing == FALSE ) {
      struct stat fs;
      stat( filename, &fs ); 
      bytes_written += fs.st_size;
      if( bytes_written > OUTPUT_BYTE_LIMIT )
	 abort_generation = TRUE;
      if( UpdateElapsedTime() ) {
	 sprintf( count, "%d", scenario_count );
	 fl_set_object_label( pfdmgs->CurrentScenario, count );
	 sprintf( written, "%d", bytes_written );
	 fl_set_object_label( pfdmgs->BytesWritten, written );
      }
   }
}

void MSCGenerator::TraverseScenario( bool first_pass )
{
   Hyperedge *current_edge;

   Synchronization::ResetHistory();
   previous_component = NULL;
   current_component = NULL;
   inline_construct = 0;

   if( scenario_tracing ) {
      while( scenario_start_index < current_scenario->StartingPoints()->Size() ) { // scan all unscanned scenario starting points
	 scenario_start_point = current_scenario->StartingPoints()->Get( ++scenario_start_index );
	 ScenarioStartPoint( scenario_start_point, first_pass );
	 current_edge = scenario_start_point;
	 while( current_edge )
	    current_edge = current_edge->GenerateMSC( first_pass );
      }
   } else {
      current_edge = scenario_start_point;
      while( current_edge )
	 current_edge = current_edge->GenerateMSC( first_pass );
   }
} 

void MSCGenerator::TerminateTraversal()
{
   while( !path_elements.is_empty() )
      path_elements.Detach()->DeleteGenerationData();
}

void MSCGenerator::PreviousComponent( ComponentReference *cp )
{
   if( cp == NULL )
      previous_component = NULL;
   else {
      if( cp->ReferencedComponent() == NULL )
	 previous_component = NULL; 
      else
	 previous_component = cp;
   }
}

bool MSCGenerator::NewComponentRole( ComponentReference *cr )
{
   if( cr == previous_component )
      return FALSE;

   if( cr == NULL )
      return( previous_component == NULL ? FALSE : TRUE );
   else if( cr->ReferencedComponent() == NULL )
      return( previous_component == NULL ? FALSE : TRUE );
   else {
      if( previous_component == NULL )
	return TRUE; 
   }      

   if( cr->ReferencedComponent() == previous_component->ReferencedComponent() ) {
      if( strequal( cr->ReferencedComponent()->GetLabel(), previous_component->ReferencedComponent()->GetLabel() ))
	 return FALSE;
   }
      
   return TRUE;
}

void MSCGenerator::ComponentCrossing( ComponentReference *component, bool first_pass )
{
   if( component == NULL ) return;
   if( component->ReferencedComponent() == NULL ) return;

   if( scenario_start && !first_pass ) { // interpret starting point as a condition rather than a message	 
      if( component == scenario_start_component ) {
	 if( strequal( component->ComponentRole(), scenario_start_component->ComponentRole() )) {
	    CreateCondition( scenario_start_point->HyperedgeName() );
	    scenario_start = FALSE;
	    return;
	 }
      }
   }

   // handles cases where different component references refer to the same component with the same role
   if( component == previous_component )
      return;
   else if( component != previous_component && previous_component != NULL ) { 
      if( component->ReferencedComponent() == previous_component->ReferencedComponent() ) {
	 if( strequal( component->ComponentRole(), previous_component->ComponentRole() ))
	    return;
      }
   }

   if( first_pass ) {
      AddComponent( component );
      component->IncrementTraversalCount();
   }
   else
      CreateMessage( component );
   previous_component = component;
}

void MSCGenerator::CreateMessage( ComponentReference *present_component, Hyperedge *path_end )
{
   char message[40], prev_comp_name[150];
   const char *pc_msc_name;

   if( scenario_start ) {
      sprintf( message, "%s,%d", MscName( scenario_start_point->HyperedgeName() ), ++message_count );
      if( scenario_start_component == NULL ) {
	 pc_msc_name = ComponentRole( present_component );
	 *msc << "Environment: out " << message << " to " << pc_msc_name << ";\n";
	 *msc << pc_msc_name << ": in " << message << " from Environment;\n";
      } else {
	 strcpy( prev_comp_name, ComponentRole( scenario_start_component ) );
	 pc_msc_name = ComponentRole( present_component );
	 *msc << prev_comp_name << ": out " << message << " to " << pc_msc_name << ";\n";
	 *msc << pc_msc_name << ": in " << message << " from " << prev_comp_name << ";\n";
      }
      scenario_start = FALSE;
      current_component = present_component;
   } else {

      if( previous_component == NULL ) return;

      if( path_end ) // create message string
	 sprintf( message, "%s,%d", MscName( path_end->HyperedgeName() ), ++message_count );
      else
	 sprintf( message, "m%d,%d", ++unnamed_count, ++message_count );

      PrintComponentLabel(); // either prints the component label or indents line

      if( present_component ) { // determine string for present component
	 if( previous_component->ReferencedComponent() != NULL )
	    pc_msc_name = ComponentRole( present_component );
	 else
	    pc_msc_name = "Environment";
      } else
	 pc_msc_name = "Environment";

      *msc << "out " << message << " to " << pc_msc_name << ";\n"; // output msc out statement
      *msc << pc_msc_name << ": in " << message; // output msc in statement
      *msc << " from " << ComponentRole( previous_component ) << ";\n";
      current_component = present_component;
      if( previous_component->FinalTraversal() ) { // output endinstance statement if component traversals are complete
	 if( inline_construct ) // store terminating components for output after parallel construct is finished
	    TerminateComponent( previous_component );
	 else {
	    *msc << ComponentRole( previous_component ) << ": endinstance;\n";
	    component_traversals.Delete( previous_component );
	    current_component = NULL;
	 }
      }
   }
}

void MSCGenerator::CreateAction( const char *resp_name )
{
   PrintComponentLabel();
   *msc << "action \'" << MscName( resp_name ) << "\';\n";
}

void MSCGenerator::CreateCondition( const char *state_name )
{
   if( PrintManager::TextNonempty( state_name ) ) {
      PrintComponentLabel();
      *msc << "condition [" << MscName( state_name ) << "];\n";
   }
}

void MSCGenerator::StartInlineConstruct( inline_type itype )
{
   *msc << "all: " << inline_structures[itype] << " begin;\n";
   inline_construct++;
   current_component = NULL;
}

void MSCGenerator::NewInlineBranch( inline_type itype, Stub **ss, int level, bool first_pass )
{
   int i;

   if( !first_pass )
      /* all: removed by D. Amyot on 2002/11/13 because of incompatibility with Telelogic Tau 4.4 */
      *msc /* << "all: " */ << inline_structures[itype] << ";\n";
   if( level > 0 ) {
      for( i = 0; i < level; i++ )
	 decomposition_stack[i] = ss[i];
      decomposition_level = level;
   }
   current_component = NULL;
}

void MSCGenerator::EndInlineConstruct( inline_type itype )
{
   /* all: removed by D. Amyot on 2002/11/13 because of incompatibility with Telelogic Tau 4.4 */
   *msc /* << "all: " */ << inline_structures[itype] << " end;\n";
   current_component = NULL;

   if( --inline_construct == 0 ) { // output list of components that have terminated during the Inline construct

      for( component_terminations.First(); !component_terminations.IsDone(); component_terminations.Next() ) {
	 *msc << ComponentRole( component_terminations.CurrentItem() ) << ": endinstance;\n";
	 component_traversals.Delete( component_terminations.CurrentItem() );
      }
      
      while( !component_terminations.is_empty() )
	 component_terminations.Detach();
   }
}

int MSCGenerator::CurrentDecompositionStack( Stub ***ss )
{
   Stub **ds;

   if( decomposition_level == 0 )
      return( 0 );

   ds = (Stub **)malloc( decomposition_level*sizeof( Stub * ) );

   for( int i = 0; i < decomposition_level; i++ )
      ds[i] = decomposition_stack[i];
   
   *ss = ds;
   return( decomposition_level );
}

void MSCGenerator::ResetDecompositionStack( Stub **ss, int level )
{
   for( int i = 0; i < level; i++ )
      decomposition_stack[i] = ss[i];

   decomposition_level = level;
}

void MSCGenerator::DecomposeStub( Stub *stub )
{
   if( decomposition_level > 0 ) {
      if( decomposition_stack[decomposition_level-1] != stub )
	 decomposition_stack[decomposition_level++] = stub;
   }
   else
      decomposition_stack[decomposition_level++] = stub;
}

Stub * MSCGenerator::GetUpperLevelStub()
{
   return( decomposition_stack[--decomposition_level] );
}

bool MSCGenerator::SubmapRecursionExists( Map *plugin )
{
   for( int i = 0; i < decomposition_level; i++ ) {
      if( decomposition_stack[i]->ParentMap() == plugin )
	 return TRUE;
   }

   if( scenario_start_point->ParentMap() == plugin )
      return TRUE;
   else
      return FALSE;
}

ComponentReference * MSCGenerator::StubComponent()
{
   int level;
   ComponentReference *ec;

   for( level = decomposition_level-1; level >= 0; level-- ) {
      if( (ec = decomposition_stack[level]->GetFigure()->GetContainingComponent()) != NULL )
	 return( ec );
   }

   return( NULL );
}

void MSCGenerator::MapContext( int **context, int &depth )
{
   int *stub_history, i;

   if( decomposition_level > 0 )
      stub_history = new int[decomposition_level];

   for( i = 0; i < decomposition_level; i++ )
      stub_history[i] = decomposition_stack[i]->GetNumber();

   depth = decomposition_level;
   *context = ( decomposition_level > 0 ) ? stub_history : NULL;
}

void MSCGenerator::OutputStatement( const char *statement )
{
   *msc << statement << ";\n";
   current_component = NULL;
}

void MSCGenerator::PrintComponentLabel()
{
   if( previous_component == NULL ) return;

   if( previous_component != current_component ) { // output component name if necessary
      *msc << ComponentRole( previous_component ) << ": ";
      current_component = previous_component;
   } else // indent message line as name is not repeated
      msc->write( space_string, (strlen( previous_component->ReferencedComponent()->GetLabel() )+strlen( previous_component->ComponentRole() ) +2) ); 
}

void MSCGenerator::AddComponent( ComponentReference *nc )
{
   for( component_traversals.First(); !component_traversals.IsDone(); component_traversals.Next() ) {
      if( component_traversals.CurrentItem()->ReferencedComponent() == nc->ReferencedComponent() ) {
	 if( strequal( component_traversals.CurrentItem()->ComponentRole(), nc->ComponentRole() ) )
	    return; // as the same component in the same role was found
      }
   }

   component_traversals.Add( nc ); // as this component in this role does not yet exist in list
   nc->ResetTraversalCount();
}

void MSCGenerator::TerminateComponent( ComponentReference *nc )
{
   for( component_terminations.First(); !component_terminations.IsDone(); component_terminations.Next() ) {
      if( component_terminations.CurrentItem()->ReferencedComponent() == nc->ReferencedComponent() ) {
	 if( strequal( component_terminations.CurrentItem()->ComponentRole(), nc->ComponentRole() ) )
	    return; // as the same component in the same role was found
      }
   }

   component_terminations.Add( nc ); // as this component in this role does not yet exist in list
}

int MSCGenerator::CurrentParallelStack( fork_traversal *temp_stack )
{
   int i;

   if( nesting_levels > 0 ) {
      for( i = 0; i < nesting_levels; i++ ) {
	 temp_stack[i].fork = fork_history[i].fork;
	 temp_stack[i].current_branch = fork_history[i].current_branch;
	 temp_stack[i].decomposition_stack = fork_history[i].decomposition_stack;
	 temp_stack[i].decomposition_level = fork_history[i].decomposition_level;
      }
   }
   return( nesting_levels );
}

void MSCGenerator::ResetParallelStack( fork_traversal *new_stack, int depth )
{
   int i;

   for( i = 0; i < depth; i++ ) {
      fork_history[i].fork = new_stack[i].fork;
      fork_history[i].current_branch = new_stack[i].current_branch;
      fork_history[i].decomposition_stack = new_stack[i].decomposition_stack ;
      fork_history[i].decomposition_level = new_stack[i].decomposition_level;
   }
   nesting_levels = depth;
}

void MSCGenerator::AddParallelToStack( Synchronization *fork )
{ 
   if( nesting_levels > 0 ) {
      if( fork_history[nesting_levels-1].fork == fork )
	 return;
   }

   fork_history[nesting_levels].fork = fork;
   fork_history[nesting_levels].current_branch = 1;
   fork_history[nesting_levels].decomposition_level = this->CurrentDecompositionStack( &fork_history[nesting_levels].decomposition_stack );
   nesting_levels++;
}

Hyperedge * MSCGenerator::NextParallelBranch()
{
   Synchronization *previous_fork = NULL;
   int current_branch, branch_count;
   bool fork_search = TRUE;

   while( fork_search ) {
      fork_search = FALSE; 
      CurrentParallelBranch( &previous_fork, current_branch );
      if( previous_fork ) {
	 branch_count = previous_fork->TargetSet()->Size();
	 if( current_branch < branch_count ) { // more branches of the previous fork need to be scanned
	    IncrementParallelBranch();
	    if( current_branch == branch_count - 1 ) {
	      EndParallelTraversal();
            }
	    return( previous_fork->TargetSet()->Get( current_branch+1 )->NextEdge() );
	 }
	 else {
	    EndParallelTraversal(); // remove previous fork from stack
	    fork_search = TRUE; // continue search for previous outstanding fork
	 }
      }	    
   }

   return( NULL );
}

Hyperedge * MSCGenerator::NextScanningPath( Hyperedge *synch )
{// use advance scans to determine if either an outstanding and fork branch or an unscanned scenario start point leads to element synch
   Synchronization *previous_fork = NULL;
   int current_branch, branch_count;
   GenerationState *gs;
   Hyperedge *current_edge, *unscanned_branch;

   if( synch != NULL ) {
      CurrentParallelBranch( &previous_fork, current_branch );
      if( previous_fork ) {
	 branch_count = previous_fork->TargetSet()->Size();
	 if( current_branch < branch_count ) { // more branches of the previous fork need to be scanned
	    IncrementParallelBranch();
	    unscanned_branch = previous_fork->TargetSet()->Get( current_branch+1 )->NextEdge();
	    synch_element = synch;
	    parallel_synch = FALSE;
	    current_edge = unscanned_branch;
	    gs = new GenerationState;
	    while( current_edge ) // perform an advance scan trying to find synch_element
	       current_edge = current_edge->ScanScenarios( ADVANCE_SCAN );
	    if( parallel_synch ) {
	       ResetDecompositionStack( fork_history[nesting_levels-1].decomposition_stack, fork_history[nesting_levels-1].decomposition_level );
	       delete gs;
	       return( unscanned_branch );
	    }
	    else {
	       fork_history[nesting_levels-1].current_branch--;
	       ResetGenerationState( gs );
	       delete gs;
	    }
	 }
      }
   }

   if( scenario_tracing == FALSE ) // scanning multiple paths requires scenario definitions with multiple start points
      return( NULL );

   // determine if scenario has any more start points that need to be scanned
   if( scenario_start_index < current_scenario->StartingPoints()->Size() )
      return( current_scenario->StartingPoints()->Get( ++scenario_start_index ) );
   else
      return( NULL ); // there are no other paths to be scanned
}

void MSCGenerator::CurrentParallelBranch( Synchronization **fork, int &branch )
{
   if( nesting_levels > 0 ) {
      *fork = fork_history[nesting_levels-1].fork;
      branch = fork_history[nesting_levels-1].current_branch;
      //ResetDecompositionStack( fork_history[nesting_levels-1].decomposition_stack, fork_history[nesting_levels-1].decomposition_level );

   }
   else {
      *fork = NULL;
      branch = 0;
   }
}

void MSCGenerator::IncrementParallelBranch()
{
   fork_history[nesting_levels-1].current_branch++;
   ResetDecompositionStack( fork_history[nesting_levels-1].decomposition_stack, fork_history[nesting_levels-1].decomposition_level );
}

void MSCGenerator::EndParallelTraversal()
{
   nesting_levels--;
   fork_history[nesting_levels].fork = NULL;
   fork_history[nesting_levels].current_branch = 0;
   fork_history[nesting_levels].decomposition_stack = NULL;
   fork_history[nesting_levels].decomposition_level = 0;
}

void MSCGenerator::StartTimer( const char *timer_name )
{
   PrintComponentLabel(); // either prints the component label or indents line
   /* Modified by D. Amyot on 2002/11/13 because of incompatibility with Telelogic Tau 4.4 */
   /* *msc << "starttimer " << MscName( timer_name ) << ";\n"; */
   *msc << "set " << MscName( timer_name ) << "," << MscName( timer_name ) << ";\n";
}

void MSCGenerator::StopTimer( const char *timer_name, ComponentReference *timer_component, Hyperedge *start )
{
   EnvironmentTrigger( timer_component, start );
   /* Modified by D. Amyot on 2002/11/13 because of incompatibility with Telelogic Tau 4.4 */
   /* *msc << "stoptimer " <<  MscName( timer_name ) << ";\n"; */
   *msc << "reset " << MscName( timer_name ) << "," << MscName( timer_name ) << ";\n";
}

void MSCGenerator::HaltTimer( const char *timer_name )
{
   PrintComponentLabel(); // either prints the component label or indents line
   /* Modified by D. Amyot on 2002/11/13 because of incompatibility with Telelogic Tau 4.4 */
   /* *msc << "stoptimer " <<  MscName( timer_name ) << ";\n"; */
   *msc << "reset " << MscName( timer_name ) << "," << MscName( timer_name ) << ";\n";
}

void MSCGenerator::Timeout( const char *timer_name )
{
   PrintComponentLabel(); // either prints the component label or indents line
   /* Modified by D. Amyot on 2002/11/13 because of incompatibility with Telelogic Tau 4.4 */
   /* *msc << "timeout " <<  MscName( timer_name ) << ";\n"; */
   *msc << "timeout " << MscName( timer_name ) << "," << MscName( timer_name ) << ";\n";
}

void MSCGenerator::EnvironmentTrigger( ComponentReference *wait_component, Hyperedge *start )
{
   char message[40], sl[150];
   const char *start_location, *wc_msc_name = NULL;
   ComponentReference *start_component = NULL;

   sprintf( message, "%s,%d", MscName( start->HyperedgeName() ), ++message_count );

   if( (start_component = start->GetFigure()->GetContainingComponent()) != NULL ) {
      strcpy( sl, ComponentRole( start_component ) );
      start_location = sl;
   } else
      start_location = "Environment";

   if( wait_component != NULL ) {
      wc_msc_name = ComponentRole( wait_component );
      current_component = wait_component;
   }
   else if( current_component != NULL )
      wc_msc_name = ComponentRole( current_component );

   if( wc_msc_name != NULL ) {
      *msc << start_location << ": out " << message << " to " << wc_msc_name << ";\n";
      *msc << wc_msc_name << ": in " << message << " from " << start_location << ";\n";
   }
   PrintComponentLabel(); // either prints the component label or indents line
}

const char * MSCGenerator::MscName( const char *name )
{
   int i, len;
   static char msc_name[LABEL_SIZE+1];

   len = strlen( name );
   for( i = 0; i < len; i++ ) {
      if( (name[i] == ' ') || (name[i] == '-') ) // replace banned characters with underscores
	 msc_name[i] = '_';
      else
	 msc_name[i] = name[i];
   }
   msc_name[i] = 0;
   return( msc_name );
}

void MSCGenerator::AbortGeneration()
{
   char buffer[200];

   abort_generation = TRUE;

   if( current_scan == SCENARIO_TRACE )
      sprintf( buffer, "Scenario Highlighting of scenario \"%s\" aborted.", current_scenario->Name() );
   else if( scenario_tracing == TRUE )
      sprintf( buffer, "MSC generation of scenario \"%s\" aborted.", current_scenario->Name() );
   else
      strcpy( buffer, "MSC generation aborted." );

   ErrorMessage( buffer );	 
}

/*
void MSCGenerator::AddWaitSynchElements(WaitSynch * ws)
{
   for( waitsynch_elements.First(); !waitsynch_elements.IsDone(); waitsynch_elements.Next() ) {
      if( waitsynch_elements.CurrentItem() == ws ) {
         return;
      }
   }
   waitsynch_elements.Add(ws);
}

void MSCGenerator::RemoveWaitSynchElements(WaitSynch * ws)
{
   for( waitsynch_elements.First(); !waitsynch_elements.IsDone(); waitsynch_elements.Next() ) {
      if( waitsynch_elements.CurrentItem() == ws ) {
         waitsynch_elements.Remove(ws);
      }
   }
}

bool MSCGenerator::WaitSynchElementExist(WaitSynch * ws)
{
   for( waitsynch_elements.First(); !waitsynch_elements.IsDone(); waitsynch_elements.Next() ) {
      if( waitsynch_elements.CurrentItem() == ws ) {
         return TRUE;
      }
   }
   return FALSE;
}

bool MSCGenerator::WaitSynchElementsEmpty()
{
    return (waitsynch_elements.Size() == 0);
}
*/

/*
void MSCGenerator::AddTimerElements(Timer * tm)
{
   for( timer_elements.First(); !timer_elements.IsDone(); timer_elements.Next() ) {
      if( timer_elements.CurrentItem() == tm ) {
         return;
      }
   }
   timer_elements.Add(tm);
}
*/

/*
void MSCGenerator::RemoveTimerElements(Timer* tm)
{
   for( timer_elements.First(); !timer_elements.IsDone(); timer_elements.Next() ) {
      if( timer_elements.CurrentItem() == tm ) {
         timer_elements.Remove(tm);
      }
   }
}
*/

/*
bool MSCGenerator::TimerElementExist(Timer* tm)
{
   for( timer_elements.First(); !timer_elements.IsDone(); timer_elements.Next() ) {
      if( timer_elements.CurrentItem() == tm ) {
         return TRUE;
      }
   }
   return FALSE;
}
*/

/*
bool MSCGenerator::TimerElementsEmpty()
{
    return (timer_elements.Size() == 0);
}
*/

