/***********************************************************
 *
 * File:			loop_null_figure.h
 * Author:			Andrew Miga
 * Created:			April 2000
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef LOOP_NULL_FIGURE_H
#define LOOP_NULL_FIGURE_H

#include "figure.h"

typedef enum { MAIN_OUT, MAIN_IN, LOOP_OUT, LOOP_IN } loop_paths;

class LoopFigure;

class LoopNullFigure : public Figure {

public:

   LoopNullFigure( LoopFigure *mp_figure, Path *new_path, loop_paths lpath ); // constructor, includes pointer to parent figure and flag for direction
   ~LoopNullFigure();

   virtual void GetPosition( float& f_rx, float& f_ry ); // returns position at which spline should begin/end, queries parent figure

private:

   LoopFigure *parent_figure; // the parent LoopFigure to which this null figure is registered
   loop_paths loop_path; // enumeration of four possible paths
};

#endif
