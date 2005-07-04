/***********************************************************
 *
 * File:			or_join.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef OR_JOIN_H
#define OR_JOIN_H

#include "multipath_edge.h"
#include <fstream>

class ElementContext {

public:

   ElementContext( int *hierarchy, int level ) { stub_hierarchy = hierarchy; decomposition_level = level; traversal_count = 0; }
   ~ElementContext();

   bool SameContext( int *hierarchy, int level  );

   void IncrementTraversalCount() { traversal_count++; }
   int TraversalCount() { return( traversal_count ); }

   int *stub_hierarchy, decomposition_level, traversal_count;
};


class OrJoin : public MultipathEdge {

public:

   OrJoin();
   OrJoin( int join_id, float x, float y, const char *desc, int orientation );
   ~OrJoin() {}

   virtual edge_type EdgeType() { return( OR_JOIN ); }  // returns identifier for subclass type
   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir );
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual void DeleteGenerationData();
   virtual void SaveXMLDetails( FILE *fp ); // saves the or join's details to file, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ); // saves the or join's details to file, in XML format

   bool Perform( transformation trans, execution_flag execute );  // executes or validates transformations with code trans
   int PathCount();  // returns number of input branches

   void ConnectForkPath( Hyperedge *edge );  // connects the path ending in hyperedge edge as an input path to the or join
   void Delete();  // deletes the join when it becomes unnecessary
   static void ResetCounter() { number_joins = 1; }  // resets global counter of Or Joins
   Hyperedge * HighlightScenario( );

protected:

   Cltn<ElementContext *> context_history;

   static int number_joins;  // global counter of number of or join instances
};

#endif
