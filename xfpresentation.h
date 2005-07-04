/***********************************************************
 *
 * File:			xfpresentation.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

/*
 *	File: 		xfpresentation.h
 *	Classes: 	XfPresentation
 */

#include <stdio.h>
#include <X11/StringDefs.h>

#include "presentation.h"

#define NUMBER_FONTS 6
#define NUMBER_ITALIC_FONTS 5

class XfPresentation : public Presentation {

public:

   XfPresentation( Window winNew, Pixmap pmNew, int new_width, int new_height );
   ~XfPresentation();

   void CreateNewPixmap( int new_width, int new_height );
   void SetWindowDimensions( int new_width, int new_height ) { window_width = new_width; window_height = new_height; }
   void SetScaleFactor( float factor );
   
   virtual void DrawLine( float fX1, float fY1, float fX2, float fY2 );
   virtual void DrawConnectedLines( float points[][2], int num_points );
   virtual void DrawPoly( float fX[], float fY[], int iNum_points );
   virtual void DrawFilledPoly( float fX[], float fY[], int iNum_points );
   virtual void DrawCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 );
   virtual void DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start=0, int iAngle_delta=64*360 );
   virtual void FillRegion( float fX, float fY, float fWidth,
		    float fHeight );
   virtual void DrawText( float fX, float fY, const char *text, bool italic, alignment al );
   virtual void DrawSelectionHandles( float fX, float fY );
   virtual void Clear();
   virtual void ClearRegion( float fX, float fY, float fWidth, float fHeight );
   virtual void Refresh();
   virtual void SetLinePattern( eLine_styles elsLine_style );
   virtual void SetFillPattern( eFill_styles efsFill_style );
   virtual void SetLineWidth( int width );
   virtual void SetFontSize( eFont_sizes efSize );
   virtual void SetStipplePattern( eStipple_patterns esPattern );
   virtual void GetFgColour( float& fR, float& fG, float& fB );
   virtual void GetBgColour( float& fR, float& fG, float& fB );
   virtual void SetFgColour( int iIndex );
   virtual int GetLineWidth();

   static float DetermineWidth( const char *label );
   
private:
  
   Window win;
   Pixmap pmPixmap, right_pixmap, bottom_pixmap;
   static Colormap cmColour_map;
   static int iColormap_initialized;
   Pixmap pmPattern1;
   Pixmap pmPattern2;

   GC gcContext, gcClear_context, gcInvalid_context;
   XFontStruct *fonts[NUMBER_FONTS], *italic_fonts[NUMBER_ITALIC_FONTS], *current_font, *current_italic_font;
   int iLine_width, right_pixmap_width, bottom_pixmap_height;
   int actual_width, actual_height, window_width, window_height;
   eLine_styles elsCurrent_line_style;
   eFill_styles efsCurrent_fill_style;

   static char acHash_bits[4];
   static char acArrow_bits[32];
   static char font_names[][35];
   static char italic_font_names[][35];
   static int font_sizes[NUMBER_FONTS];
   static int italic_font_sizes[NUMBER_ITALIC_FONTS];   
   static XFontStruct *base_font;
   static float dw;
   
   float fFg_red, fFg_green, fFg_blue;
   float fBg_red, fBg_green, fBg_blue;
};

