/***********************************************************
 *
 * File:			loop_figure.h
 * Author:			Andrew Miga
 * Created:			April 2000
 *
 * Modification history:
 ***********************************************************/

#ifndef LOOP_FIGURE_H
#define LOOP_FIGURE_H

#include "hyperedge_figure.h"
#include "loop_null_figure.h"

typedef enum { FORWARD_UP, FORWARD_DOWN, BACKWARD_UP, BACKWARD_DOWN, UPWARD_LEFT, UPWARD_RIGHT, DOWNWARD_LEFT, DOWNWARD_RIGHT } loop_orientation;

class LoopFigure : public HyperedgeFigure {

public:

   LoopFigure( Hyperedge *edge, const char *new_orientation = NULL );  // constructor, contains pointer of dependent hyperedge
   ~LoopFigure() {}
   
   virtual void Draw( Presentation *ppr );  // implementation of Draw method for or figures
   virtual void SetPosition( float f_nx, float f_ny, bool limit = TRUE, bool enclose = TRUE,  // implementation of polymorphic position setting method
			     bool dependent_update = TRUE, bool interpolate = TRUE ); // sets position and notifies dependent paths
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );
   virtual void PathChanged(); // notifies dependent paths of position change

   void ChangeOrientation(); // changes the orientation ( horizontal-up, horizontal-down, vertical-up, vertical-down )
   void AddDependent( LoopNullFigure *new_null, loop_paths lpath ) { loop_splines[lpath] = new_null; }
   void GetNullPosition( loop_paths lpath, float& f_rx, float& f_ry );
   const char * Orientation();

protected:

   LoopNullFigure *loop_splines[4]; // list of dependent null figures ( terminations of paths entering or leaving join/fork )
   loop_orientation orientation; // the orientation of the figure ( forward/backward/upward/downward )   
};

#endif

