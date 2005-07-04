/***********************************************************
 *
 * File:			presentation.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

/*
 *	File: 		presentation.h
 *	Classes: 	Presentation
 */

#ifndef PRESENTATION_H
#define PRESENTATION_H

#include "forms.h"
#include "defines.h"

typedef enum {LINE_SOLID, LINE_DASHED, LINE_DOUBLE_DASHED} eLine_styles;
typedef enum {FILL_SOLID, FILL_STIPPLE} eFill_styles;
typedef enum {PATTERN1, PATTERN2} eStipple_patterns;
typedef enum {SMALL_FONT, LARGE_FONT, SMALL_BOLD_FONT } eFont_sizes;
typedef enum {LEFT_ALIGN, CENTER, RIGHT_ALIGN} alignment;


class Stub;

extern bool postscript_output; // global variable to avoid highlighting selected components and figures

class Presentation {
public:
   
   Presentation();
   virtual ~Presentation() {};

   virtual void DrawLine( float fX1, float fY1, float fX2, float fY2 )=0; 
   virtual void DrawPoly( float fX[], float fY[], int iNum_points )=0;
   virtual void DrawFilledPoly( float fX[], float fY[], int iNum_points )=0;
   virtual void DrawStub( float fX[], float fY[], int colour, Stub *stub );
   virtual void DrawCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 )=0;
   virtual void DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 )=0;
   virtual void DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear = TRUE );

   virtual void StartCurve( float fX, float fY ) {}
   virtual void DrawCurve( float fX[], float fY[] ) {}
   virtual void EndCurve() {}
   virtual int CanDrawCurves() { return FALSE; }

   virtual void FillRegion( float fX, float fY, float fWidth, float fHeight ) = 0;
   virtual void DrawText( float fX, float fY, const char *text, bool italic = FALSE,  alignment al = LEFT_ALIGN )=0;
   virtual void DrawSelectionHandles( float fX, float fY );
   
   void DrawLine( float fX1, float fY1, float fX2, float fY2, float Xc, float Yc, figure_orientation orientation );
   virtual void DrawConnectedLines( float points[][2], int num_points ) {}
   void DrawCircle( float fX, float fY, float fRadius,  float Xc, float Yc,
		    figure_orientation orientation, int iAngle_start=0, int iAngle_delta=64*360 );
   void DrawFilledCircle( float fX, float fY, float fRadius,  float Xc, float Yc,
			  figure_orientation orientation, int iAngle_start=0, int iAngle_delta=64*360 );
   
   virtual void Clear() = 0;
   virtual void ClearRegion( float fX, float fY, float fWidth, 
			     float fHeight ) = 0;
   virtual void Refresh() = 0;
   int GetWidth() { return( width ); }
   int GetHeight() { return( height ); }
   virtual void SetLinePattern( eLine_styles esLine_style ) = 0;
   virtual void SetFillPattern( eFill_styles esFill_style ) = 0;
   virtual void SetLineWidth( int width ) = 0;
   virtual void SetStipplePattern( eStipple_patterns esPattern ) = 0;
   virtual void SetFontSize( eFont_sizes efSize ) = 0;
   
   virtual void GetFgColour( float& fR, float& fG, float& fB ) = 0;
   virtual void GetBgColour( float& fR, float& fG, float& fB ) = 0;

   virtual void SetFgColour( int iIndex ) = 0;

   virtual int GetLineWidth()=0;

   static void TranslateCoordinates( float& X, float& Y, float Xp, float Yp, float Xc, float Yc, figure_orientation orientation );

   static int MAX_COLOURS;

protected:

   static int iInitialized, iNum_colours;
   int height, width;

private:

   void ClearStubInterior( float fX[], float fY[] );
   void DrawStubOutline( float fX[], float fY[], int colour );

};

#endif
