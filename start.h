/***********************************************************
 *
 * File:			start.h
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef START_H
#define START_H

#include "waiting_place.h"
#include <fstream>

typedef enum { EXPONENTIAL, DETERMINISTIC, UNIFORM, ERLANG, EXPERT, UNDEFINED } arrival_type;
typedef enum { OPEN_ARRIVAL, CLOSED_ARRIVAL } stream_type;

class PointFigure;
class Scenario;

class Start : public WaitingPlace {

   friend class PointFigure;

public:

   Start( bool install = TRUE );
   Start( int start_id, const char *name, float x, float y, const char *desc, const char *logical_expression, arrival_type atype,
	  stream_type stype, int population, float values[], const char *expert, const char *alignment );
   ~Start();

   virtual edge_type EdgeType() { return( START ); } // returns identifier for subclass type
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual void SaveXMLDetails( FILE *fp ); // implements Save method for Starts, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ); // implements Save method for Starts, in XML format
   virtual void AddCSMResourceAcquire ();
   void AddStartResourceAcquire(Component* comp );
   void AddStartResourceRelease(Component* comp,ComponentReference* comp_ref);
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual void OutputPerformanceData( std::ofstream &pf );
   virtual bool HasTextualAnnotations();
   virtual int Search( search_direction dir ) { Hyperedge::joined_paths = TRUE; return( JOINED_PATHS ); } // terminates searches for joined paths
   virtual bool LabelCentered() { return( centered == TRUE ); }
   virtual Label * PathLabel();
   virtual bool DeleteHyperedge();
   virtual int DoubleClickAction();

   bool Perform( transformation trans, execution_flag execute ); // executes or validates the transformation with code trans
   bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ); // executes or validates the double selection transformation trans

   bool Merge( Hyperedge *edge, execution_flag execute ); // merges the start of this path with the end of another or the output of a stub
   bool ConnectPath( Hyperedge *trigger_edge, execution_flag execute ); // connects start point to a triggering path element
   bool DeletePath( execution_flag execute ); // deletes the entire path if user confirms operation
   bool CreateMSC( execution_flag execute );
   bool FollowPathInParentMap( execution_flag execute );
   void MergeWithResult( Hyperedge *result ); // merges two path together, destroying this start
   void MergeWithStub( Hyperedge *stub ); // sets the start of this path as the output of a stub, destroying this start

   bool CenterLabel( execution_flag execute ); // user operations which control alignment of label
   bool FlushLabel( execution_flag execute );

   static void ResetCounter() { number_starts = 1; } // resets global counter of Starts
   void SetBound() { bound = TRUE; }// operations on flag denoting whether path starting point is bound to a stub input
   void SetUnbound() { bound = FALSE; }
   bool IsBound() { return( bound == TRUE ); }
   arrival_type ArrivalType() { return( arrival ); } // access methods for arrival type
   void ArrivalType( arrival_type atype ) { arrival = atype; }
   stream_type StreamType() { return( stream ); }
   void StreamType( stream_type stype ) { stream = stype; }
   float Input( int index ) { return( input[index-1] ); } // access methods for distribution values
   void Input( float value, int index ) { input[index-1] = value; }
   int PopulationSize() { return( population_size ); }
   void PopulationSize( int new_size ) { population_size = new_size; }
   char * ExpertInput() { return( expert ); }  // access methods for expert input string
   void ExpertInput( const char *new_process );
   char * Identifier() { return( identifier ); } // returns user given identifier

   virtual void VerifyAnnotations();
   virtual void ResetHighlight() { annotations_complete = TRUE; }
   bool Highlight() { return( !annotations_complete ); }
   void HighlightEntry() { annotations_complete = FALSE; } // use same flag for stub entry highlight

   Cltn<Scenario *> * StartScenarios() { return( &start_scenarios ); }
   void AddScenario( Scenario *new_scenario ) { start_scenarios.Add( new_scenario ); }
   void RemoveScenario( Scenario *scenario ) { start_scenarios.Delete( scenario ); }
   Hyperedge * TraverseScenario( );
   bool IsConnectStart( );
   Hyperedge * HighlightScenario( );

private:

   bool ListBoundParentMaps( execution_flag execute );
   bool SetAsScenarioStart( execution_flag execute );
   bool ListScenarios( execution_flag execute );
   bool EditPathInitiation( execution_flag execute ); // allows user to enter time distribution characteristics through dialog
   bool SamePath( Hyperedge *first, Hyperedge *second ); // determines if two hyperedges are part of the same path

   arrival_type arrival; // enumerated type for arrival time distributions
   stream_type stream; // enumerated type for open or closed streams
   float input[2]; // values for time distributions
   int population_size; // fixed population size for closed streams
   char *expert; // string for expert mode on time distribution
   static int number_starts; // global count of start hyperedges
   bool bound, centered, annotations_complete; // flag denoting whether endpoint is bound and whether label is centered
   Cltn<Scenario *> start_scenarios;

};

#endif




