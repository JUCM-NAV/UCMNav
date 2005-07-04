/***********************************************************
 *
 * File:			path.h
 * Author:			Andrew Miga
 * Created:			August 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef PATH_H
#define PATH_H
#include "bspline.h"
#include "defines.h"
#include <stdio.h>

class Hyperedge;
class DisplayManager;

class Path : public BSpline {

public:

   Path( DisplayManager *cman, bool create = TRUE ); // constructor, flag creates specifies whether spline is to be created
   ~Path();

   void SetPathEnd( Hyperedge *end_edge ) { path_end = end_edge; }  // access methods for path start, end
   void SetPathStart( Hyperedge *start_edge ) { path_start = start_edge; }
   Hyperedge *GetPathEnd() { return( path_end ); }
   Hyperedge *GetPathStart() { return( path_start ); }
   
   void AddFigure( Figure *new_figure ); // adds a new element to the path
   void PurgeFigure( Figure *figure ); // removes an element from the path
   void AddBeforeFigure( Figure *new_figure, Figure *ref_figure ); // adds new element before reference element
   void AddAfterFigure( Figure *new_figure, Figure *ref_figure );  // adds new element after reference element
   void DeleteEndFigure(); // removes last element of path
   void DeleteStartFigure(); // removes first element of path
   bool GetCurveEnd( float &fx, float &fy ); // returns position of spline end
   void SetFigures( Cltn<Figure *> *new_figures ) { figures = new_figures; } // sets a new list of path elements
   Cltn<Figure *> *GetFigures() { return( figures ); } // returns current list of path elements
   Figure * BoundaryFigure( path_direction dir ); // returns second or second last figure in path
   bool Changed() { return( changed ); } // returns state of changed flag
   void PathChanged() { changed = TRUE; }   // sets changed flag
   void AddStartFigure( Figure *new_figure ); // adds new element as start of path
   void AddEndFigure( Figure *new_figure ); // adds new element as end of path
   Figure *GetStartFigure() { return( figures->GetFirst() ); } // returns first element of path
   Figure *GetEndFigure() { return( figures->GetLast() ); } // returns lsat element of path
   void Destroy() { destroyed = TRUE; } // manipulation methods for destroyed flag
   bool Destroyed() { return( destroyed ); }
   bool IsVisible();
   bool ForwardDirection( Figure *figure );
   
   int GetNumber() { return( path_number ); } // returns integer identifier
   int NumberElements() { return( figures->Size() ); }
   
   static void ResetCounter() { number_paths = 0; } // resets global counter of paths

protected:

   Hyperedge *path_end;  // hyperedge which starts path
   Hyperedge *path_start; // hyperedge which ends path
   DisplayManager *display_manager;

   int path_number, load_number; // current, past identifers for path
   bool destroyed; // flag set if path is to be deleted

   static int number_paths; // global count of number of paths
};

#endif
