/***********************************************************
 *
 * File:			path.cc
 * Author:			Andrew Miga
 * Created:			August 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "path.h"
#include "display.h"
#include "hyperedge.h"

int Path::number_paths = 0;

extern DisplayManager *display_manager;
extern float left_border, right_border, top_border, bottom_border;

Path::Path( DisplayManager *cman, bool create  ) : BSpline( create )
{

   display_manager = cman;
   
   path_end = NULL;
   path_start = NULL;
   display_manager->RegisterPath( this );

   path_number = number_paths++;
   destroyed = FALSE;

}

Path::~Path()
{
   while( !figures->is_empty() )
      figures->Detach();

   display_manager->PurgePath( this );

   path_start = NULL;
   path_end = NULL;
   display_manager = NULL;
   path_number = 0;
   load_number = 0;
}

void Path::AddFigure( Figure *new_figure ) // adds first figure to Path
{
   figures->Add( new_figure );  
   changed = TRUE;
}

void Path::PurgeFigure( Figure *figure )
{
   if( figures->Includes( figure ) )
      figures->Remove( figure );

   if( figures->Size() == 0 )
      destroyed = TRUE;
   changed = TRUE;
}

void Path::AddBeforeFigure( Figure *new_figure, Figure *ref_figure )
{
   figures->AddBefore( new_figure, ref_figure );
   changed = TRUE;
}


void Path::AddAfterFigure( Figure *new_figure, Figure *ref_figure )
{
   figures->AddAfter( new_figure, ref_figure );
   changed = TRUE;
}

void Path::AddStartFigure( Figure *new_figure )
{
   figures->attachl( new_figure );
   changed = TRUE;
}

void Path::DeleteEndFigure()
{
   delete figures->GetLast();
  
   if( !figures->Size() )
      delete this;
   changed = TRUE;
}

void Path::DeleteStartFigure()
{
   delete figures->GetFirst();
  
   if( !figures->Size() )
      delete this;
   changed = TRUE;
}

void Path::AddEndFigure( Figure *new_figure )
{
   figures->attachr( new_figure );
   changed = TRUE;
}

bool Path::GetCurveEnd( float &fx, float &fy )
{
   if( fend_x == UNDEFINED_POINT || fend_y == UNDEFINED_POINT)
      return FALSE;
   else {
      fx = fend_x;
      fy = fend_y;
      return TRUE;
   }
}

Figure * Path::BoundaryFigure( path_direction dir )
{
   if( dir == INPUT )
      return( figures->Get( figures->Size() - 1 ) );
   else
      return( figures->Get( 2 ) );
}

bool Path::IsVisible()
{
   float x, y, min_y = 2.0, max_y = 0.0, min_x = 2.0, max_x = 0.0;

   // determine if any part of the spline bounding box is visible on the screen given the scrolling positions
   for( figures->First(); !figures->IsDone(); figures->Next() ) {
      figures->CurrentItem()->GetPosition( x, y );
      if( x < min_x ) min_x = x;
      if( x > max_x ) max_x = x;
      if( y < min_y ) min_y = y;
      if( y > max_y ) max_y = y;
   }

   if( (max_x < left_border) || (min_x > right_border) || ( max_y < top_border) || (min_y > bottom_border)  )
      return( FALSE );
   else
      return( TRUE );
}

bool Path::ForwardDirection( Figure *figure )
{
   float xf, yf, xs, ys;
   Figure *prev_figure = NULL, *first_figure, *second_figure;

   for( figures->First(); !figures->IsDone(); figures->Next() ) {
      if( figures->CurrentItem() == figure ) {
	 if( prev_figure == NULL ) { // figure is start of path
	    first_figure = figure;
	    second_figure = figures->NextItem();
	 }
	 else {
	    first_figure = prev_figure;
	    second_figure = figure;
	 }
	 break;
      }
      prev_figure = figures->CurrentItem();
   }
	       
   first_figure->GetPosition( xf, yf );
   second_figure->GetPosition( xs, ys );

   return( (xs > xf) ? TRUE : FALSE );
}

