/***********************************************************
 *
 * File:			loop_figure.cc
 * Author:			Andrew Miga
 * Created:			April 2000
 *
 * Modification history:
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "loop_figure.h"
#include "hyperedge.h"
#include "presentation.h"

extern FD_Main *pfdmMain;

static int figure_orientations[8] = { HORIZONTAL_UP, HORIZONTAL_DOWN, HORIZONTAL_UP, HORIZONTAL_DOWN, 
				      VERTICAL_LEFT, VERTICAL_RIGHT, VERTICAL_LEFT, VERTICAL_RIGHT };

static float x_null_offsets[4][8] = {
   { OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, -OR_FORK_PATH, 0, 0, 0, 0 }, // offsets for MAIN_OUT
   { -OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH, OR_FORK_PATH, 0, 0, 0, 0 }, // offsets for MAIN_IN
   { OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, -OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH }, // offsets for LOOP_OUT
   { -OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH }, // offsets for LOOP_IN
};

static float y_null_offsets[4][8] = {
   { 0, 0, 0, 0, -OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH, OR_FORK_PATH }, // offsets for MAIN_OUT
   { 0, 0, 0, 0, OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, -OR_FORK_PATH }, // offsets for MAIN_IN
   { -OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH, OR_FORK_PATH }, // offsets for LOOP_OUT
   { -OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, OR_FORK_PATH, OR_FORK_PATH, OR_FORK_PATH, -OR_FORK_PATH, -OR_FORK_PATH }, // offsets for LOOP_IN
};

static char orientation_names[8][15] = {
   "FORWARD_UP", "FORWARD_DOWN", "BACKWARD_UP", "BACKWARD_DOWN", "UPWARD_LEFT", "UPWARD_RIGHT", "DOWNWARD_LEFT", "DOWNWARD_RIGHT"
};

LoopFigure::LoopFigure( Hyperedge *edge, const char *new_orientation ) : HyperedgeFigure( edge )
{
   if( new_orientation == NULL )
      orientation = FORWARD_UP;
   else {
      for( int i = 0; i < 8; i++ ) {
	 if( strequal( new_orientation, orientation_names[i] ) ) {
	    orientation = (loop_orientation)i;
	    break;
	 }
      }
   }

   for( int j = 0; j < 4; j++ )
      loop_splines[j] = NULL;

   path = NULL; // as path object is no longer needed set to null as it should be undefined for loops
}

void LoopFigure::Draw( Presentation *ppr )
{
   float x, y;
   figure_orientation fig_orientation = figure_orientations[orientation];

   GetPosition( x, y );

   if( selected && !postscript_output ) {
      ppr->DrawSelectionHandles( x, y );
      ppr->SetFgColour( BLUE );
   }

   // draw three lines for vertical right, rotation will handle all other cases
   ppr->DrawLine( x, y+OR_FORK_PATH, x, y-OR_FORK_PATH, x, y, fig_orientation ); // main path line
   ppr->DrawLine( x, y, x+OR_FORK_PATH, y-OR_FORK_PATH, x, y, fig_orientation );
   ppr->DrawLine( x, y, x+OR_FORK_PATH, y+OR_FORK_PATH, x, y, fig_orientation );

   if( dependent_edge->HasName() ) {

      alignment al;
      float xoffset, yoffset;

      switch( fig_orientation ) {

      case HORIZONTAL_UP: 
	 xoffset = 0;
	 yoffset = .025;
	 al = CENTER;
	 break;
      
      case HORIZONTAL_DOWN:
	 xoffset = 0;
	 yoffset = -.015;
	 al = CENTER;
	 break;
      
      case VERTICAL_LEFT:
	 xoffset = .01;
	 yoffset = 0;
	 al = LEFT_ALIGN;
	 break;
      
      case VERTICAL_RIGHT:
	 xoffset = -.01;
	 yoffset = 0;
	 al = RIGHT_ALIGN;
	 break;
      }

      ppr->DrawText( x+xoffset, y+yoffset, dependent_edge->HyperedgeName(), FALSE, al );
   }

   ppr->SetFgColour( BLACK );
}

void LoopFigure::PathChanged()
{
   for( int j = 0; j < 4; j++ )
      loop_splines[j]->GetPath()->PathChanged();
}

void LoopFigure::SetPosition( float f_nx, float f_ny, bool limit,  bool enclose,
				   bool dependent_update, bool interpolate )
{
   HyperedgeFigure::SetPosition( f_nx, f_ny );
   if( loop_splines[0] != NULL )
      this->PathChanged();
}

void LoopFigure::DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )
{
   float x, y;
   
   GetPosition( x, y );

   lb = x - .03;
   rb = x + .03;
   tb = y - .03;
   bb = y + .03;
}

void LoopFigure::ChangeOrientation()
{
   int popup, popup_choice;
   loop_orientation old_orientation = orientation;
   
   popup = fl_defpup( pfdmMain->Main->window, " Forward Path - Loop Up | Forward Path - Loop Down | Backward Path - Loop Up | Backward path - Loop Down | Upward Path - Loop Left | Upward Path - Loop Right | Downward Path - Loop Left | Downward Path - Loop Right " );
   popup_choice = fl_dopup( popup );

   if( popup_choice > 0 )
      orientation = (loop_orientation)(popup_choice-1);

   fl_freepup( popup );
   
   if( orientation != old_orientation )
      PathChanged();
}

void LoopFigure::GetNullPosition( loop_paths lpath, float& f_rx, float& f_ry )
{
   float xb, yb;

   GetPosition( xb, yb );

   f_rx = xb+x_null_offsets[lpath][orientation];
   f_ry = yb+y_null_offsets[lpath][orientation];
}

const char * LoopFigure::Orientation()
{
   return( orientation_names[(int)orientation] );
}
