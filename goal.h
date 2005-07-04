/***********************************************************
 *
 * File:			goal.h
 * Author:			Andrew Miga
 * Created:			October 1998
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef GOAL_H
#define GOAL_H

#include "collection.h"
#include "string.h"
#include <stdio.h>
#include <fstream>

class GoalTag;
class GoalList;

class Goal {
   
   friend class GoalList;
   
public:

   Goal( GoalTag *g1, GoalTag *g2, const char *new_name, const char *new_description );
   const char * Name() { return( name ); }
   void Name( const char *new_name ) { strncpy( name, new_name, 19 ); }
   const char * Description() { return( description ); }
   void Description( const char *new_description );
   GoalTag * StartGoal() { return( gt1 ); }
   GoalTag * EndGoal() { return( gt2 ); }
   Cltn<char *> * Conditions( int type ) { return( &conditions[type] ); }
   void AddCondition( int type, char *condition ) { conditions[type].Add( strdup( condition ) ); }
   void ReplaceCondition( int type, char *old_condition, char *new_condition );
   void AddPrecondition( char *new_condition ) { conditions[0].Add( new_condition ); }
   void AddPostcondition( char *new_condition ) { conditions[1].Add( new_condition ); }
   void SaveXML( FILE *file, bool val = TRUE );
   void GeneratePostScriptDescription( FILE *ps_file );
   void ResetGenerated() { generated = FALSE; }
   
private:

   GoalTag *gt1, *gt2; // the two goal tags involved in the relationship
   char *description, name[20]; // user given name for relationship
   Cltn<char *> conditions[2];
   bool generated;

};

class GoalList {

public:

   ~GoalList() {}
   static GoalList * Instance();

   void CreateGoal( GoalTag *g1, GoalTag *g2 );
   void EditGoal( Goal *goal );
   void DeleteGoal( GoalTag *gt );
   Goal * GoalExists( GoalTag *gt );
   void AddGoal( Goal *new_goal ) { goal_list.Add( new_goal ); }
   void SaveXML( FILE *file );
   void ResetGenerationState();
   
private:

   GoalList() {}

   static GoalList *SoleGoalList;
   Cltn<Goal *> goal_list;

};



#endif
