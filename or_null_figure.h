/***********************************************************
 *
 * File:			or_null_figure.h
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef OR_NULL_FIGURE_H
#define OR_NULL_FIGURE_H

#include "figure.h"

class OrFigure;

class OrNullFigure : public Figure {

public:

   OrNullFigure( OrFigure *or_figure, Path *new_path, bool normal_operation = TRUE ); // constructor, includes pointer to parent figure
   ~OrNullFigure();
   
   virtual void GetPosition( float& f_rx, float& f_ry ); // returns position at which spline should begin/end, queries parent figure
   
   int Ordinal() { return( ordinal ); } // access methods for path ordinal value
   void Ordinal( int new_ordinal ) { ordinal = new_ordinal; }

private:

   OrFigure *parent_figure; // the parent OrFigure to which this null figure is registered
   int ordinal; // path ordinal value, i.e. index of path in fork/join

};

#endif

