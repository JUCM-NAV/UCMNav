/***********************************************************
 *
 * File:			response_time.cc
 * Author:			Andrew Miga
 * Created:			October 1997
 *
 * Modification history:
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "response_time.h"
#include "ts_figure.h"
#include "print_mgr.h"

static ResponseTimeManager * SoleTRM; // class variable for sole instance of directory object


extern void EditResponseTimeDescription( ResponseTime *rt );
extern void EditNewResponseTime( TimestampFigure *ts1, TimestampFigure *ts2 );
extern void ShowResponseTimes();
extern void ShowTimestampRT( Timestamp *ts );
extern void EditResponseTimes( bool allow );
extern FD_ViewResponseTimes *pfdvrt;
extern FD_ViewTimestampRT *pfdtsr;
extern ResponseTime *crt, *old_crt;

ResponseTime * ResponseTimeManager::timestamp_responses[2][10];

ResponseTime::ResponseTime( Timestamp *t1, Timestamp *t2,
			    const char *new_name, int time, int pct )
{
   ts1 = t1;
   ts2 = t2;
   response_time = time;
   percentage = pct;
   name[0] = 0;
   name[19] = 0;
   strncpy( name, new_name, 19 );
}

Timestamp * ResponseTime::Timestamps( int index )
{
   if( index == 1 )
      return( ts1 );
   else
      return( ts2 );
}

void ResponseTime::Highlight()
{
   ((TimestampFigure *)ts1->GetFigure())->Highlight();
   ((TimestampFigure *)ts2->GetFigure())->Highlight();
}

void ResponseTime::Unhighlight()
{
   ((TimestampFigure *)ts1->GetFigure())->Unhighlight();
   ((TimestampFigure *)ts2->GetFigure())->Unhighlight();
}

ResponseTimeManager * ResponseTimeManager::Instance()
{
   if( SoleTRM == NULL )
      SoleTRM = new ResponseTimeManager();

   return( SoleTRM );
}

void ResponseTimeManager::CreateResponseTime( Timestamp *t1, Timestamp *t2 )
{
   
#if 0 // disable temporarily as verification cannot handle situations where timestamps are in separate maps or where loops exist
   Timestamp *tf, *ts;
   // need to add verification of timestamp ordering
   if( OrderCorrect( t1, t2 ) ) {
      tf = t1;
      ts = t2;
   }
   else {
      tf = t2;
      ts = t1;
   }
#endif

   EditNewResponseTime( (TimestampFigure *)t1->GetFigure(), (TimestampFigure *)t2->GetFigure());
}

void ResponseTimeManager::EditResponseTime( ResponseTime *rt )
{
   EditResponseTimeDescription( rt );
}

ResponseTime * ResponseTimeManager::ResponseTimeExists( Timestamp *t1, Timestamp *t2 )
{
   ResponseTime *rt;

   for( response_times.First(); !response_times.IsDone(); response_times.Next() ) {
      rt = response_times.CurrentItem();
      if( ((t1 == rt->ts1) || (t1 == rt->ts2)) && ((t2 == rt->ts1) || (t2 == rt->ts2)) )
	 return( rt );
   }

   return( NULL );  // if no time response requirement exists
   
}

bool ResponseTimeManager::ResponseTimesExist( Timestamp *ts )
{
   ResponseTime *rt;

   for( response_times.First(); !response_times.IsDone(); response_times.Next() ) {
      rt = response_times.CurrentItem();
      if( (ts == rt->ts1) || (ts == rt->ts2) )
	 return( TRUE );
   }

   return( FALSE );  // if no time respose requirement exists

}

bool ResponseTimeManager::NewResponseTimeValid( Timestamp *t1, Timestamp *t2 )
{
   ResponseTime *rt;

   // determine if a relationship already exists
   for( response_times.First(); !response_times.IsDone(); response_times.Next() ) {
      rt = response_times.CurrentItem();
      if( ((t1 == rt->ts1) || (t1 == rt->ts2)) && ((t2 == rt->ts1) || (t2 == rt->ts2)) )
	 return( FALSE );
   }

#if 0 // disable temporarily as verification cannot handle situations where timestamps are in separate maps or where loops exist
   // determine if the two timestamps can be reached from each other by some path
   if( OrderCorrect( t1, t2 ) )
      return( TRUE );
   else if ( OrderCorrect( t2, t1 ) )
      return( TRUE );
   else
      return( FALSE );
#endif

   return( TRUE );
}

void ResponseTimeManager::ViewAllResponseTimes()
{
   if( response_times.Size() == 0 )
      fl_show_message( " No Response Times Exist ", "\nThis design does not contain any response time requirements. ", "" );
   else
      ShowResponseTimes();  // cause browser dialog to be shown
}

void ResponseTimeManager::ListResponseTimes()
{
   char buffer[50];
   ResponseTime *rt;

   fl_clear_browser( pfdvrt->ResponseTimesBrowser );

   for( response_times.First(); !response_times.IsDone(); response_times.Next() ) {
      rt = response_times.CurrentItem();
      if( strlen( rt->Name()) < 8 ) {
	 sprintf( buffer, "%s\t\t\t%s\t%s\t    %d\t\t%d", rt->Name(), rt->Timestamps(1)->Name(),
		  rt->Timestamps(2)->Name(), rt->response_time, rt->percentage );
      }
      else {
	 sprintf( buffer, "%s\t\t%s\t%s\t    %d\t\t%d", rt->Name(), rt->Timestamps(1)->Name(),
		  rt->Timestamps(2)->Name(), rt->response_time, rt->percentage );
      }
	 
      fl_addto_browser( pfdvrt->ResponseTimesBrowser, buffer );
   }
}

void ResponseTimeManager::DeleteTimestamp( Timestamp *ts )
{
   ResponseTime *rt;

   for( response_times.First(); !response_times.IsDone(); response_times.Next() ) {
      rt = response_times.CurrentItem();
      if( (ts == rt->ts1) || (ts == rt->ts2) ) {
	 response_times.RemoveCurrentItem();
	 delete rt;
	 if( rt == crt ) crt = NULL;
	 if( rt == old_crt ) old_crt = NULL;
      }
   }

   if(  response_times.Size() == 0 )
      EditResponseTimes( FALSE );
}

void ResponseTimeManager::AddResponseTime( ResponseTime *rt )
{
   if(  response_times.Size() == 0 )
      EditResponseTimes( TRUE );
   response_times.Add( rt );
}

void ResponseTimeManager::AddNewResponseTime( Timestamp *tf, Timestamp *ts, const char *nm, int time, int pct )
{
   if(  response_times.Size() == 0 )
      EditResponseTimes( TRUE );
   response_times.Add( new ResponseTime( tf, ts, nm, time, pct ) );
}

void ResponseTimeManager::DeleteResponseTime( ResponseTime *rt )
{
   response_times.Delete( rt );
   delete rt;
   if( rt == crt ) crt = NULL;
   if( rt == old_crt ) old_crt = NULL;
   if( response_times.Size() == 0 )
      EditResponseTimes( FALSE );
}

void ResponseTimeManager::ViewResponseTimes( Timestamp *ts )
{
   ShowTimestampRT( ts );  // cause browser dialog to be shown
}

void ResponseTimeManager::ListTimestampRT( Timestamp *ts )
{
   char buffer[50];
   ResponseTime *rt;
   int tcount = 0, scount = 0;

   fl_clear_browser( pfdtsr->TerminatingTSBrowser );
   fl_clear_browser( pfdtsr->StartingTSBrowser );

   for( response_times.First(); !response_times.IsDone(); response_times.Next() ) {
      rt = response_times.CurrentItem();
      if( ts == rt->ts2 ) {
	 sprintf( buffer, "%s\t\t%s\t%d\t%d", rt->Name(), rt->Timestamps(1)->Name(),
		  rt->response_time, rt->percentage );
	 fl_addto_browser( pfdtsr->TerminatingTSBrowser, buffer );
	 timestamp_responses[0][tcount++] = rt;
      }
      else if ( ts == rt->ts1 ) {
	 sprintf( buffer, "%s\t\t%s\t%d\t%d", rt->Name(), rt->Timestamps(2)->Name(),
		  rt->response_time, rt->percentage );
	 fl_addto_browser( pfdtsr->StartingTSBrowser, buffer );
	 timestamp_responses[1][scount++] = rt;
      }
   }

}

bool ResponseTimeManager::OrderCorrect( Hyperedge *first, Hyperedge *second )
{
   Cltn<Node *> *tset;
   Hyperedge *next_edge;
   
   // search through the hyperedge chain from first and attempt to find second
   // if found return true signifying that the order is correct

   tset = first->TargetSet();

   for( tset->First(); !tset->IsDone(); tset->Next() ) {
      next_edge = tset->CurrentItem()->NextEdge();
      if( next_edge == second )
	 return( TRUE );
      if( this->OrderCorrect( next_edge, second ) == TRUE )
	 return( TRUE );
      
   }

   return( FALSE );
   
}

void ResponseTimeManager::SaveXML( FILE *fp )
{
   ResponseTime *rt;
   char buffer[300];
   
   if( response_times.Size() == 0 )
      return;

   PrintNewXMLBlock( fp, "response-time-requirements" );
   
   for( response_times.First(); !response_times.IsDone(); response_times.Next() ) {
      rt = response_times.CurrentItem();
      sprintf( buffer, "response-time-req timestamp1=\"h%d\" timestamp2=\"h%d\" resptime-name=\"%s\" response-time=\"%d\" percentage=\"%d\" /",
	       rt->ts1->GetNumber(), rt->ts2->GetNumber(), PrintDescription( rt->name ), rt->response_time, rt->percentage );
      PrintXML( fp, buffer );
   }

   PrintEndXMLBlock( fp, "response-time-requirements" );
   LinebreakXML( fp );
}

void ResponseTimeManager::GeneratePostscriptDescription( FILE *ps_file )
{
   ResponseTime *rt;

   if( response_times.Size() == 0 )
      return;

   PrintManager::PrintHeading( "Response Time Requirements" );
   fprintf( ps_file, "[/Title(Response Time Requirements) /OUT pdfmark\n" );

   for( response_times.First(); !response_times.IsDone(); response_times.Next() ) {
      rt = response_times.CurrentItem();
      fprintf( ps_file, "%d rom (Response Time Requirement - ) S\n%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size,
	       PrintManager::PrintPostScriptText( rt->name ) );
      fprintf( ps_file, "1 IN ID %d rom (Timestamp Points ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID %d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( rt->ts1->Name() ) );
      fprintf( ps_file, "(%s ) P OD\n",  PrintManager::PrintPostScriptText( rt->ts2->Name() ) );
      fprintf( ps_file, "1 IN ID %d rom (Time Requirement - ) S\n%d bol (%d ) P\n", PrintManager::text_font_size, PrintManager::text_font_size,
	       rt->response_time );
      fprintf( ps_file, "%d rom (Percentage - ) S\n%d bol (%d ) P OD\n", PrintManager::text_font_size, PrintManager::text_font_size, rt->percentage );
   }
}
