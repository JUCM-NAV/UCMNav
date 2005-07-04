 /***********************************************************
 *
 * File:			svgpresentation.h
 * Author:          MITEL
 * Created:			Septempter 2001
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef SVG_PRESENTATION_H
#define SVG_PRESENTATION_H

#include <stdio.h>
#include "presentation.h"
#include "stub.h"


class SVGPresentation : public Presentation {
public:

   SVGPresentation( FILE *file, const char *name, float width = 6.5, float height = 5.42 );
   ~SVGPresentation();

   virtual void DrawLine( float X1, float Y1, float X2, float Y2 );
   virtual void DrawConnectedLines( float points[][2], int num_points );
   virtual void DrawPoly( float fX[], float fY[], int iNum_points );
   virtual void DrawFilledPoly( float fX[], float fY[], int iNum_points );
   virtual void DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear = TRUE );
   virtual void DrawStub( float fX[], float fY[], int colour, Stub *stub );
   virtual void DrawCircle( float fX, float fY, float fRadius , int iAngle_start=0, int iAngle_delta=64*360);
   virtual void DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 );
   virtual void FillRegion( float fX, float fY, float width, float height );
   virtual void DrawText( float fX, float fY, const char *text, bool italic, alignment al );
   virtual void Clear(); 
   virtual void ClearRegion( float fX, float fY, float width, float height );
   virtual void Refresh();
   virtual void SetLinePattern( eLine_styles elsLine_style );
   virtual void SetFillPattern( eFill_styles efsFill_style );
   virtual void SetLineWidth( int width );
   virtual void SetFontSize( eFont_sizes efSize );
   virtual void SetStipplePattern( eStipple_patterns esPattern );
   virtual void SetDashPattern( char rgcPattern[], int iNum );
   virtual void SetFgColour( int iIndex );
   virtual void GetFgColour( float& fR, float& fG, float& fB );
   virtual void GetBgColour( float& fR, float& fG, float& fB );
   virtual int GetLineWidth();

private:

   float XtoInches( float fX );
   float YtoInches( float fY );
   void ClearStubInterior( float fX[], float fY[] );
   void DrawStubOutline( float fX[], float fY[], int colour );
   FILE *hFile;

   float fWidth, fHeight;
   float fiLine_width;
   int svg_R, svg_G, svg_B;
   int iLine_width;

   eLine_styles elsCurrent_line_style;
   eFill_styles efsCurrent_fill_style;

   char svg_style[10];
   char svgname[800];
};

#endif


