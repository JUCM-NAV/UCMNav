/***********************************************************
 *
 * File:			variable.h
 * Author:			Andrew Miga
 * Created:			September 2000
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef VARIABLE_H
#define VARIABLE_H

#include "collection.h"
#include <stdio.h>
#include <string.h>

#define VARIABLE_NAME 30
#define VARIABLE_DELETED 3

class BooleanVariable;
class VariableInit;

class BooleanVariable {

public:

   BooleanVariable( const char *name, bool timer = FALSE );
   BooleanVariable( int id, const char *name, int ref_count );
   ~BooleanVariable();
   int BooleanId() { return( boolean_id ); }
   const char * BooleanName() { return( boolean_name ); }
   void BooleanName( const char *new_name );

   void Set() { value = TRUE; }
   void Reset() { value = FALSE; }
   void Value( bool new_value ) { value = new_value; initialized = TRUE; }
   bool Value() { return( value ); }

   bool IsNotInitialized() { return( initialized == FALSE ); }
   void Uninitialize() { initialized = FALSE; value = UNINITIALIZED; }
   bool IsNotReferenced() { return( referenced == FALSE ); }
   void Reference() { referenced = TRUE; }
   void Dereference() { referenced = FALSE; }
   void IncrementReferenceCount() { reference_count++; }
   void DecrementReferenceCount() { reference_count--; }
   const int ReferenceCount() { return( reference_count ); }
   void SetAsTimeoutVariable() { timer_variable = TRUE; }
   void AllowTimerVariableDeletion() { timer_variable = FALSE; }
   bool IsTimeoutVariable() { return( timer_variable ); }

   static void AddBooleanVariable( int id, const char *name, int ref_count );
   static void ViewBooleanList();
   static void SaveXML( FILE *fp );
   static void GeneratePostScript( FILE *ps_file );
   static void ClearBooleanList();
   static int NameIndex( const char *new_name );
   static bool BooleanVariablesExist() { return( (boolean_variables.Size() != 0) ? TRUE : FALSE ); }
   static Cltn<BooleanVariable *> *VariableList() { return( &boolean_variables ); }
   static BooleanVariable * ReferencedVariable( int id );
   static BooleanVariable * SelectedVariable( int index );
   static bool VariableValue( int index );
   static void DereferenceVariableList();
   static void ScenarioInitialize( Cltn<VariableInit *> *initializations );
   static char * NameToReference( const char *new_condition, char *old_condition );
   static char * ReferenceToName( const char *condition );
   static bool UninitializedVariableReferenced( const char *condition );

   static int GetNumberBooleans() { return( number_booleans ); } // Apr2005 gM: required for map import to avoid conflict with constant IDs

protected:

   static void OrderVariable( BooleanVariable *bv );
   static void AdjustReferenceCounts( const char *condition, bool adjust_type );

   char boolean_name[VARIABLE_NAME+1];
   bool value, initialized, referenced, timer_variable;
   int boolean_id, reference_count;

   static Cltn<BooleanVariable *> boolean_variables;
   static int number_booleans, max_id;
};

class PathVariableList {

public:

   static void ClearVariableList();
   static void SaveXML( FILE *fp );
   static void GeneratePostScript( FILE *ps_file );
   static bool EvaluateLogicalExpression( const char *expression );

};

#endif
