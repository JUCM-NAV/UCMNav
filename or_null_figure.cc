/***********************************************************
 *
 * File:			or_null_figure.cc
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:
 *
 ***********************************************************/

#include "or_null_figure.h"
#include "or_figure.h"
#include "synch_figure.h"
#include "path.h"

OrNullFigure::OrNullFigure(  OrFigure *or_figure, Path *new_path, bool normal_operation ) : Figure()
{
   parent_figure = or_figure;
   ordinal = 0;
   parent_figure->AddDependent( this, normal_operation );
   path = new_path;
}

OrNullFigure::~OrNullFigure()
{
   if( path ) path->PurgeFigure( this );
   parent_figure->RemoveDependent( this );
}

void OrNullFigure::GetPosition( float& f_rx, float& f_ry )
{
   float fpx, fpy;

   parent_figure->GetNullPosition( ordinal, fpx, fpy );
   
   f_rx = fpx;
   f_ry = fpy;
}

