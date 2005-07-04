/***********************************************************
 *
 * File:			synch_figure.cc
 * Author:			Andrew Miga
 * Created:			December 1996
 *
 * Modification history:
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "synch_figure.h"
#include "synchronization.h"
#include "presentation.h"
#include "path.h"
#include "defines.h"
#include "hyperedge.h"
#include <math.h>
#include <stdlib.h>

extern bool paths_complete;
extern FD_Main *pfdmMain;

SynchronizationFigure::SynchronizationFigure( Hyperedge *edge ) : HyperedgeFigure( edge )
{
   input_paths = new Cltn<SynchNullFigure *>;
   output_paths = new Cltn<SynchNullFigure *>;
   orientation = FORWARD_FIG;
   deleted = FALSE;
   path = NULL; // as path object is no longer needed set to null as it should be undefined for synchronizations
}

SynchronizationFigure::~SynchronizationFigure()
{
   deleted = TRUE;

   while( !input_paths->is_empty() )
      delete input_paths->Detach();
   delete input_paths;

   while( !output_paths->is_empty() )
      delete output_paths->Detach();
   delete output_paths;
}

void SynchronizationFigure::SetPosition( float f_nx, float f_ny, bool limit,  bool enclose,
					 bool dependent_update, bool interpolate )
{
   HyperedgeFigure::SetPosition( f_nx, f_ny, TRUE, TRUE, FALSE, FALSE );
   this->PathChanged();
}

void SynchronizationFigure::PathChanged()
{
   for( input_paths->First(); !input_paths->IsDone(); input_paths->Next() )
      input_paths->CurrentItem()->GetPath()->PathChanged();
   for( output_paths->First(); !output_paths->IsDone(); output_paths->Next() )
      output_paths->CurrentItem()->GetPath()->PathChanged();
}

void SynchronizationFigure::AddDependent( SynchNullFigure *new_null, path_direction dir, bool normal_operation )
{

   Cltn<SynchNullFigure *> *paths = (( dir == INPUT ) ? input_paths : output_paths ),
      *secondary_paths = (( dir == INPUT ) ? output_paths : input_paths );
   
   if( paths->Includes( new_null ) )
      std::cerr << "\nError: Null figure already included.\n";
   else
      paths->Add( new_null );

   if( normal_operation ) {
      ReorderPaths( dir );
      for( paths->First(); !paths->IsDone(); paths->Next() )
	 paths->CurrentItem()->GetPath()->PathChanged();

      if( secondary_paths->Size() == 1 )
	 secondary_paths->GetFirst()->GetPath()->PathChanged();
   }
}

void SynchronizationFigure::RemoveDependent( SynchNullFigure *null, path_direction dir )
{
   if( deleted ) return;  // if the figure was deleted don't bother deregistering

   Cltn<SynchNullFigure *> *paths = (( dir == INPUT ) ? input_paths : output_paths ),
      *secondary_paths = (( dir == INPUT ) ? output_paths : input_paths );
  
   if( paths->Includes( null ) )
      paths->Remove( null );
   else
      std::cerr << "\nError: Null figure already removed.\n";

   ReorderPaths( dir );
   for( paths->First(); !paths->IsDone(); paths->Next() )
      paths->CurrentItem()->GetPath()->PathChanged();

   if( secondary_paths->Size() == 1 )
      secondary_paths->GetFirst()->GetPath()->PathChanged();
}

void SynchronizationFigure::ReorderPaths( path_direction dir )
{
   float x, y, min_coord, old_min = 0.0, *oc;
   SynchNullFigure *current_null, *minimum_null = NULL;
   if( !paths_complete ) return;
   Cltn<SynchNullFigure *> *paths = (( dir == INPUT ) ? input_paths : output_paths );
   int i = 1,
      path_count = paths->Size();
   bool reordered = FALSE;

   if( (orientation == FORWARD_FIG) || (orientation == BACKWARD_FIG) ) // determine ordering coordinate
      oc = &y; // for horizontal forks order on y positions
   else
      oc = &x; // for vertical forks order on x positions
   
   while( i <= path_count ) {
      min_coord = 10.0;
      for( paths->First(); !paths->IsDone(); paths->Next() ) {
	 current_null = paths->CurrentItem();
	 current_null->GetPath()->BoundaryFigure( dir )->GetPosition( x, y );
	 if( *oc > old_min && *oc < min_coord ) {
	    min_coord = *oc;
	    minimum_null = current_null;
	 }
      }
      if( minimum_null != NULL ) {
	 if( minimum_null->Ordinal() != i )
	    reordered = TRUE;
	 minimum_null->Ordinal( i++ ); // assign ordinals in order
	 old_min = min_coord;
	 minimum_null = NULL;
      }
      else {
	 current_null->Ordinal( i++ );
	 reordered = TRUE;
      }
   }
   
   if( reordered ) {
      for( paths->First(); !paths->IsDone(); paths->Next() )
	 paths->CurrentItem()->GetPath()->PathChanged();
   }
}

void SynchronizationFigure::OrderPaths()
{
   ReorderPaths( INPUT );
   ReorderPaths( OUTPUT );
}

void SynchronizationFigure::Draw( Presentation *ppr )
{ 
   int num_inputs, num_outputs, i, path_count;
   float Y, x, y, yc;

   GetPosition( x, y );

   if( selected && !postscript_output ) {
      ppr->DrawSelectionHandles( x, y );
      ppr->SetFgColour( BLUE );
   }

   synch_type stype = ((Synchronization *)dependent_edge)->SynchronizationType();   
   num_inputs = ((Synchronization *)dependent_edge)->InputCount();
   num_outputs = ((Synchronization *)dependent_edge)->OutputCount();
   path_count = Max( num_inputs, num_outputs );
   top = y-(AND_FORK_HEIGHT+2*(path_count/2-1)*AND_FORK_PATH);
   bottom = y+(AND_FORK_HEIGHT+2*(((path_count%2 == 0) ? (path_count/2) : (path_count/2+1))-1)*AND_FORK_PATH);
   yc = top + (bottom - top)/2;
  
   ppr->SetLineWidth( 4 );  // draw synchronization bar
   ppr->DrawLine( x, top, x, bottom, x, y, orientation );
   ppr->SetLineWidth( 2 );

   if( stype == FORK ) // draw centered horizontal line entering the and fork
      ppr->DrawLine( x-STRAIGHT_PATH_SEGMENT, yc, x, yc, x, y, orientation );

   if( (stype == FORK) || (stype == MULTIPATH) ) { // draw out going paths for and fork and synchronization  
      for( i = 0; i < num_outputs; i++ ) {
	 Y = y+(2*((i+1) - num_outputs/2)-1)*AND_FORK_PATH;
	 ppr->DrawLine( x, Y, x+STRAIGHT_PATH_SEGMENT, Y, x, y, orientation );
      }
   }

   if( stype == JOIN )
      ppr->DrawLine( x, yc, x+STRAIGHT_PATH_SEGMENT, yc, x, y, orientation );  //  draw centered horizontal line leaving the and join
   
   if( (stype == JOIN) || (stype == MULTIPATH) ) { // draw in coming paths for and join and synchronization
      for( i = 0; i < num_inputs; i++ ) {
	 Y = y+(2*((i+1) - num_inputs/2)-1)*AND_FORK_PATH;
	 ppr->DrawLine( x-STRAIGHT_PATH_SEGMENT, Y, x , Y, x, y, orientation );
      } 
   }

   if( selected && !postscript_output ) ppr->SetFgColour( BLACK );
}

void SynchronizationFigure::GetNullPosition( path_direction dir, int ordinal, float& f_rx, float& f_ry )
{

   Cltn<SynchNullFigure *> *paths = (( dir == INPUT ) ? input_paths : output_paths );
   Synchronization *synch = (Synchronization *)dependent_edge;
   synch_type type = synch->SynchronizationType();
   float x, y, xt, yt;

   GetPosition( x, y );

   if( orientation == DOWN_FIG ) ordinal = paths->Size() - ordinal + 1;

   if( type == FORK ) {
      if( dir == INPUT )
	 Presentation::TranslateCoordinates( xt, yt, x-STRAIGHT_PATH_SEGMENT, ((synch->PathCount()%2 == 0) ? y: y+AND_FORK_PATH), x, y, orientation );
      else  // output of and fork desired 
	 Presentation::TranslateCoordinates( xt, yt, x+STRAIGHT_PATH_SEGMENT, y+(2*(ordinal - synch->OutputCount()/2)-1)*AND_FORK_PATH, x, y, orientation );
   }
   else if( type == JOIN ) {
      if( dir == OUTPUT )
	 Presentation::TranslateCoordinates( xt, yt, x+STRAIGHT_PATH_SEGMENT, ((synch->PathCount()%2 == 0) ? y: y+AND_FORK_PATH), x, y, orientation ); 
      else  // input of and join desired
	 Presentation::TranslateCoordinates( xt, yt, x-STRAIGHT_PATH_SEGMENT, y+(2*(ordinal - synch->InputCount()/2)-1)*AND_FORK_PATH, x, y, orientation );
   }
   else { // multipath synchronization
      if( dir == INPUT )
	 Presentation::TranslateCoordinates( xt, yt, x-STRAIGHT_PATH_SEGMENT, y+(2*(ordinal - synch->InputCount()/2)-1)*AND_FORK_PATH, x, y, orientation );
      else
	 Presentation::TranslateCoordinates( xt, yt, x+STRAIGHT_PATH_SEGMENT, y+(2*(ordinal - synch->OutputCount()/2)-1)*AND_FORK_PATH, x, y, orientation );
   }

   f_rx = xt;
   f_ry = yt;
}

void SynchronizationFigure::DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )
{
   float x, y, to, bo;
   int num_inputs, num_outputs, path_count;

   num_inputs = ((Synchronization *)dependent_edge)->InputCount();
   num_outputs = ((Synchronization *)dependent_edge)->OutputCount();
   path_count = Max( num_inputs, num_outputs );
   to = AND_FORK_HEIGHT+2*(path_count/2-1)*AND_FORK_PATH;
   bo = AND_FORK_HEIGHT+2*(((path_count%2 == 0) ? (path_count/2) : (path_count/2+1))-1)*AND_FORK_PATH;
   
   GetPosition( x, y );

   switch( orientation ) {

   case FORWARD_FIG:
   case BACKWARD_FIG:

      lb = x - STRAIGHT_PATH_SEGMENT;
      rb = x + STRAIGHT_PATH_SEGMENT;
      tb = y - to;
      bb = y + bo;
      break;

   case UP_FIG:

      lb = x - to;
      rb = x + bo;
      tb = y - STRAIGHT_PATH_SEGMENT;
      bb = y + STRAIGHT_PATH_SEGMENT;
      break;

   case DOWN_FIG:

      lb = x - bo;
      rb = x + to;
      tb = y - STRAIGHT_PATH_SEGMENT;
      bb = y + STRAIGHT_PATH_SEGMENT;
      break;      
   }
}

void SynchronizationFigure::ChangeOrientation()
{
   int popup, popup_choice;
   figure_orientation old_orientation = orientation;
   
   popup = fl_defpup( pfdmMain->Main->window, " Forward | Backward | Upward | Downward " );
   popup_choice = fl_dopup( popup );

   if( popup_choice > 0 ) {
      if( popup_choice == 1 )
	 orientation = FORWARD_FIG;
      else if( popup_choice == 2 )
	 orientation = BACKWARD_FIG;
      else if( popup_choice == 3 )
	 orientation = UP_FIG;
      else // choice == 4
	 orientation = DOWN_FIG;
   }

   fl_freepup( popup );
   
   if( orientation != old_orientation )
      PathChanged();
}
