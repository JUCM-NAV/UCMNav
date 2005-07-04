/***********************************************************
 *
 * File:			timestamp.cc
 * Author:			Andrew Miga
 * Created:			October 1997
 *
 * Modification history:
 *
 ***********************************************************/

#include "timestamp.h"
#include "ts_figure.h"
#include "response_time.h"
#include "empty.h"
#include "hypergraph.h"   
#include <string.h>

extern void EditTimestampDescription( TimestampFigure *new_timestamp );
extern Timestamp *last_timestamp; // global pointer to last timestamp selected
extern Timestamp *previous_timestamp; // global pointer to previous timestamp selected

Timestamp::Timestamp() : Hyperedge()
{
   name[0] = 0;
   name[19] = 0;
   reference = PREVIOUS;

   figure = new TimestampFigure( this, TRUE );
}

Timestamp::Timestamp( int tsid, const char *ts_name, float x, float y, const char *desc,
		      int orientation, timestamp_reference ref ) : Hyperedge()
{
   load_number = tsid;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file
   hyperedge_number = tsid;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;    
   //End of the modification by Bo Jiang, April, 2005

   figure = new TimestampFigure( this, FALSE );
   figure->SetPosition( x, y );
   name[19] = 0;
   strncpy( name, ts_name, 19 );
   if( desc != NULL ) unique_desc = strdup( desc );
   ((TimestampFigure *)figure)->Direction( orientation );
   reference = ref;
}

Timestamp::~Timestamp()
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();
   rtm->DeleteTimestamp( this );
   if( last_timestamp == this ) last_timestamp = NULL; // check if this is the last selected timestamp
   if( previous_timestamp == this ) previous_timestamp = NULL; // check if this is the previous selected timestamp
}

bool Timestamp::Perform( transformation trans, execution_flag execute )
{
   switch( trans ) {

   case EDIT_TIMESTAMP:
      return( EditTimestamp( execute ) );
      break;
   case DELETE_TIMESTAMP:
      return( DeleteTimestamp( execute ) );
   case VIEW_RESPONSE_TIMES:
      return( ViewResponseTimes( execute ) );
      break;
   }
   return FALSE;
}

bool Timestamp::PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute )
{

   switch( trans ) {
      
   case CREATE_TIME_RESPONSE:
      return( CreateResponseTime( edge, execute ) );
      break;
   case EDIT_TIME_RESPONSE:
      return( EditResponseTime( edge, execute ) );
      break;
   case DELETE_TIME_RESPONSE:
      return( DeleteResponseTime( edge, execute ) );
      break;
   }
   return FALSE;
}

bool Timestamp::EditTimestamp( execution_flag execute )
{

   if( execute )
   {
      EditTimestampDescription( (TimestampFigure *)figure );
      return( TRUE );
   }
   else
      return( TRUE );
}

bool Timestamp::ViewResponseTimes( execution_flag execute )
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();

   if( execute )
      rtm->ViewResponseTimes( this );
   else
      return( rtm->ResponseTimesExist( this ) );
   return FALSE;
}

bool Timestamp::DeleteTimestamp( execution_flag execute )
{

   if ( execute )
      this->DeleteSimpleEdge();

   return ( TRUE );
}

bool Timestamp::CreateResponseTime( Hyperedge *ts, flag execute )
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();
   
   if( execute ) 
      rtm->CreateResponseTime( this, (Timestamp *)ts );
   else {

      if( ts->EdgeType() != TIMESTAMP )
	 return( INVALID );

      if( rtm->NewResponseTimeValid( this, (Timestamp *)ts ) )
	 return( AVAILABLE );
      else
	 return( UNAVAILABLE );
   }
   return FALSE;
}

bool Timestamp::EditResponseTime( Hyperedge *ts, flag execute )
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();
   static ResponseTime *rt;

   if( execute ) 
      rtm->EditResponseTime( rt );
   else {

      if( ts->EdgeType() != TIMESTAMP )
	 return( INVALID );

      if( (rt = rtm->ResponseTimeExists( this, (Timestamp *)ts )) != NULL )
	 return( AVAILABLE );
      else
	 return( UNAVAILABLE );      
   }
   return FALSE;
}

bool Timestamp::DeleteResponseTime( Hyperedge *ts, flag execute )
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();
   static ResponseTime *rt;

   if( execute ) 
      rtm->DeleteResponseTime( rt );
   else {

      if( ts->EdgeType() != TIMESTAMP )
	 return( INVALID );

      if( (rt = rtm->ResponseTimeExists( this, (Timestamp *)ts )) != NULL )
	 return( AVAILABLE );
      else
	 return( UNAVAILABLE );      
   }
   return FALSE;
}

void Timestamp::Name( const char *new_name )
{
   if ( new_name )
      strncpy( name, new_name, 19 );
}

void Timestamp::SaveXMLDetails( FILE *fp )
{
   char buffer[100];
   int degrees[4] = { 90, 270, 180, 0 };
   
   sprintf( buffer, "timestamp-point orientation=\"%d\" reference=\"%s\" /",
	    degrees[((TimestampFigure *)figure)->Direction()], ( reference == PREVIOUS ? "previous" : "next" ) );
   PrintXML( fp, buffer );
}
void Timestamp::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
