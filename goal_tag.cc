/***********************************************************
 *
 * File:			goal_tag.cc
 * Author:			Andrew Miga
 * Created:			October 1998
 *
 * Modification history:
 *
 ***********************************************************/

#include "goal_tag.h"
#include "goal_figure.h"
#include "goal.h"

GoalTag::GoalTag() : Hyperedge()
{
   name[0] = 0;
   name[19] = 0;

   figure = new GoalFigure( this, TRUE );
}

GoalTag::GoalTag( int gtid, const char *gt_name, float x, float y, const char *desc ) : Hyperedge()
{
   load_number = gtid;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = gtid;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;    
   //End of the modification by Bo Jiang, April, 2005	

   figure = new GoalFigure( this, FALSE );
   figure->SetPosition( x, y );
   name[19] = 0;
   strncpy( name, gt_name, 19 );
   if( desc != NULL ) unique_desc = strdup( desc );
}

GoalTag::~GoalTag()
{
   GoalList *goal_list = GoalList::Instance();
   goal_list->DeleteGoal( this );
}

bool GoalTag::Perform( transformation trans, execution_flag execute )
{
   if( trans == DELETE_GOAL_TAG )
      return( DeleteGoalTag( execute ) );
   else // trans == RENAME_GOAL_TAG
      return( EditGoalTagName( execute ) );
}

bool GoalTag::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute )
{
   if( trans == CREATE_GOAL )
      return( CreateGoal( edge, execute ) );
   else // trans == EDIT_GOAL
      return( EditGoal( edge, execute ) );
}

void GoalTag::Name( const char *new_name )
{
   if ( new_name )
      strncpy( name, new_name, 19 );
}

bool GoalTag::DeleteGoalTag( execution_flag execute )
{
   if( execute )
      this->DeleteSimpleEdge();
   
   return( TRUE );
}

bool GoalTag::EditGoalTagName( execution_flag execute )
{
   if( execute )
      this->Name( fl_show_input( "Enter the name for the goal tag", name ) );

   return( TRUE );
}

bool GoalTag::CreateGoal( Hyperedge *edge, execution_flag execute )
{
   GoalList *goal_list = GoalList::Instance();
   
   if( execute )
      goal_list->CreateGoal( this, (GoalTag *)edge );
   else {
       // operation is valid is edge is a goal tag and neither form part of an existing goal
      if( edge->EdgeType() != GOAL_TAG )
	 return( INVALID );

      if( goal_list->GoalExists( this ) != NULL )
	 return( UNAVAILABLE );
	 
      if( goal_list->GoalExists( (GoalTag *)edge ) != NULL  )
	 return( UNAVAILABLE );
	       
      return( AVAILABLE );
   }
   return AVAILABLE;
}

bool GoalTag::EditGoal( Hyperedge *edge, execution_flag execute )
{
   GoalList *goal_list = GoalList::Instance();
   static Goal *goal;
   
   if( execute )
      goal_list->EditGoal( goal );
   else {
       // operation is valid is edge is a goal tag and this tag forms part of an existing goal
      if( edge->EdgeType() != GOAL_TAG )
	 return( INVALID );

      if( (goal = goal_list->GoalExists( this )) != NULL )
	 return( AVAILABLE );
      else
	 return( UNAVAILABLE );
   }
   return AVAILABLE;
}

bool GoalTag::FormsGoal()
{
   return((GoalList::Instance()->GoalExists( this ) != NULL) ? TRUE : FALSE );
}

void GoalTag::SaveXMLDetails( FILE *fp )
{
   PrintXMLText( fp, "<goal-tag></goal-tag>\n" );
}

void GoalTag::GeneratePostScriptDescription( FILE *ps_file )
{
   GoalList *goal_list = GoalList::Instance();
   Goal *goal;

   if( (goal = goal_list->GoalExists( this )) != NULL )
      goal->GeneratePostScriptDescription( ps_file );
}

void GoalTag::SaveGoal( FILE *fp )
{
   GoalList *goal_list = GoalList::Instance();
   Goal *goal;

   if( (goal = goal_list->GoalExists( this )) != NULL )
      goal->SaveXML( fp );
}
void GoalTag::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
