/***********************************************************
 *
 * File:			loop_null_figure.cc
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:
 *
 ***********************************************************/

#include "loop_null_figure.h"
#include "loop_figure.h"
#include "path.h"

LoopNullFigure::LoopNullFigure(  LoopFigure *loop_figure, Path *new_path, loop_paths lpath ) : Figure()
{
   parent_figure = loop_figure;
   loop_path = lpath;
   parent_figure->AddDependent( this, lpath );
   path = new_path;
}

LoopNullFigure::~LoopNullFigure()
{
   if( path ) path->PurgeFigure( this );
}

void LoopNullFigure::GetPosition( float& f_rx, float& f_ry )
{
   float fpx, fpy;

   parent_figure->GetNullPosition( loop_path, fpx, fpy );

   f_rx = fpx;
   f_ry = fpy;
}
