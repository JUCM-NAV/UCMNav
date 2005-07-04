/***********************************************************
 *
 * File:			wait.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef WAIT_H
#define WAIT_H

#include "wait_synch.h"
#include "condition.h"
#include "path_data.h"

class Wait;

class Wait : public WaitSynch {

public:

   Wait();
   Wait( int wait_id, const char *name, float x, float y, const char *desc, const char *type = NULL, const char *logical_expression = NULL );
   ~Wait() {}

   virtual edge_type EdgeType() { return( WAIT ); }  // returns identifier for hyperedge type

   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual bool DeleteHyperedge();
   virtual void SaveXMLDetails( FILE *fp ); // implementation of Save method for waits
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){} // implementation of Save method for waits
   virtual bool HasTextualAnnotations();
   virtual int DoubleClickAction() { return( EditWaitCharacteristics( TRUE ) ); }

   bool Perform( transformation trans, execution_flag execute ); // validates or performs the single selection transformation with code trans
   bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ); // validates or performs the double selection transformation trans

   void WaitPositionChanged(); // notifies output path of position change

   Hyperedge * TraverseScenario( );
   //void RegisterWaitingElement(Path_data *);
   void RegisterTriggerPoint(Path_data * trigger) { trigger_point = trigger; }
   Hyperedge * HighlightScenario( );
   Path_data * CreateAction();
   void EmptyPathData();

private:

   bool EditWaitCharacteristics( execution_flag execute ); // allows user to edit name and type of wait
   bool DeleteWait( flag execute ); // transformation which deletes the wait

   Cltn<Path_data *> waiting_element;
   Path_data * waiting_enter, * waiting_leave, * trigger_point;
};

#endif
