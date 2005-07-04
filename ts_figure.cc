/***********************************************************
 *
 * File:			ts_figure.cc
 * Author:			Andrew Miga
 * Created:			October 1997
 *
 * Modification history:
 *
 ***********************************************************/

#include "ts_figure.h"
#include "presentation.h"
#include "xfpresentation.h"
#include "response_time.h"

extern void EditTimestampDescription( TimestampFigure *new_timestamp );

#define TIMESTAMP_HEIGHT  0.025
#define TIMESTAMP_WIDTH  0.01

TimestampFigure::TimestampFigure( Hyperedge *edge, bool normal_operation ) : HyperedgeFigure( edge )
{
   highlighted = FALSE;
   
   if( normal_operation ) {
      direction = UP;
      EditTimestampDescription( this );
   }    
}

TimestampFigure::~TimestampFigure()
{
   if( path ) path->PurgeFigure( this );
}

void TimestampFigure::Draw( Presentation *ppr )
{
   float x, y, fXoffset, fYoffset, XCoord[4], YCoord[4];
   alignment al;
   
   ResponseTimeManager *rtm = ResponseTimeManager::Instance();

   GetPosition( x, y );

   if( selected && !postscript_output ) {
      ppr->DrawSelectionHandles( x, y );
      ppr->SetFgColour( BLUE );
   }
   else if( highlighted && !postscript_output )
      ppr->SetFgColour( FL_DARKCYAN );

   // draw iscosceles triangle pointing into path

   switch( direction )
   {
   case UP:

      fXoffset = 0;
      fYoffset = -.035;
      al = CENTER;
      XCoord[0] = x-TIMESTAMP_WIDTH;
      XCoord[1] = x;
      XCoord[2] = x+TIMESTAMP_WIDTH;
      XCoord[3] = x-TIMESTAMP_WIDTH;
      YCoord[0] = y-TIMESTAMP_HEIGHT;
      YCoord[1] = y;
      YCoord[2] = y-TIMESTAMP_HEIGHT;
      YCoord[3] = y-TIMESTAMP_HEIGHT;
      break;

   case DOWN:

      fXoffset = 0;
      fYoffset = .05;
      al = CENTER;
      XCoord[0] = x-TIMESTAMP_WIDTH;
      XCoord[1] = x+TIMESTAMP_WIDTH;
      XCoord[2] = x;
      XCoord[3] = x-TIMESTAMP_WIDTH;
      YCoord[0] = y+TIMESTAMP_HEIGHT;
      YCoord[1] = y+TIMESTAMP_HEIGHT;
      YCoord[2] = y;
      YCoord[3] = y+TIMESTAMP_HEIGHT;
      break;

   case RIGHT:

      fXoffset = .035;
      fYoffset = .01;
      al = LEFT_ALIGN;
      XCoord[0] = x;
      XCoord[1] = x+TIMESTAMP_HEIGHT;
      XCoord[2] = x+TIMESTAMP_HEIGHT;
      XCoord[3] = x;
      YCoord[0] = y;
      YCoord[1] = y+TIMESTAMP_WIDTH;
      YCoord[2] = y-TIMESTAMP_WIDTH; 
      YCoord[3] = y;
      break;

   case LEFT:

      fXoffset = -.035;
      fYoffset = .01;
      al = RIGHT_ALIGN;
      XCoord[0] = x;
      XCoord[1] = x-TIMESTAMP_HEIGHT;
      XCoord[2] = x-TIMESTAMP_HEIGHT;
      XCoord[3] = x;
      YCoord[0] = y;
      YCoord[1] = y+TIMESTAMP_WIDTH;
      YCoord[2] = y-TIMESTAMP_WIDTH;
      YCoord[3] = y;
      break;
   }

   if( rtm->ResponseTimesExist( (Timestamp *)dependent_edge ) )
      ppr->DrawFilledPoly( XCoord, YCoord, 4 );
   else
      ppr->DrawPoly( XCoord, YCoord, 4 );

   // draw timestamp label
   
   if( selected || highlighted )
      ppr->SetFgColour( BLACK );

   char *name = ((Timestamp *)dependent_edge)->Name();
   if( strcmp( name, "" ) != 0 )
      ppr->DrawText( x+fXoffset, y+fYoffset, name, FALSE, al );

}

void TimestampFigure::DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )
{
   float x, y, tw;
   
   GetPosition( x, y );
   tw = XfPresentation::DetermineWidth( ((Timestamp *)dependent_edge)->Name() );

   switch( direction ) {

   case UP:
      
      lb = x - tw/2;
      rb = x + tw/2;
      tb = y - .05;
      bb = y;
      break;

   case DOWN:
      
      lb = x - tw/2;
      rb = x + tw/2;
      tb = y;
      bb = y + .05;      
      break;

   case RIGHT:

      lb = x + .035 + tw;
      rb = x - .02;
      tb = y - .02;
      bb = y + .02;
      break;

   case LEFT:

      lb = x - .035 - tw;
      rb = x + .02;
      tb = y - .02;
      bb = y + .02;
      break;
   }

}

void TimestampFigure::Direction( int dir )
{
   timestamp_direction directions[4] = { RIGHT, UP, LEFT, DOWN };
   direction = directions[dir/90];
}
