/***********************************************************
 *
 * File:			stub_figure.h
 * Author:			Andrew Miga
 * Created:			August 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef STUB_FIGURE_H
#define STUB_FIGURE_H

#include "hyperedge_figure.h"

class StubFigure : public HyperedgeFigure {

public:

   StubFigure( Hyperedge *edge, bool normal_operation = TRUE );
   ~StubFigure();

   Cltn<Path *> *GetCurves() {  return( stub_paths ); } // returns list of paths which start or terminate at the stub
   void AddDependentPath( Path *new_path ) { stub_paths->Add( new_path ); } // adds a new dependent path
   void ReplaceDependentPath( Path *new_path, Path *old_path ); // replaces a dependent path with a new one
   virtual void SetPosition( float f_nx, float f_ny, bool limit = TRUE, bool enclose = TRUE, // sets position, notifies dependent paths
			     bool dependent_update = TRUE, bool interpolate = TRUE );
   virtual void SetPath( Path *new_path, bool normal_operation ) {}
   virtual void PathChanged(); // notifies dependent paths of position change
   void Draw( Presentation *ppr ); // implements Draw method
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );

private:

   Cltn<Path *> *stub_paths; // list of paths which start or terminate at the stub

};

#endif
