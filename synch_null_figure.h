/***********************************************************
 *
 * File:			synch_null_figure.h
 * Author:			Andrew Miga
 * Created:			December 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef SYNCH_NULL_FIGURE_H
#define SYNCH_NULL_FIGURE_H

#include "figure.h"

class SynchronizationFigure;

class SynchNullFigure : public Figure {

   
public:

   SynchNullFigure( SynchronizationFigure *mp_figure, Path *new_path, path_direction dir, bool normal_operation = TRUE );
   // constructor, includes pointer to parent figure and flag for direction
   ~SynchNullFigure();

   virtual void GetPosition( float& f_rx, float& f_ry ); // returns position at which spline should begin/end, queries parent figure

   int Ordinal() { return( ordinal ); } // access methods for path ordinal value
   void Ordinal( int new_ordinal ) { ordinal = new_ordinal; } 
   
private:

   SynchronizationFigure *parent_figure; // the parent SynchronizationFigure to which this null figure is registered
   path_direction path_dir; // flag referring to whether path terminated by this figure enters or leaves a synchronization
   int ordinal; // path ordinal value, i.e. index of path in fork/join   
};

#endif

