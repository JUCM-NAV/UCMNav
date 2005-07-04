/***********************************************************
 *
 * File:			empty.h
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef EMPTY_H
#define EMPTY_H

#include "hyperedge.h"
#include "or_join.h"
#include "synchronization.h"
#include "condition.h"
#include <fstream>

#define NONE 1
#define FAILURE_POINT 2
#define SHARED_RESPONSIBILITY 3
#define DIRECTION_ARROW 4

class PointFigure;
class Presentation;

class Empty : public Hyperedge {

   friend class PointFigure;
   friend class DisplayManager;

public:

   Empty();
   Empty( int empty_id, float x, float y, Label *plabel, const char *desc, int designation, bool show_label );
   ~Empty();

   virtual edge_type EdgeType() { return( EMPTY ); }  // returns identifier for subclass type
   virtual bool Perform( transformation trans, execution_flag execute ); // executes or validates the transformation with code trans
   virtual void EdgeSelected(); // called when element is selected, places pre/postconditions in main window display

   OrJoin *AddOrJoin();   // replaces itself with an or join at the same point of the path in the hypergraph structure
   Synchronization *AddAndJoin(); // replaces itself with an and join at the same point of the path in the hypergraph structure

   virtual Label *PathLabel();  // returns the hyperedge's path label object
   virtual void SetLabel( Label *new_label );  // allows label object to be changed
   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir );

   virtual void SaveXMLDetails( FILE *fp );  // implements Save method for this empty hyperedge, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp );  // implements Save method for this empty hyperedge, in XML format

   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual bool DeleteHyperedge();
   virtual int DoubleClickAction();

   void PathNumber( int new_number ) { path_number = new_number; }  // sets the indentifier of the semi path
   int PathNumber() { return( path_number ); }  // returns the identifier of the semi path
   void CopyConditions( ConditionManager &cm );  // allows pre/post conditions to be copied to another empty hyperedge when this instance is deleted
   void AddPrecondition( char *condition ) { condition_manager.AddCondition( PRECONDITION, condition ); }
   void AddPostcondition( char *condition ) { condition_manager.AddCondition( POSTCONDITION, condition ); }
   void ReorderPaths();
   void InstallSimpleElement( Hyperedge *new_ref ) { AddSimple( new_ref ); }

   void Highlight() { highlighted = TRUE; }
   void Unhighlight() { highlighted = FALSE; }
   bool Highlighted() { return( highlighted ); }
   bool AddDirectionArrow( execution_flag execute ); // user operations which add and remove arrows along the path specifying its direction
   virtual Hyperedge * TraverseScenario();
   Hyperedge * HighlightScenario( );

private:

   bool ConnectPath( Hyperedge *trigger_edge, execution_flag execute );  // transformation which connects this empty point as a trigger for a waiting place
   bool DisconnectPath( Hyperedge *trigger_edge, execution_flag execute );  // transformation which disconnects a triggering connection if one exists
   bool Cut( execution_flag execute ); // transformation which cuts the path into two sections at the location of this empty
   bool AndCompose( Hyperedge *edge, execution_flag execute );  // transformation which creates and and join with the path of edge at the current location
   bool OrCompose( Hyperedge *edge, execution_flag execute );  // transformation which creates and or join with the path of edge at the current location
   bool DecomposeFromJoin( execution_flag execute );  // transformation which detaches the path of this empty point from an and/or join
   bool DecomposeFromStub( execution_flag execute );  // transformation which detaches the path of this empty point from the input or output of a stub
   bool DeleteBranch( execution_flag execute );  // transformation which deletes the branch of an and/or fork on which this empty point resides

   bool AddResponsibility( execution_flag execute );  // transformation which replaces this object with a responsibility hyperedge at the current location
   bool AddAndFork( execution_flag execute ); // transformation which replaces this point on the path with an and fork structure
   bool AddGoalTag( execution_flag execute ); // transformation which replaces this point on the path with a goal tag
   bool AddOrFork( execution_flag execute ); // transformation which replaces this point on the path with an or fork structure
   bool AddStub( execution_flag execute ); // transformation which replaces this object with a stub hyperedge at the current location
   bool AddTimer( execution_flag execute ); // transformation which replaces this object with a timer hyperedge at the current location
   bool AddTimestamp( execution_flag execute ); // transformation which replaces this object with a timestamp hyperedge at the current location
   bool AddWait( execution_flag execute ); // transformation which replaces this object with a wait hyperedge at the current location
   bool AddLoop( execution_flag execute ); // transformation which replaces this object with a loop hyperedge at the current location

   bool DeletePoint( execution_flag execute ); // transformation which deletes this empty point on the path
   bool AddPoint( execution_flag execute );  // transformation which adds another empty point to the current path located to the right of this one
   bool DeletePath( execution_flag execute );  // transformation which deletes the path on which this empty point is located at user's confirmation
   bool EditPathLabel( execution_flag execute ); // allows user to edit the scenario label of this path segment
   bool EnablePathLabel( execution_flag execute );  // user operation which causes the scenario label to be visible if the viewing of labels is disabled
   bool DisablePathLabel( execution_flag execute );  // user operation which cancels the visibility of the scenario label
   bool AddFailurePoint( execution_flag execute ); // user operations which add and remove the failure point notation to this empty point
   bool RemoveFailurePoint( execution_flag execute );
   bool AddSharedResponsibility( execution_flag execute ); // user operations which add and remove the failure point notation to this empty point
   bool RemoveSharedResponsibility( execution_flag execute );
   bool RemoveDirectionArrow( execution_flag execute );
   bool FollowPathInSubmap( execution_flag execute );

   void AddSimple( Hyperedge *edge, bool new_left = FALSE, bool new_right = FALSE );  // replaces this with hyperedge, adds empty points if necessary
   void DecomposeFrom( MultipathEdge *mp_edge );  // decomposes the path of this empty point from multipath edge edge
   void TriggerAsynchronousConnection();

   Label *label;  // path label object
   ConditionManager condition_manager;  // object which manages the pre/postconditions
   bool display_label;  // flag representing displaying of scenario label
   int characteristics, path_number, highlight_id; // flag representing annotations to this empty point ( failure point / shared responsibility ) and stub binding label
   bool highlighted;
};

#endif
