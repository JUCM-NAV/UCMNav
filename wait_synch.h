/***********************************************************
 *
 * File:			wait_synch.h
 * Author:			Andrew Miga
 * Created:			September 2001
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef WAIT_SYNCH_H
#define WAIT_SYNCH_H

#include "waiting_place.h"
#include "path_data.h"

class WaitContext;

class WaitContext {
public:

   WaitContext( bool main_path, int * =NULL, int = 0 );
   ~WaitContext();

   bool SameContext(  WaitContext *wc );

   int *stub_hierarchy;
   int decomposition_level;
   bool main_arrived, trigger_arrived;
   bool fork_synchronization;
   int generation_id;
   bool isSynchronized;
};

class WaitSynch : public WaitingPlace {

public:

   WaitSynch() : WaitingPlace() {}
   ~WaitSynch() {}

   virtual void DeleteGenerationData();
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual void TriggerWaitingPlace() { TriggerWaitSynch(); };
   Cltn<WaitContext *> * getWaitContexts() { return &wait_contexts; }
   void StubContext(int ** stub_hierarchy, int &decomposition_level);
   
   void Collapse( Cltn<Path_data *>& wait_element, Path_data *& wait_leave );
   Path_data * Advance(Path_data * current_data );
   void SearchRoot(Cltn<Path_data *> & wait_element, Path_data *& wait_leave );
   void SetDestination( Cltn<Path_data *> & wait_element, Path_data *& wait_leave );
   void AddForkAfter( Path_data * root, Path_data * des );
   virtual void EmptyPathData() = 0;

protected:
   
   bool InputsSynchronized();
   void TriggerWaitSynch();
   void StoreSynchType( bool fork_synch );
   bool ForkSynchronization();
   bool InputsSynchronization(  Path_data * & );
   virtual Path_data * CreateAction() = 0;
   Cltn<WaitContext *> wait_contexts;
   char wait_type[DISPLAYED_LABEL+1];  // user given description of wait type
  
private:

   WaitContext * ElementContext();
};

#endif
