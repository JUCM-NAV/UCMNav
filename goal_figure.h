/***********************************************************
 *
 * File:			goal_figure.h
 * Author:			Andrew Miga
 * Created:			October 1998
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef GOAL_FIGURE_H
#define GOAL_FIGURE_H

#include "hyperedge_figure.h"

class GoalFigure : public HyperedgeFigure {

public:

   GoalFigure( Hyperedge *edge, bool dialog );
   ~GoalFigure();

   virtual void Draw( Presentation *ppr ); // implements Draw method
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );
   virtual bool IsResponsibility()  { return( FALSE ); }

   void Highlight() { highlighted = TRUE; } // set, reset methods for highlighted flag
   void Unhighlight() { highlighted = FALSE; }

private:

   bool highlighted; // flag set if figure is to be highlighted

};

#endif
