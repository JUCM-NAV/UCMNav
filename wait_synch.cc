/***********************************************************
 *
 * File:			wait_synch.cc
 * Author:			Andrew Miga
 * Created:			September 2001
 *
 * Modification history:        Feb 2003
 *                              Shuhua Cui
 *
 ***********************************************************/

#include "wait_synch.h"
#include "msc_generator.h"
#include "empty.h"
#include "print_mgr.h"
#include "variable.h"
#include "map.h"
#include "stub.h"
#include "scenario_generator.h"
#include "action.h"
#include "component_ref.h"

WaitContext::WaitContext( bool main_path, int * stub_history, int depth )
{
   MSCGenerator *msc = MSCGenerator::Instance();

   //msc->MapContext( &stub_hierarchy, decomposition_level );
   generation_id = msc->CurrentGenerationId();
   stub_hierarchy = stub_history;
   decomposition_level = depth;
   fork_synchronization = FALSE;
   isSynchronized = FALSE;
   if( main_path ) {
      main_arrived = TRUE;
      trigger_arrived = FALSE;
   } else {
      main_arrived = FALSE;
      trigger_arrived = TRUE;
   }
}

WaitContext::~WaitContext()
{
   if( stub_hierarchy != NULL ) delete [] stub_hierarchy;
}

bool WaitContext::SameContext( WaitContext *wc )
{
   if( wc->decomposition_level != decomposition_level )
      return FALSE;
   else {
      for( int i = 0; i < decomposition_level; i++ ) {
	 if( wc->stub_hierarchy[i] != stub_hierarchy[i] )
	    return FALSE;
      }
   }
   return TRUE;
}

bool WaitSynch::InputsSynchronized()
{
   bool main_path, return_code;
   WaitContext *new_context, *current_context;
   MSCGenerator *msc = MSCGenerator::Instance();
   int *stub_hierarchy = NULL;
   int decomposition_level = 0;
   
   main_path = (msc->PreviousEmpty()->FirstOutput()->EdgeType() != RESULT) ? TRUE : FALSE;
   //StubContext(&stub_hierarchy, decomposition_level);
   new_context = new WaitContext( main_path, stub_hierarchy, decomposition_level );

   for( wait_contexts.First(); !wait_contexts.IsDone(); wait_contexts.Next() ) {
      current_context = wait_contexts.CurrentItem();
      if( current_context->SameContext( new_context ) ) { // context already exits
	 if( ((main_path == TRUE) && (current_context->trigger_arrived == TRUE)) || // current path is main, previous was trigger
	     ((main_path == FALSE) && (current_context->main_arrived == TRUE)) ) { // current path is trigger, previous was main
	    current_context->main_arrived = FALSE; // reset flags for when same context is encountered again, in another generation pass or possibly in a loop
	    current_context->trigger_arrived = FALSE;
	    return_code = TRUE; // both main and trigger paths have arrived at this wait point
	 }
	 else {
	    if( main_path )
	       current_context->main_arrived = TRUE;
	    else
	       current_context->trigger_arrived = TRUE;
	    return_code = FALSE;
	 }
	 wait_contexts.Remove(current_context);
	 delete new_context;
	 return( return_code );
      }      
   }
   if( wait_contexts.Size() == 0 )
      msc->RegisterPathElement( this );
   wait_contexts.Add( new_context );
   return FALSE;
}

void WaitSynch::TriggerWaitSynch()
{
   WaitContext *new_context, *current_context;

   new_context = new WaitContext( FALSE );

   for( wait_contexts.First(); !wait_contexts.IsDone(); wait_contexts.Next() ) {
      current_context = wait_contexts.CurrentItem();
      if( current_context->SameContext( new_context ) ) { // context already exits
	 current_context->trigger_arrived = TRUE;
	 delete new_context;
	 return;
      }
   }

   if( wait_contexts.Size() == 0 )
      MSCGenerator::Instance()->RegisterPathElement( this );
   wait_contexts.Add( new_context );
}

void WaitSynch::StoreSynchType( bool fork_synch )
{
   WaitContext * temp_context;
   if((temp_context = ElementContext()) != NULL) 
        temp_context->fork_synchronization = fork_synch;
}

bool WaitSynch::ForkSynchronization()
{
   WaitContext *temp_context = ElementContext();   
   if( temp_context != NULL ) {
      return temp_context->fork_synchronization;
   }
   return false;
}

WaitContext * WaitSynch::ElementContext()
{
   WaitContext *temp_context = new WaitContext( FALSE );

   for( wait_contexts.First(); !wait_contexts.IsDone(); wait_contexts.Next() ) {
      if( wait_contexts.CurrentItem()->SameContext( temp_context ) ) {
	 delete temp_context;
	 return( wait_contexts.CurrentItem() );
      }
   }
   return NULL;
}

void WaitSynch::DeleteGenerationData()
{
   while( !wait_contexts.is_empty() ) {
      delete wait_contexts.Detach();
   }
   EmptyPathData();  
}

void WaitSynch::GeneratePostScriptDescription( FILE *ps_file )
{
   bool conditions_empty = condition_manager.ConditionsEmpty(),
      has_description = HasDescription(),
      has_type = PrintManager::TextNonempty( wait_type );
   char *guard_condition;

   if( conditions_empty &&  !has_description && !has_type && (logical_condition == NULL) ) return;

   fprintf( ps_file, "%d rom (%s Point - ) S\n", PrintManager::text_font_size, EdgeName() );
   fprintf( ps_file, "%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( identifier ));
   if( has_type ) {
      fprintf( ps_file, "%d rom\n1 IN ID (%s Type - ) S \n%d bol (%s ) P OD\n",
	       PrintManager::text_font_size, EdgeName(), PrintManager::text_font_size, PrintManager::PrintPostScriptText( wait_type ));
   }
   if( has_description ) GeneratePSElementDescription( ps_file );

   if( PrintManager::include_scenarios ) {
      if(logical_condition != NULL) {
         guard_condition = BooleanVariable::ReferenceToName( logical_condition ); 
         //if( (guard_condition = BooleanVariable::ReferenceToName( logical_condition )) != NULL ) {
	 fprintf( ps_file, "%d rom 1 IN ID (Guard Condition ) P OD\n2 IN ID\n", PrintManager::text_font_size );
	 fprintf( ps_file, "%d bol (%s ) P OD\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( guard_condition ) );
	 free( guard_condition );
      }
     
   }

   if( !conditions_empty ) condition_manager.GeneratePostScriptDescription( ps_file );
}

void WaitSynch::StubContext(int ** stub_hierarchy, int &decomposition_level) 
{
   Map * parent_map;
   Stub * parent_stub; 
   decomposition_level = 0;
   int * stub_history = NULL;
   int i = 0;
   
   parent_map = this->ParentMap();
   parent_stub = parent_map->getScanningParentStub();
   while ( parent_stub != NULL ) {
       parent_map = parent_stub->ParentMap();
       decomposition_level++;
       parent_stub = parent_map->getScanningParentStub();

   }

   if( decomposition_level > 0 )
      stub_history = new int[decomposition_level];

   parent_map = this->ParentMap();
   parent_stub = parent_map->getScanningParentStub();
   while ( parent_stub != NULL ) {
       stub_history[i] = parent_stub->GetNumber();
       parent_map = parent_stub->ParentMap();
       parent_stub = parent_map->getScanningParentStub();
       i++;
   }
   
   *stub_hierarchy = ( decomposition_level > 0 ) ? stub_history : NULL;
}

bool WaitSynch::InputsSynchronization( Path_data * & p_data )
{
   bool main_path, return_code;
   WaitContext *new_context, *current_context;
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   int *stub_hierarchy = NULL;
   int decomposition_level = 0;
   ComponentReference * component = NULL;

   main_path = (sce->PreviousEmpty()->FirstOutput()->EdgeType() != RESULT) ? TRUE : FALSE;
   //StubContext(&stub_hierarchy, decomposition_level);
   new_context = new WaitContext( main_path, stub_hierarchy, decomposition_level );
   
   if( main_path ) {
      p_data = CreateAction();
      if( (component = figure->GetEnclosingComponent() ) != NULL ) {
         p_data->SetComponentName( component->ReferencedComponent()->GetLabel() );
	 p_data->SetComponentId( component->ReferencedComponent()->GetComponentNumber() );
	 p_data->SetComponentRole( component->ComponentRole() );
	 p_data->SetComponentReferenceId( component->ComponentReferenceNumber() );			 
      }
      sce->AddPathTraversal( p_data );
   }
   else {
      p_data = sce->getCurrentPathData();
   }
   for( wait_contexts.First(); !wait_contexts.IsDone(); wait_contexts.Next() ) {
      current_context = wait_contexts.CurrentItem();
      if( current_context->SameContext( new_context ) ) { // context already exits
	 if( ((main_path == TRUE) && (current_context->trigger_arrived == TRUE)) || // current path is main, previous was trigger
	     ((main_path == FALSE) && (current_context->main_arrived == TRUE)) ) { // current path is trigger, previous was main
	    current_context->main_arrived = FALSE; // reset flags for when same context is encountered again, in another generation pass or possibly in a loop
	    current_context->trigger_arrived = FALSE;
	    return_code = TRUE; // both main and trigger paths have arrived at this wait point
	 }
	 else {
	    if( main_path )
	       current_context->main_arrived = TRUE;
	    else
	       current_context->trigger_arrived = TRUE;
	    return_code = FALSE;
	 }
	 wait_contexts.Remove(current_context);
	 delete new_context;
	 return( return_code );
      }      
   }
   if( wait_contexts.Size() == 0 )
      sce->RegisterPathElement( this );
   wait_contexts.Add( new_context );
   return FALSE;
}

// This method collapse the map to make it in well_nested form. First it set the destination of each path 
// element to be parameter wait_leave along each branch of parameter wait_element. Then, for each branch, it 
// gets the current path data and checks its list of next path, if any of the element in next path with a 
// destination not equal to wait_leave, that element is removed from next path list and append at wait_leave
// Finally it add one additional data element between root and the parallel block to prevent future intervention
// from collapsing the root element.     
void  WaitSynch::Collapse( Cltn<Path_data *>& wait_element, Path_data *& wait_leave )
{
   SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Path_data * current_element = NULL;
   Cltn<Path_data *> * next_path;
   Path_data * next_data = NULL;
   bool isRoot = false;
   Path_data * new_root = NULL;		   
   Cltn<Path_data *> deleteList;
   
   SetDestination( wait_element, wait_leave );
   if( wait_leave->GetRoot() == NULL ) {  
      new_root = new Action( 0, "Addition", Fork ); 
      sce->ReplacePathTraversal(new_root,  wait_leave );
   }
   for( wait_element.First(); !wait_element.IsDone(); wait_element.Next() ) {
      current_element = wait_element.CurrentItem();
      while( current_element != wait_leave->GetRoot() ) {
         sce->UpdateParallelStack( current_element, wait_leave );
         if( !isRoot ) {
            next_path = current_element->NextPath();
	    if( next_path->Includes( wait_leave ) ) {
	       for( next_path->First(); !next_path->IsDone(); next_path->Next() ) {
                  next_data = next_path->CurrentItem();
                  if( next_data != wait_leave ) {
	              deleteList.Add(next_data);
	          }
               }
	    }
	    else {
               for( next_path->First(); !next_path->IsDone(); next_path->Next() ) {
                  next_data = next_path->CurrentItem();
                  if( next_data->GetDestination() != wait_leave ) {
	              deleteList.Add(next_data);
	          }
               }
	    }
	    
	    for(deleteList.First(); !deleteList.IsDone(); deleteList.Next() ) {
	       Path_data * delete_element = deleteList.CurrentItem();
	       if( next_path->Includes( delete_element ) ) {
	           current_element->RemoveFromNext( delete_element );
	           wait_leave->AddNext( delete_element );
	       }
	    }
	 }	
         if( current_element->PreviousPath()->Size() == 1 ) {      
            current_element = current_element->Previous();
	    isRoot = false;
         }
         else {
            current_element = current_element->GetRoot(); 
	    isRoot = true; 
         } 
      }
   }
   AddForkAfter( wait_leave->GetRoot(), wait_leave );
}

// This method makes the current_data go backward one step, if current_data's previous pointer point to multiple
// path_data elements, then according to the nature that collapse is from inside out, the previous pointer is 
// pointing to a parallel block, so it skips over to the root of the parallel block; if the pointer has only one 
// element, it simply returns that element,; otherwise it return NULL 
Path_data *  WaitSynch::Advance(Path_data * current_data )
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

// This method searches through the two branches of parameter wait_element, and find the first common item 
// of the two branches. This common item must be the root element of parameter wait_leave 
void  WaitSynch::SearchRoot(Cltn<Path_data *>& wait_element, Path_data *& wait_leave )
{
   // Commented out by DA, August 2004
   // SCENARIOGenerator *sce = SCENARIOGenerator::Instance();
   Cltn<Path_data *> first_path;
   Path_data * reference_data = NULL;
   Path_data *  current_element;
   Path_data * new_root = NULL;
   
   reference_data = wait_element.GetFirst();  
   while( reference_data != NULL ) { // scan through the first branch
       first_path.Add( reference_data );
       reference_data = Advance( reference_data );
   }
   
   wait_element.First(); 
   wait_element.Next();  
   current_element = wait_element.CurrentItem();  
   while( current_element != NULL ) { // scan through the second branch
      if( !first_path.Includes( current_element ) ) { 
         current_element = Advance( current_element );     
      }
      else { // find the common item
         new_root = current_element;
         break;
      }
   }
   wait_leave->SetRoot( new_root );
}

// This method scans down each branch of the wait_element, sets the destination attribute of each 
// path_data element to be the parameter wait_leave, so all elements with the same destination are in the same
// parallel block
void  WaitSynch::SetDestination( Cltn<Path_data *>& wait_element, Path_data *& wait_leave )
{
   Path_data * current_element = NULL;
      
   for( wait_element.First(); !wait_element.IsDone(); wait_element.Next() ) {
      current_element = wait_element.CurrentItem();
      while( current_element !=  wait_leave->GetRoot() ) {
         current_element->RegisterDestination( wait_leave );
	 current_element = Advance( current_element );
	 }
   }
}

// Add an additional root element before the parallel block, so that during future operation of the root element,
// the parallel block will be treated as an entity
void WaitSynch::AddForkAfter( Path_data * root, Path_data * des )
{
   Cltn<Path_data *> deleteList;
   Path_data * current_element = NULL;
   Cltn<Path_data *> * rootNextList = root->NextPath();
   Path_data * new_root = new Action(0, "Addition", Fork);
   
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
   
   
