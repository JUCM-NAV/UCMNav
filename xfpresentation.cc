/***********************************************************
 *
 * File:			xfpresentation.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

/*
 *	File: 		xfpresentation.cc
 *	Classes: 	XfPresentation
 */

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "xfpresentation.h"
#include "defines.h"
#include <iostream>
#include <math.h>
#include <assert.h>
#include <unistd.h>

char XfPresentation::acHash_bits[4]={0x0a, 0x05, 0x0a, 0x05};
char XfPresentation::acArrow_bits[32]={0x44, 0x44, 0x44, 0x44,
				       0x47, 0x44, 0x40, 0x44,
				       0x40, 0x44, 0x40, 0x44,
				       0x7f, 0x44, 0x00, 0x44,
				       0x00, 0x44, 0x00, 0x44,
				       0xff, 0x47, 0x00, 0x40,
				       0x00, 0x40, 0x00, 0x40,
				       0xff, 0x7f, 0x00, 0x00};

int XfPresentation::iColormap_initialized = 0;

extern int left_boundary, top_boundary;
extern float scale, selection_radius;
extern FD_Main *pfdmMain;

#define default_width   width
#define default_height  height
#define X_COORD(x) ((int)(((x)*default_width*scale)-left_boundary))
#define Y_COORD(y) ((int)(((y)*default_height*scale)-top_boundary))
#define EMPTY_PIXMAP 0

char XfPresentation::font_names[][35] = {
   
   "-adobe-times-medium-r-normal--",
   "*-times-medium-r-normal--",
   "*-helvetica-medium-r-normal--",
   "*-courier-medium-r-normal--",
   "-misc-fixed-medium-r-normal--",
   "fixed"

};
char XfPresentation::italic_font_names[][35] = {

   "-adobe-times-medium-i-normal--",
   "*-times-medium-i-normal--",
   "*-helvetica-medium-o-normal--",
   "*-courier-medium-o-normal--",
   "fixed"

};

int XfPresentation::font_sizes[] = { 8, 10, 12, 14, 18, 24 };
int XfPresentation::italic_font_sizes[] = { 8, 10, 12, 14, 18 };
XFontStruct *XfPresentation::base_font;
float XfPresentation::dw;

XfPresentation::XfPresentation( Window winNew, Pixmap pmNew, int new_width, int new_height )
{
   int i, j;
   char **font_list, font_name[50];
   int list_size;
  
   win = winNew;
  
   default_width = new_width;
   default_height = new_height;

   actual_width = default_width;  // set initial actual values
   actual_height = default_height;
   window_width = default_width;
   window_height = default_height;
   dw = (float)default_width;
   
   const int iHash_width=4;
   const int iHash_height=4;
   const int iArrow_width=16;
   const int iArrow_height=16;
 
   pmPixmap = pmNew;
   right_pixmap = EMPTY_PIXMAP;
   bottom_pixmap = EMPTY_PIXMAP;
   
   gcContext = XCreateGC( fl_get_display(), fl_default_window(), 0, 0);
   gcClear_context = XCreateGC( fl_get_display(), fl_default_window(), 0, 0);
   gcInvalid_context = XCreateGC( fl_get_display(), fl_default_window(), 0, 0);

   XSetForeground( fl_get_display(), gcContext, fl_get_flcolor(FL_BLACK) );
   XSetBackground( fl_get_display(), gcContext, fl_get_flcolor(FL_WHITE) );

   XSetForeground( fl_get_display(), gcClear_context, fl_get_flcolor(FL_WHITE) );
   XSetBackground( fl_get_display(), gcClear_context, fl_get_flcolor(FL_BLACK) );

   XSetForeground( fl_get_display(), gcInvalid_context, fl_get_flcolor(FL_GRAY90) );
   XSetBackground( fl_get_display(), gcInvalid_context, fl_get_flcolor(FL_GRAY16) );

   fFg_red = 0.0;
   fFg_green = 0.0;
   fFg_blue = 0.0;
   fBg_red = 1.0;
   fBg_green = 1.0;
   fBg_blue = 1.0;

   for( j = 0; j < NUMBER_FONTS; j++ ) {

      i = 0;
      forever {
	 if( i != NUMBER_FONTS - 1 )
	    sprintf( font_name, "%s%d-*", font_names[i], font_sizes[j] );
	 else
	    strcpy( font_name, font_names[i] );
	 font_list = XListFonts( fl_get_display(), font_name, 1, &list_size);
	 if( font_list != NULL ) {
	    fonts[j] = XLoadQueryFont( fl_get_display(), font_list[0] );
	    XFreeFontNames( font_list );
	    break;
	 }
	 else {
	    std::cout << "\nFont: { " << font_name << " } not found. Substituting fonts.\n" << std::flush;
	    i++;
	    if( i == NUMBER_FONTS ) {
	       std::cout << "\nNone of the available fonts were found. Aborting program.\n" << std::flush;
	       _exit( 1 );
	    }
	 }
      }
   }
   
   for( j = 0; j < NUMBER_ITALIC_FONTS; j++ ) {

      i = 0;
      forever {
	 if( i != NUMBER_ITALIC_FONTS - 1 )
	    sprintf( font_name, "%s%d-*", italic_font_names[i], italic_font_sizes[j] );
	 else
	    strcpy( font_name, italic_font_names[i] );
	 font_list = XListFonts( fl_get_display(), font_name, 1, &list_size);
	 if( font_list != NULL ) {
	    italic_fonts[j] = XLoadQueryFont( fl_get_display(), font_list[0] );
	    XFreeFontNames( font_list );
	    break;
	 }
	 else {
	    std::cout << "\nFont: { " << font_name << " } not found. Substituting fonts.\n" << std::flush;
	    i++;
	    if( i == NUMBER_ITALIC_FONTS ) {
	       std::cout << "\nNone of the available fonts were found. Aborting program.\n" << std::flush;
	       _exit( 1 );
	    }
	 }
      }
   }
   
   current_font = fonts[3]; // install 14 pt. font as the current font
   base_font = fonts[3];
   current_italic_font = italic_fonts[2]; // install 12 pt. font as the current font

   pmPattern1 = XCreateBitmapFromData( fl_get_display(),
				       win,
				       acArrow_bits, iArrow_width, iArrow_height );

   pmPattern2 = XCreateBitmapFromData( fl_get_display(),
				       win,
				       acHash_bits, iHash_width, iHash_height );

   XSetStipple( fl_get_display(), gcInvalid_context, pmPattern2 );
   XSetFillStyle( fl_get_display(), gcInvalid_context, FillOpaqueStippled );

   elsCurrent_line_style = LINE_SOLID;
   efsCurrent_fill_style = FILL_SOLID;
   SetLineWidth( 2 );
}

XfPresentation::~XfPresentation() {}

void XfPresentation::CreateNewPixmap( int new_width, int new_height )
{
   XFreePixmap( fl_get_display(), pmPixmap );

   // free old right and bottom pixmaps, if they exist
   if( right_pixmap != EMPTY_PIXMAP ) {
      XFreePixmap( fl_get_display(), right_pixmap );
      right_pixmap = EMPTY_PIXMAP;
   }
   
   if( bottom_pixmap != EMPTY_PIXMAP ) {
      XFreePixmap( fl_get_display(), bottom_pixmap );
      bottom_pixmap = EMPTY_PIXMAP;
   }

   // create right and bottom pixmaps if needed
   if( window_width > new_width ) {
      right_pixmap_width = window_width - new_width;
      right_pixmap =  XCreatePixmap( fl_get_display(), fl_get_canvas_id( pfdmMain->DrawingArea ),
			      right_pixmap_width, window_height, fl_get_visual_depth() );
      XFillRectangle( fl_get_display(), right_pixmap, gcInvalid_context,
		      0, 0, right_pixmap_width, window_height );
   }

   if( window_height > new_height ) {
      bottom_pixmap_height = window_height - new_height;
      bottom_pixmap =  XCreatePixmap( fl_get_display(), fl_get_canvas_id( pfdmMain->DrawingArea ),
			      new_width, bottom_pixmap_height, fl_get_visual_depth() );
      XFillRectangle( fl_get_display(), bottom_pixmap, gcInvalid_context,
		      0, 0, new_width, bottom_pixmap_height );
   }

   pmPixmap = XCreatePixmap( fl_get_display(), fl_get_canvas_id( pfdmMain->DrawingArea ),
			     new_width, new_height, fl_get_visual_depth() );
   actual_width = new_width;
   actual_height = new_height;
}

void XfPresentation::SetScaleFactor( float factor )
{
   float scale = factor*factor;

   if( scale > 1.99 ) {
      current_font = fonts[5]; // 24 pt.
      current_italic_font = italic_fonts[4]; // 18 pt.
   }
   else if( scale >= 1.5 ) {
      current_font = fonts[4]; // 18 pt.
      current_italic_font = italic_fonts[3]; // 14 pt.
   }
   else if( scale >= 1.0 ) {
      current_font = fonts[3]; // 14 pt.
      current_italic_font = italic_fonts[2]; // 12 pt.
   }
   else if( scale >= 0.75 ) {
      current_font = fonts[2]; // 12 pt.
      current_italic_font = italic_fonts[1]; // 10 pt.
   }
   else if( scale >= 0.5 ) {
      current_font = fonts[1]; // 10 pt.
      current_italic_font = italic_fonts[0]; // 8 pt.
   }
   else {
      current_font = fonts[0]; // 8 pt.
      current_italic_font = italic_fonts[0]; // 8 pt.
   }

}

void XfPresentation::DrawLine( float fX1, float fY1, float fX2, float fY2 )
{
   XDrawLine( fl_get_display(), pmPixmap, gcContext,
	      X_COORD(fX1), Y_COORD(fY1), X_COORD(fX2), Y_COORD(fY2) );
}

void XfPresentation::DrawConnectedLines( float points[][2], int num_points )
{
   static XPoint *points_array;
   static int array_size = 0;

   if( num_points > array_size ) {
      if( array_size != 0 )
	 delete [] points_array;
      points_array = new XPoint[num_points];
      array_size = num_points;
   }

   for( int i = 0; i < num_points; i++ ) {
      points_array[i].x = X_COORD(points[i][0]);
      points_array[i].y = Y_COORD(points[i][1]);
   }

   XDrawLines( fl_get_display(), pmPixmap, gcContext, points_array, num_points, CoordModeOrigin );
}

void XfPresentation::DrawPoly( float fX[], float fY[], int num_points )
{
   XPoint axpPoints[5];
   int i;

   for( i = 0; i < num_points; i++ ) {
      axpPoints[i].x = X_COORD(fX[i]);
      axpPoints[i].y = Y_COORD(fY[i]);
   }

   axpPoints[i].x = X_COORD(fX[0]);
   axpPoints[i].y = Y_COORD(fY[0]);
  
   XDrawLines( fl_get_display(), pmPixmap, gcContext, axpPoints, num_points+1, CoordModeOrigin );   
}

void XfPresentation::DrawFilledPoly( float fX[], float fY[], int num_points )
{
   int i;
   XPoint axpPoints[5];

   for( i=0; i < num_points; i++ ) {
      axpPoints[i].x = X_COORD(fX[i]);
      axpPoints[i].y = Y_COORD(fY[i]);
   }
  
   XFillPolygon( fl_get_display(), pmPixmap, gcContext, axpPoints, num_points, Complex, CoordModeOrigin );
}

void XfPresentation::Refresh()
{
   XCopyArea( fl_get_display(), pmPixmap, win, gcContext,
	      0, 0, actual_width, actual_height, 0, 0 );

   if( right_pixmap != EMPTY_PIXMAP ) {
      XCopyArea( fl_get_display(), right_pixmap, win, gcInvalid_context,
		 0, 0, right_pixmap_width, window_height, actual_width, 0 );
   }

   if( bottom_pixmap != EMPTY_PIXMAP ) {
      XCopyArea( fl_get_display(), bottom_pixmap, win, gcInvalid_context,
	      0, 0, actual_width, bottom_pixmap_height, 0, actual_height );
   }

}
void XfPresentation::Clear()
{
   XFillRectangle( fl_get_display(), pmPixmap, gcClear_context,
		   0, 0, actual_width, actual_height );
}

void XfPresentation::DrawSelectionHandles( float fX, float fY )
{
   float selection_box = selection_radius - SELECTION_MARKER;
   
   FillRegion( fX-selection_box-SELECTION_MARKER, fY-selection_box-SELECTION_MARKER, SELECTION_MARKER, SELECTION_MARKER ); // top left
   FillRegion( fX+selection_box, fY-selection_box-SELECTION_MARKER, SELECTION_MARKER, SELECTION_MARKER ); // top right
   FillRegion( fX+selection_box, fY+selection_box, SELECTION_MARKER, SELECTION_MARKER ); // bottom right
   FillRegion( fX-selection_box-SELECTION_MARKER, fY+selection_box, SELECTION_MARKER, SELECTION_MARKER ); // bottom left
}

void XfPresentation::ClearRegion( float fX, float fY, float fWidth, float fHeight )
{
   XFillRectangle( fl_get_display(), pmPixmap, gcClear_context, X_COORD(fX), Y_COORD(fY),
		   (int)(fWidth*default_width*scale), (int)(fHeight*default_height*scale) );
}
void XfPresentation::DrawCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta )
{
   XDrawArc( fl_get_display(), pmPixmap, gcContext, X_COORD(fX-fRadius), Y_COORD(fY-fRadius),
	     (int)(fRadius*2.0*default_width*scale), (int)(fRadius*2.0*default_height*scale), iAngle_start, iAngle_delta );
}

void XfPresentation::DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta )
{

   XFillArc( fl_get_display(), pmPixmap, gcContext, X_COORD(fX-fRadius), Y_COORD(fY-fRadius),
	     (int)(2.0*fRadius*default_width*scale), (int)(2.0*fRadius*default_height*scale), iAngle_start, iAngle_delta );
}
void XfPresentation::FillRegion( float fX, float fY, float fWidth, float fHeight )
{
   this->SetFgColour( BLUE );
   XFillRectangle( fl_get_display(), pmPixmap, gcContext,		  
		   (int)(ceil((fX*default_width*scale)-left_boundary)), 
		   Y_COORD(fY),
		   (int)(fWidth*default_width*scale), (int)(fHeight*default_height*scale));
   this->SetFgColour( BLACK );
}

void XfPresentation::DrawText( float fX, float fY, const char *text, bool italic, alignment al )
{
   XTextItem xtiItem;
   int text_width, x;
   char buffer[100];

   strcpy( buffer, text );
   xtiItem.chars = buffer;
   xtiItem.nchars = strlen( buffer );
   xtiItem.delta = 0;
   xtiItem.font = (italic ? current_italic_font->fid : current_font->fid );

   //XFillRectangle( fl_get_display(), pmPixmap, gcClear_context, X_COORD(fX), Y_COORD(fY), text_width, 10 );

   x = X_COORD(fX);

   if( al != LEFT_ALIGN ) {
      text_width = XTextWidth( (italic ? current_italic_font : current_font ), buffer, strlen( buffer ) );
      if( al == CENTER )
	 x-= text_width/2;
      else // al == RIGHT_ALIGN
	x-= text_width; 
   }
   
   XDrawText( fl_get_display(), pmPixmap, gcContext, x, Y_COORD(fY), &xtiItem, 1 );
}

float XfPresentation::DetermineWidth( const char *label )
{
   float width, fwid;
   
   if( label ) {
      width = (float)(XTextWidth( base_font, label, strlen(label) ));
      fwid = width/dw;
      return( fwid );
   }
   else
      return( 0.0 );
}

void XfPresentation::SetLinePattern( eLine_styles elsLine_style )
{
   elsCurrent_line_style = elsLine_style;
  
   XSetLineAttributes( fl_get_display(),
		       gcContext,
		       iLine_width,
		       (elsLine_style==LINE_SOLID)?LineSolid:
		       (elsLine_style==LINE_DASHED)?LineOnOffDash:LineDoubleDash,
		       CapButt,
		       JoinMiter );
}

void XfPresentation::SetFillPattern( eFill_styles efsFill_style )
{
   efsCurrent_fill_style = efsFill_style;
  
   XSetFillStyle( fl_get_display(),
		  gcContext,
		  (efsFill_style==FILL_SOLID)?FillSolid:FillStippled );
}

void XfPresentation::SetLineWidth( int iwidth )
{
   iLine_width = iwidth;
  
   XSetLineAttributes( fl_get_display(),
		       gcContext,
		       iLine_width,
		       (elsCurrent_line_style==LINE_SOLID)?LineSolid:
		       (elsCurrent_line_style==LINE_DASHED)?LineOnOffDash:LineDoubleDash,
		       CapButt,
		       JoinMiter );
}

void XfPresentation::SetFgColour( int iIndex )
{
   XSetForeground( fl_get_display(), gcContext, fl_get_flcolor( iIndex ) );
}

void XfPresentation::GetFgColour( float& fR, float &fG, float &fB )
{
   fR = fFg_red;
   fG = fFg_green;
   fB = fFg_blue;
}

void XfPresentation::GetBgColour( float& fR, float &fG, float &fB )
{
   fR = fBg_red;
   fG = fBg_green;
   fB = fBg_blue;
}

void XfPresentation::SetStipplePattern( eStipple_patterns esPattern )
{
   if( esPattern == PATTERN1 ) {
      XSetStipple( fl_get_display(), gcContext, pmPattern1 );
   } else {
      XSetStipple( fl_get_display(), gcContext, pmPattern2 );
   }
}

int XfPresentation::GetLineWidth()
{
   return( iLine_width );
}

void XfPresentation::SetFontSize( eFont_sizes efSize ) {}
