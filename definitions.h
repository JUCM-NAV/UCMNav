// external definitions for data classes to avoid template problems

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "defines.h"

// allow reuse of BoundaryListItem as Mapping class
#define Mapping BoundaryListItem
#define replaced_identifier hyperedge_id
#define existing_identifier stub_boundary_id 

class Map;
class Stub;

class Binding {

public:

   Binding( Stub *s, Map *m, int n, int pe, int pl )
      { stub = s; map = m; node = n; path_endpoint = pe; plugin = pl; }

   bool operator==( Binding &b )
      {
	 if( (b.stub == stub) && (b.node == node) )
	    return TRUE;
	 else
	    return FALSE;
      }
  
   Stub *stub;
   Map *map;
   int node, path_endpoint, plugin;
};

class BoundaryListItem {

public:

   BoundaryListItem( int hid, int sid ) { hyperedge_id = hid; stub_boundary_id = sid; }

   bool operator==( const BoundaryListItem &b )
      {
	 if( (b.hyperedge_id == hyperedge_id) && (b.stub_boundary_id == stub_boundary_id) )
	    return TRUE;
	 else
	    return FALSE;
      }
   
   int hyperedge_id, stub_boundary_id;
};

class Trans_Rec {
   
public:

   Trans_Rec( unsigned int trans, bool avail ) { transformation = trans; available = avail; }

   bool operator==( const Trans_Rec &e )
      {
	 if( (e.transformation == transformation ) && (e.available == available) )
	    return TRUE;
	 else
	    return FALSE;
      }
   
   unsigned int transformation;
   bool available;
   
};

#endif
