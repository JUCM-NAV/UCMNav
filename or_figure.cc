/***********************************************************
 *
 * File:			or_figure.cc
 * Author:			Andrew Miga
 * Created:			September 1996
 *
 * Modification history:        Redone July 1997
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "or_figure.h"
#include "presentation.h"
#include "hyperedge.h"
#include "or_fork.h"
#include "path.h"

extern bool paths_complete;
extern FD_Main *pfdmMain;

OrFigure::OrFigure( Hyperedge *edge ) : HyperedgeFigure( edge )
{
   fork_paths = new Cltn<OrNullFigure *>;
   orientation = FORWARD_FIG;
   deleted = FALSE;
}

OrFigure::~OrFigure()
{
   if( path ) path->PurgeFigure( this );
   deleted = TRUE;

   while( !fork_paths->is_empty() )
      delete fork_paths->Detach();
   delete fork_paths;
}

void OrFigure::AddDependent( OrNullFigure *new_null, bool normal_operation )
{
   if( fork_paths->Includes( new_null ) )
      std::cerr << "\nError: Null figure already included.\n";
   else
      fork_paths->Add( new_null );

   if( normal_operation ) {
      ReorderPaths();
      for( fork_paths->First(); !fork_paths->IsDone(); fork_paths->Next() )
	 fork_paths->CurrentItem()->GetPath()->PathChanged();
   }
}

void OrFigure::RemoveDependent( OrNullFigure *null )
{
   if( deleted ) return;  // if the figure was deleted don't bother deregistering
   
   if( fork_paths->Includes( null ) )
      fork_paths->Remove( null );
   else
      std::cerr << "\nError: Null figure already removed.\n";

   ReorderPaths();
   for( fork_paths->First(); !fork_paths->IsDone(); fork_paths->Next() )
      fork_paths->CurrentItem()->GetPath()->PathChanged();
}

void OrFigure::ReorderPaths()
{
   float x, y, min_coord, old_min = 0.0, *oc;
   OrNullFigure *current_null, *minimum_null = NULL;
   if( !paths_complete ) return;
   int i = 1,
      path_count = fork_paths->Size();
   path_direction dir = ((dependent_edge->EdgeType() == OR_FORK) ? OUTPUT : INPUT );
   bool reordered = FALSE;

   if( (orientation == FORWARD_FIG) || (orientation == BACKWARD_FIG) ) // determine ordering coordinate
      oc = &y; // for horizontal forks order on y positions
   else
      oc = &x; // for vertical forks order on x positions
   
   while( i <= path_count ) {
      min_coord = 10.0;
      for( fork_paths->First(); !fork_paths->IsDone(); fork_paths->Next() ) {
	 current_null = fork_paths->CurrentItem();
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
      for( fork_paths->First(); !fork_paths->IsDone(); fork_paths->Next() )
	 fork_paths->CurrentItem()->GetPath()->PathChanged();
   }
}

void OrFigure::SetPosition( float f_nx, float f_ny, bool limit,  bool enclose,
				   bool dependent_update, bool interpolate )
{
   HyperedgeFigure::SetPosition( f_nx, f_ny );

   for( fork_paths->First(); !fork_paths->IsDone(); fork_paths->Next() )
      fork_paths->CurrentItem()->GetPath()->PathChanged();
}

void OrFigure::PathChanged()
{
   path->PathChanged();
   for( fork_paths->First(); !fork_paths->IsDone(); fork_paths->Next() )
      fork_paths->CurrentItem()->GetPath()->PathChanged();
}

void OrFigure::Draw( Presentation *ppr )
{ 
   float x, y, step;
   int i, num_paths = fork_paths->Size();
   figure_orientation draw_orientation;
   
   GetPosition( x, y );

   // flip direction of joins as meanings of names will be different
   draw_orientation = (dependent_edge->EdgeType() == OR_JOIN) ? (orientation+180)%360 : orientation;

   if( selected && !postscript_output ) {
      ppr->DrawSelectionHandles( x, y );
      ppr->SetFgColour( BLUE );
   }

   if( dependent_edge->EdgeType() == OR_FORK ) {
      if( ((OrFork *)dependent_edge)->Highlight() )
	 ppr->SetFgColour( RED );
   }
   
   step = (2*OR_FORK_PATH)/(num_paths-1);

   for( i = 0; i < num_paths; i++ )
      ppr->DrawLine( x, y, x+OR_FORK_PATH, y-OR_FORK_PATH+(i*step), x, y, draw_orientation );
  
   // draw name if it is defined

   if( dependent_edge->HasName() ) {

      alignment al;
      float xoffset, yoffset;

      switch( orientation ) {

      case FORWARD_FIG: 
	 xoffset = -.01;
	 yoffset = -.015;
	 al = RIGHT_ALIGN;
	 break;
      
      case BACKWARD_FIG:
	 xoffset = .01;
	 yoffset = -.015;
	 al = LEFT_ALIGN;
	 break;
      
      case UP_FIG:
	 xoffset = -.01;
	 yoffset = .015;
	 al = RIGHT_ALIGN;
	 break;
      
      case DOWN_FIG:
	 xoffset = -.01;
	 yoffset = -.005;
	 al = RIGHT_ALIGN;
	 break;
      }

      ppr->DrawText( x+xoffset, y+yoffset, dependent_edge->HyperedgeName(), FALSE, al );
   }

   ppr->SetFgColour( BLACK );
}

void OrFigure::DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )
{
   float x, y;
   
   GetPosition( x, y );

   lb = x - .03;
   rb = x + .03;
   tb = y - .03;
   bb = y + .03;
}

void OrFigure::GetNullPosition( int ordinal, float& f_rx, float& f_ry )
{
   float x, y, xt, yt, step;
   figure_orientation draw_orientation;

   GetPosition( x, y );
   
   // flip direction of joins as meanings of names will be different
   draw_orientation = (dependent_edge->EdgeType() == OR_JOIN) ? (orientation+180)%360 : orientation;
   step = (2*OR_FORK_PATH)/(fork_paths->Size()-1);

   if( draw_orientation == DOWN_FIG ) ordinal = fork_paths->Size() - ordinal + 1;
   
   Presentation::TranslateCoordinates( xt, yt, x+OR_FORK_PATH, y-OR_FORK_PATH+((ordinal-1)*step), x, y, draw_orientation );

   f_rx = xt;
   f_ry = yt;
}

void OrFigure::ChangeOrientation()
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
