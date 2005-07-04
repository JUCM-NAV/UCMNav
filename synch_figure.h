/***********************************************************
 *
 * File:			synch_figure.h
 * Author:			Andrew Miga
 * Created:			December 1996
 *
 * Modification history:        July 1997
 *
 ***********************************************************/

#ifndef SYNCHRONIZATION_FIGURE_H
#define SYNCHRONIZATION_FIGURE_H

#include "hyperedge_figure.h"
#include "synch_null_figure.h"
#include "display.h"

class SynchronizationFigure : public HyperedgeFigure {

   friend class DisplayManager;
  
public:

   SynchronizationFigure( Hyperedge *edge );
   ~SynchronizationFigure();

   virtual void SetPosition( float f_nx, float f_ny, bool limit = TRUE, bool enclose = TRUE,
			     bool dependent_update = TRUE, bool interpolate = TRUE );  // sets position, notifies dependent paths
   virtual void PathChanged(); // notifies dependent paths of position change
   void ChangeOrientation(); // changes the orientation from forward to backward and vice versa
   virtual void Draw( Presentation *ppr ); // implements Draw method
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );
   virtual void OrderPaths();  // orders input/output paths of forks and joins

   void AddDependent( SynchNullFigure *new_null, path_direction dir, bool normal_operation = TRUE ); // add/remove dependent placeholder figures as dependents
   void RemoveDependent( SynchNullFigure *null, path_direction dir );
   void GetNullPosition( path_direction dir, int ordinal, float& f_rx, float& f_ry );
   // returns the position at which a null figure with path ordering ordinal and input/output flag should be drawn
   figure_orientation Orientation() { return( orientation ); }    // returns the figure's orientation
   void Orientation( figure_orientation orient ) { orientation = orient; } // set method for figure orientation
   void ReorderPaths( path_direction dir );
   
private:

   Cltn<SynchNullFigure *> *input_paths;  // collection of placeholder figures for input paths
   Cltn<SynchNullFigure *> *output_paths;  // collection of placeholder figures for output paths
   figure_orientation orientation; // the orientation of the figure, forward/backward
   float top, bottom;  // coordinates for the top and bottom of the bar
   bool deleted;   // flag set when element has been deleted   
};

#endif
