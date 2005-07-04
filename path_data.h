/***********************************************************
 * File:			path_data.h
 * Author:			Andrew Miga
 * Created:			November 1999
 *
 * Modification history:         Feb 2003
 *                               Shuhua Cui
 *
 ***********************************************************/

#ifndef PATH_DATA_H
#define PATH_DATA_H

#include "collection.h"
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "xml_generator.h"

class Hyperedge;

typedef enum { start, End_Point, WP_Enter, WP_Leave, Timer_Set, Timer_Reset, Time_Out, And_join, Resp, Waiting_place, Fork, Connect_Start, Connect_End, Trigger_End } action_type;

class Path_data {

public:

   Path_data();
   virtual ~Path_data();
   Cltn<Path_data *>* NextPath();

   void AddNext(Path_data * next);
   void RemoveFromNext(Path_data * p_data);
   Path_data * Next();

   void RegisterDestination( Path_data * des ) { destination = des; }
   Path_data * GetDestination() { return destination; }
   Path_data * Previous();
   void AddPrevious(Path_data * pre ) { previous.Add(pre); }
   Cltn<Path_data *>* PreviousPath();
   void RemoveFromPrevious(Path_data * p_data);

   virtual Path_data * GetRoot() { return root; }
   void SetRoot( Path_data * new_root ) { root = new_root; }
   virtual char * GetName() = 0;
   virtual void SaveParallelXML(FILE * outfile, Path_data * root);
   virtual void SaveParallelDXL(FILE * outfile, Path_data * root ,char * parentID);     // Added by Bo Jiang, for DXL exporting.  July, 2004
   void IncrementScanCount( ) { scanCount++; }
   void ResetScanCount( ) { scanCount = 0; }
   int GetScanCount( ) { return scanCount; }
   virtual void SetComponentName( const char * ) = 0;
   virtual void SetComponentId( int ) = 0;
   virtual void SetComponentRole( const char * )= 0;
   virtual void SetComponentReferenceId( int)= 0;      // Added by Bo Jiang,.  Sept, 2004
   virtual Path_data * SaveScenarioXML( FILE * ) = 0;
   virtual Path_data * SaveScenarioDXL( FILE *, const char * ) = 0;      // Added by Bo Jiang, for DXL exporting.  July, 2004
   virtual int GetHyperedge() =0;  //for dxl purpose, Bo Jiang
protected:

   Cltn<Path_data *> next_path;
   Path_data * root;
   Path_data * destination;
   Cltn<Path_data *> previous;
   int scanCount;
};

#endif
