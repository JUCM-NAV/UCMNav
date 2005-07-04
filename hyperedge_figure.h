/***********************************************************
 *
 * File:			hyperedge_figure.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef HYPEREDGE_FIGURE_H
#define HYPEREDGE_FIGURE_H

#include "figure.h"
#include "defines.h"
#include "path.h"
#include "collection.h"
#include <stdio.h>

class Hyperedge;
class Presentation;
class ComponentReference;
class Component;
class Path;

class HyperedgeFigure : public Figure {

public:

   HyperedgeFigure( Hyperedge *edge ); // constructor, stores corresponding hyperedge object
   virtual ~HyperedgeFigure();

   Hyperedge *Edge() {  return( dependent_edge ); }  // returns corresponding hyperedge object
   HyperedgeFigure *DependentFigure() { return( dependent_figure ); } // access methods for dependent figure object
   void DependentFigure( HyperedgeFigure *new_dependent ) { dependent_figure = new_dependent; }

   void SetActualPosition( float f_nx, float f_ny ) { fX = f_nx; fY = f_ny; } // access method for internal position coordinates
   virtual void GetPosition( float& f_rx, float& f_ry );  // implements polymorphic position retrieval method for hyperedge figures
   virtual void SetPosition( float f_nx, float f_ny, bool limit = TRUE, bool enclose = TRUE, // implements polymorphic position setting method
			     bool dependent_update = TRUE, bool interpolate = TRUE ); // for hyperedge figures
   virtual void DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )=0;
   virtual void Draw( Presentation *ppr )=0; // polymorphic function for drawing figure on screen or in postscript file
   virtual bool IsResponsibility(); // returns if figure is that of a responsibility
   virtual void OrderPaths();  // orders input/output paths of forks and joins
   
   void ShiftPosition( float x_offset, float y_offset );
   void ValidatePositionShift( float x_offset, float y_offset, float& x_limit, float& y_limit );
   void SetSelected() { selected = TRUE; }  // access and reset methods for selected flag
   void ResetSelected() { selected = FALSE; }
   bool IsSelected() { return( selected == TRUE ); }
   bool IsVisible();
   
   ComponentReference *GetEnclosingComponent() { return( component ); } // access and reset methods for enclosing component
   void SetEnclosingComponent( ComponentReference *encl_component ) { component = encl_component; }
   void ResetEnclosingComponent();
   ComponentReference *GetContainingComponent(); // returns enclosing component regardless of whether it is bound
   Component * BoundComponent();
   void BindComponents();

protected:
   
   Hyperedge *dependent_edge;  // corresponding hyperedge object
   HyperedgeFigure *dependent_figure;  // dependent figure object for path element connections
   ComponentReference *component; // the enclosing component of the figure
   bool selected;  // flag signifying whether figure is selected or not

};

#endif
