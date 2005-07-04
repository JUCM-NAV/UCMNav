/***********************************************************
 *
 * File:			point_figure.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "point_figure.h"
#include "presentation.h"
#include "xfpresentation.h"
#include "point.h"
#include "path.h"
#include "display.h"
#include "hyperedge.h"
#include "empty.h"
#include "start.h"
#include "result.h"
#include "timer.h"
#include "wait.h"
#include <math.h>
#include <stdlib.h>

#define FP_SIZE    0.015
#define SR_SIZE    0.02
#define ABORT_SIZE 0.012

extern int showing_data_points, showing_labels, show_stub_io;

PointFigure::PointFigure( Hyperedge *edge, point_type type ) : HyperedgeFigure( edge )
{ 
   ptype = type;
}

PointFigure::~PointFigure()
{
   if( path ) path->PurgeFigure( this );
}

void PointFigure::SetPosition( float f_nx, float f_ny, bool limit,  bool enclose, bool dependent_update, bool interpolate )
{
   HyperedgeFigure::SetPosition( f_nx, f_ny, limit, enclose, dependent_update, interpolate );
   
   if( ptype == EMPTY_FIG )  // reorder paths of fork or join if necessary
      ((Empty *)dependent_edge)->ReorderPaths();

   if( ptype == TIMER_FIG )
      ((Timer *)dependent_edge)->TimerPositionChanged();

   if( ptype == WAIT_FIG )
      ((Wait *)dependent_edge)->WaitPositionChanged();
}

void PointFigure::PathChanged()
{
   path->PathChanged();

   if( ptype == EMPTY_FIG )  // reorder paths of fork or join if necessary
      ((Empty *)dependent_edge)->ReorderPaths();

   if( ptype == TIMER_FIG )
      ((Timer *)dependent_edge)->TimerPositionChanged();

   if( ptype == WAIT_FIG )
      ((Wait *)dependent_edge)->WaitPositionChanged();
}

void PointFigure::Draw( Presentation *ppr )
{ 
#if defined( TRACE ) 
   Trace trace( __FILE__, __LINE__, "void PointFigure::Draw( Presentation *ppr )" ); 
#endif /* TRACE */ 

   float x, y, theta, xd, yd;
   int number;
   char path_number[10];
#ifdef MSC_DEBUG	       
   char hnumber[10];
#endif
   Point *psl, *pl;
   
   GetPosition( x, y );

   if( selected && !postscript_output ) {
      ppr->DrawSelectionHandles( x, y );
      ppr->SetFgColour( BLUE );
   }
   
   switch( ptype ) {

   case START_FIG:

      if( ((Start *)dependent_edge)->Highlight() )
	 ppr->SetFgColour( RED );
      ppr->DrawFilledCircle( x, y, WAIT_FIGURE_SIZE );      
      ppr->DrawText( x, y-.02, dependent_edge->HyperedgeName(), FALSE, Alignment() );      
      break;
    
   case WAIT_FIG:

      ppr->DrawFilledCircle( x, y, WAIT_FIGURE_SIZE );
      ppr->DrawText( x, y-.02, dependent_edge->HyperedgeName(), FALSE, CENTER );      
      break;
    
   case EMPTY_FIG:

      if( dependent_edge->FirstInput()->EdgeType() == ABORT ) { // timeout path of timer
	 ppr->DrawLine( x-ABORT_SIZE, y-ABORT_SIZE, x-ABORT_SIZE, y );
	 ppr->DrawLine( x-ABORT_SIZE, y-ABORT_SIZE, x+ABORT_SIZE, y+ABORT_SIZE );
	 ppr->DrawLine( x+ABORT_SIZE, y+ABORT_SIZE, x+ABORT_SIZE, y );
      }
      else if( ((Empty *)dependent_edge)->characteristics == FAILURE_POINT ) {
	 ppr->DrawLine( x, y, x, y+2*FP_SIZE );
	 ppr->DrawLine( x-FP_SIZE, y+2*FP_SIZE, x+FP_SIZE, y+2*FP_SIZE );
	 ppr->DrawLine( x-FP_SIZE/2, y+3*FP_SIZE, x+FP_SIZE/2, y+3*FP_SIZE );
	 ppr->DrawLine( x-FP_SIZE/4, y+4*FP_SIZE, x+FP_SIZE/4, y+4*FP_SIZE );
      }
      else if( ((Empty *)dependent_edge)->characteristics == SHARED_RESPONSIBILITY ) {
	 ppr->DrawLine( x, y-SR_SIZE, x, y+SR_SIZE );
	 ppr->DrawLine( x-SR_SIZE, y-SR_SIZE, x, y-SR_SIZE );
	 ppr->DrawLine( x-SR_SIZE, y-SR_SIZE, x-SR_SIZE, y );
	 ppr->DrawLine( x+SR_SIZE, y+SR_SIZE, x, y+SR_SIZE );
	 ppr->DrawLine( x+SR_SIZE, y+SR_SIZE, x+SR_SIZE, y );
      }
      else if( ((Empty *)dependent_edge)->characteristics == DIRECTION_ARROW ) {
	 theta = path->GetTangentAngle( this );
	 ppr->DrawLine( x-RESULT_BAR*sin(PI/4+theta), y-RESULT_BAR*cos(PI/4+theta), x, y );
	 ppr->DrawLine( x-RESULT_BAR*sin(PI/4-theta), y+RESULT_BAR*cos(PI/4-theta), x, y );
      }
      else if ( showing_data_points ){
	 ppr->SetLineWidth( 1 );
	 if( ((Empty *)dependent_edge)->Highlighted() )
	    ppr->SetFgColour( RED );
	 ppr->DrawCircle( x, y, EMPTY_FIGURE_SIZE );
	 ppr->SetLineWidth( 2 );
#ifdef MSC_DEBUG	       
	 sprintf( hnumber, "HN%d", dependent_edge->GetNumber() );
 	 ppr->DrawText( x, y-.04, hnumber, TRUE, CENTER );
#endif
     }

      if( (( number = ((Empty *)dependent_edge)->PathNumber() ) != 0) && (show_stub_io || postscript_output) )
      {  // display temporary input/output path number during stub binding
	 if( number > FORK_OFFSET ) {
	    sprintf( path_number, "BR %d", (number-FORK_OFFSET) );
	    ppr->SetFgColour( BLUE );
	 }
	 else {
	    if( ((Empty *)dependent_edge)->Highlighted() )
	       ppr->SetFgColour( RED );
	    else
	       ppr->SetFgColour( (selected && !postscript_output) ? BLUE : FL_GRAY63 );
	    if( number > 0 )
	       sprintf( path_number, "IN%d", number );
	    else
	       sprintf( path_number, "OUT%d", abs(number) );
	 }

	 ppr->DrawText( x, y-.02, path_number, TRUE, CENTER );
	 ppr->SetFgColour( BLACK );
      }
      else { // draw label for path
	 if( dependent_edge->PathLabel() && ( showing_labels || ((Empty *)dependent_edge)->display_label ) )
	    ppr->DrawText( x, y-.02, dependent_edge->PathLabel()->TextLabel(), FALSE, CENTER );
      }
      
      break;            
    
   case RESULT_FIG:
      
      if( ((Result *)dependent_edge)->Highlighted() )
	 ppr->SetFgColour( RED );
      ppr->SetLineWidth( 3 );
      path->GetLastControlPoints( &psl, &pl );
      theta = TangentAngle( psl, pl );
      xd = RESULT_BAR*sin(theta);
      yd = RESULT_BAR*cos(theta);
      ppr->DrawLine(  x-xd, y-yd, x+xd, y+yd );
      ppr->SetLineWidth( 2 );
      ppr->DrawText( x, y-.03, dependent_edge->HyperedgeName(), FALSE, Alignment() );
      
      break;
    
   case TIMER_FIG:
   
      ppr->SetFgColour( WHITE );
      ppr->DrawFilledCircle( x, y, TIMER_FIG_SIZE );
      ppr->SetFgColour( ((selected && !postscript_output) ? BLUE : BLACK ) );
      ppr->DrawCircle( x, y, TIMER_FIG_SIZE );
      ppr->DrawLine( x, y, x+TIMER_FIG_SIZE, y );
      ppr->DrawLine( x, y, x, y-TIMER_FIG_SIZE );
      ppr->DrawText( x, y-.02, dependent_edge->HyperedgeName(), FALSE, CENTER );
      break;
     
   }

   ppr->SetFgColour( BLACK );
}

void PointFigure::DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )
{
   float x, y, tw;

   GetPosition( x, y );

   tb = y - .035; // .035 = .02 offset + .15 text height
   bb = y + .02;

   tw = XfPresentation::DetermineWidth( dependent_edge->HyperedgeName() );

   switch( Alignment() ) {
   case CENTER:
      lb = x - tw/2;
      rb = x + tw/2;
      break;
   case LEFT_ALIGN:
      lb = x - .02;
      rb = x + tw;
      break;
   case RIGHT_ALIGN:
      lb = x - tw;
      rb = x + .02;
      break;
   }
}

alignment PointFigure::Alignment()
{
   alignment al;

   if( dependent_edge->LabelCentered() )
      al = CENTER;
   else {
      if( dependent_edge->EdgeType() == RESULT )
	 al = (path->ForwardDirection( this )) ? RIGHT_ALIGN : LEFT_ALIGN;
      else
	 al = (path->ForwardDirection( this )) ? LEFT_ALIGN : RIGHT_ALIGN;
   }

   return( al );
}

float PointFigure::TangentAngle( Point *psl, Point *pl )
{
   float X1, Y1, X2, Y2, theta;
   
   psl->GetPosition( X1, Y1 );
   pl->GetPosition( X2, Y2 );
	  
   if( (X1 - X2) == 0.0 )
      theta = PI/2.0;
   else 
      theta = -1.0*(atan2( Y1-Y2, X1-X2 ) + PI);

   return( theta );
}
