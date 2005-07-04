/***********************************************************
 *
 * File:			or_figure.h
 * Author:			Andrew Miga
 * Created:			September 1996
 *
 * Modification history:        June 1997
 *                              changed to multipath
 *                              July 1997
 *
 ***********************************************************/

#ifndef OR_FIGURE_H
#define OR_FIGURE_H

#include "hyperedge_figure.h"
#include "or_null_figure.h"

class OrFigure : public HyperedgeFigure {

public:

   OrFigure( Hyperedge *edge );  // constructor, contains pointer of dependent hyperedge
   ~OrFigure();

   virtual void SetPosition( float f_nx, float f_ny, bool limit = TRUE, bool enclose = TRUE,  // implementation of polymorphic position setting method
			     bool dependent_update = TRUE, bool interpolate = TRUE ); // sets position and notifies dependent paths
   virtual void PathChanged(); // notifies dependent paths of position change
   void ChangeOrientation(); // changes the orientation ( forward, backward, upward, downward )
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );
   virtual void Draw( Presentation *ppr );  // implementation of Draw method for or figures
   virtual void OrderPaths() { ReorderPaths(); }
   
   void AddDependent( OrNullFigure *new_null, bool normal_operation = TRUE ); // adds a terminating null figure of a path as a dependent, returns path order
   void RemoveDependent( OrNullFigure *null ); // removes a null figure as a dependent
   void ReorderPaths();
   void GetNullPosition( int ordinal, float& f_rx, float& f_ry ); // returns the position at which a null figure with path ordering ordinal should be drawn
   figure_orientation Orientation() { return( orientation ); }    // returns the figure's orientation
   void Orientation( figure_orientation orient ) { orientation = orient; } // set method for figure orientation
   
protected:

   Cltn<OrNullFigure *> *fork_paths; // list of dependent null figures ( terminations of paths entering or leaving join/fork )
   figure_orientation orientation; // the orientation of the figure ( forward/backward/upward/downward )
   bool deleted; // flag set when figure is deleted
   
};

#endif
