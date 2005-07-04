/***********************************************************
 *
 * File:			figure.cc
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "figure.h"
#include "display.h"
#include "path.h"

extern DisplayManager *display_manager;

int Figure::number_figures = 0;

Figure::Figure()
{
   path = display_manager->CurrentPath();
   figure_number = number_figures++;
}

Figure::~Figure() {}

void Figure::SetPath( Path *new_path, bool  normal_operation )
{
   path = new_path;
}

void Figure::PathChanged()
{
   path->PathChanged();
}
