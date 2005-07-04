/***********************************************************
 *
 * File:			hypergraph.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef HYPERGRAPH_H
#define HYPERGRAPH_H

#include "hyperedge.h"
#include <fstream>

class Node;
class TransformationManager;
class Responsibility;

class Hypergraph {

   friend class DisplayManager;

public:

   Hypergraph();
   ~Hypergraph();

   Cltn<Hyperedge *> * Hyperedges() { return( edge_pool ); } // returns list of hyperedges
   Cltn<Node *> * Nodes() { return( node_pool ); } // returns list of nodes
   void PurgeEdge( Hyperedge *edge );  // removes hyperedge edge from hypergraph
   void PurgeNode( Node *node ); // removes node from hypergraph
   void PurgeSegmentFrom( Node *start_node, Node *end_node ); // removes section of graph between start_node and end_node
   void PurgeLoop( Node *input_node, Node *output_node ); // removes looped section of graph between start_node and end_node
   void PurgeGraph();
   void RegisterEdge( Hyperedge *edge ); // registers an edge as part of the hypergraph
   void RegisterNode( Node *node ); // registers a node as part of the hypergraph

   bool TrashEdge( Hyperedge *edge, int index = 0 ); // marks hyperedge as deleted by placing in deleted list
   void TrashNode( Node *node, int index = 0 ); // marks node as deleted by placing in deleted list
   Cltn<Hyperedge *> * EdgeTrash() { return( edge_trash[0] ); } // returns list of "deleted" hyperedges
   Cltn<Node *> * NodeTrash() { return( node_trash[0] ); } // returns list of "deleted" nodes
   void EmptyAllTrash( int index = 0 );  // permananently deletes all "deleted" hyperedge, nodes
   void ResetVisited(); // resets the visited flags for all hyperedges

   void SaveXML( FILE *fp ); // saves the contents of the hypergraph to disk in XML format, calls Save functions of elements
   void SaveDXL( FILE *fp );     //added by Bo Jiang for graph info exporting in DXL, August, 2004
   void SaveCSMXML( FILE *fp );
   void SavePluginBindings( FILE *fp );  // saves the bindings of all stubs to their plugin maps
   bool HypergraphExists() { return( edge_pool->Size() > 0 ? TRUE : FALSE ); }

   Responsibility * FindResponsibility( int resp_id );
   void ConnectHyperedges( int source_id, int target_id );
   Hyperedge * FindHyperedge( int hyperedge_id );

#ifdef DEBUG
   void Output();  // debugging method
#endif

private:

   void PurgeSubSegment( Node *start_node, Node *end_node );  // removes a section of hypergraph

   Cltn<Node *> *node_pool, *node_trash[2]; // lists of current nodes and deleted nodes
   Cltn<Hyperedge *> *edge_pool, *edge_trash[2]; // lists of current hyperedges and deleted hyperedges
   TransformationManager *trans_manager;

};

#endif





