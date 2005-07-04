
// callbacks for timestamp dialogs

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "timestamp.h"
#include "ts_figure.h"
#include "response_time.h"
#include <stdlib.h>

void EditTimestampDescription( TimestampFigure *new_tsfigure );
void EditResponseTimeDescription( ResponseTime *rt );
void EditNewResponseTime( TimestampFigure *ts1, TimestampFigure *ts2 );
void ShowResponseTimes();

void UpdateResponseTimesDisplay();

void ShowTimestampRT( Timestamp *ts );
void UpdateTimestampRTDisplay();

extern void DrawScreen();
extern void SetPromptUserForSave();

FD_TimestampCreationPopup *pfdtcp = NULL;
FD_ResponseTimePopup *pfdtrp = NULL;
FD_ViewResponseTimes *pfdvrt = NULL;
FD_ViewTimestampRT *pfdtsr = NULL;

int TCPvisible, TRPvisible, VRTvisible, TSRvisible;

static TimestampFigure *cts_figure = NULL;
ResponseTime *crt = NULL, *old_crt = NULL;
static Timestamp *current_timestamp = NULL, *tf = NULL, *ts = NULL, *cts = NULL;
static int operation;

static int TimestampDialogClose( FL_FORM *, void * );
static int ResponseTimeDialogClose( FL_FORM *, void * );
static int ViewResponseTimesDialogClose( FL_FORM *, void * );
static int ViewTimestampRTDialogClose( FL_FORM *, void * );

void EditTimestampDescription( TimestampFigure *new_tsfigure )
{

   cts_figure = new_tsfigure;
   current_timestamp = (Timestamp *)cts_figure->Edge();
   
   FREEZE_TOOL();

   if( !pfdtcp ) {
      pfdtcp = create_form_TimestampCreationPopup();
      fl_set_form_atclose( pfdtcp->TimestampCreationPopup, TimestampDialogClose, NULL );
   }

   fl_set_input( pfdtcp->TimestampLabel, current_timestamp->Name() );

   fl_set_button( pfdtcp->UpButton, 0 );
   fl_set_button( pfdtcp->DownButton, 0 );
   fl_set_button( pfdtcp->LeftButton, 0 );
   fl_set_button( pfdtcp->RightButton, 0 );

   switch( cts_figure->Direction() ) {

   case UP:
      fl_set_button( pfdtcp->UpButton, 1 );
      break;
   case DOWN:
      fl_set_button( pfdtcp->DownButton, 1 );
      break;
   case LEFT:
      fl_set_button( pfdtcp->LeftButton, 1 );
      break;
   case RIGHT:
      fl_set_button( pfdtcp->RightButton, 1 );
      break;
   }

   if( current_timestamp->Reference() == PREVIOUS ) {
      fl_set_button( pfdtcp->PreviousButton, 1 );
      fl_set_button( pfdtcp->NextButton, 0 );
   }
   else {
      fl_set_button( pfdtcp->NextButton, 1 );
      fl_set_button( pfdtcp->PreviousButton, 0 );
   }
   
   fl_show_form( pfdtcp->TimestampCreationPopup, FL_PLACE_CENTER,
		 FL_TRANSIENT, "Create Timestamp Point" );

   TCPvisible = 1;

   
}


void TimestampCreationOKButton_cb(FL_OBJECT *, long)
{
   timestamp_direction direction;
   timestamp_reference reference;
   
   fl_hide_form( pfdtcp->TimestampCreationPopup );
   TCPvisible = 0;

   current_timestamp->Name( fl_get_input( pfdtcp->TimestampLabel ) );

   if( fl_get_button( pfdtcp->UpButton ) )
      direction = UP;
   else if( fl_get_button( pfdtcp->DownButton ) )
      direction = DOWN;
   else if( fl_get_button( pfdtcp->LeftButton ) )
      direction = LEFT;
   else if( fl_get_button( pfdtcp->RightButton ) )
      direction = RIGHT;
 
   cts_figure->Direction( direction );

   if( fl_get_button( pfdtcp->PreviousButton ) )
      reference = PREVIOUS;
   else
      reference = NEXT;

   current_timestamp->Reference( reference );
   
   UNFREEZE_TOOL();
   SetPromptUserForSave();
   DrawScreen();

}

void EditResponseTimeDescription( ResponseTime *rt )
{
   char time[10], pct[10], buffer[50];

   crt = rt;

   FREEZE_TOOL();

   if( !pfdtrp ) {
      pfdtrp = create_form_ResponseTimePopup();
      fl_set_form_atclose( pfdtrp->ResponseTimePopup, ResponseTimeDialogClose, NULL );
   }

   sprintf( time, "%d", crt->Time() );
   sprintf( pct, "%d", crt->Percentage() );
   
   fl_set_input( pfdtrp->ResponseTimeValue, time );
   fl_set_input( pfdtrp->ResponseTimePercentage, pct );
   fl_set_input( pfdtrp->Name, crt->Name() );
   
   sprintf( buffer, "From Timestamp %s to %s",
	    crt->Timestamps(1)->Name(),
	    crt->Timestamps(2)->Name() );

   fl_set_object_label( pfdtrp->ResponseTimePrompt, buffer );
   
   fl_show_form( pfdtrp->ResponseTimePopup, FL_PLACE_CENTER,
		 FL_TRANSIENT, "Response Time Requirement" );

   TRPvisible = 1;
   operation = EDIT_EXISTING;
   
}

void EditNewResponseTime( TimestampFigure *ts1, TimestampFigure *ts2 )
{
   char time[10], pct[10], buffer[50];

   tf = (Timestamp *)ts1->Edge();
   ts = (Timestamp *)ts2->Edge();

   FREEZE_TOOL();
   
   if( !pfdtrp ) {
      pfdtrp = create_form_ResponseTimePopup();
      fl_set_form_atclose( pfdtrp->ResponseTimePopup, ResponseTimeDialogClose, NULL );
   }

   sprintf( time, "%d", DEFAULT_TIME ); 
   sprintf( pct, "%d", DEFAULT_PERCENTAGE );
   fl_set_input( pfdtrp->ResponseTimeValue, time );
   fl_set_input( pfdtrp->ResponseTimePercentage, pct );
   fl_set_input( pfdtrp->Name, "" );

   sprintf( buffer, "From Timestamp %s to %s", tf->Name(), ts->Name() );
   fl_set_object_label( pfdtrp->ResponseTimePrompt, buffer );
   
   fl_show_form( pfdtrp->ResponseTimePopup, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Response Time Requirement" );

  TRPvisible = 1;
  operation = CREATE_NEW;
}

void ResponseTimeResult_cb(FL_OBJECT *, long accept )
{

   fl_hide_form( pfdtrp->ResponseTimePopup );
   TRPvisible = 0;
   UNFREEZE_TOOL();

   if( accept ) {
      if( operation == EDIT_EXISTING ) {
	 crt->Time( atoi( fl_get_input( pfdtrp->ResponseTimeValue ) ) );
	 crt->Percentage( atoi( fl_get_input( pfdtrp->ResponseTimePercentage ) ) );
	 crt->Name( fl_get_input( pfdtrp->Name ) );
      }
      else {
	 ResponseTimeManager *rtm = ResponseTimeManager::Instance();
	 rtm->AddNewResponseTime( tf, ts, fl_get_input( pfdtrp->Name ),
				  atoi(fl_get_input( pfdtrp->ResponseTimeValue )),
				  atoi(fl_get_input( pfdtrp->ResponseTimePercentage )) );
	 SetPromptUserForSave();
	 DrawScreen();
      }
   }
}

void ShowResponseTimes()
{
   
   FREEZE_TOOL();

   if( !pfdvrt ) {
      pfdvrt = create_form_ViewResponseTimes();
      fl_set_browser_fontsize( pfdvrt->ResponseTimesBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdvrt->ResponseTimesBrowser, FL_BOLD_STYLE );
      fl_set_form_atclose( pfdvrt->ViewResponseTimes, ViewResponseTimesDialogClose, NULL );
   }

   UpdateResponseTimesDisplay();

   fl_show_form( pfdvrt->ViewResponseTimes, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Response Time Requirements" );

   VRTvisible = 1;
   
}

void UpdateResponseTimesDisplay()
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();

   rtm->ListResponseTimes();  // call response time manager to fill browser

   // deactivate editing buttons initially and clear inputs
   DISABLE_WIDGET( pfdvrt->EditRTButtons );
   fl_set_input( pfdvrt->ResponseTimeValue, "" );
   fl_set_input( pfdvrt->ResponseTimePercentage, "" );
   fl_set_input( pfdvrt->Name, "" );
}

void VRTRemoveDialogButton_cb(FL_OBJECT *, long)
{
   if( crt ) {
      crt->Unhighlight();
      DrawScreen();
   }
   fl_hide_form(  pfdvrt->ViewResponseTimes );
   VRTvisible = 0;
   UNFREEZE_TOOL();
}

void VRTAcceptButton_cb(FL_OBJECT *, long)
{

   crt->Time( atoi( fl_get_input( pfdvrt->ResponseTimeValue ) ) );
   crt->Percentage( atoi( fl_get_input( pfdvrt->ResponseTimePercentage ) ) );
   crt->Name( fl_get_input( pfdvrt->Name ) );
  
   UpdateResponseTimesDisplay();

   crt->Unhighlight();
   SetPromptUserForSave();
   DrawScreen();
}

void ResponseTimesBrowser_cb(FL_OBJECT *, long)
{
   char buffer[10];
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();

   int choice = fl_get_browser( pfdvrt->ResponseTimesBrowser );

   crt = rtm->GetResponseTime( choice );

   if( crt != old_crt ) {
      if( old_crt ) old_crt->Unhighlight();
      crt->Highlight();
      DrawScreen();
   }
   old_crt = crt;
      
   sprintf( buffer, "%d", crt->Time() );
   fl_set_input( pfdvrt->ResponseTimeValue, buffer );
   sprintf( buffer, "%d", crt->Percentage() );
   fl_set_input( pfdvrt->ResponseTimePercentage, buffer );
   fl_set_input( pfdvrt->Name, crt->Name() );
      
   ENABLE_WIDGET( pfdvrt->EditRTButtons ); // re-enable editing buttons
}

void VRTCancelButton_cb(FL_OBJECT *, long)
{
   char buffer[10];

   sprintf( buffer, "%d", crt->Time() );
   fl_set_input( pfdvrt->ResponseTimeValue, buffer );
   sprintf( buffer, "%d", crt->Percentage() );
   fl_set_input( pfdvrt->ResponseTimePercentage, buffer );
   fl_set_input( pfdvrt->Name, crt->Name() );
}

void VRTDeleteButton_cb(FL_OBJECT *, long)
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();

   crt->Unhighlight();
   DrawScreen();
   rtm->DeleteResponseTime( crt );
   old_crt = NULL;
   UpdateResponseTimesDisplay();

   SetPromptUserForSave();
}

void ShowTimestampRT( Timestamp *ts )
{
   char buffer[100];

   cts = ts;
   FREEZE_TOOL();

   if( !pfdtsr ) {
      pfdtsr = create_form_ViewTimestampRT();
      fl_set_browser_fontsize( pfdtsr->TerminatingTSBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdtsr->TerminatingTSBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdtsr->StartingTSBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdtsr->StartingTSBrowser, FL_BOLD_STYLE );
      fl_set_form_atclose( pfdtsr->ViewTimestampRT, ViewTimestampRTDialogClose, NULL );
   }

   sprintf( buffer, "Timestamp %s Terminates and Starts the following Requirements.", cts->Name() );
   fl_set_object_label( pfdtsr->Prompt, buffer );
      
   UpdateTimestampRTDisplay();
   
   fl_show_form( pfdtsr->ViewTimestampRT, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Response Time Requirements" );

   TSRvisible = 1;
}

void UpdateTimestampRTDisplay()
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();

   rtm->ListTimestampRT( cts );  // call response time manager to fill browser
   
   // deactivate editing buttons initially and clear inputs
   DISABLE_WIDGET( pfdtsr->EditRTButtons );
   fl_set_input( pfdtsr->ResponseTimeValue, "" );
   fl_set_input( pfdtsr->ResponseTimePercentage, "" );
}

void TSRAcceptButton_cb(FL_OBJECT *, long)
{

   crt->Time( atoi( fl_get_input( pfdtsr->ResponseTimeValue ) ) );
   crt->Percentage( atoi( fl_get_input( pfdtsr->ResponseTimePercentage ) ) );
   crt->Name( fl_get_input( pfdtsr->Name ) );
   
   UpdateTimestampRTDisplay();

   crt->Unhighlight();
   SetPromptUserForSave();
   DrawScreen();
}

void TSRCancelButton_cb(FL_OBJECT *, long)
{
   char buffer[10];

   sprintf( buffer, "%d", crt->Time() );
   fl_set_input( pfdtsr->ResponseTimeValue, buffer );
   sprintf( buffer, "%d", crt->Percentage() );
   fl_set_input( pfdtsr->ResponseTimePercentage, buffer );
   fl_set_input( pfdtsr->Name, crt->Name() );
}

void TSRDeleteButton_cb(FL_OBJECT *, long)
{
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();

   crt->Unhighlight();
   DrawScreen();
   rtm->DeleteResponseTime( crt );
   old_crt = NULL;
   UpdateTimestampRTDisplay();
   SetPromptUserForSave();
}

void TSRRemoveDialogButton_cb(FL_OBJECT *, long)
{
   if( crt ) {
      crt->Unhighlight();
      DrawScreen();
   }
   fl_hide_form(  pfdtsr->ViewTimestampRT );
   TSRvisible = 0;
   UNFREEZE_TOOL();
}

void TSBrowser_cb(FL_OBJECT *pflo, long browser )
{
   char buffer[10];
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();

   int choice = fl_get_browser( pflo );

   if( browser == 0 )
      fl_deselect_browser( pfdtsr->StartingTSBrowser );
   else
      fl_deselect_browser( pfdtsr->TerminatingTSBrowser );
      
   crt = rtm->GetResponseTime( browser, choice );

   if( crt != old_crt ) {
      if( old_crt ) old_crt->Unhighlight();
      crt->Highlight();
      DrawScreen();
   }
   old_crt = crt;
 
   sprintf( buffer, "%d", crt->Time() );
   fl_set_input( pfdtsr->ResponseTimeValue, buffer );
   sprintf( buffer, "%d", crt->Percentage() );
   fl_set_input( pfdtsr->ResponseTimePercentage, buffer );
   fl_set_input( pfdtsr->Name, crt->Name() );
      
   ENABLE_WIDGET( pfdtsr->EditRTButtons ); // re-enable editing buttons
}

int TimestampDialogClose( FL_FORM *, void * )
{
   TimestampCreationOKButton_cb( 0, 0 );
   return( FL_IGNORE );
}

int ResponseTimeDialogClose( FL_FORM *, void * )
{
   ResponseTimeResult_cb( 0, 0 );
   return( FL_IGNORE );
}

int ViewResponseTimesDialogClose( FL_FORM *, void * )
{
   VRTRemoveDialogButton_cb( 0, 0 );
   return( FL_IGNORE );
}

int ViewTimestampRTDialogClose( FL_FORM *, void * )
{
   TSRRemoveDialogButton_cb( 0, 0 );
   return( FL_IGNORE );
}

void EditResponseTimes( bool allow )
{
   fl_set_menu_item_mode( pfdmMain->PerformanceMenu, 1, ( allow ?  FL_PUP_NONE : FL_PUP_GREY ) );
}
