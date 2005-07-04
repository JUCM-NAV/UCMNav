/***********************************************************
 *
 * File:			pspresentation.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

/*
 *	File: 		pspresentation.cc
 *	Classes: 	PSPresentation
 */

#include "pspresentation.h"
#include "print_mgr.h"
#include "stub.h"
#include "map.h"
#include <iostream>
#include <math.h>
#include <time.h>

#define POINTS_PER_INCH  72
#define ASPECT_RATIO 0.8333333
#define BORDER_PADDING 10
#define CAPTION_HEIGHT 30
#define TITLE_HEIGHT 40

#define TX(x) (int)(((x+x_offset)*scale_factor)*width)+xpad
#define TY(y) (int)(graphic_height - ((y-y_offset)*scale_factor)*height)+ypad

PSPresentation::PSPresentation( FILE *file, const char *name, float drawing_width, float left, float right, float top,
				float bottom, bool standalone, const char *title, const char *map_type, const char *pdf_index )
{
   float x_scale, y_scale, map_width, map_height;
   int graphic_base, box_width, box_height;
   char font[60], italic_font[60];

   standalone_graphic = standalone;
   graphic_base = standalone_graphic ? 0 : CAPTION_HEIGHT;
   map_width = right - left;
   map_height = bottom - top;
   x_scale = ( map_width > 1 ) ? (1-2*MAP_BORDER)/map_width : 1;
   y_scale = ( map_height > 1 ) ? (1-2*MAP_BORDER)/map_height : 1; 
   scale_factor = Min( x_scale, y_scale );

   width = (int)(drawing_width*POINTS_PER_INCH)-2*BORDER_PADDING;
   height = (int)(width*ASPECT_RATIO);
   graphic_width = width+2*BORDER_PADDING;
   graphic_height = (int)(width*ASPECT_RATIO*map_height*scale_factor)+2*BORDER_PADDING;

   x_offset = (( 1.0 - (right*scale_factor) ) - (left*scale_factor) )/2;
   y_offset = top;
   xpad = BORDER_PADDING;
   ypad = graphic_base - BORDER_PADDING;
   hFile = file;

   box_width = graphic_width;
   box_height = standalone_graphic ? graphic_height : (graphic_height + CAPTION_HEIGHT + TITLE_HEIGHT);
   
   // scale default font
   sprintf( font, "/normal /Times-Roman findfont %.3f scalefont def\n", 8.8*scale_factor );
   sprintf( italic_font, "/italic /Times-BoldItalic findfont %.3f scalefont def\n", (8.8*scale_factor)-2 );

   if( pdf_index ) {
      fprintf( hFile, "%d %d place\n", box_width, box_height );
      fprintf( hFile, "%s", pdf_index );
   }
   
   fprintf( hFile, "%%!PS-Adobe-2.0 EPSF-2.0\n" );
   if( standalone_graphic ) {
      fprintf( hFile, "%%%%Title: %s\n", DesignName( name ) );
      fprintf( hFile, "%%%%Creator: ucmnav\n" );
      fprintf( hFile, "%%%%CreationDate: %s", CurrentDate() );
   }
   fprintf( hFile, "%%%%Orientation: Portrait\n" );
   fprintf( hFile, "%%%%BoundingBox: 0 0 %d %d\n", box_width, box_height );

   if( !standalone_graphic ) {  // print map title (type/name) above and caption (title) below
      
      fprintf( hFile, "/Times-Bold findfont 13 scalefont setfont\n1 setlinecap\n3 setlinewidth\n" );
      fprintf( hFile, "2 %d moveto %d %d lineto stroke\n", box_height-10, box_width-2, box_height-10 );
      fprintf( hFile, "2 %d moveto %d %d lineto stroke\n", box_height-32, box_width-2, box_height-32 );
      fprintf( hFile, "1 setlinewidth\n" );
      fprintf( hFile, "1 %d moveto %d %d lineto stroke\n", box_height-7, box_width-1, box_height-7 );
      fprintf( hFile, "1 %d moveto %d %d lineto stroke\n", box_height-35, box_width-1, box_height-35 );
      fprintf( hFile, "0 setlinecap\n" );
      fprintf( hFile, "2 %d moveto\n(%s) show\n", box_height-25, PrintManager::PrintPostScriptText( name ) ); // left align name
      fprintf( hFile, "(%s Map)\ndup stringwidth pop %d exch sub 2 sub %d moveto\nshow\n", map_type, box_width, box_height-25 ); // right align type
      if( title ) {
	 fprintf( hFile, "/Times-Bold findfont 12 scalefont setfont\n" );
	 fprintf( hFile, "(%s)\ndup stringwidth pop %d exch sub 2 div %d moveto\nshow\n", PrintManager::PrintPostScriptText( title ),
		  box_width, CAPTION_HEIGHT/2 ); // center title
      }
   }

   // output PostScript macros
   fprintf( hFile, "/l { moveto show } bind def\n" ); // left alignment of text 
   fprintf( hFile, "/c { 2 index stringwidth pop 2 div 3 -1 roll exch sub exch moveto show } bind def\n" ); // centered alignment of text 
   fprintf( hFile, "/r { 2 index stringwidth pop 3 -1 roll exch sub exch moveto show } bind def\n" ); // right alignment of text 
   
   fprintf( hFile, font );
   fprintf( hFile, italic_font );
   fprintf( hFile, "/nf {normal setfont} bind def\n" );
   fprintf( hFile, "/if {italic setfont} bind def\n" );
   fprintf( hFile, "nf\n" ); // set default font to normal font
   fprintf( hFile, "newpath\n" );

   SetLineWidth( 2 );
   elsCurrent_line_style = LINE_SOLID;
   efsCurrent_fill_style = FILL_SOLID;

   // draw bounding rectangle
   fprintf( hFile, "1 %d moveto\n%d %d lineto\n%d %d lineto\n1 %d lineto\nclosepath stroke\n", graphic_base,  graphic_width-1, graphic_base,
	    graphic_width-1, graphic_base+graphic_height-1, graphic_base+graphic_height-1 );
}

PSPresentation::~PSPresentation()
{
   if( standalone_graphic ) {
      fprintf( hFile, "showpage\n" );
      fclose( hFile );
   }
}

const char * PSPresentation::DesignName( const char *filename )
{
   int i = 0;
   const char *design_name;

   while( filename[i] != 0 ) {
      if( filename[i] == '/' )
	 design_name = &filename[i];
      i++;
   }
   design_name++; // advance to first character of filename

   return( design_name );
}

const char * PSPresentation::CurrentDate()
{
   time_t ct;
   time(&ct);
   
   return( ctime( &ct ) );
}

void PSPresentation::DrawLine( float fX1, float fY1, float fX2, float fY2 )
{
#if 0   
   if( eoOrientation == LANDSCAPE ) {
      float fT = fX1;
      fX1 = fY1;
      fY1 = fT;
      fT = fX2;
      fX2 = fY2;
      fY2 = fT;
   }
#endif
   
   fprintf( hFile, "%d %d moveto\n%d %d lineto\nstroke\n", TX(fX1), TY(fY1), TX(fX2), TY(fY2) );
}

void PSPresentation::DrawPoly( float fX[], float fY[], int iNum_points )
{
   int i;
   
#if 0   
   float fNew_x[5], fNew_y[5];
   for( i=0; i < iNum_points; i++ ) {
      if( eoOrientation == LANDSCAPE ) {
	 fNew_x[i] = fY[i];
	 fNew_y[i] = fX[i];
      } else {
	 fNew_x[i] = fX[i];
	 fNew_y[i] = fY[i];
      }
   }
#endif

   fprintf( hFile, "%d %d moveto\n", TX(fX[0]), TY(fY[0]) );
   for( i = 1; i < iNum_points; i++ )
      fprintf( hFile, "%d %d lineto\n", TX(fX[i]), TY(fY[i]) );
   fprintf( hFile, "closepath stroke\n" );
}

void PSPresentation::DrawFilledPoly( float fX[], float fY[], 
				     int iNum_points )
{
   int i;

#if 0   
   float fNew_x[5], fNew_y[5];
   for( i=0; i < iNum_points; i++ ) {
      if( eoOrientation == LANDSCAPE ) {
	 fNew_x[i] = fY[i];
	 fNew_y[i] = fX[i];
      } else {
	 fNew_x[i] = fX[i];
	 fNew_y[i] = fY[i];
      }
   }
#endif

   fprintf( hFile, "%d %d moveto\n", TX(fX[0]), TY(fY[0]) );
   for( i=1; i < iNum_points; i++ )
      fprintf( hFile, "%d %d lineto\n", TX(fX[i]), TY(fY[i]) );
   fprintf( hFile, "closepath eofill\n" );
}

void PSPresentation::DrawStub( float fX[], float fY[], int colour, Stub *stub )
{
   Presentation::DrawStub( fX, fY, colour, stub );
   
   // insert pdf anchor, if applicable
   if( PrintManager::pdf_output ) {
      fprintf( hFile, "[/Dest /%s", PrintManager::PrintPostScriptText( stub->ParentMap()->MapLabel(), FALSE ) );
      if( stub->IsShared() )
	 fX[0]-=2*EMPTY_FIGURE_SIZE;
      fprintf( hFile, "Stub%s /Rect[%d %d %d %d]\n", PrintManager::PrintPostScriptText( stub->StubName(), FALSE ),
	       TX(fX[0])-1, TY(fY[1])-2, TX(fX[2])+1, TY(fY[3])+10 );
      fprintf( hFile, "/Border [0 0 0] /Color [1 0 0] /InvisibleRect /I /Subtype /Link /ANN pdfmark\n" );
   }
}

void PSPresentation::DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear )
{
   float radius;
   int r;
   
   radius = Min( fWidth, fHeight ); // calculate radius of curvature
   radius *= RADIUS_FACTOR;
   r = (int)(radius*scale_factor*width);

   // define rounded rectangle path
   fprintf( hFile, "newpath\n%d %d moveto\n", TX(fX+radius), TY(fY) ); // move to origin at left of top line
   fprintf( hFile, "%d %d lineto\n", TX(fX+fWidth-radius), TY(fY) ); // define top line
   fprintf( hFile, "%d %d %d %d %d arct\n", TX(fX+fWidth), TY(fY), TX(fX+fWidth), TY(fY+radius), r ); // define top right arc
   fprintf( hFile, "%d %d lineto\n", TX(fX+fWidth), TY(fY+fHeight-radius) ); // define right line
   fprintf( hFile, "%d %d %d %d %d arct\n", TX(fX+fWidth), TY(fY+fHeight), TX(fX+fWidth-radius), TY(fY+fHeight), r ); // define bottom right arc
   fprintf( hFile, "%d %d lineto\n", TX(fX+radius), TY(fY+fHeight) ); // define bottom line
   fprintf( hFile, "%d %d %d %d %d arct\n", TX(fX), TY(fY+fHeight), TX(fX), TY(fY+fHeight-radius), r ); // define bottom left arc
   fprintf( hFile, "%d %d lineto\n", TX(fX), TY(fY+radius) ); // define left line
   fprintf( hFile, "%d %d %d %d %d arct\nclosepath\n", TX(fX), TY(fY), TX(fX+radius), TY(fY), r ); // define top left arc

   if( clear ) {
      this->SetFgColour( WHITE );  // clear object interiors
      fprintf( hFile, "gsave eofill grestore\n" );  // close the path and clear
   }

   this->SetFgColour( colour ); // draw objects
   fprintf( hFile, "stroke\n" );  // draw the rounded rectangle path
}

void PSPresentation::DrawCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta )
{
#if 0   
   if( eoOrientation == LANDSCAPE ) {
      float fT = fX;
      fX = fY;
      fY = fT;
   }
#endif

   fprintf( hFile, "newpath %d %d %d %d %d arc stroke\n",
	    TX(fX), TY(fY),
	    (int)(fRadius*scale_factor*width),
	    iAngle_start/64,
	    iAngle_delta/64+iAngle_start/64 );
}

void PSPresentation::DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta )
{
#if 0   
   if( eoOrientation == LANDSCAPE ) {
      float fT = fX;
      fX = fY;
      fY = fT;
   }
#endif

   fprintf( hFile, "newpath %d %d %d %d %d arc fill\n",
	    TX(fX), TY(fY),
	    (int)(fRadius*scale_factor*width),
	    iAngle_start/64,
	    iAngle_delta/64+iAngle_start/64 );
}


void PSPresentation::DrawText( float fX, float fY, const char *text,	bool italic, alignment al )
{
#if 0   
   if( eoOrientation == LANDSCAPE ) {
      float fT = fX;
      fX = fY;
      fY = fT;
   }
#endif

   static char ps_align[][2] = { "l", "c", "r" };
   
   if( italic ) fprintf( hFile, "if\n" );
   fprintf( hFile, "(%s) %d %d %s\n", PrintManager::PrintPostScriptText( text ), TX(fX), TY(fY), ps_align[al] );
   if( italic ) fprintf( hFile, "nf\n" );
}

void PSPresentation::SetLinePattern( eLine_styles elsLine_style )
{

   elsCurrent_line_style = elsLine_style;

   if( elsLine_style == LINE_DASHED ) {
      fprintf( hFile, "[1 3.3] 3.3 setdash\n" );
   } else if( elsLine_style == LINE_SOLID ) {
      fprintf( hFile, "[] 0.0 setdash\n" );
   }
}

void PSPresentation::SetFillPattern( eFill_styles efsFill_style )
{
   efsCurrent_fill_style = efsFill_style;
}

void PSPresentation::SetLineWidth( int iwidth )
{
   iLine_width = (int)(iwidth*scale_factor);
   fprintf( hFile, "%f setlinewidth\n", iwidth*scale_factor );
}

void PSPresentation::SetFgColour( int iIndex )
{
   int iR, iG, iB;

   if( iIndex != FL_GRAY63 ) { // hack to make printout darker      
      fl_get_icm_color( iIndex, &iR, &iG, &iB );
      SetFgColour( (float)iR/255.0, (float)iG/255.0, (float)iB/255.0 );
   } else
      SetFgColour( 0.45, 0.45, 0.45 );
}

void PSPresentation::SetFgColour( float fR, float fG, float fB )
{
   fprintf( hFile, "%f %f %f setrgbcolor\n", fR, fG, fB );
}

void PSPresentation::SetBgColour( float fR, float fG, float fB ) {}

void PSPresentation::GetFgColour( float& fR, float &fG, float &fB )
{
   fR = fFg_red;
   fG = fFg_green;
   fB = fFg_blue;
}

void PSPresentation::GetBgColour( float& fR, float &fG, float &fB )
{
   fR = fBg_red;
   fG = fBg_green;
   fB = fBg_blue;
}

void PSPresentation::SetStipplePattern( eStipple_patterns esPattern ) {}

void PSPresentation::SetFontSize( eFont_sizes efSize )
{
   switch( efSize ) {

   case LARGE_FONT:
      fprintf( hFile, "/Times-Roman findfont 15 scalefont setfont\n" );
      break;
    
   case SMALL_FONT:
      fprintf( hFile, "/Times-Roman findfont 10 scalefont setfont\n" );
      break;

   case SMALL_BOLD_FONT:
      fprintf( hFile, "/Times-Bold findfont 10 scalefont setfont\n" );
      break;
   }
}

void PSPresentation::StartCurve( float fX, float fY )
{
   fprintf( hFile, "newpath %d %d moveto\n",  TX(fX), TY(fY) );
}

void PSPresentation::DrawCurve( float fX[], float fY[] )
{
   fprintf( hFile, "%d %d %d %d %d %d curveto\n", TX(fX[1]), TY(fY[1]), TX(fX[2]), TY(fY[2]), TX(fX[3]), TY(fY[3]) );
}

void PSPresentation::EndCurve()
{
   fprintf( hFile, "stroke\n" );
}

int PSPresentation::GetLineWidth()
{
   return( iLine_width );
}

void PSPresentation::Refresh() {}
void PSPresentation::Clear() {}
void PSPresentation::ClearRegion( float fX, float fY, float fWidth, float fHeight ) {}
void PSPresentation::FillRegion( float fX, float fY, float fWidth, float fHeight ) {}

