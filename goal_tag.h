/***********************************************************
 *
 * File:			goal_tag.h
 * Author:			Andrew Miga
 * Created:			October 1998
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef GOAL_TAG_H
#define GOAL_TAG_H

#include "hyperedge.h"

class GoalTag : public Hyperedge {

public:

   GoalTag();
   GoalTag( int gtid, const char *gt_name, float x, float y, const char *desc );
   ~GoalTag();

   virtual edge_type EdgeType() { return( GOAL_TAG ); }
   virtual const char * HyperedgeName() { return( name ); }
   virtual void SaveXMLDetails( FILE *fp );
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){}

   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual bool DeleteHyperedge() { return( this->DeleteElement() ); }

   virtual bool Perform( transformation trans, execution_flag execute );// validates or performs the single selection transformation with code trans
   virtual bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ); // validates/performs double selection transformation trans

   virtual bool HasName() { return( TRUE ); }
   char * Name() { return( name ); }  // access method for Name
   void Name( const char *new_name ); // set method for name
   bool FormsGoal();
   void SaveGoal( FILE *fp );

private:

   bool DeleteGoalTag( execution_flag execute ); // transformation which deletes the timestamp point
   bool EditGoalTagName( execution_flag execute ); // allows user to edit timestamp point characteristics
   bool CreateGoal( Hyperedge *edge, execution_flag execute ); // allows user to create a goal between this and other goal tag
   bool EditGoal( Hyperedge *edge, execution_flag execute ); // allows user to edit the goal between this and other goal tag

   char name[20];  // user given name for goal tag

};

#endif
