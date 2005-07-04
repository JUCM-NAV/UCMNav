/***********************************************************
 *
 * File:			transformation.cc
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:	completed July 1996
 *
 ***********************************************************/

#include <stdlib.h>
#include "transformation.h"
#include "hyperedge.h"
#include "hypergraph.h"
#include "empty.h"
#include "start.h"
#include "result.h"
#include <iostream>
#include <string.h>
#include <stdio.h>

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

static TransformationManager *SoleInstance;

extern FD_Main *pfdmMain;
extern int show_unavailable_transformations;

char transformations[][33]= {

   " Edit Path Label ",
   " Decompose From Join ",
   " Add Responsibility ",
   " Add Wait ",
   " Add Stub ",
   " Add Timer ",
   " Add Or Fork ",
   " Add And Fork ",
   " Cut Path ",
   " Delete Responsibility ",
   " Delete Stub ",
   " Delete Timer ",
   " Create New Sub Map ",
   " Delete Wait ",
   " Delete Point ",
   " Add Point ",
   " Delete Path ",
   " Edit Responsibility ",
   " Add Branch ",
   " Delete Branch ",
   " Enable Path Label ",
   " Disable Path Label ",
   " View Sub Map ",
   " Edit Stub Attributes ",
   " Decompose From Stub ",
   " Add Failure Point ",
   " Remove Failure Point ",
   " Add Shared Responsibility ",
   " Remove Shared Responsibility ",
   " Rotate ",
   " Install Existing Plugin ",
   " Remove Plugin ",
   " Bind Plugin to Stub ",
   " Add Timestamp Point ",
   " Delete Timestamp Point ",
   " Edit Timestamp Point ",
   " Edit Arrival Characteristics ",
   " Edit Start Name ",
   " Edit Timer Characteristics ",
   " Edit Wait Characteristics ",
   " Edit Path End Name ",
   " View Response Times ",
   " Image of Existing Plugin ",
   " Rename Plugin ",
   " Add Goal Tag ",
   " Delete Goal Tag ",
   " Rename Goal Tag ",
   " Import Plugin From File ",
   " Add Direction Arrow ",
   " Remove Direction Arrow ",
   " Center Label ",
   " Flush Label ",
   " Add Timeout Path ",
   " Delete Timeout Path ",
   " Edit Fork Specification ",
   " Transfer Root Map ",
   " Generate All MSC Paths ",
   " Add Loop ",
   " Edit Loop Characteristics ",
   " Delete Loop ",
   " Follow Path In Submap ",
   " Follow Path In Parent Map ",
   " List Bound Parent Maps ",
   " Set as Scenario Start ",
   " List Scenarios ",
   " Edit Logical Guard Condition " };

char transformations2[][35]= {

   " And Join ",
   " Or Join ",
   " Join With Stub ",
   " Merge ",
   " Connect ",
   " Disconnect ",
   " Synchronize Path ",
   " Create Response Time Requirement ",
   " Edit Response Time Requirement ",
   " Delete Response Time Requirement ",
   " Create Goal ",
   " Edit Goal " };

TransformationManager::TransformationManager()
{
   CreateDictionaries();
}

TransformationManager::~TransformationManager() {}

TransformationManager * TransformationManager::Instance()
{
   if( SoleInstance == NULL )
      SoleInstance = new TransformationManager();
   return( SoleInstance );
}

Cltn<Hyperedge *> *TransformationManager::CreateNewPath()
{

   // create a simple UCM path consisting of a start, empty, and a result
   // return a collection of the three edges
  
   Node *node1 = new Node( B );
   Node *node2 = new Node( A );
   Start *start = new Start;
   Empty *empty = new Empty;
   Result *result = new Result;
  
   Cltn<Hyperedge *> *edges = new Cltn<Hyperedge *>;
   edges->Add( start );
   edges->Add( empty );
   edges->Add( result );
  
   start->AttachTarget( node1 );
   empty->AttachSource( node1 );
   empty->AttachTarget( node2 );
   result->AttachSource( node2 );
  
   return( edges );

}

void TransformationManager::TransApplicableTo( Hyperedge *edge )
{

   // give a list of applicable transformations to the given edge and perform if
   // user so chooses.  Requires creating a popup dialog box
 
   int position = edge->EdgeType();
   Cltn<transformation> *trans_list = &transformation_dictionary[ position ];
   Cltn<Trans_Rec> applicable_trans;
   int popup, popup_choice;
  
   if( show_unavailable_transformations ) {
      for( trans_list->First(); !trans_list->IsDone(); trans_list->Next() )
	 applicable_trans.Add( Trans_Rec( trans_list->CurrentItem(), edge->Validate( trans_list->CurrentItem() ) ?  TRUE : FALSE ) );
   }
   else {
      for( trans_list->First(); !trans_list->IsDone(); trans_list->Next() ) {
	 if( edge->Validate( trans_list->CurrentItem() ) == TRUE )
	    applicable_trans.Add( Trans_Rec( trans_list->CurrentItem(), TRUE ) );
      }
   }

   if( applicable_trans.length() )
   {
      char menu[1000];
      strcpy( menu, " Applicable Transformations %t " );

      for( applicable_trans.First(); !applicable_trans.IsDone(); applicable_trans.Next() ){
	 strcat( menu, " | " );
	 strcat( menu, transformations[ applicable_trans.CurrentItem().transformation ]);
	 if ( applicable_trans.CurrentItem().available == FALSE )
	    strcat( menu, " %d " );
      }
  
      popup = fl_defpup( pfdmMain->Main->window, menu );
      popup_choice = fl_dopup( popup );
  
      if( popup_choice > 0 )
      {
	 const Trans_Rec &temp_trec = applicable_trans.Get( popup_choice );
	 edge->Transform( temp_trec.transformation );
      }

      fl_freepup( popup );
   }
   else
   {
      fl_show_message( " No Applicable Transformations ",
		       "There is no single selection transformation for the selected figure.", "");
   }

}

void TransformationManager::TransApplicableToPair( Hyperedge *edge1, Hyperedge *edge2 )
{

   // give a list of applicable transformations to the given edges and perform if
   // user so chooses.  Requires creating a popup dialog box
 
   int position = edge1->EdgeType();
   Cltn<transformation> *trans_list = &transformation_dictionary2[ position ];
   Cltn<Trans_Rec> applicable_trans;
   int popup, popup_choice, rc;
   Hyperedge *first_edge, *second_edge;
  
   for( trans_list->First(); !trans_list->IsDone(); trans_list->Next() ) {
      rc = edge1->ValidateDouble( trans_list->CurrentItem(), edge2 );
      if( show_unavailable_transformations ) {
	 if( rc == AVAILABLE || rc == UNAVAILABLE )
	    applicable_trans.Add( Trans_Rec( trans_list->CurrentItem(), (rc == AVAILABLE) ? TRUE : FALSE ) );
      }
      else {
	 if( rc == AVAILABLE )
	    applicable_trans.Add( Trans_Rec( trans_list->CurrentItem(), TRUE ) );
      }
   }

   if( applicable_trans.length() ){	// if transformations are valid for current edge order
  
      first_edge = edge1;
      second_edge = edge2;
   }
   else {

      // try other order of hyperedges
  
      position = edge2->EdgeType();
      trans_list = &transformation_dictionary2[ position ];

      for( trans_list->First(); !trans_list->IsDone(); trans_list->Next() ) {
	 rc = edge2->ValidateDouble( trans_list->CurrentItem(), edge1 );
	 if( show_unavailable_transformations ) {
	    if( rc == AVAILABLE || rc == UNAVAILABLE )
	       applicable_trans.Add( Trans_Rec( trans_list->CurrentItem(), (rc == AVAILABLE) ? TRUE : FALSE ) );
	 }
	 else {
	    if( rc == AVAILABLE )
	       applicable_trans.Add( Trans_Rec( trans_list->CurrentItem(), TRUE ) );
	 }
      }

	 display_manager->SetPath( edge1->GetFigure()->GetPath() ); // sets same current path as
      // normal selection order
      first_edge = edge2 ;
      second_edge = edge1;

   }

   if( applicable_trans.length() ){	// if any transformations are valid

      char menu[500];
      strcpy( menu, " Applicable Transformations %t " );

      for( applicable_trans.First(); !applicable_trans.IsDone(); applicable_trans.Next() ){
	 strcat( menu, " | " );
	 strcat( menu, transformations2[ applicable_trans.CurrentItem().transformation ]);
	 if ( applicable_trans.CurrentItem().available == FALSE )
	    strcat( menu, " %d " );
      }
  
      popup = fl_defpup( pfdmMain->Main->window, menu );
      popup_choice = fl_dopup( popup );
  
      if( popup_choice > 0 ) {
	 const Trans_Rec &temp_trec = applicable_trans.Get( popup_choice );	 
	 first_edge->TransformDouble( temp_trec.transformation, second_edge );  
      }
      
      fl_freepup( popup );
   }

   else {
  
      fl_show_message( " No Applicable Transformations ",
		       "There is no double selection transformations for the selected figures.", "");
   }
    
}

void TransformationManager::CreateDictionaries()
{
  
   //  SINGLE SELECTION TRANSFORMATIONS
  
   // create list of Empty transformations
  
   transformation_dictionary[EMPTY].Add( FOLLOW_PATH_SUBMAP );
   transformation_dictionary[EMPTY].Add( ADD_RESPONSIBILITY );
   transformation_dictionary[EMPTY].Add( ADD_OR_FORK );
   transformation_dictionary[EMPTY].Add( ADD_AND_FORK );
   transformation_dictionary[EMPTY].Add( ADD_WAIT );
   transformation_dictionary[EMPTY].Add( ADD_STUB);
   transformation_dictionary[EMPTY].Add( ADD_TIMER );
   transformation_dictionary[EMPTY].Add( ADD_LOOP );
   transformation_dictionary[EMPTY].Add( ADD_TIMESTAMP );
   transformation_dictionary[EMPTY].Add( ADD_GOAL_TAG );
   transformation_dictionary[EMPTY].Add( ADD_POINT );
   transformation_dictionary[EMPTY].Add( DELETE_POINT );
   transformation_dictionary[EMPTY].Add( CUT );
   transformation_dictionary[EMPTY].Add( ENABLE_PATH_LABEL );
   transformation_dictionary[EMPTY].Add( DISABLE_PATH_LABEL );	
   transformation_dictionary[EMPTY].Add( EDIT_LABEL );
   transformation_dictionary[EMPTY].Add( DECOMPOSE_FROM_JOIN );
   transformation_dictionary[EMPTY].Add( DECOMPOSE_FROM_STUB );
   transformation_dictionary[EMPTY].Add( DELETE_BRANCH );
   transformation_dictionary[EMPTY].Add( ADD_DIRECTION_ARROW );
   transformation_dictionary[EMPTY].Add( REMOVE_DIRECTION_ARROW );
   transformation_dictionary[EMPTY].Add( ADD_FAILURE_POINT );
   transformation_dictionary[EMPTY].Add( REMOVE_FAILURE_POINT );
   transformation_dictionary[EMPTY].Add( ADD_SHARED_RESPONSIBILITY );
   transformation_dictionary[EMPTY].Add( REMOVE_SHARED_RESPONSIBILITY );

   // create list of Or Fork transformations
  
   transformation_dictionary[OR_FORK].Add( EDIT_FORK_SPECIFICATION );
   transformation_dictionary[OR_FORK].Add( ADD_BRANCH );
   transformation_dictionary[OR_FORK].Add( ROTATE );
  
   // create list of Or Join transformations
  
   transformation_dictionary[OR_JOIN].Add( ROTATE );
  
   // create list of ResponsibilityReference transformations
  
   transformation_dictionary[RESPONSIBILITY_REF].Add( EDIT_RESPONSIBILITY );
   transformation_dictionary[RESPONSIBILITY_REF].Add( DELETE_RESPONSIBILITY );
  
   // create list of Start transformations

   transformation_dictionary[START].Add( FOLLOW_PATH_PARENT_MAP );
   transformation_dictionary[START].Add( EDIT_START_NAME );
   transformation_dictionary[START].Add( EDIT_LOGICAL_CONDITION );
   transformation_dictionary[START].Add( LIST_BOUND_PARENT_MAPS );
   transformation_dictionary[START].Add( EDIT_PATH_INITIATION );
   transformation_dictionary[START].Add( DELETE_PATH );
   transformation_dictionary[START].Add( CENTER_LABEL );
   transformation_dictionary[START].Add( FLUSH_LABEL );
   transformation_dictionary[START].Add( GENERATE_MSC );
//   transformation_dictionary[START].Add( SET_SCENARIO_START );
   transformation_dictionary[START].Add( LIST_SCENARIOS );

   // create list of Stub transformations
  
   transformation_dictionary[STUB].Add( EDIT_STUB_LABEL );
   transformation_dictionary[STUB].Add( EXPAND_STUB );
   transformation_dictionary[STUB].Add( VIEW_SUB_MAP );
   transformation_dictionary[STUB].Add( INSTALL_EXISTING_PLUGIN );
   transformation_dictionary[STUB].Add( REMOVE_SUBMAP );
   transformation_dictionary[STUB].Add( RENAME_SUBMAP );
   transformation_dictionary[STUB].Add( BIND_STUB );
   transformation_dictionary[STUB].Add( COPY_EXISTING_PLUGIN );
   transformation_dictionary[STUB].Add( IMPORT_PLUGIN_FILE );
   transformation_dictionary[STUB].Add( TRANSFER_ROOT_PLUGIN );
   transformation_dictionary[STUB].Add( DELETE_STUB );
   
   // create list of Synchronization transformations
  
   transformation_dictionary[SYNCHRONIZATION].Add( ADD_BRANCH );
   transformation_dictionary[SYNCHRONIZATION].Add( ROTATE );
    
   // create list of Timer transformations
  
   transformation_dictionary[TIMER].Add( EDIT_TIMER_CHARACTERISTICS );
   transformation_dictionary[TIMER].Add( EDIT_LOGICAL_CONDITION );
   transformation_dictionary[TIMER].Add( DELETE_TIMER );
   transformation_dictionary[TIMER].Add( ADD_TIMEOUT_PATH );
   transformation_dictionary[TIMER].Add( DELETE_TIMEOUT_PATH );

   //  create list of Timestamp transformations

   transformation_dictionary[TIMESTAMP].Add( EDIT_TIMESTAMP );
   transformation_dictionary[TIMESTAMP].Add( VIEW_RESPONSE_TIMES );
   transformation_dictionary[TIMESTAMP].Add( DELETE_TIMESTAMP );

   // create list of Wait transformations
  
   transformation_dictionary[WAIT].Add( EDIT_WAIT_CHARACTERISTICS );
   transformation_dictionary[WAIT].Add( EDIT_LOGICAL_CONDITION );
   transformation_dictionary[WAIT].Add( DELETE_WAIT );

   // create list of Result transformations

   transformation_dictionary[RESULT].Add( FOLLOW_PATH_PARENT_MAP );
   transformation_dictionary[RESULT].Add( LIST_BOUND_PARENT_MAPS );
   transformation_dictionary[RESULT].Add( EDIT_END_NAME );
   transformation_dictionary[RESULT].Add( CENTER_LABEL );
   transformation_dictionary[RESULT].Add( FLUSH_LABEL );
 
   // create list of GoalTag transformations

   transformation_dictionary[GOAL_TAG].Add( RENAME_GOAL_TAG );
   transformation_dictionary[GOAL_TAG].Add( DELETE_GOAL_TAG );

   // create list of Loop transformations

   transformation_dictionary[LOOP].Add( EDIT_LOOP_CHARACTERISTICS );
   transformation_dictionary[LOOP].Add( ROTATE );
   transformation_dictionary[LOOP].Add( DELETE_LOOP );

   // NOTE:  OrJoin has no applicable single selection
   // transformations and thus is empty.
 
   //	DOUBLE SELECTION TRANSFORMATIONS

   //  create list of Result transformations
  
   transformation_dictionary2[RESULT].Add( AND_COMPOSE );
   transformation_dictionary2[RESULT].Add( OR_COMPOSE );
   transformation_dictionary2[RESULT].Add( STUB_COMPOSE );
  
   //  create list of Start transformations
  
   transformation_dictionary2[START].Add( MERGE );
   transformation_dictionary2[START].Add( CONNECT );
   transformation_dictionary2[START].Add( DISCONNECT );
 
   // create list of Synchronization transformations

   transformation_dictionary2[SYNCHRONIZATION].Add( ADD_INCOMING_SYNCHRONIZATION );

   //  create list of Timer transformations
  
   transformation_dictionary2[TIMER].Add( CONNECT );
   transformation_dictionary2[TIMER].Add( DISCONNECT );

   //  create list of Timestamp transformations

   transformation_dictionary2[TIMESTAMP].Add( CREATE_TIME_RESPONSE );
   transformation_dictionary2[TIMESTAMP].Add( EDIT_TIME_RESPONSE );
   transformation_dictionary2[TIMESTAMP].Add( DELETE_TIME_RESPONSE );
   
   //  create list of Wait transformations
  
   transformation_dictionary2[WAIT].Add( CONNECT );
   transformation_dictionary2[WAIT].Add( DISCONNECT );
  
   // create list of GoalTag transformations

   transformation_dictionary2[GOAL_TAG].Add( CREATE_GOAL );
   transformation_dictionary2[GOAL_TAG].Add( EDIT_GOAL );

   // NOTE: The rest of the hyperedges have no valid double selection commands and thus are empty.

}
