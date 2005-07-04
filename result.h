/***********************************************************
 *
 * File:			result.h
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef RESULT_H
#define RESULT_H

#include "hyperedge.h"
#include "condition.h"
#include <fstream>

class PointFigure;

class Result : public Hyperedge {

   friend class PointFigure;

public:

   Result( bool install = TRUE );
   Result( int result_id, const char *name, float x, float y, const char *desc, const char *alignment );
   ~Result();

   bool Perform( transformation trans, execution_flag execute ); // executes or validates the transformation with code trans
   bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ); // executes or validates the double selection transformation trans
   bool AndCompose( Hyperedge *edge, flag execute ); // transformations which attach the end of path to either an existing and/or join or to another
   bool OrCompose( Hyperedge *edge, flag execute );  // path at an empty point, in which case an and/or join is created
   bool FollowPathInParentMap( execution_flag execute );

   virtual edge_type EdgeType() { return( RESULT ); } // returns identifier for subclass type
   virtual void EdgeSelected(); // called when element selected, installs textual conditions in main window
   virtual void SaveXMLDetails( FILE *fp ); // implements Save method for Results, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ); // implements Save method for Results, in XML format
   void AddCSMEndResourceRelease ();
   void AddEndResourceAcquire(Component* comp );
   void AddEndResourceRelease(Component* comp, ComponentReference* comp_ref );
   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual const char * HyperedgeName() { return( identifier ); } // returns textual identfier
   virtual bool HasName() { return( TRUE ); }
   virtual bool LabelCentered() { return( centered == TRUE ); }
   virtual int Search( search_direction dir ) { return( QUIT ); } // terminates branch of search for joined paths
   virtual bool HasConditions() { return( !condition_manager.ConditionsEmpty() ); }
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual int DoubleClickAction();

   static void ResetCounter() { number_results = 1; } // resets global counter of Results
   void SetBound() { bound = TRUE; } // operations on flag denoting whether path endpoint is bound to a stub output
   void SetUnbound() { bound = FALSE; }
   bool IsBound() { return( bound == TRUE ); }
   void PathEndName( char *new_name ); // set method for identifier
   void AddResultingEvent( char *condition ) { condition_manager.AddCondition( RESULTING, condition ); }
   void AddPostcondition( char *condition ) { condition_manager.AddCondition( POSTCONDITION, condition ); }

   void Highlight() { highlighted = TRUE; }
   void Unhighlight() { highlighted = FALSE; }
   bool Highlighted() { return( highlighted ); }
   virtual Hyperedge * TraverseScenario( );
   Hyperedge * HighlightScenario( );

private:

   bool ConnectPath( Hyperedge *trigger_edge, flag execute ); // transformation which attaches this path end as the triggering element of a waiting place
   bool DisconnectPath( Hyperedge *trigger_edge, flag execute ); // transformation which detaches a triggering connection if one exists
   bool StubCompose( Hyperedge *edge, flag execute ); // transformation which attaches the end of the result's path as an input path to a stub

   bool CenterLabel( execution_flag execute ); // user operations which control alignment of label
   bool FlushLabel( execution_flag execute );
   bool EditPathEndName( execution_flag execute ); // allows user to enter identifier
   bool ListBoundParentMaps( execution_flag execute );

   bool ValidComposition();

   ConditionManager condition_manager; // object which manages the textual conditions
   char identifier[20]; // user given identifier
   static int number_results; // global count of result hyperedges
   bool bound, centered; // flag denoting whether endpoint is bound and whether label is centered
   bool highlighted;

};

#endif
