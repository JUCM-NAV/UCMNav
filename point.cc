/***********************************************************
 *
 * File:			point.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:        redone May 1997
 *
 ***********************************************************/

#include "point.h"
#include "math.h"

Point::Point( float fNew_x, float fNew_y )
{
   fX = fNew_x;
   fY = fNew_y;
}

Point::Point( Point& cpRhs )
{
   fX = cpRhs.fX;
   fY = cpRhs.fY;
}

void Point::SetPosition( float fNew_x, float fNew_y )
{
   fX = fNew_x;
   fY = fNew_y;
}

void Point::GetPosition( float& fReturn_x, float& fReturn_y )
{
   fReturn_x = fX;
   fReturn_y = fY;
}

float Point::operator*( Point& cpRhs )
{
   float fMy_x, fMy_y, fRhs_x, fRhs_y;

   GetPosition( fMy_x, fMy_y );
   cpRhs.GetPosition( fRhs_x, fRhs_y );

   return( fMy_x * fRhs_x + fMy_y * fRhs_y );
}

float Point::Distance( float x, float y )
{
   return( sqrt( pow((fX - x), 2) + pow((fY - y), 2) ) );
}


