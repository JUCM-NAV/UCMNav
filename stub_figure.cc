/***********************************************************
 *
 * File:			stub_figure.cc
 * Author:			Andrew Miga
 * Created:			August 1996
 *
 * Modification history:        May 1997
 *
 ***********************************************************/

#include "stub_figure.h"
#include "presentation.h"
#include "xfpresentation.h"
#include "component.h"
#include "stub.h"

StubFigure::StubFigure( Hyperedge *edge, bool normal_operation ) : HyperedgeFigure( edge )
{
   stub_paths = new Cltn<Path *>;
   if( normal_operation )
      stub_paths->Add( path ); // add path object of incoming path into stub paths list
   path = NULL; // as path object is no longer needed set to null as it should be undefined for stubs
}

StubFigure::~StubFigure()
{
   for( stub_paths->First(); !stub_paths->IsDone(); stub_paths->Next() ) {
      if( stub_paths->CurrentItem() != path )
	 stub_paths->CurrentItem()->PurgeFigure( this );
   }
   delete stub_paths;
}

void StubFigure::SetPosition( float f_nx, float f_ny, bool limit, bool enclose,
			      bool dependent_update, bool interpolate )
{
   HyperedgeFigure::SetPosition( f_nx, f_ny );
   this->PathChanged();
}

void StubFigure::ReplaceDependentPath( Path *new_path, Path *old_path )
{
   if( stub_paths->Includes( old_path ))
      stub_paths->Remove( old_path );

   stub_paths->Add( new_path );
}

void StubFigure::PathChanged()
{
   for( stub_paths->First(); !stub_paths->IsDone(); stub_paths->Next() )
      stub_paths->CurrentItem()->PathChanged();
}

void StubFigure::Draw( Presentation *ppr )
{

   float x, y;
   int colour;
   Stub *stub = (Stub *)dependent_edge;
   
   GetPosition( x, y );

   // draw diamond
   float XCoord[4] = { x-STUB_BOX_WIDTH, x, x+STUB_BOX_WIDTH, x };
   float YCoord[4] = { y, y+STUB_BOX_HEIGHT, y, y-STUB_BOX_HEIGHT };

   if( stub->stype == DYNAMIC )
      ppr->SetLinePattern( LINE_DASHED );

   if( selected && !postscript_output ) {
      ppr->DrawSelectionHandles( x, y );
      ppr->SetFgColour( BLUE );
   }
   
   if( stub->Highlight() )
      colour = RED;
   else if( selected && !postscript_output )
      colour = BLUE;
   else
      colour = BLACK;
   
   ppr->DrawStub( XCoord, YCoord, colour, stub );
      
   if( stub->stype == DYNAMIC )
      ppr->SetLinePattern( LINE_SOLID );
     
   if( strcmp( stub->stub_label, "" ) != 0 )                   // draw label for stub
      ppr->DrawText( x, y-0.035, stub->stub_label, FALSE, CENTER );

   ppr->SetFgColour( BLACK );   
}

void StubFigure::DetermineBoundingBox( float& lb, float& rb, float& tb, float& bb )
{
   float x, y, tw;
   Stub *stub = (Stub *)dependent_edge;

   GetPosition( x, y );
   tw = XfPresentation::DetermineWidth( stub->stub_label );

   lb = x - tw/2;
   rb = x + tw/2;
   tb = y - .05;
   bb = y + .04;
}
