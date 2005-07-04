/***********************************************************
 *
 * File:			presentation.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include <math.h>
#include "presentation.h"
#include "stub.h"

int Presentation::iInitialized = 0;
int Presentation::iNum_colours = 0;
int Presentation::MAX_COLOURS = 10;

Presentation::Presentation()
{
  if( !iInitialized ) {
    iInitialized = 1;
  }
}

void Presentation::TranslateCoordinates( float& X, float& Y, float Xp, float Yp, float Xc, float Yc, figure_orientation orientation )
{
   switch( orientation ) {

   case FORWARD_FIG: 

      X = Xp;
      Y = Yp;
      break;
      
   case BACKWARD_FIG:

      X = 2*Xc - Xp;
      Y = Yp;
      break;
      
   case UP_FIG:

      X = Xc + Yp - Yc;
      Y = Yc + Xc - Xp;
      break;
      
   case DOWN_FIG:

      X = Xc + Yc - Yp;
      Y = Yc + Xp - Xc;
      break;
   }
}

void Presentation::DrawLine( float fX1, float fY1, float fX2, float fY2, float Xc, float Yc, figure_orientation orientation )
{
   float x1, y1, x2, y2;

   TranslateCoordinates( x1, y1, fX1, fY1, Xc, Yc, orientation );
   TranslateCoordinates( x2, y2, fX2, fY2, Xc, Yc, orientation );

   this->DrawLine( x1, y1, x2, y2 );
}

void Presentation::DrawCircle( float fX, float fY, float fRadius,  float Xc, float Yc,
		    figure_orientation orientation, int iAngle_start, int iAngle_delta )
{
   float x, y;

   TranslateCoordinates( x, y, fX, fY, Xc, Yc, orientation );

   this->DrawCircle( x, y, fRadius, iAngle_start, iAngle_delta );
}

void Presentation::DrawFilledCircle( float fX, float fY, float fRadius,  float Xc, float Yc,
			  figure_orientation orientation, int iAngle_start, int iAngle_delta )
{
   float x, y;

   TranslateCoordinates( x, y, fX, fY, Xc, Yc, orientation );

   this->DrawFilledCircle( x, y, fRadius, iAngle_start, iAngle_delta );
}

void Presentation::DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear )
{
   float radius;
   
   radius = Min( fWidth, fHeight ); // calculate radius of curvature
   radius *= RADIUS_FACTOR;

   if( clear ) {
      this->SetFgColour( WHITE );  // clear object interiors

      float cx[5] = { fX, fX+fWidth-radius, fX+fWidth, fX+fWidth, fX };
      float cy[5] = { fY, fY, fY+radius, fY+fHeight, fY+fHeight };

      this->DrawFilledPoly( cx, cy, 5 );
   }
   
   this->SetFgColour(  colour ); // draw objects

   this->DrawLine( fX+radius, fY, fX+fWidth-radius, fY ); // draw top line
   this->DrawLine( fX+fWidth, fY+radius, fX+fWidth, fY+fHeight-radius ); // draw right line
   this->DrawLine( fX+radius, fY+fHeight, fX+fWidth-radius, fY+fHeight ); // draw bottom line
   this->DrawLine( fX, fY+radius, fX, fY+fHeight-radius ); // draw left line

   this->DrawCircle( fX+radius, fY+radius, radius, 90*64, 90*64 ); // draw top left quarter circle
   this->DrawCircle( fX+fWidth-radius, fY+radius, radius, 0, 90*64 ); // draw top right quarter circle
   this->DrawCircle( fX+radius, fY+fHeight-radius, radius, 180*64, 90*64 ); // draw bottom left quarter circle
   this->DrawCircle( fX+fWidth-radius, fY+fHeight-radius, radius, 270*64, 90*64 ); // draw bottom right quarter circle
}

void Presentation::DrawStub( float fX[], float fY[], int colour, Stub *stub )
{
   int i;
   float X2[4];
   
   if( !stub->IsShared() ) {
      this->ClearStubInterior( fX, fY );
      this->DrawStubOutline( fX, fY, colour );
   }
   else {
      for( i = 0; i < 4; i++ )
	 X2[i]= fX[i] - EMPTY_FIGURE_SIZE;
      
      for( i = 0; i < 4; i++ )
	 fX[i]+= EMPTY_FIGURE_SIZE;

      this->ClearStubInterior( X2, fY );
      this->ClearStubInterior( fX, fY );
      
      this->DrawStubOutline( X2, fY, colour );
      this->DrawStubOutline( fX, fY, colour );
   }
}

void Presentation::ClearStubInterior( float fX[], float fY[] )
{
   this->SetFgColour( WHITE );
   this->DrawFilledPoly( fX, fY, 4 );
}

void Presentation::DrawStubOutline( float fX[], float fY[], int colour )
{
   this->SetFgColour( colour );
   this->DrawPoly( fX, fY, 4 );
}

void Presentation::DrawSelectionHandles( float fX, float fY ) {}
