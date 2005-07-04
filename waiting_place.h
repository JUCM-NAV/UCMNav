/***********************************************************
 *
 * File:			waiting_place.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef WAITING_PLACE_H
#define WAITING_PLACE_H

#include "hyperedge.h"
#include "condition.h"
#include <fstream>

class WaitingPlace : public Hyperedge {

public:

   WaitingPlace();
   ~WaitingPlace();

   virtual void EdgeSelected();  // implementation of polymorphic method which displays conditions in main window
   virtual const char * HyperedgeName() { return( identifier ); }  // returns textual identifier
   virtual bool HasName() { return( TRUE ); }
   virtual bool HasConditions() { return( !condition_manager.ConditionsEmpty() ); }

   bool ConnectPath( Hyperedge *trigger_edge, execution_flag execute );  // hypergraph transformation which attaches a triggering element to waiting place
   bool DisconnectPath( Hyperedge *trigger_edge, execution_flag execute );  // hypergraph transformation which detaches triggering elements if they exist

   static void ResetCounter() { number_waits = 1; }  // resets global counter for class
   void WaitingName( char *new_name );  // sets new identifier
   void AddTriggeringEvent(  char *condition ) { condition_manager.AddCondition( TRIGGERING, condition ); }
   void AddPrecondition( char *condition ) { condition_manager.AddCondition( PRECONDITION, condition ); }

   Hyperedge * EvaluateGuardCondition();
   virtual void TriggerWaitingPlace() {};
   bool EvaluateCondition();
   
protected:

   void DeleteWaiting();  // deletes the waiting place map element from map, called by subclasses
   bool EditWaitingName( execution_flag execute );  // allows user to edit identifier of element
   bool EditGuardCondition( execution_flag execute );  // allows user to edit the logical condition of element
   void BreakConnections();  // breaks triggering connections to this element if they exist

   ConditionManager condition_manager;  // instance of object which manages textual conditions
   char identifier[20];  // user given textual identifier
   char *logical_condition;

   static int number_waits;  // global count of instances of subclass
};

#endif
