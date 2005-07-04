/***********************************************************
 *
 * File:			connect.h
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:	June 1999
 *                              Added Abort hyperedge
 *
 ***********************************************************/

#ifndef CONNECT_H
#define CONNECT_H

#include "hyperedge.h"


class Connect : public Hyperedge {

public:

   Connect();
   Connect( int connect_id );
   ~Connect() {}

   virtual edge_type EdgeType() { return( CONNECTION ); }  // returns flag identifing hyperedge type
   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir ) { return FALSE; } // end branch of search
   virtual void SaveXMLDetails( FILE *fp ); // polymorphic method which saves the hyperedge's data to a file, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){} // polymorphic method which saves the hyperedge's data to a file, in XML format

};

class Abort : public Hyperedge {

public:

   Abort();
   Abort( int abort_id );
   ~Abort() {}

   virtual edge_type EdgeType() { return( ABORT ); }  // returns flag identifing hyperedge type
   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir ) { return FALSE; } // end branch of search
   virtual HyperedgeFigure * GetFigure();
   virtual void SaveXMLDetails( FILE *fp ); // polymorphic method which saves the hyperedge's data to a file, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){} // polymorphic method which saves the hyperedge's data to a file, in XML format

};

#endif
