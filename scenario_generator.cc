/***********************************************************
 *
 * File:			scenario_generator.cc
 * Author:			XiangYang He
 * Created:	                August 2002
 *
 * Modification history:        Mar 2003
 *                              Shuhua Cui
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "scenario_generator.h"
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
#include <sys/stat.h>
#include <iostream>
#include "xml_generator.h"
#include "time.h"

extern DisplayManager *display_manager;
 int  parID=0;  // global variable for par ID generation in DXL exporting to DOORS , added by Bo Jiang August 2004
 int  seqID=0;  // global variable for seq ID generation in DXL exporting to DOORS , added by Bo Jiang August 2004

SCENARIOGenerator * SCENARIOGenerator::SoleSCENARIOGenerator = NULL;

#define OUTPUT_BYTE_LIMIT 50000000
#define VALUE(v) ((v == TRUE) ?  "T" : "F")

//static FD_MSCGenerationStatus *pfdmgs = NULL;

SCENARIOGenerator::SCENARIOGenerator() 
{
   current_path_data = NULL; 
   nesting_levels = 0;

   current_stub = NULL;

}


void SCENARIOGenerator::setCurrentStub(Stub* aStub)
{ 
  current_stub = aStub;
}

Stub* SCENARIOGenerator::getCurrentStub()
{ 
  return current_stub;
}
  

void SCENARIOGenerator::IncrementScenarioCount()
{ 
//   char count[10];
   scenario_count++;
}

SCENARIOGenerator * SCENARIOGenerator::Instance()
{
   if( SoleSCENARIOGenerator == NULL )      
      SoleSCENARIOGenerator = new SCENARIOGenerator();
   
   return( SoleSCENARIOGenerator );
}

void SCENARIOGenerator::ClearParallelStack()
{
   for( int i = 0; i < MAX_NESTING_LEVELS; i++ ) {
      fork_history[i].fork = NULL;
      fork_history[i].current_branch = 0;
      fork_history[i].parent = NULL;
   }
}

void SCENARIOGenerator::TraverseScenarioGroup( Cltn<Scenario *>* scenarios )
{
   char buffer[200];
   FILE * outfile;
   GenerateScenarioGroup( outfile, scenarios );
  
   for( scenarios->First(); !scenarios->IsDone(); scenarios->Next() ) {
      current_scenario = scenarios->CurrentItem();
      ClearErrorLog();
      if( !current_scenario->HasStartingPoints() ) {
         sprintf( buffer, "Scenario \"%s\" does not have a start point specified.\n\
               scenario generation aborted", current_scenario->Name() );
         ErrorMessage( buffer, TRUE );
         return;
      }

      BooleanVariable::ScenarioInitialize( current_scenario->VariableInitializations() );
      abort_generation = FALSE;
      scenario_start_index = 0;
      ScanScenarios(); 
      SaveScenarioXML( outfile );
      TerminateTraversal();
      ValidatePostconditionConsistency();
      Path_data * toBedeleted = path_traversal.GetFirst();
      while( toBedeleted != NULL ) {
         toBedeleted = EmptyPathTraversal( toBedeleted );
      }
      path_traversal.Empty();
      ResetSynchElementStatus();
      synch_elements.Empty();

      if( abort_generation == TRUE ) {
         break;
      }
   }
   
   PrintEndXMLBlock(outfile, "group");
   PrintEndXMLBlock(outfile, "scenarios");
   fclose(outfile);

}

void SCENARIOGenerator::SaveScenarioGroupDXL(FILE * outfile, Cltn<Scenario *>* scenarios, char * parentID )
{
   char buffer[200];
  
   for( scenarios->First(); !scenarios->IsDone(); scenarios->Next() ) {
      current_scenario = scenarios->CurrentItem();
      ClearErrorLog();
      if( !current_scenario->HasStartingPoints() ) {
         sprintf( buffer, "Scenario \"%s\" does not have a start point specified.\n\
               scenario generation aborted", current_scenario->Name() );
         ErrorMessage( buffer, TRUE );
         return;
      }

      BooleanVariable::ScenarioInitialize( current_scenario->VariableInitializations() );
      abort_generation = FALSE;
      scenario_start_index = 0;
      ScanScenarios(); 
      SaveScenarioDXL( outfile, parentID );
      TerminateTraversal();
      ValidatePostconditionConsistency();
      Path_data * toBedeleted = path_traversal.GetFirst();
      while( toBedeleted != NULL ) {
         toBedeleted = EmptyPathTraversal( toBedeleted );
      }
      path_traversal.Empty();
      ResetSynchElementStatus();
      synch_elements.Empty();
 
      if( abort_generation == TRUE ) {
         break;
      }
   }
   IndentEndXMLBlock(outfile);
}


void SCENARIOGenerator::TraverseScenario(Scenario * scenario )
{
   char buffer[200];

   // evaluate logical conditions at or fork branches and plugin bindings
   // based on variable initializations specified by the scenario
   // to trace the defined path for the scenario
   
   ClearErrorLog();
   if( !scenario->HasStartingPoints() ) {
      sprintf( buffer, "Scenario \"%s\" does not have a start point specified.\n\
               scenario generation aborted", scenario->Name() );
      ErrorMessage( buffer, TRUE );
      return;
   }
   
   strncpy( scenario_basename, scenario->Name(), LABEL_SIZE ); 
   BooleanVariable::ScenarioInitialize( scenario->VariableInitializations() );
   abort_generation = FALSE;
   current_scenario = scenario;
   scenario_start_index = 0;
   ScanScenarios(); 
   GenerateScenario();
   TerminateTraversal();
   ValidatePostconditionConsistency();
   /*
   Path_data * toBedeleted = path_traversal.GetFirst();
   while( toBedeleted != NULL ) {
      toBedeleted = EmptyPathTraversal( toBedeleted );
   }*/
   path_traversal.Empty(); 
   EmptyPathList();  
   ResetSynchElementStatus();
   synch_elements.Empty();   
}

void SCENARIOGenerator::ValidatePostconditionConsistency()
{
   bool inconsistency_found = FALSE;
   Cltn<VariableInit *> *postconditions = 
                     current_scenario->ScenarioPostconditions();
   VariableInit *pc;
   char buffer[200];

   if( abort_generation == TRUE ) return; // don't check consistency 
                                          // for incomplete traversals

   for( postconditions->First(); !postconditions->IsDone(); 
                                  postconditions->Next() ) {
      pc = postconditions->CurrentItem();
      if( pc->Variable()->IsNotInitialized() || 
                       pc->Variable()->Value() != pc->Value() ) {
	 if( !inconsistency_found ) {
	    sprintf( buffer, "Postcondition Inconsistencies for Scenario \"%s\"",
                                         current_scenario->Name() );
	    ErrorMessage( buffer, TRUE );
	    inconsistency_found = TRUE;
	 }
	 sprintf( buffer, "variable \"%s\" postcondition: %s actual value: %s", 
                      pc->Variable()->BooleanName(),
		  VALUE(pc->Value()), ((pc->Variable()->IsNotInitialized()) 
                        ? "uninitialized" : VALUE(pc->Variable()->Value()) ));
	 ErrorMessage( buffer );
      }
   }

   if( inconsistency_found )
      ErrorMessage( "<------------------------------------------>" );
}

void SCENARIOGenerator::ScanScenarios()
{
   Hyperedge *current_edge, *current_element;
   Synchronization::ResetHistory();
   ClearParallelStack();
   scenario_count = 0;
   edge_type etype;
   char buffer[200];
   
   while( scenario_start_index < current_scenario->StartingPoints()->Size())
   { // scan all unscanned scenario starting points
       scenario_start_point = current_scenario->StartingPoints()->Get( ++scenario_start_index );
       current_edge = scenario_start_point;
       current_path_data = NULL;
       while( current_edge ){
	   current_edge = current_edge->TraverseScenario();
       }
       if( abort_generation == TRUE )                           
	   break;
   }  
   for( path_elements.First(); ! path_elements.IsDone();  path_elements.Next() ) {
       current_element = path_elements.CurrentItem();
       etype = current_element->EdgeType();
       if( etype == WAIT || etype == TIMER ) {
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

void SCENARIOGenerator::GenerateScenarioGroup( FILE * & outfile, Cltn<Scenario *> * scenarios )
{ 
   char buffer[100];
   char group_name[LABEL_SIZE+5], filename[LABEL_SIZE+10];
// char count[10], written[20];
//   ComponentReference *start_component;
//   Path_data * current_data;
//   Path_data * temp = NULL;
   ScenarioGroup * group = scenarios->GetFirst()->Group();
   
   sprintf( group_name, "%s", group->GroupName() );
   sprintf( filename, "%s.xml", group_name );
   if( !( outfile = fopen( filename, "w" )) ) {
      std::cout<<"Cannot open file"<<std::endl;
   }
   
   PrintXML( outfile, "?xml version='1.0' standalone='no'?" );
   PrintXML( outfile, "!DOCTYPE scenarios SYSTEM \"scenarios1.dtd\"" ); 
   LinebreakXML( outfile );

   time_t ct;
   struct tm * ptr;
   time(&ct);
   ptr = localtime(&ct);
   char * date = asctime(ptr);
   char new_date[50];
   RemoveNewLine( date, new_date );    
   sprintf( buffer, "<scenarios date =\"%s\" ", new_date );
    
   PrintXMLText( outfile, buffer );
    
   sprintf( buffer, "ucm-file = \"%s.ucm\" ", display_manager->DesignName() );
   PrintXMLText( outfile, buffer );

   if( display_manager->DesignName() != NULL ) {
      sprintf( buffer, "design-name = \"%s\" ", display_manager->DesignName() );
      PrintXMLText( outfile, buffer );
   }
    
   sprintf( buffer, "ucm-design-version = \"%d\"", display_manager->DesignId() );
   PrintXMLText( outfile, buffer );
   fprintf( outfile, ">\n" );
   IndentNewXMLBlock( outfile );
   sprintf( buffer, "group name = \"%s\" group-id = \"%d\" ", group->GroupName(), ScenarioList::ScenarioGroupId( group ));
   PrintNewXMLBlock( outfile,  buffer );
}


void SCENARIOGenerator::GenerateScenario()
{ 
   char buffer[100];
   char scenario_name[LABEL_SIZE+5], filename[LABEL_SIZE+10];
// char count[10], written[20];
//   ComponentReference *start_component;
//   Path_data * current_data;
//   Path_data * temp = NULL;
   
   if( scenario_start_index < current_scenario->StartingPoints()->Size() ) // determine if unscanned scenario starting points exist
	 return;        // don't generate scenario at this point
 
   if( scenario_count == 0 ) // disable numbering for first ( possibly only ) defined scenario
      strcpy( scenario_name, scenario_basename );
   else
      sprintf( scenario_name, "%s_%d", scenario_basename, ++scenario_count );
   sprintf( filename, "%s.xml", scenario_name );
   FILE * outfile;
   if( !( outfile = fopen( filename, "w" )) ) {
      std::cout<<"can not open file"<<std::endl;
   }
        
   PrintXML( outfile, "?xml version='1.0' standalone='no'?" );
   PrintXML( outfile, "!DOCTYPE scenarios SYSTEM \"scenarios1.dtd\"" ); 
   LinebreakXML( outfile );

   time_t ct;
   struct tm * ptr;
   time(&ct);
   ptr = localtime(&ct);
   char * date = asctime(ptr);
   char new_date[50];
   RemoveNewLine( date, new_date );    
   sprintf( buffer, "<scenarios date =\"%s\" ", new_date );
    
   PrintXMLText( outfile, buffer );
    
   sprintf( buffer, "ucm-file = \"%s.ucm\" ", display_manager->DesignName() );
   PrintXMLText( outfile, buffer );

   if( display_manager->DesignName() != NULL ) {
      sprintf( buffer, "design-name = \"%s\" ", display_manager->DesignName() );
      PrintXMLText( outfile, buffer );
   }
    
   sprintf( buffer, "ucm-design-version = \"%d\"", display_manager->DesignId() );
   PrintXMLText( outfile, buffer );
   fprintf( outfile, ">\n" );
   IndentNewXMLBlock( outfile );
   sprintf( buffer, "group name = \"%s\"  group-id = \"%d\" ", current_scenario->Group()->GroupName(), ScenarioList::ScenarioGroupId( current_scenario->Group() ) );
   PrintNewXMLBlock( outfile,  buffer );
   
   SaveScenarioXML(outfile);

   PrintEndXMLBlock(outfile, "group");
   PrintEndXMLBlock(outfile, "scenarios");
   fclose(outfile);
   	   
}

void SCENARIOGenerator::TerminateTraversal()
{
   while( !path_elements.is_empty() )
      path_elements.Detach()->DeleteGenerationData();
}

void SCENARIOGenerator::AddParallelToStack( Synchronization *fork, Path_data * new_parent )
{ 
   if( nesting_levels > 0 ) {
      if( (fork_history[nesting_levels-1].fork == fork) && (fork != NULL) )
	 return;
   }
   
   fork_history[nesting_levels].fork = fork;
   fork_history[nesting_levels].current_branch = 1;
   fork_history[nesting_levels].parent = new_parent;
   nesting_levels++;
}

Hyperedge * SCENARIOGenerator::NextParallelBranch()
{
   Synchronization *previous_fork = NULL;
   int current_branch, branch_count;
   bool fork_search = TRUE;
   
   while( fork_search ) {
      fork_search = FALSE; 
      CurrentParallelBranch( &previous_fork, current_branch );
      if( previous_fork ) {
	 branch_count = previous_fork->TargetSet()->Size();
	 if( current_branch < branch_count ) { // more branches of the previous
                                               // fork need to be scanned
	   IncrementParallelBranch();
	   if( current_branch == branch_count - 1 ) {
	      EndParallelTraversal();
           }
	   return(previous_fork->TargetSet()->Get(current_branch+1)->NextEdge());
	 }
	 else {
	    EndParallelTraversal(); // remove previous fork from stack
	    fork_search = TRUE; // continue search for previous outstanding fork
	 }
      }	    
   }

   return( NULL );
}

void SCENARIOGenerator::CurrentParallelBranch( Synchronization **fork, int &branch )
{
   if( nesting_levels > 0 ) {
      *fork = fork_history[nesting_levels-1].fork;
      branch = fork_history[nesting_levels-1].current_branch;
      current_path_data = fork_history[nesting_levels-1].parent;
   }
   else {
      *fork = NULL;
      branch = 0;
      current_path_data = NULL;
   }
   
}

void SCENARIOGenerator::IncrementParallelBranch()
{
   fork_history[nesting_levels-1].current_branch++;
}

void SCENARIOGenerator::EndParallelTraversal()
{
   nesting_levels--;
   fork_history[nesting_levels].fork = NULL;
   fork_history[nesting_levels].current_branch = 0;
   fork_history[nesting_levels].parent = NULL;
}

void SCENARIOGenerator::SaveScenarioXML( FILE * outfile )
{
   char buffer[200];
   Path_data * current_data = NULL;
//   Path_data * temp = NULL;
   
   ClearParallelStack();
   ResetScenarioStartIndex();
   IncrementScenarioStartIndex(); 
   
   sprintf( buffer, "<scenario name = \"%s\" scenario-definition-id = \"%d\" ", current_scenario->Name(), current_scenario->ScenarioId() );
   PrintXMLText( outfile, buffer );
   if( current_scenario->Description() != NULL ) {
       sprintf( buffer, "description = \"%s\" ", current_scenario->Description() );
      PrintXMLText( outfile, buffer );
   }
   fprintf( outfile, ">\n" );
   
   IndentNewXMLBlock( outfile );
   if( path_traversal.Size() > 1 ) {
      PrintNewXMLBlock(outfile, "par");
      for( path_traversal.First(); !path_traversal.IsDone(); path_traversal.Next() ) {
         current_data = path_traversal.CurrentItem();
	 PrintNewXMLBlock(outfile, "seq");
         while( (current_data != NULL) ) {      
            current_data = current_data->SaveScenarioXML(outfile);      
         }
         PrintEndXMLBlock(outfile, "seq");
      }
      PrintEndXMLBlock(outfile, "par");
   } 
   else {
      PrintNewXMLBlock(outfile, "seq");     
      current_data =  path_traversal.GetFirst();
      while( (current_data != NULL) ) {
         current_data = current_data->SaveScenarioXML(outfile);      
      }
      PrintEndXMLBlock(outfile, "seq");
   }
   
   PrintEndXMLBlock( outfile, "scenario" );
}

void SCENARIOGenerator::SaveScenarioDXL( FILE * outfile, char * parentID )
{
   char buffer[600];
   char tempBuffer[600];
   char parentIDBuffer[100];
   Path_data * current_data = NULL;
   
   ClearParallelStack();
   ResetScenarioStartIndex();
   IncrementScenarioStartIndex(); 
   
   if( PrintManager::TextNonempty( current_scenario->Description() ) ){
       sprintf( buffer, "scenario( \"%s\", \"%s_scenario-%s\", \"%s\" )\n", current_scenario->Name(), parentID,current_scenario->Name() ,removeNewlineinString(current_scenario->Description()));
   }
   else
       sprintf( buffer, "scenario( \"%s\", \"%s_scenario-%s\", \"\" )\n", current_scenario->Name(), parentID,current_scenario->Name());
   
   PrintXMLText( outfile, buffer );
   sprintf( buffer, "\"%s_scenario-%s\"", parentID, current_scenario->Name() );
   IndentNewXMLBlock( outfile );
   if( path_traversal.Size() > 1 ) {
      sprintf( tempBuffer, "par( \"par%d\", %s )\n", parID, buffer );
      PrintXMLText(outfile, tempBuffer);
      IndentNewXMLBlock( outfile );
      sprintf( parentIDBuffer, "\"par%d\"", parID );
      parID++;
      for( path_traversal.First(); !path_traversal.IsDone(); path_traversal.Next() ) {
         sprintf( tempBuffer, "seq( \"seq%d\", %s )\n", seqID, parentIDBuffer );
         PrintXMLText( outfile, tempBuffer );
         sprintf( buffer, "\"seq%d\"", seqID );
         seqID++;
    	 IndentNewXMLBlock( outfile );
         current_data = path_traversal.CurrentItem();	 
         while( (current_data != NULL) ) {      
            current_data = current_data->SaveScenarioDXL( outfile, buffer );       
         }
         IndentEndXMLBlock(outfile);
      }
      IndentEndXMLBlock(outfile);
   } 
   else {
      sprintf(tempBuffer,"seq( \"seq%d\", %s )\n", seqID, buffer);
      PrintXMLText(outfile,tempBuffer);
      sprintf( buffer, "\"seq%d\"",seqID );
      seqID++;
      IndentNewXMLBlock( outfile );
      current_data =  path_traversal.GetFirst();
      while( (current_data != NULL) ) {      
             current_data = current_data->SaveScenarioDXL( outfile, buffer );       
      }
      IndentEndXMLBlock(outfile);
   }
   IndentEndXMLBlock(outfile);
}

void SCENARIOGenerator::AbortGeneration()
{
   char buffer[200];

   abort_generation = TRUE;
   sprintf( buffer, "Scenario generation of scenario \"%s\" aborted.", 
                                          current_scenario->Name() );
   ErrorMessage( buffer );
   ResetSynchElementStatus();	 
}

void SCENARIOGenerator::AddPathTraversal(Path_data * new_data) //note
{
   if( current_path_data == NULL ) {
      path_traversal.Add(new_data);      
   }
   else {
      current_path_data->AddNext(new_data);
   }
   current_path_data = new_data;
}

void SCENARIOGenerator::ReplacePathTraversal(Path_data * new_root, Path_data * des)
{
   Path_data * current_data = NULL;
   for( path_traversal.First(); !path_traversal.IsDone(); path_traversal.Next() ) {
      current_data = path_traversal.CurrentItem();
      if( current_data->GetDestination() == des ) {
         path_traversal.RemoveCurrentItem();
         new_root->AddNext( current_data );
      }
   }
   path_traversal.Add(new_root);
   des->SetRoot(new_root);
}

Path_data * SCENARIOGenerator::EmptyPathList( )
{
   Path_data * current_data = NULL; 
   
   while( !path_list.is_empty() ) {
      current_data = path_list.RemoveFirstElement();
      delete  current_data;
   }     
   return 0;
}

Path_data * SCENARIOGenerator::EmptyPathTraversal(Path_data * p_data )
{
   Path_data * current_data = NULL;
//   Cltn<Path_data *> * previous_path;
   if( p_data == NULL ) {
      return NULL;
   }
   if( p_data->NextPath() != NULL ) {
      if( p_data->NextPath()->Size() > 1 ) {	  
         current_data = p_data->NextPath()->GetFirst()->GetDestination();
         DeleteParallel( p_data);
      }
      else {
         if( p_data->NextPath()->Size() == 1 ) {
            current_data = p_data->NextPath()->GetFirst();
         }
         else {
            current_data = NULL;
         }
      }
   } 
   else {
      current_data = NULL;
   }
   delete  p_data;
   p_data = NULL;
   
   return current_data;  
}

void SCENARIOGenerator::DeleteParallel( Path_data * root)
{
   Path_data * current_data = NULL;
   Path_data * des = root->NextPath()->GetFirst()->GetDestination();
   for( root->NextPath()->First(); !root->NextPath()->IsDone(); root->NextPath()->Next() ) {
       current_data = root->NextPath()->CurrentItem();      
       while( current_data != des && current_data != NULL ) {
          current_data = EmptyPathTraversal( current_data );
       }
   }
}

void SCENARIOGenerator::UpdateParallelStack( Path_data * old_data, Path_data * new_data )
{
   for( int i = 0; i < nesting_levels; i++ ) {
     if( fork_history[i].parent == old_data ) {
        fork_history[i].parent = new_data;
	break;
     }
   }	
}

bool SCENARIOGenerator::SubmapRecursionExists( Map *plugin )
{
   if( scenario_start_point->ParentMap() == plugin )
      return TRUE;
   else
      return FALSE;
}

void SCENARIOGenerator::RemoveNewLine(char * date, char * new_date) 
{
   char ch = date[0];
   int i = 0;
   while( ch != '\n' ) {
       new_date[i] = ch;
       i++;
       ch = date[i];
   }
   new_date[i] = 0;   
}

void SCENARIOGenerator::RegisterSynchElements( Synchronization * synch )
{
   if( !synch_elements.Includes( synch ) ) {
       synch_elements.Add( synch );
   }
}

void SCENARIOGenerator::ResetSynchElementStatus( ) 
{
  for( synch_elements.First(); !synch_elements.IsDone(); synch_elements.Next() ) {
     ( ( Synchronization *)synch_elements.CurrentItem() )->ResetTraverseState( );
     //( ( Synchronization *)synch_elements.CurrentItem() )->ResetAllVisited( );
     //( ( Synchronization *)synch_elements.CurrentItem() )->ResetScenarioGenerated( false );
  }
}

void SCENARIOGenerator::AddPathList( Path_data * current_path )
{
   path_list.Add( current_path );
}
