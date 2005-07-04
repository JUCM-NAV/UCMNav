/***********************************************************
 *
 * File:			figure.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef FIGURE_H
#define FIGURE_H

#include "defines.h"
#include "path.h"
#include "collection.h"
#include <stdio.h>

class Path;

class Figure {

public:

   Figure();
   virtual ~Figure();

   Path *GetPath() {  return( path ); }  // access method for path object of which figure is part
   int GetNumber() { return( figure_number ); }  // returns integer identifier for object

   virtual void GetPosition( float& f_rx, float& f_ry )=0; // polymorphic method which returns coordinates at which figure is drawn
   virtual void SetPath( Path *new_path, bool  normal_operation = TRUE );  // polymorphic method for changing the figure's path object
   virtual void PathChanged(); // polymorphic method for notifying dependent splines that they are out of date
   void SetPosition (float f_nx, float f_ny ) { fX = f_nx; fY = f_ny; }

   static void ResetFigureCount() { number_figures = 0; }  // resets global count of figure objects

protected:
   
   float fX, fY;  // internal position coordinates
   int figure_number, load_number; // current and past integer identifiers for object
   Path *path; // path object of which this figure is a part

   static int number_figures; // global count of number of figure objects

};

#endif
