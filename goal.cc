/***********************************************************
 *
 * File:			goal.cc
 * Author:			Andrew Miga
 * Created:			October 1998
 *
 * Modification history:
 *
 ***********************************************************/

#include "goal.h"
#include "goal_tag.h"
#include "xml_generator.h"
#include "condition.h"
#include "print_mgr.h"
#include <string.h>
#include <stdlib.h>

extern void EditNewGoal( GoalTag *gs, GoalTag *ge );
extern void EditExistingGoal( Goal *goal );

GoalList *GoalList::SoleGoalList = NULL;

Goal::Goal( GoalTag *g1, GoalTag *g2, const char *new_name, const char *new_description )
{
   gt1 = g1;
   gt2 = g2;
   name[0] = 0;
   name[19] = 0;
   strncpy( name, new_name, 19 );
   description = NULL;
   if( new_description ) description = ( (strequal( new_description, "" )) ? NULL : strdup( new_description ) );
   generated = FALSE;
}

void Goal::Description( const char *new_description )
{
   if( new_description ) {
      if( description != NULL )
	 free( description );
      description = ( (strequal( new_description, "" )) ? NULL : strdup( new_description ) );
   }
}

void Goal::ReplaceCondition( int type, char *old_condition, char *new_condition )
{
   conditions[type].Replace( old_condition, strdup( new_condition ) );
   free( old_condition );
}

void Goal::SaveXML( FILE *file, bool val )
{
   char buffer[300];

   if( val ) {
      if( generated )
	 return;
      generated = TRUE;
   }
   
   sprintf( buffer, "<goal start-point=\"h%d\" end-point=\"h%d\" goal-name=\"%s\" ", gt1->GetNumber(), gt2->GetNumber(), PrintDescription( name ) );
   PrintXMLText( file, buffer );
   if( description != NULL )
      fprintf( file, "description=\"%s\" >\n", PrintDescription( description ) );
   else
      fprintf( file, ">\n" );
   IndentNewXMLBlock( file );

   for( int i = 0; i < 2; i++ ) {
      if( conditions[i].Size() != 0 ) {
	 sprintf( buffer, "%scondition-list composition=\"%s\"", ((i == 0) ? "pre" : "post" ), "AND" );
	 PrintNewXMLBlock( file, buffer );
	 ConditionManager::SaveConditionList( file, &conditions[i], "condition" );
	 PrintEndXMLBlock( file, (i == 0) ? "precondition-list" : "postcondition-list" );
      }
   }
   
   PrintEndXMLBlock( file, "goal" );
}

void Goal::GeneratePostScriptDescription( FILE *ps_file )
{
   if( generated )
      return;
   generated = TRUE;

   fprintf( ps_file, "%d rom (Goal - ) S\n%d bol (%s ) P\n", PrintManager::text_font_size, 
	    PrintManager::text_font_size, PrintManager::PrintPostScriptText( name ) );
   fprintf( ps_file, "1 IN ID %d rom (Goal Tags ) P OD\n", PrintManager::text_font_size );
   fprintf( ps_file, "2 IN ID %d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( gt1->Name() ) );
   fprintf( ps_file, "(%s ) P OD\n",  PrintManager::PrintPostScriptText( gt2->Name() ) );
   if( description != NULL ) {
      if( !strequal( description, "" ) ) {
	 fprintf( ps_file, "1 IN ID %d rom (Description ) P OD\n", PrintManager::text_font_size );
	 fprintf( ps_file, "2 IN ID (%s ) P OD\n", PrintManager::PrintPostScriptText( description ) );
      }
   }
   fprintf( ps_file, "1 IN ID %d rom (Preconditions ) P OD\n", PrintManager::text_font_size );
   ConditionManager::GeneratePostscriptConditionList( ps_file, &conditions[0] );
   fprintf( ps_file, "1 IN ID %d rom (Postconditions ) P OD\n", PrintManager::text_font_size );
   ConditionManager::GeneratePostscriptConditionList( ps_file, &conditions[1] );
   fprintf( ps_file, "L\n" );
}

GoalList * GoalList::Instance()
{
   if( SoleGoalList == NULL )
      SoleGoalList = new GoalList();

   return( SoleGoalList );
}

Goal * GoalList::GoalExists( GoalTag *gt )
{
   Goal *goal;

   for( goal_list.First(); !goal_list.IsDone(); goal_list.Next() ) {
      goal = goal_list.CurrentItem();
      if( (gt == goal->gt1) || (gt == goal->gt2) )
	 return( goal );
   }

   return( NULL ); // if no goal exists
}

void GoalList::CreateGoal( GoalTag *g1, GoalTag *g2 )
{
   EditNewGoal( g1, g2 );
}

void GoalList::EditGoal( Goal *goal )
{
   EditExistingGoal( goal );
}

void GoalList::DeleteGoal( GoalTag *gt )
{
   Goal *current_goal;

   for( goal_list.First(); !goal_list.IsDone(); goal_list.Next() ) {
      current_goal = goal_list.CurrentItem();
      if( (gt == current_goal->gt1) || (gt == current_goal->gt2) ) {
	 goal_list.RemoveCurrentItem();
	 delete current_goal;
	 break;
      }
   }
}

void GoalList::SaveXML( FILE *file )
{
   if( goal_list.Size() == 0 ) return;
   
   PrintNewXMLBlock( file, "agent-annotations" );
   PrintNewXMLBlock( file, "goal-list" );

   for( goal_list.First(); !goal_list.IsDone(); goal_list.Next() )
      goal_list.CurrentItem()->SaveXML( file, FALSE );

   PrintEndXMLBlock( file, "goal-list" );
   PrintEndXMLBlock( file, "agent-annotations" );
}

void GoalList::ResetGenerationState()
{
   for( goal_list.First(); !goal_list.IsDone(); goal_list.Next() )
      goal_list.CurrentItem()->ResetGenerated();
}
