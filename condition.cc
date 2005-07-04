/***********************************************************
 *
 * File:			condition.cc
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:
 *
 ***********************************************************/

#include "condition.h"
#include "defines.h"
#include "xml_generator.h"
#include "print_mgr.h"
#include <string.h>
#include <stdlib.h>

char ConditionManager::ConditionNames[][37] = {

   "Preconditions for Path",
   "Triggering events for Path",
   "Preconditions for Stub",
   "Postconditions for Stub",
   "Postconditions for Path",
   "Resulting Events for Path",
   "Preconditions for Path Point",
   "Postconditions for Path Point",
   "Preconditions for Timer",
   "Triggering events for Timer",
   "Preconditions for Waiting place",
   "Triggering events for Waiting place",
   "Preconditions for Responsibility ",
   "Postconditions for Responsibility " };

char ConditionManager::ConditionTypes[][18] = {

   "Preconditions",
   "Triggering events",
   "Preconditions",
   "Postconditions",
   "Postconditions",
   "Resulting Events",
   "Preconditions",
   "Postconditions",
   "Preconditions",
   "Triggering events",
   "Preconditions",
   "Triggering events",
   "Preconditions",
   "Postconditions" };

char ConditionManager::Prompts[][38] = {

   "Enter Path Precondition",
   "Enter Path Triggering event",
   "Enter Stub Precondition",
   "Enter Stub Postcondition",
   "Enter Path Postcondition",
   "Enter Path Resulting event",
   "Enter Path Point Precondition",
   "Enter Path Point Postcondition",
   "Enter Timer Precondition",
   "Enter Timer Triggering event",
   "Enter Waiting place Precondition",
   "Enter Waiting place Triggering event",
   "Enter Responsibility Precondition",
   "Enter Responsibility Postcondition" };

ConditionManager::ConditionManager() {}

ConditionManager::~ConditionManager()
{
   for( int i = 0; i < 2; i++ )
      for( conditions[i].First(); !conditions[i].IsDone(); conditions[i].Next() )
	 free( conditions[i].CurrentItem() );  
}

bool ConditionManager::ConditionsEmpty()
{
   if( (conditions[0].Size() != 0) || (conditions[1].Size() != 0) )
      return( FALSE );
   else
      return( TRUE );
}

const char * ConditionManager::ConditionName( int condition_list )
{
   return( ConditionNames[(2*ctype)+condition_list-1] );
}
   
Cltn<char *> * ConditionManager::ConditionList( int condition_list )
{
   return( &conditions[condition_list-1] );
}

const char * ConditionManager::Prompt( condition_type ctype, int condition_list )
{
   return( Prompts[(2*ctype)+condition_list] );
}

void ConditionManager::SaveXML( FILE *fp, const char *pretype, const char *posttype )
{
   char buffer[400], *condition_type[2];
   int i, j;

   //  give correct names to event/conditions

   condition_type[0] = (char *)((strstr( pretype, "event" ) != NULL) ? "event" : "condition" );
   condition_type[1] = (char *)((strstr( posttype, "event" ) != NULL) ? "event" : "condition" );

   bool reverse = FALSE;

   if( ctype == PATH_START || ctype == PATH_END )
      reverse = TRUE;
   
   for( j = 0; j <= 1; j++ ) {

      i = ( reverse ) ? (1-j) : j;

      if( conditions[i].Size() != 0 ) {
	 sprintf( buffer, "%s-list composition=\"%s\"", ((i == 0) ? pretype : posttype), "AND" );
	 PrintNewXMLBlock( fp, buffer );
	 SaveConditionList( fp, &conditions[i], condition_type[i] );
	 sprintf( buffer, "%s-list", ((i == 0) ? pretype : posttype) );
	 PrintEndXMLBlock( fp, buffer );
      }
   }
}

void ConditionManager::GeneratePostScriptDescription( FILE *ps_file )
{
   int i, j;
   bool reverse = FALSE;

   if( ctype == PATH_START || ctype == PATH_END )
      reverse = TRUE;

   for( j = 0; j <= 1; j++ ) {

      i = ( reverse ) ? (1-j) : j;

      if( conditions[i].Size() != 0 ) {
	 fprintf( ps_file, "1 IN ID %d rom (%s ) P OD\n", PrintManager::text_font_size, ConditionTypes[(2*ctype)+i] );
	 GeneratePostscriptConditionList( ps_file, &conditions[i] );
      }
   }
}

void ConditionManager::SaveConditionList( FILE *fp, Cltn<char *> *list, char *condition_name )
{
   char buffer[400], name[100], description[1000], *cp;
   int len, index;
   
   for( list->First(); !list->IsDone(); list->Next() ) {
	    
      cp = list->CurrentItem();
	    
      // strip out spaces from condition label and replace with dashes
      *description = 0;
      *name = 0;
      index = 0;
	    
      while( (*cp != 0) && (*cp != '&') ) {

	 if( *cp != ' ' )
	    name[index] = *cp;
	 else
	    name[index] = '-';

	 index++;
	 cp++;
      }
      name[index] = 0;

      if( *cp == '&' )
	 cp+=2;  // skip over other ampersand

      sprintf( buffer, "<%s name=\"%s\"", condition_name, PrintDescription( name ) );
      PrintXMLText( fp, buffer );
      
      if( *cp != 0 ) {
	       
	 strcpy( description, cp );
	 len = strlen( description );
	 if( description[len-1] == NEWLINE )
	    description[len-1] = 0;
	 fprintf( fp, " description=\"%s\" />\n", PrintDescription( description ) );
      }
      else
	 fprintf( fp, " />\n" );
   }
}

void ConditionManager::GeneratePostscriptConditionList( FILE *ps_file, Cltn<char *> *list )
{
   char name[100], description[1000], *cp;
   int len, index;
   
   for( list->First(); !list->IsDone(); list->Next() ) {
	    
      cp = list->CurrentItem();
	    
      // strip out spaces from condition label and replace with dashes
      *description = 0;
      *name = 0;
      index = 0;
	    
      while( (*cp != 0) && (*cp != '&') ) {

	 if( *cp != ' ' )
	    name[index] = *cp;
	 else
	    name[index] = '-';

	 index++;
	 cp++;
      }
      name[index] = 0;

      if( *cp == '&' )
	 cp+=2;  // skip over other ampersand

      fprintf( ps_file, "2 IN ID %d bol (%s ) P OD\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( name ));
      
      if( *cp != 0 ) {	       
	 strcpy( description, cp );
	 len = strlen( description );
	 if( description[len-1] == NEWLINE )
	    description[len-1] = 0;
	 fprintf( ps_file, "3 IN ID %d rom\n", PrintManager::text_font_size );
	 PrintManager::PrintDescription( description );
	 fprintf( ps_file, "OD\n" );
      }
   }
}

void ConditionManager::AddConditions( ConditionManager &cm )
{
   for( int i = 0; i < 2; i++ ) {
      for( cm.conditions[i].First(); !cm.conditions[i].IsDone(); cm.conditions[i].Next() )
	 conditions[i].Add( strdup( cm.conditions[i].CurrentItem() ) );
   }
}

void ConditionManager::AddCondition( etype type, char *condition )
{
   int index;
   
   switch( type ) {
   case TRIGGERING:
      index = 1; break;
   case RESULTING:
      index = 1; break;
   case PRECONDITION:
      index = 0; break;
   case POSTCONDITION:
      index = (( ctype == PATH_END ) ? 0 : 1 ); break;
   }
   
   conditions[index].Add( condition );
}

