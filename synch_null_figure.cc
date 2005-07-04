/***********************************************************
 *
 * File:			synch_null_figure.cc
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:
 *
 ***********************************************************/

#include "synch_null_figure.h"
#include "synch_figure.h"
#include "path.h"

SynchNullFigure::SynchNullFigure(  SynchronizationFigure *synch_figure, Path *new_path,
				   path_direction dir, bool normal_operation ) : Figure()
{
   parent_figure = synch_figure;
   path_dir = dir;
   ordinal = 0;
   parent_figure->AddDependent( this, dir, normal_operation );
   path = new_path;
}

SynchNullFigure::~SynchNullFigure()
{
   if( path ) path->PurgeFigure( this );
   parent_figure->RemoveDependent( this, path_dir );
}

void SynchNullFigure::GetPosition( float& f_rx, float& f_ry )
{
   float fpx, fpy;

   parent_figure->GetNullPosition( path_dir, ordinal, fpx, fpy );

   f_rx = fpx;
   f_ry = fpy;
}
