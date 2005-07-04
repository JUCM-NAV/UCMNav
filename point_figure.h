/***********************************************************
 *
 * File:			point_figure.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef POINT_FIGURE_H
#define POINT_FIGURE_H

#include "hyperedge_figure.h"

typedef enum { EMPTY_FIG, START_FIG, WAIT_FIG, RESULT_FIG, TIMER_FIG } point_type;

class Point;

class PointFigure : public HyperedgeFigure {

public:

   PointFigure( Hyperedge *edge, point_type type ); // constructor, includes data type for represented element
   ~PointFigure();
   virtual void Draw( Presentation *ppr ); // implements Draw method
   virtual void SetPosition( float f_nx, float f_ny, bool limit = TRUE, bool enclose = TRUE,
			     bool dependent_update = TRUE, bool interpolate = TRUE );  // sets position, notifies dependent paths
   virtual void PathChanged(); // notifies dependent paths of position change
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );

private:

   float TangentAngle( Point *p1, Point *p2 );
   alignment Alignment();
   
   point_type ptype;  // the type of hyperedge the figure represents

};

#endif
