/***********************************************************
 *
 * File:			bspline.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef BSPLINE_H
#define BSPLINE_H

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xlib.h>

#include "point.h"
#include "presentation.h"
#include "collection.h"
#include "defines.h"

class Figure;

#define UNDEFINED_POINT 10
#define DEGREE 3

class Array {

public:

   Array() { size = 0; }
   void attachr( float new_value ) { values[size++] = new_value; }
   float getr() { return( values[size-1] ); }
   float getnth( int index ) { return( values[index] ); }
   int length() { return( size ); }
   void clear() { size = 0; }
   void First() { counter = 0; }
   bool IsDone() { return( counter == size ); }
   void Next() { counter++; }
   float CurrentItem() { return( values[counter] ); }
   
private:

   float values[100];
   int size, counter;
};

class BSpline {

public:
   friend std::ostream& operator<<(std::ostream& osStream, BSpline& bsRhs ) {
      int i;

      std::cout << "(";
      for( i=0; i < bsRhs.knot_vector.length(); i++ )
	 std::cout << bsRhs.knot_vector.getnth( i ) << " ";
      std::cout << ")\n";
      return( osStream );
   }
  
   BSpline( bool create = TRUE );
   virtual ~BSpline();
   void AddControlPoint( float fNew_x, float fNew_y );
   int FindControlPoint( float fCheck_x, float fCheck_y, float fTolerance );
   int GetKnotMultiplicity( double fKnot );
   BSpline* InsertKnot( double fKnot );

   void Interpolate();	       
   void DrawPath( Presentation *ppr );
   void HighlightPath( int new_id ) { highlight_id = new_id; }
   int HighlightColour() { return( highlight_colour ); }
   void HighlightColour( int new_colour ) { highlight_colour = new_colour; }

   int GetBezierControlPoints( Cltn<Point *> &lppPoints );
   void GetLastControlPoints( Point **pppSecond_last, Point **pppLast );
   float GetTangentAngle( Figure *ref_figure );
   
protected:

   Cltn<Point *> lControl_points;
   Cltn<Figure *> *figures;
   Array knot_vector;

   float fend_x, fend_y;
   bool changed;
   int highlight_id;

   static int highlight_colour, multiple_traversal_colour;

private:

   void Interpolate_bspline( float rgrgflData[][3], float rgrgflControl[][3], float rgflKnots[], int iNum );
   void Tridiag( float rgflA[], float rgflB[], float rgflC[], float rgflD[], float rgflX[], int n );
  
};

#endif
