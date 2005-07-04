 /***********************************************************
 *
 * File:			component_ref.h
 * Author:			Andrew Miga
 * Created:			December 1999
 *
 * Modification history:        Mar 2003
 *                              Shuhua Cui
 *
 ***********************************************************/

#ifndef COMPONENT_REF_H
#define COMPONENT_REF_H

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <fstream>

#include "component.h"
#include "presentation.h"
#include "collection.h"
#include "map.h"

#include <string.h>

#define traversal_count load_number

class HyperedgeFigure;

class ComponentReference {

   friend std::ostream& operator<<( std::ostream&, ComponentReference * );

public:

   ComponentReference( float fNew_x, float fNew_y, float fNew_width, float fNew_height, ComponentReference *new_parent = NULL );
   ComponentReference( int component_id, float x, float y, float w, float h, ComponentReference *parent,
		       bool is_anchored, int reference_id, Component *referenced_component = NULL, const char *component_role = NULL );
   ~ComponentReference();

   void SetPosition( float fNew_x, float fNew_y ); // access methods for component position
   void GetPosition( float &fReturn_x, float &fReturn_y );
   void ShiftPosition( float x_offset, float y_offset );
   void ValidatePositionShift( float x_offset, float y_offset, float& x_limit, float& y_limit );

   float GetHeight(); // access methods for component width, height
   float GetWidth();
   void SetHeight( float fNew_height, int iResizing_decoupled = 0 );
   void SetWidth( float fNew_width, int iResizing_decoupled = 0 );
   bool PointInBox( float fCheck_x, float fCheck_y ); // hit detection method
   // bool PointOnBorder( float fCheck_x, float fCheck_y ); // hit detection method for component border
   void ResetGeometry();  // methods which set and check dimensions with respect to
   void CheckGeometry();  // parent and child components

   void ReferencedComponent( Component *new_ref );
   Component * ReferencedComponent() { return( component ); }
   ComponentReference *GetParent() { return( component_parent ); } // returns the parent component
   etComponent_type ComponentType() { return( component ? component->GetType() : TEAM ); }
   int SetParent( ComponentReference *new_parent ); // sets a new component as parent
   void AddChild( ComponentReference *new_child ); // adds a component as a child
   void RemoveChild( ComponentReference *child ); // removes a component as a child
   int GetNumChildren() { return( children.Size()); } // returns number of children
   ComponentReference *GetChild( int iChild_num );  // returns a child component

   int IsPassive(); // checks if component is passive and does not contain active components
   int CheckTypeContainment( etComponent_type ect ); // checks type containment validity of type
   int CheckTypesDownward( etComponent_type ect );

   int IsComponentDescendent( ComponentReference *cr ); // determines if component is a descendent of this one
   ComponentReference *FindRoot(); // finds top level parent component
   void Draw( Presentation *ppr ); // draws the component on screen or in postscript file
   int PointinComponent( float fCheck_x, float fCheck_y );

   const int ComponentReferenceNumber() { return( component_ref_number ); } // returns integer identifier

   ComponentReference *Copy( ComponentReference *copy_parent ); // copies this component reference

   void GetAllChildren( Cltn<ComponentReference *> &components ); // returns list of child components

   void SaveXML( FILE *fp, bool ignore_children = TRUE );  // saves component's data to file in XML format
   void SaveGraphDXL( FILE *fp );  // saves component's graph data to file in DXL for DOORS use, added by Bo Jiang, Sept 2004
   void SaveCSMXML( FILE *fp, bool ignore_children = TRUE );  // saves component reference data to file in XML format(CSM)
   void GeneratePostScriptDescription( FILE *ps_file );
   void NotifyDependents(); // notifies dependent objects of size, position change


   void RegisterDependentFigure( HyperedgeFigure *new_figure ); // adds a figure as a dependent of this component
   void PurgeDependentFigure( HyperedgeFigure *dependent ); // removes a figure as a dependent of this component

   bool Anchored() { return( anchored ); }
   void Anchored( int anc ) { anchored = anc; }
   bool Fixed () { return( component_fixed ); }
   void Fixed( bool isfixed );

   void RecheckBindings ();
   void BindEnclosedFigures();
   void UnbindEnclosedFigures();
   void UnbindEnclosedComponents();

   static void ResetComponentReferenceCount() { number_component_refs = 0; } // resets global count of components
   static int NumberComponents() { return( number_component_refs ); } // Apr2005 gM: required for map import to avoid conflict with constant IDs
   int LoadIdentifier() { return( load_number ); }
   void AddContainedElement( HyperedgeFigure *dependent );
   void GetBoundaries( float& top, float& bottom, float& left, float& right );
   void SetSelected() {  selected = TRUE; }  // access and reset methods for selected flag
   void ResetSelected() {  selected = FALSE; }
   bool Selected() { return( selected ); }

   void ResetTraversalCount() { traversal_count = 0; }
   void IncrementTraversalCount() { traversal_count++; }
   bool FinalTraversal() { return((--traversal_count <= 0) ? TRUE : FALSE ); }

   const char * ReferenceName();
   const char * ComponentRole() { return( role ); }
   void ComponentRole( const char *new_role ) { strncpy( role, new_role, DISPLAYED_LABEL ); }

private:

   float fX, fY, fWidth, fHeight; // current dimensions of component
   float fOld_x, fOld_y, fOld_width, fOld_height; // previous dimensions of component

   Component *component; // the global component definition referenced by this definition
   char role[DISPLAYED_LABEL+1];
   ComponentReference *component_parent;  // the parent component of this component
   ComponentReference *old_parent; // the previous parent component

   Cltn<ComponentReference *> children; // the child components of this component
   Cltn<HyperedgeFigure *> dependent_figures; // the figures that are inside the component boundaries

   int component_ref_number, load_number; // current and past integer identifiers
   static int number_component_refs; // global count of number of component references
   bool anchored;  // flag specifying if component reference is anchored
   bool component_fixed;
   bool selected;  // flag signifying whether component is selected or not

   static float STACK_OFFSET; // class variables for drawing offsets
   static float PROTECT_OFFSET;
   static char reference_name[LABEL_SIZE+DISPLAYED_LABEL+5];

};

#endif

