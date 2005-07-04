/***********************************************************
 *
 * File:			responsibility_figure.h
 * Author:			Andrew Miga
 * Created:			August 1996
 *
 * Modification history:	Adding dynamic arrows as part
 *				of responsibilities
 *
 ***********************************************************/

#ifndef RESPONSIBILITY_FIGURE_H
#define RESPONSIBILITY_FIGURE_H

#include "hyperedge_figure.h"
#include "dynarrow.h"
#include <fstream>

typedef enum {RESP_UP, RESP_DOWN, RESP_LEFT, RESP_RIGHT} etResponsibility_direction;

class ResponsibilityFigure : public HyperedgeFigure {

public:

   ResponsibilityFigure( Hyperedge *edge );
   ~ResponsibilityFigure();
   
   virtual void Draw( Presentation *ppr ); // implements Draw method
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );
   virtual bool IsResponsibility()  { return( TRUE ); }

   void Highlight() { highlighted = TRUE; } // set, reset methods for highlight flag
   void Unhighlight() { highlighted = FALSE; }

   void Direction( etResponsibility_direction erdNew_direction ) { erdDirection = erdNew_direction; } // access methods for drawing direction
   etResponsibility_direction Direction() { return( erdDirection ); }

private:

   etResponsibility_direction erdDirection; // position of label
   bool highlighted; // flag set when figure is to be highlighted

};

#endif
