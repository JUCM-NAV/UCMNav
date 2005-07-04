 /***********************************************************
 *
 * File:			svgpresentation.cc
 * Author:			Mitel
 * Created:			September, 2001
 *
 * Modification history:
 *
 ***********************************************************/


//	File: 		SVGpresentation.cc
//	Classes: 	SVGPresentation
//  Description:

#include "svgpresentation.h"
#include "stub.h"
#include "collection.h"
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>

//Class:	SVGPresentation
// Function:	SVGPresentation( char *szFilename, int new_width,
//				int new_height )
// Description:
SVGPresentation::SVGPresentation( FILE *file, const char *name, float new_width, float new_height )
{
   strcpy(svgname, name); //get the name of .svg file.
   fWidth = new_width;
   fHeight = new_height;
   svg_R = 0;  //initialize the RGB factors.
   svg_G = 0;
   svg_B = 0;
   fiLine_width = 0.02; //initialize the width of line.
   strcpy(svg_style, "none"); //initialize the line style.
   
   hFile = file;

   fprintf( hFile, "<?xml version=\"1.0\" standalone=\"no\"?>\n"
	    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" \n"
	    " \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\"> \n" );
   fprintf( hFile, "<svg width=\"800px\" height=\"600px\" viewBox=\"0 0 10 8\"\n");
   fprintf( hFile, "  xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n" );
}

// Class:	SVGPresentation
// Function:	~SVGPresentation()
// Description:
SVGPresentation::~SVGPresentation()
{
   fprintf( hFile, "</svg>\n" );
   fclose( hFile );
}

// Class: SVGPresentation
// Function: DrawLine( float fX1, float fY1, float fX2, float fY2 )
void SVGPresentation::DrawLine( float X1, float Y1, float X2, float Y2 )
{
   fprintf( hFile, "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" stroke-width=\"%f\"  stroke-dasharray=\"%s\" stroke=\"rgb(%d,%d,%d)\" />\n",
                    XtoInches( X1 ), YtoInches( Y1 ), XtoInches( X2 ), YtoInches( Y2 ), fiLine_width, svg_style, svg_R, svg_G, svg_B);
}

// Class:  SVGPresentation
// Function: DrawConnectedLines( float points[][2], int num_points )
void SVGPresentation::DrawConnectedLines( float points[][2], int num_points )
{
   if((svg_R==0)&&(svg_G==0)&&(svg_B==0)) fiLine_width=0.02;
   fprintf( hFile, "<polyline fill=\"none\" stroke=\"rgb(%d,%d,%d)\" stroke-width=\"%f\" stroke-dasharray=\"%s\"\n",
                   svg_R, svg_G, svg_B, fiLine_width, svg_style );
   fprintf( hFile, "  points=\"\n");
      
   for( int i = 0; i < num_points; i++ )
      fprintf( hFile, "          %f, %f\n", XtoInches( points[i][0] ), YtoInches( points[i][1] ) );
   fprintf( hFile, "\"/>\n");
}

// Class: SVGPresentation
// Function: DrawPoly( float fX[], float fY[], int iNum_points )
// Description:
void SVGPresentation::DrawPoly( float fX[], float fY[], int iNum_points ) {
   int i;

   fprintf( hFile, "<polygon fill=\"none\" stroke=\"rgb(%d,%d,%d)\" stroke-width=\"%f\" stroke-dasharray=\"%s\"\n",
                   svg_R, svg_G, svg_B,  fiLine_width, svg_style );
   fprintf( hFile, "  points=\"\n");

   for( i=0; i < iNum_points; i++ ) {
      fprintf( hFile, "          %f, %f\n", XtoInches( fX[i] ), YtoInches( fY[i] ) );
   }
   fprintf( hFile, "\"/>\n");
}

// Class: SVGPresentation
// Function: DrawFilledPoly( float fX[], float fY[], int iNum_points )
// Description:
void SVGPresentation::DrawFilledPoly( float fX[], float fY[],
				      int iNum_points )
{int i;

   if((svg_R==0)&&(svg_G==0)&&(svg_B==0)) fiLine_width=0.02;
   fprintf( hFile, "<polygon fill=\"white\"");
   fprintf( hFile, "  stroke=\"rgb(%d,%d,%d)\" stroke-width=\"%f\" stroke-dasharray=\"%s\"\n",
                    svg_R, svg_G, svg_B, fiLine_width, svg_style);
   fprintf( hFile, "  points=\"\n");

   for( i=0; i < iNum_points; i++ ) {
      fprintf( hFile, "          %f, %f\n", XtoInches( fX[i] ), YtoInches( fY[i] ) );
   }
   fprintf( hFile, "\"/>\n");
}

// Class: SVGPresentation
// Function: DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear = TRUE )
// Description:
void SVGPresentation::DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear )
{
   float xradius, yradius;

   xradius = fWidth; // calculate radius of curvature
   xradius *= RADIUS_FACTOR;
   yradius = fHeight;
   yradius *= RADIUS_FACTOR;

   SetFgColour( colour );  // set the colour of the rounded rectangle

   fprintf( hFile, "<rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" rx=\"%f\" ry=\"%f\" fill=\"none\" stroke-width=\"%f\"  stroke-dasharray=\"%s\" stroke=\"rgb(%d,%d,%d)\" />\n",
                    XtoInches( fX), YtoInches( fY ), XtoInches( fWidth ), YtoInches( fHeight ), XtoInches( xradius ), XtoInches( yradius ), fiLine_width, svg_style, svg_R, svg_G, svg_B);
}

// Class: SVGPresentation
// Function: DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear = TRUE )
// Description: Drawing stubs and create hyperlinks to the plugin maps.
void SVGPresentation::DrawStub( float fX[], float fY[], int colour, Stub *stub )
{   char* psvgname;
    int i =0;
    char stubname[30];
    // char fstubname[80];
    psvgname = svgname;  //make "psvgname" point to the name of .svg file.
    while(psvgname[i]!='\0') i++; //scan the string to the end.
    while(psvgname[i]!='/') i--;   //scan  back until meeting the '/' and get the position of last '/' in the string.
    psvgname+=i+1;  //move the pointer to the position of next to '/'.
    strcpy( stubname, stub->StubName() );  //get the name of stub.
    strcat( stubname, ".svg" );  //concatenate ".svg" to  the name of stub.
    i=0;
    while(psvgname[i]!='-') i++;
    psvgname[i+1]='\0';
    strcat(psvgname, stubname);   //change "root" into the name of stub.

    fprintf( hFile, "<a xlink:href=\"%s\">\n", psvgname);
    Presentation::DrawStub( fX, fY, colour, stub );
    fprintf( hFile, "</a>\n");
}

void SVGPresentation::ClearStubInterior( float fX[], float fY[] )
{
   //SetFgColour( WHITE );
   DrawFilledPoly( fX, fY, 4 );
}

void SVGPresentation::DrawStubOutline( float fX[], float fY[], int colour )
{
   //SetFgColour( colour );
   DrawPoly( fX, fY, 4 );
}
// Class:	void SVGPresentation
// Function:	Refresh()
// Description:
void SVGPresentation::Refresh() {}

// Class:	void SVGPresentation
// Function:	Clear()
// Description:
void SVGPresentation::Clear() {}

void SVGPresentation::ClearRegion( float fX, float fY, float w, float h ) {}

// Class:	void SVGPresentation
// Function:	DrawCircle( float fX, float fY, float fRadius )
// Description:
void SVGPresentation::DrawCircle( float fX, float fY, float fRadius , int iAngle_start, int iAngle_delta) {
   fprintf( hFile, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\"\n", 
	    XtoInches( fX ), YtoInches( fY ), XtoInches( fRadius ));
   fprintf( hFile, "    fill=\"none\" stroke=\"black\" stroke-width=\"%f\" stroke-dasharray=\"%s\"/>\n", fiLine_width, svg_style );
}

// Class:	void SVGPresentation
// Function:	DrawFilledCircle( float fX, float fY, float fRadius )
// Description:
void SVGPresentation::DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta ) {
   fprintf( hFile, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\"\n", 
	    XtoInches( fX ), YtoInches( fY ), XtoInches( fRadius ));
   fprintf( hFile, "fill=\"rgb(%d,%d,%d)\"", svg_R, svg_G, svg_B);
   fprintf( hFile, "  stroke=\"black\" stroke-width=\"%f\" stroke-dasharray=\"%s\" />\n", fiLine_width, svg_style );
}

void SVGPresentation::FillRegion( float fX, float fY, float w, float h ) {}

// Class:	float SVGPresentation
// Function:	DrawText( float fX, float fY, char *text,
// Description:
void SVGPresentation::DrawText( float fX, float fY, const char *text, bool italic, alignment al )
{
   static char ps_align[][7] = {
      "start",
      "middle",
      "end" };
   fprintf( hFile, "<text x=\"%f\" y=\"%f\"\n", XtoInches( fX ), YtoInches( fY ) );
   fprintf( hFile, "   text-anchor=\"%s\"", ps_align[al] );
   if( italic )
       fprintf( hFile, " font-style=\"italic\" font-family=\"Verdana\" font-size=\"0.07\" fill=\"black\" >\n");
   else
       fprintf( hFile, " font-family=\"Verdana\" font-size=\"0.1\" fill=\"black\" >\n");
   fprintf( hFile, "  %s\n", text );
   fprintf( hFile, "  </text>\n");
}

// Class:	void MIFPresentation
// Function:	SetLinePattern( eLine_styles elsLine_style )
// Description:
void SVGPresentation::SetLinePattern( eLine_styles elsLine_style )
{
   elsCurrent_line_style = elsLine_style;
   if( elsLine_style == LINE_DASHED )
      strcpy(svg_style, "0.05,0.02");
   else if( elsLine_style == LINE_DOUBLE_DASHED )
      strcpy(svg_style, "0.05,0.02");
   else
      strcpy(svg_style, "none");
}


// Class:	void SVGPresentation
// Function:	SetFillPattern( eFill_styles efsFill_style )
// Description:
void SVGPresentation::SetFillPattern( eFill_styles efsFill_style )
{
 efsCurrent_fill_style = efsFill_style;
}

// Class:	void SVGPresentation
// Function:	SetLineWidth( int width )
// Description:
void SVGPresentation::SetLineWidth( int wid ) {
if(wid==0) fiLine_width = 0.02;
else
    {iLine_width = wid;
     fiLine_width= float(iLine_width)/100;
    }
}

// Class:	void SVGPresentation
// Function:	SetFgColour( int iIndex )
// Description:
void SVGPresentation::SetFgColour( int iIndex ) {
    int iR, iG, iB;
    if( iIndex != FL_GRAY63 ) { // hack to make printout darker
      fl_get_icm_color( iIndex, &iR, &iG, &iB );
   } else
   {  iR = 114;
      iG = 114;
      iB = 114;
   }
    svg_R=iR; svg_G=iG; svg_B=iB;
}


// Class:	void SVGPresentation
// Function:	GetFgColour( float& fR, float &fG, float &fB )
// Description:
void SVGPresentation::GetFgColour( float& fR, float &fG, float &fB ) {
fR = svg_R; fG = svg_G; fB = svg_B;
}

// Class:	void SVGPresentation
// Function:	GetBgColour( float& fR, float &fG, float &fB )
// Description:
void SVGPresentation::GetBgColour( float& fR, float &fG, float &fB ) {}

// Class:	void SVGPresentation
// Function:	SetStipplePattern( eStipple_patterns esPattern )
// Description:
void SVGPresentation::SetStipplePattern( eStipple_patterns esPattern ) {}

// Class:	void SVGPresentation
// Function:	SetFontSize( eFont_sizes efSize )
// Description:
void SVGPresentation::SetFontSize( eFont_sizes efSize ) {}

// Class:	void SVGPresentation
// Function: GetLineWidth()
// Description:
int SVGPresentation::GetLineWidth()
{
  return( iLine_width );
}

void SVGPresentation::SetDashPattern( char rgcPattern[], int iNum ) {}

float SVGPresentation::XtoInches( float fX )
{
   return( fX*fWidth );
}

float SVGPresentation::YtoInches( float fY )
{
   return( fY*fHeight );
}


