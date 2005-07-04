/***********************************************************
 *
 * File:			hyperedge_figure.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "hyperedge_figure.h"
#include "hyperedge.h"
#include "display.h"
#include "path.h"
#include "component.h"
#include "component_mgr.h"
#include <math.h>
#include <stdlib.h>

extern ComponentManager *component_manager;
extern DisplayManager *display_manager;
extern Figure *active_figure;
extern float left_border, right_border, top_border, bottom_border;
extern void DisplayMapDescription();

#define BOX_SIZE 0.08

HyperedgeFigure::HyperedgeFigure( Hyperedge *edge ) : Figure()
{ 
   dependent_edge = edge;
   selected = FALSE;
   dependent_figure = NULL;
   component = NULL;
   display_manager->RegisterFigure( this );
   figure_number = number_figures++;
}

HyperedgeFigure::~HyperedgeFigure()
{
   display_manager->PurgeFigure( this );
   if( component ) component->PurgeDependentFigure( this );
   if( dependent_figure ) dependent_figure->DependentFigure( NULL );
   if( active_figure == this ) {
      active_figure = NULL;
      DisplayMapDescription();
   }
}

bool HyperedgeFigure::IsResponsibility()
{
   return( FALSE );
}

void HyperedgeFigure::GetPosition( float &f_rx, float &f_ry )
{ 
   float fComponent_x, fComponent_y;

   if( component )
   {
      component->GetPosition( fComponent_x, fComponent_y );
      f_rx = fComponent_x + fX*component->GetWidth();
      f_ry = fComponent_y + fY*component->GetHeight();
   } else {
      f_rx = fX;
      f_ry = fY;
   }
}

void HyperedgeFigure::SetPosition( float f_nx, float f_ny, bool limit, bool enclose,
			  bool dependent_update, bool interpolate )
{ 

   ComponentReference *new_component;
   float fprev_x, fprev_y, fdiff_x, fdiff_y, fdx, fdy;

   if( (dependent_figure != NULL) && (dependent_update == TRUE) )
      GetPosition( fprev_x, fprev_y );

   if( enclose ) {
      new_component = component_manager->FindEnclosingComponent( f_nx, f_ny );
    
      if( new_component != component ) {
	 if( component )
	    component->PurgeDependentFigure( this );
	 if( new_component )
	    new_component->RegisterDependentFigure( this );

	 component = new_component;
      } 
   }
  
   if( component ) {
      float fComponent_x, fComponent_y;
      component->GetPosition( fComponent_x, fComponent_y );
      fX = (f_nx - fComponent_x)/component->GetWidth();
      fY = (f_ny - fComponent_y)/component->GetHeight();
   }
   else {
      if( ((f_nx < WINDOW_MIN) || (f_nx > WINDOW_MAX)) && limit )	// avoids objects being drawn off the screen
	 fX = ((f_nx > WINDOW_MAX) ? WINDOW_MAX : WINDOW_MIN);	// especially result bars
      else
	 fX = f_nx;
    
      if( ((f_ny < WINDOW_MIN) || (f_ny > WINDOW_MAX)) && limit )
	 fY = ((f_ny > WINDOW_MAX) ? WINDOW_MAX : WINDOW_MIN);	
      else
	 fY = f_ny;
   }

   if( (dependent_figure != NULL) && (dependent_update == TRUE) ) { 
      float cx, cy;
      GetPosition( cx, cy );

      fdiff_x = cx - fprev_x;
      fdiff_y = cy - fprev_y;
      dependent_figure->GetPosition( fdx, fdy );

      dependent_figure->SetPosition( (fdx + fdiff_x), (fdy + fdiff_y),
				     TRUE, TRUE, FALSE );
      dependent_figure->PathChanged();
   }

   if( interpolate && path )
      path->PathChanged();
   
}

ComponentReference * HyperedgeFigure::GetContainingComponent()
{
   if( component )
      return( component ); // default implementation
   else
      return( component_manager->FindEnclosingComponent( fX, fY, dependent_edge->ParentMap() ) );
}

Component * HyperedgeFigure::BoundComponent()
{
   return( component ? component->ReferencedComponent() : NULL );
}

void HyperedgeFigure::ValidatePositionShift( float x_offset, float y_offset, float& x_limit, float& y_limit )
{
   float x, y, new_x, new_y;

   x_limit = 0; // initialize to zero in case function returns early
   y_limit = 0;

   if( component ) {
      if( component->Selected() ) // return immediately to avoid shifting
	 return;                  // multiple times
   }

   GetPosition( x, y );
   new_x = x+x_offset;
   new_y = y+y_offset;
   
   if( new_x < WINDOW_MIN )
      x_limit = WINDOW_MIN-x_offset-x;
   else if( new_x > WINDOW_MAX )
      x_limit = x_offset-(WINDOW_MAX-x);
   else
      x_limit = 0;
   
   if( new_y < WINDOW_MIN )
      y_limit = WINDOW_MIN-y_offset-y;
   else if( new_y > WINDOW_MAX )
      y_limit = y_offset-(WINDOW_MAX-y);
   else
      y_limit = 0;

}

void HyperedgeFigure::ShiftPosition( float x_offset, float y_offset )
{
   float x, y;
   bool dependent_update = TRUE;

   if( component ) {
      if( component->Selected() ) // return immediately to avoid shifting
	 return;                  // multiple times
   }

   GetPosition( x, y );
   x += x_offset;
   y += y_offset;
   if( dependent_figure ) {
      if( dependent_figure->IsSelected() )
	 dependent_update = FALSE;
   }
   SetPosition( x, y, TRUE, FALSE, dependent_update );
}

void HyperedgeFigure::ResetEnclosingComponent()
{
   float x, y;

   GetPosition( x, y );

   fX = x;
   fY = y;
   
   component = NULL;
}

void HyperedgeFigure::BindComponents()
{
   float x, y;

   GetPosition( x, y );
   SetPosition( x, y );
}

bool HyperedgeFigure::IsVisible()
{
   float x, y;

   GetPosition( x, y );

   if( ((((x-BOX_SIZE) > left_border) && ((x-BOX_SIZE) < right_border)) || (((x+BOX_SIZE) > left_border) && ((x+BOX_SIZE) < right_border))) &&
       ((((y-BOX_SIZE) > top_border) && ((y-BOX_SIZE) < bottom_border)) || (((y+BOX_SIZE) > top_border) && ((y+BOX_SIZE) < bottom_border))) )
      return( TRUE );
   else
      return( FALSE );
}

void HyperedgeFigure::OrderPaths() {}

