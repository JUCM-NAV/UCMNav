/***********************************************************
 *
 * File:			cgmpresentation.h
 * Author:			Kevin Lam
 * Created:			October 1999
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef CGM_PRESENTATION_H
#define CGM_PRESENTATION_H

#include <stdio.h>
#include "presentation.h"

class CGMPresentation : public Presentation {
public:

   CGMPresentation( FILE *file, const char *name, float left, float right, float top, float bottom );
   ~CGMPresentation();
   
   virtual void DrawLine( float fX1, float fY1, float fX2, float fY2 );
   virtual void DrawPoly( float fX[], float fY[], int iNum_points );
   virtual void DrawFilledPoly( float fX[], float fY[], int iNum_points );
   virtual void DrawCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 );
   virtual void DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 );
   virtual void DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear = TRUE );

   virtual void StartCurve( float fX, float fY );
   virtual void DrawCurve( float fX[], float fY[] );
   virtual void EndCurve();
   
   virtual int CanDrawCurves() { return TRUE; }
   virtual void FillRegion( float fX, float fY, float fWidth, float fHeight ) {}
   virtual void DrawText( float fX, float fY, const char *text, bool italic, alignment al );
   virtual void Clear() {}
   virtual void ClearRegion( float fX, float fY, float fWidth, float fHeight ) {}
   virtual void Refresh() {}
   virtual void SetLinePattern( eLine_styles elsLine_style );
   virtual void SetFillPattern( eFill_styles efsFill_style );
   virtual void SetLineWidth( int width );
   virtual void SetFontSize( eFont_sizes efSize );
   virtual void SetStipplePattern( eStipple_patterns esPattern );
   virtual void SetFgColour( float fR, float fG, float fB );
   virtual void SetBgColour( float fR, float fG, float fB );
   virtual void SetFgColour( int iIndex );
   virtual void GetFgColour( float& fR, float& fG, float& fB );
   virtual void GetBgColour( float& fR, float& fG, float& fB );  
   virtual int GetLineWidth();

private:

   void outputString ( const char *outputS );
   void outputCoord ( float coval );
   void outputElementCode(int CL, int ID, int len);
   void outputInt( unsigned int ival );

   FILE *hFile;

   int iLine_width;
   eLine_styles elsCurrent_line_style;
   eFill_styles efsCurrent_fill_style;

   float fFg_red, fFg_green, fFg_blue;
   float fBg_red, fBg_green, fBg_blue;

   float scale_factor, left_border, right_border, top_border, bottom_border;
};

#endif
