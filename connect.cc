/***********************************************************
 *
 * File:			connect.cc
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:	June 1999
 *                              Added Abort hyperedge
 *
 ***********************************************************/

#include "connect.h"

Connect::Connect() : Hyperedge()
{
   figure = NULL;
}

Connect::Connect( int connect_id ) : Hyperedge()
{
   load_number = connect_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file
   hyperedge_number = connect_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;   
   //End of the modification by Bo Jiang, April, 2005
   
   figure = NULL;
}

void Connect::SaveXMLDetails( FILE *fp )
{
   PrintXMLText( fp, "<connect></connect>\n" );
}
void Connect::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "\n" );
}

Abort::Abort() : Hyperedge()
{
   figure = NULL;
}

Abort::Abort( int abort_id ) : Hyperedge()
{
   load_number = abort_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file   
   hyperedge_number = abort_id;
   if (number_hyperedges <= hyperedge_number) number_hyperedges = hyperedge_number+1;      
   //End of the modification by Bo Jiang, April, 2005
   
   figure = NULL;
}

HyperedgeFigure * Abort::GetFigure()
{
   return( source->GetFirst()->PreviousEdge()->GetFigure() );
}

void Abort::SaveXMLDetails( FILE *fp )
{
   PrintXMLText( fp, "<abort></abort>\n" );
}
void Abort::SaveDXLDetails( FILE *fp )
{
   PrintXMLText( fp, "\n" );
}
