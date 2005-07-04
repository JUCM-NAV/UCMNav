
// callbacks for arrival process dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "start.h"
#include <stdlib.h>

void EditArrivalProcess( Start *start );
void SingleInput( char *label, double value = 0 );
void DualInput( char *label1, char *label2, double value1 = 0, double value2 = 0 );
void ExpertInput( char *characteristics = NULL );

FD_ArrivalProcess *pfdarp = NULL;
extern void SetPromptUserForSave();

static Start *current_start = NULL;
static arrival_type atype;
static stream_type stype;
static bool input1_visible, input2_visible, expert_visible;

static int ArrivalDialogClose( FL_FORM *, void * );

void OpenStream()
{
   fl_set_object_label( pfdarp->DistributionLabel, "Interarrival Time Distribution" );
   fl_hide_object( pfdarp->PopulationSize );
   fl_deactivate_object( pfdarp->PopulationSize );
}

void ClosedStream()
{
   fl_set_object_label( pfdarp->DistributionLabel, "External Delay Distribution" );
   fl_show_object( pfdarp->PopulationSize );
   fl_activate_object( pfdarp->PopulationSize );
}

void EditArrivalProcess( Start *start )
{
   char population[10];

   current_start = start;
   FREEZE_TOOL();

   if( !pfdarp ) {
      pfdarp = create_form_ArrivalProcess();
      fl_set_form_atclose( pfdarp->ArrivalProcess, ArrivalDialogClose, NULL );
      fl_addto_choice( pfdarp->WorkloadType, " Open Arrival Stream | Closed Arrivals with Finite Population " );
      fl_set_choice_fontsize( pfdarp->WorkloadType, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfdarp->WorkloadType, FL_BOLD_STYLE );      
      input1_visible = TRUE;
      input2_visible = TRUE;
      expert_visible = TRUE;
   }

   stype = current_start->StreamType();
   if( stype == OPEN_ARRIVAL ) {
      fl_set_choice( pfdarp->WorkloadType, 1 );
      fl_set_input( pfdarp->PopulationSize, "" );
      OpenStream();
   } else { // stype == CLOSED_ARRIVAL
      fl_set_choice( pfdarp->WorkloadType, 2 );
      sprintf( population, "%d", current_start->PopulationSize() );
      fl_set_input( pfdarp->PopulationSize, population );
      ClosedStream();
   }

   fl_set_button( pfdarp->APExponentialButton, 0 );
   fl_set_button( pfdarp->APDeterministicButton, 0 );
   fl_set_button( pfdarp->APUniformButton, 0 );
   fl_set_button( pfdarp->APErlangButton, 0 );
   fl_set_button( pfdarp->APExpertButton, 0 );

   atype = current_start->ArrivalType();
   switch( atype ) {

   case EXPONENTIAL:

      SingleInput( "Mean", current_start->Input(1) );
      fl_set_button( pfdarp->APExponentialButton, 1 );
      break;

   case DETERMINISTIC:

      SingleInput( "Value", current_start->Input(1) );
      fl_set_button( pfdarp->APDeterministicButton, 1 );
      break;

   case UNIFORM:

      DualInput( "Low", "High", current_start->Input(1), current_start->Input(2) );
      fl_set_button( pfdarp->APUniformButton, 1 );
      break;

   case ERLANG:

      DualInput( "Mean", "Kernel", current_start->Input(1), current_start->Input(2) );
      fl_set_button( pfdarp->APErlangButton, 1 );
      break;

   case EXPERT:

      ExpertInput( current_start->ExpertInput() );
      fl_set_button( pfdarp->APExpertButton, 1 );
      break;
      
   default:
      
      fl_set_object_label( pfdarp->Input1, "" );
      fl_hide_object( pfdarp->Input1 );
      fl_deactivate_object( pfdarp->Input1 );

      fl_set_object_label( pfdarp->Input2, "" );
      fl_hide_object( pfdarp->Input2 );
      fl_deactivate_object( pfdarp->Input2 );
   
      fl_hide_object( pfdarp->ExpertInput );
      fl_deactivate_object( pfdarp->ExpertInput );

      input1_visible = FALSE;
      input2_visible = FALSE;
      expert_visible = FALSE;
      break;
      
   }

   fl_show_form( pfdarp->ArrivalProcess, FL_PLACE_CENTER,
		 FL_TRANSIENT, " Start Point Workload " );

}

extern "C"
void WorkloadType_cb(FL_OBJECT *choice, long)
{
   if( fl_get_choice( choice ) == 1 ) {
      if( stype != OPEN_ARRIVAL ) {
	 OpenStream();
	 stype = OPEN_ARRIVAL; 
      }
   } else {
     if( stype != CLOSED_ARRIVAL ) {
	ClosedStream();
	stype = CLOSED_ARRIVAL;
     }
   }
}

extern "C"
void APExponentialButton_cb(FL_OBJECT *, long)
{
   if( atype != EXPONENTIAL ) {
      SingleInput( "Mean" );
      atype = EXPONENTIAL;
   }
}

extern "C"
void APDeterministicButton_cb(FL_OBJECT *, long)
{
   if( atype != DETERMINISTIC ) {
      SingleInput( "Value" );
      atype = DETERMINISTIC;
   }
}

extern "C"
void APUniformButton_cb(FL_OBJECT *, long)
{
   if( atype != UNIFORM ) {
      DualInput( "Low", "High" );
      atype = UNIFORM;
   }
}

void APErlangButton_cb(FL_OBJECT *, long)
{
   if( atype != ERLANG ) {
      DualInput( "Mean", "Kernel"  );
      atype = ERLANG;
   }
}

extern "C"
void APExpertButton_cb(FL_OBJECT *, long)
{
   if( atype != EXPERT ) { 
      ExpertInput();
      atype = EXPERT;
   }
} 

extern "C"
void APOKButton_cb(FL_OBJECT *, long)
{
   current_start->StreamType( stype );   
   current_start->ArrivalType( atype ); 

   if( stype == CLOSED_ARRIVAL )
      current_start->PopulationSize( atoi( fl_get_input( pfdarp->PopulationSize ) ) );

   switch( atype ) {

   case EXPONENTIAL:
   case DETERMINISTIC:

      current_start->Input( atof( fl_get_input( pfdarp->Input1 )), 1 );
      break;

   case UNIFORM:
   case ERLANG:

      current_start->Input( atof( fl_get_input( pfdarp->Input1 )), 1 );
      current_start->Input( atof( fl_get_input( pfdarp->Input2 )), 2 );
      break;

   case EXPERT:

      current_start->ExpertInput( fl_get_input( pfdarp->ExpertInput ) );
      break;
   
   case UNDEFINED:
      break;   
   }
   
   fl_hide_form( pfdarp->ArrivalProcess );
   UNFREEZE_TOOL();
   SetPromptUserForSave();
}

extern "C"
void APCancelButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdarp->ArrivalProcess );
   UNFREEZE_TOOL();
}

void SingleInput( char *label, double value )
{
   char buffer[10];
   
   if( !input1_visible ) {
      fl_show_object( pfdarp->Input1 );
      fl_activate_object( pfdarp->Input1 );
      input1_visible = TRUE;
   }
   fl_set_object_label( pfdarp->Input1, label );
   if( value != 0 ) {
      sprintf( buffer, "%f", value );
      fl_set_input(  pfdarp->Input1, buffer );
   }
   else
      fl_set_input(  pfdarp->Input1, "" );

   if( input2_visible ) {
      fl_set_object_label( pfdarp->Input2, "" );
      fl_set_input( pfdarp->Input2, "" );
      fl_hide_object( pfdarp->Input2 );
      fl_deactivate_object( pfdarp->Input2 );
      input2_visible = FALSE;
   }

   if( expert_visible ) {
      fl_set_input( pfdarp->ExpertInput, "" );
      fl_hide_object( pfdarp->ExpertInput );
      fl_deactivate_object( pfdarp->ExpertInput );
      expert_visible = FALSE;
   }

}

void DualInput( char *label1, char *label2, double value1, double value2 )
{
   char buffer[10];

   if( !input1_visible ) {
      fl_show_object( pfdarp->Input1 );
      fl_activate_object( pfdarp->Input1 );
      input1_visible = TRUE;
   }
   fl_set_object_label( pfdarp->Input1, label1 );
   if( value1 != 0 ) {
      sprintf( buffer, "%f", value1 );
      fl_set_input(  pfdarp->Input1, buffer );
   }
   else
      fl_set_input(  pfdarp->Input1, "" );

   if( !input2_visible ) {
      fl_show_object( pfdarp->Input2 );
      fl_activate_object( pfdarp->Input2 );
      input2_visible = TRUE;
   }
   fl_set_object_label( pfdarp->Input2, label2 );
   if( value2 != 0 ) {
      sprintf( buffer, "%f", value2 );
      fl_set_input(  pfdarp->Input2, buffer );
   }
   else
      fl_set_input(  pfdarp->Input2, "" );

   if( expert_visible ) {
      fl_set_input( pfdarp->ExpertInput, "" );
      fl_hide_object( pfdarp->ExpertInput );
      fl_deactivate_object( pfdarp->ExpertInput );
      expert_visible = FALSE;
   }

}

void ExpertInput( char *characteristics )
{

   if( !expert_visible ) {
      fl_activate_object( pfdarp->ExpertInput );
      fl_show_object( pfdarp->ExpertInput );
      expert_visible = TRUE;
   }
   if( characteristics != NULL )
      fl_set_input( pfdarp->ExpertInput, characteristics );
   else
      fl_set_input( pfdarp->ExpertInput, "" );

   if( input1_visible ) {
      fl_set_object_label( pfdarp->Input1, "" );
      fl_set_input( pfdarp->Input1, "" );
      fl_hide_object( pfdarp->Input1 );
      fl_deactivate_object( pfdarp->Input1 );
      input1_visible = FALSE;
   }

   if( input2_visible ) {
      fl_set_object_label( pfdarp->Input2, "" );
      fl_set_input( pfdarp->Input2, "" );
      fl_hide_object( pfdarp->Input2 );
      fl_deactivate_object( pfdarp->Input2 );
      input2_visible = FALSE;
   }

}

int ArrivalDialogClose( FL_FORM *, void * )
{
   APCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}
