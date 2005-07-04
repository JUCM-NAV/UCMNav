/***********************************************************
 *
 * File:			point.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:        redone May 1997
 *
 ***********************************************************/

#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <stdio.h>

class BSpline;

class Point {

public:

   Point( float fNew_x, float fNew_y );
   Point( Point& cpRhs );
   ~Point() {}

   float operator*( Point& cpRhs );
  
   void SetPosition( float fNew_x, float fNew_y );
   void GetPosition( float& fReturn_x, float& fReturn_y );

   float Distance( float x, float y );
   
private:

   float fX, fY;

};

#endif

