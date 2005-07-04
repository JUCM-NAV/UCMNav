/***********************************************************
 *
 * File:			multipath_edge.h
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef MULTIPATH_EDGE_H
#define MULTIPATH_EDGE_H

#include "hyperedge.h"

class MultipathEdge : public Hyperedge {

public:

   MultipathEdge() : Hyperedge() {}
   ~MultipathEdge() {}

   virtual void Delete()=0;  // polymorphic method which deletes multipath edge when it becomes redundant
   virtual int PathCount()=0;  // polymorphic method which returns number of inputs or outputs, or maximum if both exist
   virtual int Search( search_direction dir ) { return( Hyperedge::Search( BIDIRECTIONAL ) ); } // polymorphic method used to search for path joinings

};

#endif
