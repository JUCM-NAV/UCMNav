/***********************************************************
 *
 * File:			loop.h
 * Author:			Andrew Miga
 * Created:			April 2000
 *
 * Modification history:
 ***********************************************************/

#ifndef LOOP_H
#define LOOP_H

#include "hyperedge.h"

class Path;

class Loop : public Hyperedge {

public:

   Loop();
   Loop( int loop_id, const char *name, float x, float y, const char *desc, const char *orientation, const char *count );
   ~Loop() {}

   virtual edge_type EdgeType() {  return( LOOP ); }  // returns identifier for subclass type
   virtual const char * HyperedgeName() { return( identifier ); } // returns textual identfier
   virtual bool HasName() { return( !strequal( identifier, "" ) ); }
   virtual bool Perform( transformation trans, execution_flag execute );  // executes or validates transformations with code trans
   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir );
   virtual int DoubleClickAction() { return( EditLoopCharacteristics( TRUE ) ); }
   virtual bool DeleteHyperedge();
   virtual void SaveXMLDetails( FILE *fp ); // polymorphic method which saves the hyperedge's data to a file, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){} // polymorphic method which saves the hyperedge's data to a file, in XML format

   virtual void OutputPerformanceData( std::ofstream &pf );
   virtual bool HasTextualAnnotations();
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual int Search( search_direction dir );
   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual Hyperedge * ScanScenarios( scan_type type );

   virtual const char * LoopCount() { return( loop_count ); }
   virtual bool HasCount() { return( !strequal( loop_count, "" ) ); }
   void LoopCount( const char *new_count ) { strncpy( loop_count, new_count, 19 ); }

   void Create();
   int LoopInput( Hyperedge *previous_edge );
   Path * LoopOutputPath();

private:

   bool DeleteLoop( execution_flag execute );
   bool EditLoopCharacteristics( execution_flag execute );

   char identifier[20], loop_count[20]; // user given identifier and loop count
};

#endif
