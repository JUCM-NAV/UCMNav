/***********************************************************
 *
 * File:			marker_figure.h
 * Author:			Andrew Miga
 * Created:			May 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef MARKER_FIGURE_H
#define MARKER_FIGURE_H

class Presentation;

class MarkerFigure {

public:
   
   MarkerFigure( float x, float y )  { X = x; Y = y; }
   ~MarkerFigure();
   void Draw( Presentation *ppr );
   void GetPosition( float& f_rx, float& f_ry )  { f_rx = X;  f_ry = Y; }


private:

   float X, Y;

};

#endif
