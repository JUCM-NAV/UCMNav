/***********************************************************
 *
 * File:			responsibility.cc
 * Author:			Andrew Miga
 * Created:			June 1996
 *	
 * Modification history:	April 1999
 *				Separated from responsibility
 *				references
 *
 ***********************************************************/

#include "responsibility.h"
#include "resp_mgr.h"
#include "empty.h"
#include "hypergraph.h"
#include "resp_figure.h"
#include "data.h"
#include "devices.h"
#include "print_mgr.h"
#include "variable.h"
#include <iostream>

extern void UpdateResponsibilityList();
extern void InstallConditions( ConditionManager &conditions );

int Responsibility::number_responsibilities = 0;

VariableOperation::VariableOperation( BooleanVariable *bvar, int val, char *lexp, bool translate )
{ 
   variable = bvar;
   if( translate )
      variable->IncrementReferenceCount();
   value = val;
   logical_expression = NULL;
   if( translate && lexp != NULL )
      LogicalExpression( lexp );
   else
      logical_expression = lexp;
}

VariableOperation::~VariableOperation()
{
   variable->DecrementReferenceCount();
   if( logical_expression != NULL ) free( logical_expression );
}

void VariableOperation::SetVariable()
{
   if( value != EVALUATED_EXPRESSION )
      variable->Value( value );
   else {
      if( logical_expression != NULL ) {
	 if( !strequal( logical_expression, "" ) )
	    variable->Value( PathVariableList::EvaluateLogicalExpression( logical_expression ) );
      }   
   }
}

char * VariableOperation::LogicalExpression()
{
   return( (logical_expression != NULL) ? BooleanVariable::ReferenceToName( logical_expression ) : NULL );
}

void VariableOperation::LogicalExpression( const char *new_expression )
{
   logical_expression = BooleanVariable::NameToReference( new_expression, logical_expression );
}

void VariableOperation::SaveVariableOperations( Cltn<VariableOperation *> *operations, FILE *fp )
{
   VariableOperation *vo;
   char buffer[200];

   if( operations->Size() == 0 ) return;

   PrintNewXMLBlock( fp, "variable-operation-list" );

   for( operations->First(); !operations->IsDone(); operations->Next() ) {
      vo = operations->CurrentItem();
      if( vo->value != EVALUATED_EXPRESSION )
	 sprintf( buffer, "variable-operation variable-id=\"bv%d\" value=\"%s\" /", vo->variable->BooleanId(), ((vo->value == TRUE) ? "T" : "F" ) );
      else
	 sprintf( buffer, "variable-operation variable-id=\"bv%d\" value=\"%s\" /", vo->variable->BooleanId(), PrintDescription( vo->logical_expression ));
      PrintXML( fp, buffer );
   }

   PrintEndXMLBlock( fp, "variable-operation-list" );
}

void VariableOperation::GeneratePostScriptDescription( Cltn<VariableOperation *> *operations, FILE *ps_file )
{
   VariableOperation *vo;
   char *expanded_expression;

   if( operations->Size() == 0 ) return;
   fprintf( ps_file, "%d rom\n1 IN ID (Variable Operations ) P OD\n2 IN ID \n", PrintManager::text_font_size );

   for( operations->First(); !operations->IsDone(); operations->Next() ) {
      vo = operations->CurrentItem();
      if( vo->value != EVALUATED_EXPRESSION )
	 fprintf( ps_file, "%d bol (%s -> %s ) S\n", PrintManager::text_font_size, vo->variable->BooleanName(), ((vo->value == TRUE) ? "T" : "F" ) );
      else {
	 if( (expanded_expression = vo->LogicalExpression()) != NULL ) {
	    fprintf( ps_file, "%d bol (%s -> EVAL(%s) ) P\n", PrintManager::text_font_size, vo->variable->BooleanName(),
		     PrintManager::PrintPostScriptText( expanded_expression ) );
	    free( expanded_expression );
	 }
      }
   }
   fprintf( ps_file, "OD\n" );
}

Responsibility::Responsibility()
{
   data = new Cltn<DataReference *>;
   service_requests = new Cltn<ServiceRequest *>;
   variable_operations = new Cltn<VariableOperation *>;
   responsibility_number = number_responsibilities++;
   reference_count = 0;
   name[0] = 0;
   description = NULL;
   execution_sequence = NULL;
   dynamic_arrow = NULL;
   annotations_complete = TRUE;
   load_number = NA;
   condition_manager.ConditionType( RESP );
}

Responsibility::Responsibility( int resp_id, const char *resp_name, const char *es, const char *desc )
{
   ResponsibilityManager *rm = ResponsibilityManager::Instance();
   
   data = new Cltn<DataReference *>;
   service_requests = new Cltn<ServiceRequest *>;
   variable_operations = new Cltn<VariableOperation *>;

   dynamic_arrow = NULL;
   execution_sequence = NULL;
   description = NULL;
   name[0] = 0;
   name[31] = 0;

   load_number = resp_id;
   
   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   responsibility_number = resp_id;
   if (number_responsibilities <= responsibility_number) number_responsibilities = responsibility_number+1;  
   //End of the modification by Bo Jiang, April, 2005

   strncpy( name, rm->VerifyUniqueResponsibilityName( resp_name ), 32 );
   if( es ) ExecutionSequence( es );
   if( desc ) Description( desc );
//   responsibility_number = number_responsibilities++;     //Modified by Bo Jiang. April,2005, fixed that ID changes when load a .ucm file
   reference_count = 0;
   annotations_complete = TRUE;
   rm->AddResponsibility( this );
   condition_manager.ConditionType( RESP );
}

Responsibility::~Responsibility()
{
   if( dynamic_arrow ) delete dynamic_arrow;
   if( description ) free( description );
   if( execution_sequence ) free( execution_sequence );

   while( !data->is_empty() )
      delete data->Detach();
   delete data;

   while( !service_requests->is_empty() )
      delete service_requests->Detach();
   delete service_requests;

   while( !variable_operations->is_empty() )
      delete variable_operations->Detach();
   delete variable_operations;


   ResponsibilityManager::Instance()->RemoveResponsibility( this );
}

void Responsibility::DecrementCount()
{
   if( --reference_count == 0 )
      delete this;
}

void Responsibility::AddDataReference( int data_id, int access_id )
{
   data->Add( new DataReference( data_id, access_id ) );
}

void Responsibility::AddServiceRequest( int device_id, const char *amount )
{
   service_requests->Add( new ServiceRequest( device_id, amount ) );
}

void Responsibility::EditConditions()
{
   InstallConditions( condition_manager );
}

bool Responsibility::ReferencesData( int data_id )
{
   for( data->First(); !data->IsDone(); data->Next() ) {  // return immediately if selected data 
      if( data->CurrentItem()->Data() == data_id )        // store is already referenced
	 return( TRUE );
   }

   return( FALSE );
}

bool Responsibility::VerifyAnnotations()
{
   annotations_complete = ( service_requests->Size() != 0 ) ? TRUE : FALSE;
   return( annotations_complete );
}

void Responsibility::AddVariableOperation( int boolean_id, int value, const char *expression )
{
   variable_operations->Add( new VariableOperation( BooleanVariable::ReferencedVariable( boolean_id ), value, (expression ? strdup(expression) : NULL )));
}

void Responsibility::ExecuteVariableOperations()
{
   if( variable_operations == NULL ) return;
   for( variable_operations->First(); !variable_operations->IsDone(); variable_operations->Next() )
      variable_operations->CurrentItem()->SetVariable();
}

void Responsibility::SaveXML( FILE *fp )
{
   DataReference *dr;
   char buffer [2000];

   sprintf( buffer, "<responsibility resp-id =\"r%d\" resp-name=\"%s\"", responsibility_number, PrintDescription( name ) );
   PrintXMLText( fp, buffer );

   if( PrintManager::TextNonempty( execution_sequence ) )
      fprintf( fp,  " exec-sequence=\"%s\"", PrintDescription( execution_sequence ) );

   if( PrintManager::TextNonempty( description ) )
      fprintf( fp, " description=\"%s\"", PrintDescription( description ) );

   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );

   // save data-store-spec

   if( data->Size() != 0 ) {
   
      DataStoreDirectory *dsd = DataStoreDirectory::Instance();
      PrintNewXMLBlock( fp, "data-store-spec" );

      for( data->First(); !data->IsDone(); data->Next() ) {
	 dr = data->CurrentItem();
	 if( dsd->IsReferenceValid( dr ) ) {
	    sprintf( buffer, "data-store-access data-store-id=\"ds%d\" access-mode-id=\"am%d\" /", dr->Data(), dr->Access() );
	    PrintXML( fp, buffer );
	 }
      }

      PrintEndXMLBlock( fp, "data-store-spec" );
   }

   ServiceRequest::SaveRequests( service_requests, fp );
   VariableOperation::SaveVariableOperations( variable_operations, fp );
   condition_manager.SaveXML( fp, "precondition", "postcondition" );
   
   // save dynamic-resp
   if( dynamic_arrow ) {

      sprintf( buffer, "dynamic-resp type=\"%s\" direction=\"%s\" sourcepool=\"%s\" arrow-length=\"%f\" /", dynamic_arrow->ActionDescription(),
	       dynamic_arrow->Direction(), PrintDescription( dynamic_arrow->SourcePool() ), dynamic_arrow->GetLength() );
      PrintXML( fp, buffer );
   }
   // save raise, handle

   PrintEndXMLBlock( fp, "responsibility" );
}

void Responsibility::SaveCSMXML( FILE *fp )
{
   char buffer [2000];

   sprintf( buffer, "<Step id =\"r%d\" name=\"%s\"", responsibility_number, PrintDescription( name ) );
   PrintXMLText( fp, buffer );

   if( PrintManager::TextNonempty( execution_sequence ) )
      fprintf( fp,  " exec-sequence=\"%s\"", PrintDescription( execution_sequence ) );

   if( PrintManager::TextNonempty( description ) )
      fprintf( fp, " description=\"%s\"", PrintDescription( description ) );

   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );
/*
   // save data-store-spec

   if( data->Size() != 0 ) {
   
      DataStoreDirectory *dsd = DataStoreDirectory::Instance();
      PrintNewXMLBlock( fp, "data-store-spec" );

      for( data->First(); !data->IsDone(); data->Next() ) {
	 dr = data->CurrentItem();
	 if( dsd->IsReferenceValid( dr ) ) {
	    sprintf( buffer, "data-store-access data-store-id=\"ds%d\" access-mode-id=\"am%d\" /", dr->Data(), dr->Access() );
	    PrintXML( fp, buffer );
	 }
      }

      PrintEndXMLBlock( fp, "data-store-spec" );
   }
*/
   ServiceRequest::SaveRequests( service_requests, fp );
  // VariableOperation::SaveVariableOperations( variable_operations, fp );
  // condition_manager.SaveXML( fp, "precondition", "postcondition" );
   /*
   // save dynamic-resp
   if( dynamic_arrow ) {

      sprintf( buffer, "dynamic-resp type=\"%s\" direction=\"%s\" sourcepool=\"%s\" arrow-length=\"%f\" /", dynamic_arrow->ActionDescription(),
	       dynamic_arrow->Direction(), PrintDescription( dynamic_arrow->SourcePool() ), dynamic_arrow->GetLength() );
      PrintXML( fp, buffer );
   }
   // save raise, handle
*/
   PrintEndXMLBlock( fp, "Step" );
}

void Responsibility::GeneratePostScriptDescription( FILE *ps_file )
{
   DataReference *dr;
   ServiceRequest *sr;

   DataStoreDirectory *dsd = DataStoreDirectory::Instance();
   DeviceDirectory *dd = DeviceDirectory::Instance();

   fprintf( ps_file, "%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( name ) );

   if( PrintManager::TextNonempty( description ) ) {
      fprintf( ps_file, "%d rom\n1 IN ID (Description ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID (%s ) P OD\n", PrintManager::PrintPostScriptText( description ));
   }
   
   if( PrintManager::include_scenarios )
      VariableOperation::GeneratePostScriptDescription( variable_operations, ps_file );

   if( PrintManager::TextNonempty( execution_sequence ) ) {
      fprintf( ps_file, "%d rom\n1 IN ID (Execution Sequence ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID\n" );
      PrintManager::PrintSequence( execution_sequence );
      fprintf( ps_file, "OD\n" );
   }

   if( PrintManager::include_performance ) {
      if( service_requests->Size() != 0 ) {
	 fprintf( ps_file, "%d rom\n1 IN ID (Service Requests ) P OD\n2 IN ID \n", PrintManager::text_font_size );
	 for( service_requests->First(); !service_requests->IsDone(); service_requests->Next() ) {
	    sr = service_requests->CurrentItem();
	    if( dd->IsReferenceValid( sr->DeviceId() ) )
	       fprintf( ps_file, "%d bol (%s - ) S\n%d rom ( - %s ) P\n", PrintManager::text_font_size, dd->DeviceName( sr->DeviceId() ),
			PrintManager::text_font_size, sr->Amount() );
	 }
	 fprintf( ps_file, "OD\n" );
      }

      if( data->Size() != 0 ) {
	 fprintf( ps_file, "%d rom\n1 IN ID (Data References ) P OD\n2 IN ID \n", PrintManager::text_font_size );
	 for( data->First(); !data->IsDone(); data->Next() ) {
	    dr = data->CurrentItem();
	    if( dsd->IsReferenceValid( dr ) ) {
	       fprintf( ps_file, "%d bol (%s - ) S\n%d rom ( - %s ) P\n", PrintManager::text_font_size, dsd->Item( DATA_STORES, dr->Data() ),
			PrintManager::text_font_size,  dsd->Item( ACCESS_MODES, dr->Access() ) );
	    }
	 }
	 fprintf( ps_file, "OD\n" );
      }
   }
  
   fprintf( ps_file, "L\n" );
}

void Responsibility::AddDynamicArrow( const char *type, const char *direction, const char *sourcepool, float length )
{
   dynamic_arrow = new DynamicArrow( type, direction, sourcepool, length );
}

void Responsibility::Name( const char *new_name )
{
   strncpy( name, new_name, 32 );
}

void Responsibility::Description( const char *new_description )
{
   if( new_description != NULL ) {
      if( description != NULL ) {
	 free( description );
	 description = NULL;
      }
      if( strlen( new_description ) > 0 )
	 description = strdup( new_description );
   }
}

void Responsibility::ExecutionSequence( const char *new_es )
{
   if( execution_sequence )
      free( execution_sequence );
   execution_sequence = strdup( new_es );
}

bool Responsibility::HasDynarrow()
{
   return( (dynamic_arrow != NULL) ? TRUE : FALSE );
}
