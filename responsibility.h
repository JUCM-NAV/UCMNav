/***********************************************************
 *
 * File:			responsibility.h
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:	April 1999
 *				Separated from responsibility
 *				references
 *
 ***********************************************************/

#ifndef RESPONSIBILITY_H
#define RESPONSIBILITY_H

#include "hyperedge.h"
#include "node.h"
#include "transformation.h"
#include "resp_figure.h"
#include "condition.h"
#include "service.h"
#include <fstream>

#define EVALUATED_EXPRESSION 2

class BooleanVariable;

class DataReference {  // data objects used by Responsibility

public:

   DataReference( int data, int mode ) { data_store = data; access_mode = mode; }
   ~DataReference() {}

   int Data() { return( data_store ); }
   void Data( int data ) { data_store = data; }
   int Access() { return( access_mode ); }
   void Access( int mode ) { access_mode = mode; }
   
private:

   int data_store, // integer reference to data store stored in DataStoreDirectory object
      access_mode; // integer reference to access mode

};

class VariableOperation {
public:
 
   VariableOperation( BooleanVariable *bvar, int val, char *lexp, bool translate = FALSE );
   ~VariableOperation();

   void SetVariable();
   int Value() { return( value ); }
   void Value( int new_value ) { value = new_value; }
   char * Expression() { return( logical_expression ); }
   BooleanVariable * Variable() { return( variable ); }
   char * LogicalExpression();
   void LogicalExpression( const char *new_expression );

   static void SaveVariableOperations( Cltn<VariableOperation *> *operations, FILE *fp );
   static void GeneratePostScriptDescription( Cltn<VariableOperation *> *operations, FILE *ps_file );

private:

   BooleanVariable *variable;
   int value;
   char *logical_expression;
};

class Responsibility {

public:

   Responsibility();
   Responsibility( int resp_id, const char *name, const char *es, const char *desc ); // constructor for file loading
   ~Responsibility();
   
   char * Name() { return( name ); } // access methods for name, description, and execution sequence
   void Name( const char *new_name );
   char * Description() { return( description ); }
   void Description( const char *new_description );
   char * ExecutionSequence() { return( execution_sequence ); }
   void ExecutionSequence( const char *new_es );
   
   bool HasDynarrow();  // returns flag signifying if dynamic arrow present
   void SetDynamicArrow( DynamicArrow *new_arrow ) { dynamic_arrow = new_arrow; } // access methods for dynamic arrow
   DynamicArrow * GetDynamicArrow() { return( dynamic_arrow ); }

   void SaveXML( FILE *fp ); // implements Save method for responsibilities, in XML format
   void SaveCSMXML( FILE *fp ); // implements Save method for responsibilities, in XML format
   void GeneratePostScriptDescription( FILE *ps_file );
   bool HasConditions() { return( !condition_manager.ConditionsEmpty() ); }
   void EditConditions();
   int ResponsibilityNumber() { return( responsibility_number ); }
   int LoadIdentifier() { return( (load_number != NA) ? load_number : responsibility_number ); }
   void ResetLoadIdentifier() { load_number = NA; }

   Cltn<DataReference *> * DataReferences() { return( data ); }  // returns list of data references
   void AddDataReference( int data_id, int access_id );  // adds new data store reference to list
   Cltn<ServiceRequest *> *ServiceRequests() { return( service_requests ); }
   void AddServiceRequest( int device_id, const char *amount ); // adds new service request reference to list
   void DeleteDataReference( DataReference *dr ) { data->Delete(dr); delete dr; } // deletes data store reference
   bool ReferencesData( int data_id ); // returns whether a certain data store is referenced or not
   void AddDynamicArrow( const char *type, const char *direction, const char *sourcepool, float length );
   void AddPrecondition( char *condition ) { condition_manager.AddCondition( PRECONDITION, condition ); }
   void AddPostcondition( char *condition ) { condition_manager.AddCondition( POSTCONDITION, condition ); }
   Cltn<VariableOperation *> * VariableOperations() { return( variable_operations ); }
   void AddVariableOperation( int boolean_id, int value, const char *expression );
   void ExecuteVariableOperations();

   void IncrementCount() { reference_count++; } // increments reference count
   void DecrementCount(); // decrements count and deletes itself if count is zero
   bool IsNotReferenced() { return( reference_count == 0 ); }
   void ResetVisited() { visited = FALSE; }
   void SetVisited() { visited = TRUE; }
   bool Visited() { return( visited ); }

   bool VerifyAnnotations();
   void ResetHighlight() { annotations_complete = TRUE; }
   bool Highlight() { return( !annotations_complete ); }

   static void ResetResponsibilityCount() { number_responsibilities = 0; }
   
   void ExportDXL();  // saves the DXL calls for the responsibility
   
private:

   char name[32], *description, *execution_sequence; // responsibility name, description, and execution sequence
   DynamicArrow *dynamic_arrow; // pointer to possible dynamic pointer
   Cltn<DataReference *> *data;  // list of DataReference objects which store references to global data stores
   Cltn<ServiceRequest *> *service_requests; // list of ServiceRequest objects which store access to services
   Cltn<VariableOperation *> *variable_operations;
   int reference_count, responsibility_number, load_number;
   bool visited, annotations_complete;
   static int number_responsibilities;
   ConditionManager condition_manager; // object which manages the textual conditions
   
};

#endif

