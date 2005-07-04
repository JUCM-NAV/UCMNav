/***********************************************************
 *
 * File:			responsibility_figure.cc
 * Author:			Andrew Miga
 * Created:			August 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "resp_figure.h"
#include "resp_ref.h"
#include "presentation.h"
#include "xfpresentation.h"
#include "component.h"
#include <string.h>
#include <stdlib.h>

extern void UpdateResponsibilityList();

ResponsibilityFigure::ResponsibilityFigure( Hyperedge *edge ) : HyperedgeFigure( edge )
{
   erdDirection = RESP_UP;
   highlighted = FALSE;
}

ResponsibilityFigure::~ResponsibilityFigure()
{
   if( path ) path->PurgeFigure( this );
}

void ResponsibilityFigure::Draw( Presentation *ppr )
{

   float x, y;
   Responsibility *parent_resp = ((ResponsibilityReference *)dependent_edge)->ParentResponsibility();
   bool draw_cross = TRUE;
   alignment al;
   
   GetPosition( x, y );

   if( selected && !postscript_output ) {
      ppr->DrawSelectionHandles( x, y );
      ppr->SetFgColour( BLUE );
   }
   
   if( parent_resp->Highlight() )
      ppr->SetFgColour( RED );

   if( parent_resp != NULL ) {
      if( parent_resp->HasDynarrow() ) {
	 parent_resp->GetDynamicArrow()->Draw( ppr, this );
	 draw_cross = FALSE;
      }
   }
   
   if( draw_cross ) {

      ppr->SetLineWidth( 3 );
      ppr->DrawLine( x+RESP_BAR, y+RESP_BAR, x-RESP_BAR, y-RESP_BAR );
      ppr->DrawLine( x-RESP_BAR, y+RESP_BAR, x+RESP_BAR, y-RESP_BAR );
      ppr->SetLineWidth( 2 );
  
   }

   float fXoffset, fYoffset;
  
   switch( erdDirection ) {
   case RESP_UP:
      fXoffset = 0;
      fYoffset = -.017;
      al = CENTER;
      break;
   case RESP_DOWN:
      fXoffset = 0;
      fYoffset = .032;
      al = CENTER;
      break;
   case RESP_RIGHT:
      fXoffset = .02;
      fYoffset = .007;
      al = LEFT_ALIGN;
      break;
   case RESP_LEFT:
      fXoffset = -.02;
      fYoffset = .007;
      al = RIGHT_ALIGN;
      break;
   }

   ppr->DrawText( x+fXoffset, y+fYoffset, (( parent_resp ) ? parent_resp->Name() : "Unnamed" ), FALSE, al );
   ppr->SetFgColour( BLACK );
}

void ResponsibilityFigure::DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )
{
   float x, y, tw;

   Responsibility *parent_resp = ((ResponsibilityReference *)dependent_edge)->ParentResponsibility();

   GetPosition( x, y );
   tw = XfPresentation::DetermineWidth( parent_resp ? parent_resp->Name() : "Unnamed" );

   switch( erdDirection ) {
   case RESP_UP:
      tb = y - .032;
      bb = y + .02;
      lb = x - tw/2;
      rb = x + tw/2;
      break;
   case RESP_DOWN:
      tb = y - .02;
      bb = y + .032;
      lb = x - tw/2;
      rb = x + tw/2;
      break;
   case RESP_RIGHT:
      tb = y -.02;
      bb = y + .02;
      lb = x - .02;
      rb = x +.02 + tw;
      break;
   case RESP_LEFT:
      tb = y -.02;
      bb = y + .02;
      lb = x -.02 - tw;
      rb = x + .02;
      break;
   }
}
