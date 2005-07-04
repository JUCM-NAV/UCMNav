/***********************************************************
 *
 * File:			marker_figure.cc
 * Author:			Andrew Miga
 * Created:			May 1997
 *
 * Modification history:
 *
 ***********************************************************/


#include "marker_figure.h"
#include "presentation.h"
#include "defines.h"

MarkerFigure::~MarkerFigure() {}

void MarkerFigure::Draw( Presentation *ppr )
{

   ppr->DrawFilledCircle( X, Y, WAIT_FIGURE_SIZE );
   ppr->SetLineWidth(3);
   ppr->DrawLine( X, Y+START_BAR_SIZE, X, Y-START_BAR_SIZE );
   ppr->SetLineWidth(2);

}
