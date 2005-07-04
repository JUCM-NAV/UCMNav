/***********************************************************
 *
 * File:			timer.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:        June 1999
 *                              Added timeout paths
 *
 ***********************************************************/

#ifndef TIMER_H
#define TIMER_H

#include "wait_synch.h"
#include "condition.h"
#include "path_data.h"

class Timer;
class BooleanVariable;

class Timer : public WaitSynch {

public:

   Timer( Empty *empty );
   Timer( int timer_id, const char *name, float x, float y, const char *desc, BooleanVariable *tv,
	  const char *type = NULL, const char *logical_expression = NULL );
   ~Timer();

   virtual edge_type EdgeType() { return( TIMER ); }  // returns identifier for hyperedge type

   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual bool DeleteHyperedge();
   virtual void SaveXMLDetails( FILE *fp ); // implementation of Save method for timers
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){} // implementation of Save method for timers
   virtual bool HasTextualAnnotations();
   //virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir );
   virtual int DoubleClickAction() { return( EditTimerCharacteristics( TRUE ) ); }

   bool DeleteTimer( flag execute ); // transformation which deletes the timer
   bool Perform( transformation trans, execution_flag execute ); // validates or performs the single selection transformation with code trans
   bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ); // validates or performs the double selection transformation trans

   void TimerPositionChanged(); // notifies output paths of position change if one exists
   void DisconnectTimeoutPath();

   static void ValidateTimerList();
   Hyperedge * TraverseScenario( );
   Hyperedge * HighlightScenario( );
   Path_data * CreateAction();
   void EmptyPathData();

private:

   bool EditTimerCharacteristics( execution_flag execute ); // allows user to edit name and type of timer
   bool AddTimeoutPath( flag execute );    // adds a timeout path to the timer if one doesn't exist
   bool DeleteTimeoutPath( flag execute ); // deletes the timeout path if one exists
   void VerifyUniqueTimerName();

   static bool TimerNameValid( const char *new_name, const char *old_name = NULL );

   bool normal_path; // flag used in MSC generation

   BooleanVariable *timeout_variable;
   Empty *install_empty;

   static Cltn<Timer *> timer_list;
   Cltn<Path_data *> timer_element;
   Path_data * timer_set, * timer_reset, * trigger_point;
   //Cltn<Stub *> scanning_stub_list
};

#endif
