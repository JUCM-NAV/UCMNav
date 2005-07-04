/***********************************************************
 *
 * File:			node.h
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef NODE_H
#define NODE_H

#include "defines.h"
#include "transformation.h"
#include <stdio.h>

typedef enum { A, B, C, D } nodeColour;

class Hyperedge;
class TransformationManager;

class Node {

public:

   Node( nodeColour newColour );  // constructor, with node colour as a parameter
   ~Node() {}

   void AttachNext( Hyperedge *edge ) { next = edge; } // manipulation methods for next, previous pointers
   void AttachPrevious( Hyperedge *edge ) { previous = edge; }
   void SetColour( nodeColour newColour ) { colour = newColour; }

   void DetachAll() {   next = NULL; previous = NULL; }
   void DetachNext() { next = NULL; }
   void DetachPrevious() { previous = NULL; }

   nodeColour GetColour() { return ( colour ); }  // returns colour of node
   bool NextAttached() { return( next ? TRUE : FALSE ); } // bool functions specifying whether node is attached
   bool PreviousAttached() { return( previous ? TRUE : FALSE ); }
   Hyperedge *NextEdge() { return ( next ); } // returns pointer to the next hyperedge
   Hyperedge *PreviousEdge() { return ( previous ); } // returns pointer to the previous hyperedge
   int GetNumber() { return( node_number ); } // returns integer identifier of node

   static void ResetNodeCount() { number_nodes = 0; } // resets global count of nodes
   void setVisited(bool bv) { visited = bv; }
   bool getVisited() { return visited; }
   
private:

   Hyperedge *next, *previous;  // pointers to next and previous hyperedges in hypergraph
   nodeColour colour;  // node colour attribute

   int node_number, load_number; // integer identifiers for node
   static int number_nodes; // global count of number of nodes
   bool visited;

};

#endif
