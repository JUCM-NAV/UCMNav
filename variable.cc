/***********************************************************
 *
 * File:			variable.cc
 * Author:			Andrew Miga
 * Created:			September 2000
 *
 * Modification history:
 *
 ***********************************************************/

#include "variable.h"
#include "scenario.h"
#include "xml_generator.h"
#include "print_mgr.h"
#include <stdlib.h>
#include <ctype.h>

extern void ListBooleanVariables( Cltn<BooleanVariable *> *variables, bool subdialog = FALSE );
extern "C" int EvaluateExpression( const char *expression, int state );

#define INCREMENT 0
#define DECREMENT 1

int BooleanVariable::number_booleans = 0;
int BooleanVariable::max_id = -1;
Cltn<BooleanVariable *> BooleanVariable::boolean_variables;

BooleanVariable::BooleanVariable( const char *name, bool timer )
{
   boolean_name[VARIABLE_NAME] = 0;
   strncpy( boolean_name, name, VARIABLE_NAME );
   value = UNINITIALIZED;
   initialized = FALSE;
   reference_count = 0;
   boolean_id = number_booleans++;
   timer_variable = timer;
   BooleanVariable::OrderVariable( this );
}

BooleanVariable::BooleanVariable( int id, const char *name, int ref_count )
{
   boolean_name[VARIABLE_NAME] = 0;
   strncpy( boolean_name, name, VARIABLE_NAME );
   value = FALSE;
   boolean_id = id;
   reference_count = ref_count;
   timer_variable = FALSE;

   if( id > max_id ) {  // avoid reallocating the same identifier
      max_id = id;
      number_booleans = max_id+1;
   }
}

BooleanVariable::~BooleanVariable()
{
   boolean_variables.Delete( this );
}

void BooleanVariable::OrderVariable( BooleanVariable *bv )
{
   BooleanVariable *current_variable;

   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() ) {
      current_variable = boolean_variables.CurrentItem();
      if( strcmp( bv->boolean_name, current_variable->boolean_name ) < 0 ) {	 
	 boolean_variables.AddBefore( bv, current_variable );
	 return;
      }
   }
   
   boolean_variables.Add( bv );
}

void BooleanVariable::BooleanName( const char *new_name )
{
   strncpy( boolean_name, new_name, VARIABLE_NAME );
   boolean_variables.Delete( this ); // delete and then re-add this variable to the list
   BooleanVariable::OrderVariable( this );
}

void BooleanVariable::AddBooleanVariable( int id, const char *name, int ref_count )
{
   boolean_variables.Add( new BooleanVariable( id, name, ref_count ) );
}

int BooleanVariable::NameIndex( const char *new_name )
{
   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() ) {
      if( strequal( new_name, boolean_variables.CurrentItem()->boolean_name ) )
	 return( boolean_variables.CurrentItem()->BooleanId() );
   }

   return( NOT_CONTAINED );
}

BooleanVariable * BooleanVariable::ReferencedVariable( int id )
{
   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() ) {
      if( boolean_variables.CurrentItem()->boolean_id == id )
	 return( boolean_variables.CurrentItem() );
   }

   return NULL;
}

bool BooleanVariable::VariableValue( int index )
{
   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() ) {
      if( boolean_variables.CurrentItem()->BooleanId() == index )
	 return( boolean_variables.CurrentItem()->Value() );
   }

   return VARIABLE_DELETED;
}

void BooleanVariable::ScenarioInitialize( Cltn<VariableInit *> *initializations )
{
   VariableInit *cvi;

   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() )
      boolean_variables.CurrentItem()->Uninitialize();

   for( initializations->First(); !initializations->IsDone(); initializations->Next() ) {
      cvi = initializations->CurrentItem();
      cvi->Variable()->Value( cvi->Value() );
   }
}

char * BooleanVariable::NameToReference( const char *new_condition, char *old_condition )
{ // replace variable names with references, eventually do reference counting on variable objects
   char buffer[2000], name[25], ref[10], *bp, *np, *rp;
   const char *cp;

   if( old_condition ) { // decrement reference count of all variables in old_condition
      AdjustReferenceCounts( old_condition, DECREMENT );
      free( old_condition );
   }

   if( strequal( new_condition, "" ) )
      return( NULL );

   cp = new_condition;
   bp = buffer;

   while( *cp ) {
      if( strchr( "() +&!=~\n", *cp ) != NULL ) {
	 *bp++ = *cp++;
	 continue;
      } else if( strchr( "TF", *cp ) != NULL ) {
	 if( strchr( "() +&!=~\n", *(cp+1) ) != NULL ) {
	    *bp++ = *cp++;
	    continue;
	 }
      }

      np = name; // a variable name has been encountered
      while( strchr( "() +&!=~\n", *cp ) == NULL ) 
	 *np++ = *cp++; // store name characters in name buffer
      *np = 0; // null terminate name buffer
      sprintf( ref, "bv%d", NameIndex( name ) );
      rp = ref;
      while( *rp )
	 *bp++ = *rp++;
   }

   *bp = 0; // null terminate buffer
   AdjustReferenceCounts( buffer, INCREMENT ); // increment reference counts of variables in new condition
   return( strdup( buffer ) );
}

void BooleanVariable::AdjustReferenceCounts( const char *condition, bool adjust_type )
{
   int variables[200], i, current_id, num_variables = 0;
   char integer[5], *ip;
   bool id_found;

   while( *condition ) {
      if( *condition == 'b' ) { // a reference bv n was found
	 condition += 2; // skip over character v
	 ip = integer;
	 while( isdigit( *condition ) )
	    *ip++ = *condition++; // store integer characters in integer buffer
	 *ip = 0; // null terminate integer buffer
	 current_id = atoi( integer );
	 id_found = FALSE;
	 for( i = 0; i < num_variables; i++ ) {
	    if( variables[i] == current_id ) {
	       id_found = TRUE;
	       break;
	    }
	 }
	 if( !id_found )
	    variables[num_variables++] = current_id;
      }
      else
	 condition++;
   }

   if( adjust_type == INCREMENT ) {
      for( i = 0; i < num_variables; i++ )
	 ReferencedVariable( variables[i] )->IncrementReferenceCount();
   } else { // DECREMENT
      for( i = 0; i < num_variables; i++ )
	 ReferencedVariable( variables[i] )->DecrementReferenceCount(); 
   }
}

char * BooleanVariable::ReferenceToName( const char *condition )
{ // replace references to variables ( in form bvn ) with actual variable names
   char buffer[2000], integer[5], *bp, *ip; 
   const char *vname, *cp;

   if ( condition == NULL ) {
      // Added by Daniel Amyot, August 15, 2004
      // Fixes a printing bug when a start point has an empty logical condition
      return ( NULL ); 
   }  

   cp = condition;
   bp = buffer;

   while( *cp ) {
      if( *cp == 'b' ) { // a reference bv n was found
     	 cp += 2; // skip over character v
	 ip = integer;
	 while( isdigit( *cp ) )
	    *ip++ = *cp++; // store integer characters in integer buffer
	 *ip = 0; // null terminate integer buffer
	 vname = ReferencedVariable( atoi( integer ) )->BooleanName();
	 while( *vname )
	    *bp++ = *vname++; // copy actual name into buffer
      }
      else
	 *bp++ = *cp++;
   }

   *bp = 0; // null terminate buffer
   return( strdup( buffer ) );
}

bool BooleanVariable::UninitializedVariableReferenced( const char *condition )
{
   char integer[5], *ip; 
   const char *cp;

   cp = condition;

   while( *cp ) {
      if( *cp == 'b' ) { // a reference bv n was found
     	 cp += 2; // skip over character v
	 ip = integer;
	 while( isdigit( *cp ) )
	    *ip++ = *cp++; // store integer characters in integer buffer
	 *ip = 0; // null terminate integer buffer
	 if( ReferencedVariable( atoi( integer ) )->IsNotInitialized() )
	    return( TRUE );
      }
      else
	 cp++;
   }

   return( FALSE );
}

BooleanVariable * BooleanVariable::SelectedVariable( int index )
{
   int count = 0;

   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() ) {
      if( boolean_variables.CurrentItem()->referenced == FALSE ) {
	 if( index == ++count )
	    return( boolean_variables.CurrentItem() );
      }
   }

   std::cout << "Warning: problem in BooleanVariable::SelectedVariable... should not get here!";
   return( NULL );  // DA: Added August 2004
}

void BooleanVariable::DereferenceVariableList()
{
   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() )
      boolean_variables.CurrentItem()->Dereference();
}

void BooleanVariable::ViewBooleanList()
{
   ListBooleanVariables( &boolean_variables );
}

void BooleanVariable::SaveXML( FILE *fp )
{
   BooleanVariable *bv;
   char buffer[300];
   
   if( boolean_variables.Size() == 0 ) return;

   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() ) {
      bv = boolean_variables.CurrentItem();
      sprintf( buffer, "boolean-variable boolvar-id=\"bv%d\" name=\"%s\" ref-count=\"%d\" /",
	       bv->BooleanId(), PrintDescription( bv->BooleanName() ), bv->ReferenceCount() );
      PrintXML( fp, buffer );
   }
}

void BooleanVariable::GeneratePostScript( FILE *ps_file )
{
   if( boolean_variables.Size() == 0 ) return;

   PrintManager::PrintHeading( "Global Boolean Control Variables" );
   fprintf( ps_file, "[/Title(Global Boolean Control Variables) /OUT pdfmark\n" );

   fprintf( ps_file, "%d rom 1 IN ID", PrintManager::text_font_size );
   for( boolean_variables.First(); !boolean_variables.IsDone(); boolean_variables.Next() )
      fprintf( ps_file, "(%s ) P\n", PrintManager::PrintPostScriptText( boolean_variables.CurrentItem()->BooleanName() ));
   fprintf( ps_file, "OD L\n" );
}

void BooleanVariable::ClearBooleanList()
{
   while( !boolean_variables.is_empty() )
      delete boolean_variables.Detach();

   number_booleans = 0;
   max_id = -1;
}

void PathVariableList::SaveXML( FILE *fp )
{
   if( !BooleanVariable::BooleanVariablesExist() ) return;

   PrintNewXMLBlock( fp, "path-variable-list" );
   BooleanVariable::SaveXML( fp );
   PrintEndXMLBlock( fp, "path-variable-list" );
}

void PathVariableList::GeneratePostScript( FILE *ps_file )
{
   BooleanVariable::GeneratePostScript( ps_file );
}

void PathVariableList::ClearVariableList()
{
   BooleanVariable::ClearBooleanList();
}

bool PathVariableList::EvaluateLogicalExpression( const char *expression )
{
   if( expression == NULL )
      return( FALSE );
   else if( BooleanVariable::UninitializedVariableReferenced( expression ) )
      return FALSE;
   else
      return( EvaluateExpression( expression, TRUE ) );
}

extern "C"
int VariableValue( const char *index )
{
   return( BooleanVariable::VariableValue( atoi( index+2 ) ) );
}

extern "C"
int VariableExists( const char *name )
{
   return( BooleanVariable::NameIndex( name ) != NOT_CONTAINED );
}
