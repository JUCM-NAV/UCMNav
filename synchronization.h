/***********************************************************
 *
 * File:			synchronization.h
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "multipath_edge.h"
#include <fstream>
#include "path_data.h"
#include "collection.h"

typedef enum { FORK, JOIN, MULTIPATH } synch_type;

class Synchronization : public MultipathEdge {

public:

   Synchronization( synch_type type );
   Synchronization( int synch_id, float x, float y, const char *desc, int orientation );
   ~Synchronization() {}

   edge_type EdgeType() {  return( SYNCHRONIZATION ); }  // returns identifier of subclass type
   synch_type SynchronizationType() { return( stype ); }  // returns type of synchronization

   virtual void AttachSource( Node *new_node );  // node methods keep track of current synchronization type
   virtual void AttachTarget( Node *new_node );
   virtual void DetachSource( Node *node );
   virtual void DetachTarget( Node *node );

   int PathCount();  // returns maximum of input/ouput counts
   int InputCount() { return( source->Size() ); }  // returns number of input paths
   int OutputCount() { return( target->Size() ); } // returns number of output paths

   void ConnectForkPath( Hyperedge *edge ); // connect a path as an input to this synchronization
   void ExtendFork();  // creates second output branch of a fork when it is first placed in a path
   void Delete();  // deletes the synchronization from the path when it is no longer needed

   bool Perform( transformation trans, execution_flag execute );  // validates or performs the single selection transformation with code trans
   bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ); // validates or performs the double selection transformation trans

   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir );
   virtual bool IsAndJoin() { return((source->Size() > 1) ? TRUE : FALSE ); }
   virtual void SaveXMLDetails( FILE *fp ); // implements Save method for this hyperedge, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ); // implements Save method for this hyperedge, in XML format
   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual Hyperedge * ScanScenarios( scan_type type );

   static void ResetHistory() { previous_join = NULL; }
   bool allVisited();
   void ResetAllVisited();

   Hyperedge * TraverseScenario( );
   void RegisterWaitingElement(Path_data *);
   void Collapse( Path_data *, bool );
   //bool SearchRoot( );
   void SetDestination( Path_data * );
   void ResetScenarioGenerated(bool);
   Path_data * FindDataWithCount( int );
   Path_data * AddSynchronizationPoint( Path_data * );
   void ResetReferenceCount( );
   Path_data * Advance(Path_data * current_data );
   void MoveToWaitingPlace( Path_data * root, Path_data * des );
   void AddForkAfter( Path_data * root, Path_data * des );
   void ResetTraverseState( );
   Hyperedge * HighlightScenario( );

private:

   static Synchronization *previous_join;

   bool AddBranch( flag execute );  // adds a new output branch to the synchronization

   bool AddIncomingSynch( Hyperedge *edge, flag execute ); // allows user to add a new input path to the synchronization

   void RecheckSynchronizationType();

   synch_type stype;  // type of synchronization
   int current_branch_index; // integer specifying the branch being
   Cltn<Path_data *> waiting_element; // collection of previous path_data of each branch
   Path_data * waiting_place;   // path_data corresponding to current synchronization point
   bool scenarioGenerated;
};

#endif



