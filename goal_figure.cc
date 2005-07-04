/***********************************************************
 *
 * File:			goal_figure.cc
 * Author:			Andrew Miga
 * Created:			October 1998
 *
 * Modification history:
 *
 ***********************************************************/

#include "goal_figure.h"
#include "goal_tag.h"
#include "presentation.h"
#include "xfpresentation.h"

#define GOAL_WIDTH 0.01
#define GOAL_HEIGHT 0.012
#define GOAL_W2 0.003333
#define GOAL_H2 0.005333

GoalFigure::GoalFigure( Hyperedge *edge, bool dialog ) : HyperedgeFigure( edge )
{
   highlighted = FALSE;
   if( dialog )
      ((GoalTag *)edge)->Name( fl_show_input( "Enter the name for the goal tag", "" ) );
}

GoalFigure::~GoalFigure()
{
   if( path ) path->PurgeFigure( this );
}

void GoalFigure::Draw( Presentation *ppr )
{
   float x, y;

   GetPosition( x, y );

   // draw outline of square for goal
   float XCoord[5] = { x-GOAL_WIDTH, x+GOAL_WIDTH, x+GOAL_WIDTH, x-GOAL_WIDTH, x-GOAL_WIDTH };
   float YCoord[5] = { y+GOAL_HEIGHT, y+GOAL_HEIGHT, y-GOAL_HEIGHT, y-GOAL_HEIGHT, y+GOAL_HEIGHT };
   float XCoord2[5] = { x-GOAL_W2, x+GOAL_W2, x+GOAL_W2, x-GOAL_W2, x-GOAL_W2 };
   float YCoord2[5] = { y+GOAL_H2, y+GOAL_H2, y-GOAL_H2, y-GOAL_H2, y+GOAL_H2 };
   
   if( selected && !postscript_output ) {
      ppr->DrawSelectionHandles( x, y );
      ppr->SetFgColour( BLUE );
   }

   ppr->DrawFilledPoly( XCoord, YCoord, 5 );
   ppr->SetFgColour( WHITE );
   ppr->DrawFilledPoly( XCoord2, YCoord2, 5 );
   ppr->SetFgColour( BLACK );

   char *name = ((GoalTag *)dependent_edge)->Name();
   if( strcmp( name, "" ) != 0 )
      ppr->DrawText( x, y-2*GOAL_WIDTH, name, FALSE, CENTER );
}

void GoalFigure::DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )
{
   float x, y, tw;
   
   GetPosition( x, y );
   tw = XfPresentation::DetermineWidth( ((GoalTag *)dependent_edge)->Name() );

   lb = x - tw/2;
   rb = x + tw/2;
   tb = y - .035;
   bb = y + .02;
}
