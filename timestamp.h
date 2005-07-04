/***********************************************************
 *
 * File:			timestamp.h
 * Author:			Andrew Miga
 * Created:			October 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "hyperedge.h"
#include <fstream>

typedef enum { PREVIOUS, NEXT } timestamp_reference;  // enumerated data type which refers to whether timestamps references previous or next element


class Timestamp : public Hyperedge {

public:

   Timestamp();
   Timestamp( int tsid, const char *ts_name, float x, float y, const char *desc, int orientation, timestamp_reference ref );
   ~Timestamp();

   virtual edge_type EdgeType() { return( TIMESTAMP ); }  // returns identifier for hyperedge type
   virtual const char * HyperedgeName() { return( name ); } // default ( empty ) identfier access method
   virtual bool Perform( transformation trans, execution_flag execute ); // validates or performs the single selection transformation with code trans
   virtual bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ); // validates/performs double selection transformation trans
   virtual void SaveXMLDetails( FILE *fp ); // implements Save method for timestamp, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){} // implements Save method for timestamp, in XML format
   virtual bool HasName() { return( TRUE ); }
   virtual bool DeleteHyperedge() { return( this->DeleteElement() ); }

   char *Name() { return( name ); }  // access method for Name
   void Name( const char *new_name ); // set method for name
   timestamp_reference Reference() { return( reference ); } // access method for reference
   void Reference( timestamp_reference tr ) { reference = tr; } // set method for reference

private:

   bool DeleteTimestamp( execution_flag execute ); // transformation which deletes the timestamp point
   bool EditTimestamp( execution_flag execute ); // allows user to edit timestamp point characteristics
   bool ViewResponseTimes( execution_flag execute ); // allows user to view all response time requirements involving this timestamp point
   bool CreateResponseTime( Hyperedge *ts, flag execute ); // allows user to create a response time requirement involving this timestamp point
   bool EditResponseTime( Hyperedge *ts, flag execute ); // allows user to edit a response time requirement involving this timestamp point
   bool DeleteResponseTime( Hyperedge *ts, flag execute );  // allows user to delete a response time requirement involving this timestamp point

   char name[20];  // user given name for timestamp
   timestamp_reference reference; // data type which refers to element referenced by timestamp

};

#endif
