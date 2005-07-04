/***********************************************************
 *
 * File:			mifpresentation.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef MIF_PRESENTATION_H
#define MIF_PRESENTATION_H

#include <stdio.h>
#include "presentation.h"

class MIFPresentation : public Presentation {
public:

   MIFPresentation( FILE *file, float width = 6.5, float height = 5.42 );
   ~MIFPresentation();

   void DrawLine( float fX1, float fY1, float fX2, float fY2 );
   void DrawConnectedLines( float points[][2], int num_points );
   void DrawPoly( float fX[], float fY[], int iNum_points );
   void DrawFilledPoly( float fX[], float fY[], int iNum_points );
   void DrawCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 );
   void DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 );
   void FillRegion( float fX, float fY, float width, float height );
   void DrawText( float fX, float fY, const char *text, bool italic, alignment al );
   void Clear(); 
   void ClearRegion( float fX, float fY, float width, float height );
   void Refresh();
   void SetLinePattern( eLine_styles elsLine_style );
   void SetFillPattern( eFill_styles efsFill_style );
   void SetLineWidth( int width );
   void SetFontSize( eFont_sizes efSize );
   void SetStipplePattern( eStipple_patterns esPattern );
   void SetDashPattern( char rgcPattern[], int iNum );
   
   void SetFgColour( int iIndex );
   void GetFgColour( float& fR, float& fG, float& fB );
   void GetBgColour( float& fR, float& fG, float& fB );
  
   int GetLineWidth();

   void StartGroup();
   void EndGroup();

private:
   void RGBtoCMYK( float fR, float fG, float fB,
		   float &fC, float &fM, float &fY, float &fK );
   char *AddColour( int iR, int iG, int iB );
   void DumpColours();

   float XtoInches( float fX );
   float YtoInches( float fY );
   void PrintGroupID();

   FILE *hFile, *hFinal_file;

   float fWidth, fHeight;

   int iLine_width;
   int iCurrent_group;
   int iGrouped;

   eLine_styles elsCurrent_line_style;
   eFill_styles efsCurrent_fill_style;

   float fFg_red, fFg_green, fFg_blue;
   float fBg_red, fBg_green, fBg_blue;

};

#endif


