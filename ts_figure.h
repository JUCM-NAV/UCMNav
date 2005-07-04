/***********************************************************
 *
 * File:			ts_figure.h
 * Author:			Andrew Miga
 * Created:			October 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef TIMESTAMP_FIGURE_H
#define TIMESTAMP_FIGURE_H

#include "hyperedge_figure.h"
#include <fstream>

typedef enum {UP, DOWN, LEFT, RIGHT} timestamp_direction;

class TimestampFigure : public HyperedgeFigure {

public:

   TimestampFigure( Hyperedge *edge, bool normal_operation = TRUE );
   ~TimestampFigure();
   
   virtual void Draw( Presentation *ppr ); // implements Draw method
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb );
   virtual bool IsResponsibility()  { return( FALSE ); }

   void Direction( timestamp_direction new_direction ) { direction = new_direction; }
   void Direction( int dir );
   timestamp_direction Direction() { return( direction ); } // access methods for direction
   void Highlight() { highlighted = TRUE; } // set, reset methods for highlighted flag
   void Unhighlight() { highlighted = FALSE; }

private:

   timestamp_direction direction;  // orientation of timestamp point
   bool highlighted; // flag set if figure is to be highlighted

};

#endif
