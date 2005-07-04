/**********************************************************************
 *
 * File:			or_fork.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ********************************************************************/

#ifndef OR_FORK_H
#define OR_FORK_H

#include "multipath_edge.h"
#include <fstream>

class ExitPoint {  // data class used by OrFork

public:

   ExitPoint( Node *exit, char *choice = NULL, char *condition = NULL )
   {
      exit_point = exit;
      branch_choice = choice;
      logical_condition = condition;
      probability = 1.0;
   }

   ~ExitPoint() { if( logical_condition ) free( logical_condition ); }

   Node * ExitNode() { return( exit_point ); }

   char * LogicalCondition();
   void LogicalCondition( const char *new_condition );
   void LoadLogicalCondition( char *new_condition ) { logical_condition = new_condition; }
   const char *ReferencedCondition() { return( logical_condition ); }

   char * BranchChoice() { return( branch_choice ); }
   void BranchChoice( char *new_choice );
   void LoadBranchChoice( char *new_choice ) { branch_choice = new_choice; }

   float Probability() { return( probability ); }
   void Probability( float new_probability) { probability = new_probability; }
   Hyperedge * TraverseScenario();

private:

   Node *exit_point;  // an output node of the fork
   char *logical_condition; // text string for logical branch selection condition
   char *branch_choice;  // text string representing branch selection mechanism
   float probability;

};

class OrFork : public MultipathEdge {

public:

   OrFork();
   OrFork( int fork_id, const char *name, float x, float y, const char *desc, int orientation );
   ~OrFork() {}

   virtual edge_type EdgeType() {  return( OR_FORK ); }  // returns identifier for subclass type
   virtual bool Perform( transformation trans, execution_flag execute );  // executes or validates transformations with code trans
   virtual const char * HyperedgeName() { return( identifier ); } // returns textual identfier
   virtual bool HasName() { return( !strequal( identifier, "" ) ); }
   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir );
   virtual bool HasTextualAnnotations();
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual int DoubleClickAction() { return( EditForkSpecification( TRUE ) ); }

   virtual void VerifyAnnotations();
   virtual void ResetHighlight() { annotations_complete = TRUE; }
   bool Highlight() { return( !annotations_complete ); }

   void ExtendFork();  // creates second output branch of fork
   void Delete();  // deletes the fork when it becomes unnecessary
   void ForkName( const char *name ) { strncpy( identifier, name, 19 ); }

   virtual void AttachTarget( Node *new_node ); // adds ExitPoint data object to exit points list for each new target node
   virtual void DetachTarget( Node *node ); // removes corresponding ExitPoint from exit points list
   int PathCount(); // returns number of output branches

   virtual void SaveXMLDetails( FILE *fp ); // implements Save method for this hyperedge, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ); // implements Save method for this hyperedge, in XML format

   virtual void OutputPerformanceData( std::ofstream &pf );
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual void DeleteGenerationData();
   virtual void SavePathBranchingSpec( FILE *fp ); // polymorphic method which saves branch selection characteristics
   void AddBranchingCharacteristic( Hyperedge *next_edge, const char *condition, const char *characteristic, float branch_prob ); // adds branch spec from XML
   ExitPoint * BranchSpecification( Node *output_node );
   void PositionOrderOutputs();

   static void ResetCounter() { number_forks = 1; }  // resets global counter of Or Forks
   Hyperedge * TraverseScenario();
   Hyperedge * HighlightScenario( );

protected:

   bool AddBranch( flag execute );  // transformation which adds a branch to the fork
   bool EditForkSpecification( execution_flag execute ); // allows user to enter identifier

   char identifier[20]; // user given identifier
   Cltn<ExitPoint *> exit_points;  // dictionary of branch selection strings indexed on node pointers
   bool annotations_complete;  // flag used to determine whether the branching for this fork has been specified
   static int number_forks;  // global counter of number of or fork instances
   char *previous_selection;  // temporary storage for branch selection pointer

   Cltn<Hyperedge *> traversal_outputs;
   int traversal_context; // count of contexts of this object in traversal
   bool generation_pass;
};

#endif


