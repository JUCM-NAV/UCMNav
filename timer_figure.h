/***********************************************************
 *
 * File:			timer_figure.h
 * Author:			Andrew Miga
 * Created:			October 2001
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef TIMER_FIGURE_H
#define TIMER_FIGURE_H

#include "hyperedge_figure.h"

class TimerFigure : public HyperedgeFigure {

public:

   TimerFigure( Hyperedge *edge, bool normal_operation = TRUE );
   ~TimerFigure();

   void ReplaceDependentPath( Path *new_path, Path *old_path ); // replaces a dependent path with a new one
   virtual void SetPosition( float f_nx, float f_ny, bool limit = TRUE, bool enclose = TRUE, // sets position, notifies dependent paths
			     bool dependent_update = TRUE, bool interpolate = TRUE );
   virtual void SetPath( Path *new_path, bool normal_operation ) {}
   virtual void PathChanged(); // notifies dependent paths of position change
   void Draw( Presentation *ppr ); // implements Draw method
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );

private:

   Path *output_path, *timeout_path; // list of paths which start or terminate at the stub

};

#endif
