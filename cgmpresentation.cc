/**************************************************************************************************************
 *
 * File:			cgmpresentation.cc
 * Author:			Kevin Lam
 * Created:			October 1999
 *
 * Modification history:	01/15/00 - started work (finally!)
 *                              01/21/00 - test file output
 *                              02/15/00 - gave up on cleartext output; start binary encoding
 *                              04/11/00 - preliminary release (beta) - minor known bugs
 *                              04/12/00 - fixed colour bugs
 *                              04/13/00 - fixed rounded rectangles, removed spline curves
 *                              04/30/00 - final set of tweaks and bug fixes for "version 1.0"
 *                              05/06/00 - hand off code to Andrew for integration into UCMNav tool
 *
 **************************************************************************************************************/


#include "cgmpresentation.h"
#include "print_mgr.h"
#include <iostream>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// CGM Binary Format specifications
// For the purposes of this implementation, codes are given in Hex-formatted character strings
// Function outputString() sends binary characters corresponding to them, ie. "FF" --> 0xFF

#define BORDER 0.03
#define TX(x) (x+BORDER)
#define TY(y) ((bottom_border-y)+top_border+BORDER)

CGMPresentation::CGMPresentation( FILE *file, const char *name, float left, float right, float top, float bottom )
{
   float xwidth, ywidth, coverage;

   hFile = file;
   
   // a CGM file must be initialized with the following codes: (from deconstructing an existing CGM)
   // NOTE:  I may not have deconstructed the CGM file very well, as PowerPoint imports still produce errors
   
   // BEGIN METAFILE
   outputElementCode(0, 1, 10);  // BEGIN METAFILE tag + a string, 10 characters long
   fprintf(hFile, "UCMNav CGM"); // this is the name of the CGM file
   
   //    METAFILE VERSION
   outputElementCode(1, 1, 2);   // tag + 2 byte int
   outputInt(3);                 // metafile version 3 (perhaps should be 2?)
   
   //    METAFILE DESCRIPTION
   outputString("104A55434D4E61762043474D");               // Metafile Description: "UCMNav CGM"
   
   //    VDC Type  outputString("10620000");                               // VDCType: Integer

   //    Integer Precision
   outputString("10820010");                               // Integer precision: 16 bits

   //    Real Precision
   outputString("10A6000100100010");                       // Fixed-point, 16 bits each part

   //    Colour Precision
   outputString("10E20008");                               // Colour Precision: 8 bits per RGB value

   //    Font List: Arial and Arial Italic (as font 1 and font 2)
   outputElementCode(1, 13, 19);         // 19 total bytes follow this element code
   fprintf(hFile,"%cArial", 5);          // output the length of the string "Arial" followed by the string itself
   fprintf(hFile,"%cArial Italic", 12);  // same for "Arial Italic"
   fprintf(hFile,"%c", 0);               // 19 is an odd number so null-pad to 20 bytes
   
   //    BEGIN PICTURE
   outputString("006A55434D4E61762043474D");               // Begin Picture: "UCMNav CGM"
   
   //    Scaling Mode
   outputString("2026000100000483");                       // Scaling mode: not sure (from disassembled CGM)

   //    Background Colour
   outputString("20E3FFFFFF00");                           // Background colour: white (255/255/255)

   //    Line Width and Edge Width modes
   outputString("2062000120A20001");                       // Set line width and edge width modes: scaled

   //    Line Type and Edge Type Continuation (restart dashed pattern)
   outputString("54E2000355C20003");
   
   //    Colour Selection Mode
   outputString("20420001");                               // Set colour selection mode to direct (RGB)

   //    VDC Extent: output the (x1,y1)-(x2,y2) dimensions of the graphic
   outputString("20C8");
   
   if (left < 0.001) left = 0.001;                            // No negative coordinates allowed!
   if (top < 0.001) top = 0.001;

   outputCoord( left );
   outputCoord( top );
   outputCoord( right + 2*BORDER );
   outputCoord( bottom + 2*BORDER );

   left_border = left ;
   right_border = right;
   top_border = top;
   bottom_border = bottom;
   
   xwidth = right - left;
   ywidth = bottom - top;
   coverage = Max( xwidth, ywidth );
   scale_factor = ((coverage > 1) ? 1/coverage : 1);

   //       BEGIN PICTURE BODY
   outputString("0080");

   //       VDC Integer Precision
   outputString("30220010");                               // Use 16-bit integers in VDC's

   //       Interior Fill Style and Edge Visibility
   outputString("52C2000153C20001");                       // Solid filled interiors, visible edges

   //       Default colors to black, line width set, etc.
   SetFgColour(0);
   SetLineWidth(2);
   
   // set Line Cap to Projection Square End, and Line Join to Mitre
   outputElementCode(5, 37, 4);
   outputInt(4);   // project regular lines
   outputInt(3);   // match dashed lines

   outputElementCode(5, 38, 2);
   outputInt(2);   // Mitre
   
   //          <elements>            the rest of this class takes care of these
   //    END PICTURE                 the deconstructor takes care of these
   // END METAFILE
   
}

CGMPresentation::~CGMPresentation()
{
   outputString("00A00040");            // end the CGM file
   fclose( hFile );
}


// ****************************************************************************************************
//  Utility functions: to generate properly-formatted CGM codes
// ****************************************************************************************************

// outputCoord: outputs a coordinate to the CGM file stream, encoded as a 16-bit unsigned integer.
//              i.e. 6.5535 -> 65535 -> FF FF
//              This method of outputting UCM coordinates does lose a few significant digits of
//              accuracy, but encoding ints is easier than encoding reals into binary form.
void CGMPresentation::outputCoord(float coval)
{
   unsigned int ival;
   char hibyte, lobyte;

   if (coval < 0.0)           // negative coordinates not allowed!
      coval = 0.0;
   
   ival = (unsigned int)( coval * 10000.0 );

   hibyte = ival / 256;
   lobyte = ival % 256;

   fprintf( hFile, "%c%c", hibyte, lobyte );
}

// outputElementCode: outputs a CGM format opcode given a 4-bit class, 7-bit ID, and 5-bit length
//               DOES NOT (and CANNOT) CHECK FOR VALIDITY OF CODES!  So have a CGM handbook at your side
//               if debugging is necessary.
void CGMPresentation::outputElementCode(int CL, int ID, int len)
{
   unsigned int ival;
   char hibyte, lobyte;

   ival = (CL * 4096) + (ID * 32) + len;
   
   hibyte = ival / 256;
   lobyte = ival % 256;

   fprintf( hFile, "%c%c", hibyte, lobyte );
}

// outputInt: outputs an integer as a 16-bit value, i.e. 65535 --> FF FF
//            does not check bounds, assumes an unsigned value between 0 to 65535
void CGMPresentation::outputInt( unsigned int ival )
{
   char hibyte, lobyte;

   hibyte = ival / 256;
   lobyte = ival % 256;

   fprintf( hFile, "%c%c", hibyte, lobyte );
}

// outputString: takes a string and extracts bytes out of it, outputting them to a file
//              i.e. "001234ABCD" results in bytes output: 00, then 12, 34, AB, and CD
//              Assumes that the string is composed of an EVEN number of characters
//              and does NOT check for errors; this function should only be used to output hard-coded values
void CGMPresentation::outputString( const char *byteString )
{
   unsigned long int extractB;
   char wordStr[3];

   wordStr[2] = 0; // null terminate input string
   
   for (int i = 0, j = strlen(byteString); i < j; i+=2 )
   {
      wordStr[0] = byteString[i];       // copy a 2-character "word" string (such as "A0") into the string
      wordStr[1] = byteString[i+1];
      extractB = strtol(wordStr, NULL, 16);   // convert the word into an integer from 0-255
      fprintf( hFile, "%c", (char)extractB);  // and dump the raw byte into the file
   }

}

// ****************************************************************************************************
//  End of utility functions -- begin graphical element functions
//  note the inversion of the y coordinates due to the "upside down" coordinate system
//  i.e. newY = bottom_border - oldY
// ****************************************************************************************************

// works correctly: draws a straight line between the two given points
// line width is defined by previous calls to SetLineWidth() function
void CGMPresentation::DrawLine( float fX1, float fY1, float fX2, float fY2 )
{
   outputElementCode(4, 1, 8);
   outputCoord(TX(fX1));
   outputCoord(TY(fY1));
   outputCoord(TX(fX2));
   outputCoord(TY(fY2));
}


// draws polygons of any number of points (usually called with 4 to generate quadrilateral shapes)
// draws unfilled polygons; line width and color as set through previous calls to SetLineWidth() and SetFgColor()
void CGMPresentation::DrawPoly( float fX[], float fY[], int iNum_points )
{
   // set up unfilled type (intstyle hollow)
   outputElementCode(5, 22, 2);
   outputInt(0);
   
   if ( iNum_points <= 7) {                   // 7 points = 14 coordinates = 28 bytes, so write short form

      outputElementCode(4, 7, iNum_points * 4);       // POLYGON line primitive
      
      for ( int i = 0 ; i < iNum_points ; i++ ) {
	 outputCoord(TX(fX[i]));
	 outputCoord(TY(fY[i]));
      }
   }
   else
   {
      // This part has never been called in general operation and thus has not been tested thoroughly, but should work
      printf("DrawPoly: Untested multiple-point polygon requested!\n");

      outputElementCode(4, 7, 31);
      outputInt(iNum_points * 4);
      for ( int i = 0 ; i < iNum_points ; i++ ) {
	 outputCoord(TX(fX[i]));
	 outputCoord(TY(fY[i]));
      }
   }
}
 
// draws polygons of any number of points (usually called with 4 to generate quadrilateral shapes)
// draws filled polygons; line and fill color as set through previous calls to SetFgColor()
void CGMPresentation::DrawFilledPoly( float fX[], float fY[], int iNum_points )
{
   // set up filled type (intstyle filled)
   outputElementCode(5, 22, 2);
   outputInt(1);
   
   if ( iNum_points <= 7) {                   // 7 points = 14 coordinates = 28 bytes, so write short form

      outputElementCode(4, 7, iNum_points * 4);       // POLYGON line primitive
      
      for ( int i = 0 ; i < iNum_points ; i++ ) {
	 outputCoord(TX(fX[i]));
	 outputCoord(TY(fY[i]));
      }
   }
   else
   {
      // This part has never been called in general operation and thus has not been tested thoroughly.
      printf("DrawFilledPoly: Untested multiple-point polygon requested!\n");
      
      outputElementCode(4, 7, 31);
      outputInt(iNum_points * 4);
	 
      for ( int i = 0 ; i < iNum_points ; i++ ) {
	 outputCoord(TY(fX[i]));
	 outputCoord(TY(fY[i]));
      }
   }
}

// creates rectangles with rounded corners.
// current implementation actually composites rounded rectangles out of lines, circles and polygons
// because partial circular arcs seem to be a problem with CGM generation
// current implementation ignores clear flag, and only draws unfilled rounded rectangles
void CGMPresentation::DrawRoundedRectangle( float fX, float fY, float fWidth, float fHeight, int colour, bool clear )
{
   float radius;
   float fXb[4], fYb[4];
   
   radius = Min( fWidth, fHeight ); // calculate radius of curvature
   radius *= RADIUS_FACTOR;

   SetFgColour( colour );           // set the colour of the rounded rectangle

   // draw the circles which will become the rounded corners
   DrawCircle( fX + radius, fY + radius, radius, 0, 23040);
   DrawCircle( fX + fWidth - radius, fY + radius, radius, 0, 23040);
   DrawCircle( fX + radius, fY + fHeight - radius, radius, 0, 23040);
   DrawCircle( fX + fWidth - radius, fY + fHeight - radius, radius, 0, 23040);

   SetFgColour( WHITE );

   // erase the unwanted circle segments with white filled boxes

   fXb[0] = fX; fYb[0] = fY + radius;
   fXb[1] = fX; fYb[1] = fY + fHeight - radius;
   fXb[2] = fX + fWidth; fYb[2] = fY + fHeight - radius;
   fXb[3] = fX + fWidth; fYb[3] = fY + radius;
   DrawFilledPoly( fXb, fYb, 4);

   fXb[0] = fX + radius; fYb[0] = fY;
   fXb[1] = fX + radius; fYb[1] = fY + fHeight;
   fXb[2] = fX + fWidth - radius; fYb[2] = fY + fHeight;
   fXb[3] = fX + fWidth - radius; fYb[3] = fY;
   DrawFilledPoly( fXb, fYb, 4);
   
   // Draw the "real" perimeter lines
   SetFgColour ( colour );
   DrawLine(fX + radius, fY, fX + fWidth - radius, fY);         
   DrawLine(fX + fWidth, fY + radius, fX + fWidth, fY + fHeight - radius);  
   DrawLine(fX + fWidth - radius, fY + fHeight, fX + radius, fY + fHeight); 
   DrawLine(fX, fY + fHeight - radius, fX, fY + radius);
}

// draws unfilled circles.  Current implementation draws complete circles, ignoring requests for partial
// arcs.  I don't know how to interpret the angles, or the "magic" number 23040.
void CGMPresentation::DrawCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta )
{
   if((iAngle_start != 0)||(iAngle_delta != 23040))
   {
      printf("DrawCircle: Unimplemented circle angle requested...!\n");
   }

   outputElementCode(5, 22, 2);    // set type unfilled
   outputInt(0);

   outputElementCode(4, 12, 6);
   outputCoord(TX(fX));
   outputCoord(TY(fY));
   outputCoord(fRadius);

}

// draws filled circles.  Current implementation draws complete circles, ignoring requests for partial
// arcs.  I don't know how to interpret the angles, or the "magic" number 23040.
void CGMPresentation::DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta )
{
   if((iAngle_start != 0)||(iAngle_delta != 23040))
   {
      printf("DrawFilledCircle: Unimplemented circle angle requested...!\n");
   }
   
   outputElementCode(5, 22, 2);    // set type filled
   outputInt(1);

   outputElementCode(4, 12, 6);
   outputCoord(TX(fX));
   outputCoord(TY(fY));
   outputCoord(fRadius);

}

// text output function.  Accepts varying length strings; generates appropriate CGM long-form or short-form commands.
void CGMPresentation::DrawText( float fX, float fY, const char *text, bool italic, alignment al )
{
   int i, l = strlen(text);

   // if the string length is less than 30 characters then output a short form word
   // if the string does not contain an even number of bytes then we must add a null byte at the end
   
   if(italic == TRUE) {
      // Select Font Index 2 (Arial Italic)
      outputString("51420002");

      // select smaller text size
      outputElementCode(5, 15, 2);     // character height
      outputCoord( scale_factor/60 );
   }
   else
   {
      // Select Font Index 1 (Arial)
      outputString("51420001");

      // select larger text size
      outputElementCode(5, 15, 2);     // character height
      outputCoord( scale_factor/40 );
   }

   if (al == LEFT_ALIGN)                   // check alignment flag and set left, right or center alignment
   {
      outputString("52480001000000000000"); // left alignment
   }
   else if (al == CENTER)
   {
      outputString("52480002000000000000"); // center alignment
   }
   else if (al == RIGHT_ALIGN)
   {
      outputString("52480003000000000000"); // right alignment
   }
   
   if (l <= 23)
   {
      outputElementCode(4, 4, l+7);     // 7 bytes taken up by the text command format:
      outputCoord(TX(fX));              // 2 bytes for X coordinate
      outputCoord(TY(fY));              // 2 bytes for Y coordinate
      outputInt(1);                     // 2 bytes for the final flag
      fprintf(hFile, "%c", l);          // 1 byte for the length of the string

      for ( i = 0; i < l ; i++ )
	 fprintf(hFile, "%c", text[i]);
      
      if (l % 2 == 0)                 // null-pad if there was an even number of characters
	 fprintf(hFile, "%c", 0);
   }
   else                                 // else output a long form word including the length byte
   {
      outputElementCode(4, 4, 31);      // long form word    
      outputInt(l + 7);                 //  output number of bytes in total
      outputCoord(TX(fX));              //     2 bytes for X coordinate
      outputCoord(TY(fY));              //     2 bytes for Y coordinate
      outputInt(1);                     //     2 bytes for the final flag
      fprintf(hFile, "%c", l);          //     1 byte for the length of the string

      for (int i = 0; i < l ; i++ )
	 fprintf(hFile, "%c", text[i]);
      
      if (l % 2 == 0)                 // null-pad if there was an even number of characters
	 fprintf(hFile, "%c", 0);
   }
}

// Current implementation seems to do funny things with dashed line styles; output is strange looking
// tested only with cgmVA viewer for Java and PowerPoint import
// function affects both line and edge style patterns
void CGMPresentation::SetLinePattern( eLine_styles elsLine_style )
{

   if(elsLine_style == LINE_SOLID)
   {
      outputElementCode(5, 2, 2);   // set both line type and edge type to requested style
      outputInt(1);
      outputElementCode(5, 27, 2);
      outputInt(1);      
   }
   else if(elsLine_style == LINE_DASHED)
   {
      // Microsoft import filter asked to set line width back to 1 in order to see dashed lines???

      outputElementCode(5, 2, 2);
      outputInt(2);
      outputElementCode(5, 27, 2);
      outputInt(2);
   }
   else if(elsLine_style == LINE_DOUBLE_DASHED)
   {
      outputElementCode(5, 2, 2);
      outputInt(4);
      outputElementCode(5, 27, 2);
      outputInt(4);
   }
}

void CGMPresentation::SetFillPattern( eFill_styles efsFill_style )
{
   printf("SetFillPattern: Unimplemented function called!\n");
}


// SetLineWidth:  Line thickness.  Adjust to look pleasing on target.  A value of 2 seems to work
// reasonably well with most viewers...
void CGMPresentation::SetLineWidth( int iwidth )
{
   iLine_width = iwidth;

   outputElementCode(5, 3, 2);
   outputInt(iwidth * 2);

   outputElementCode(5, 28, 2);
   outputInt(iwidth * 2);

}

void CGMPresentation::SetFgColour( int iIndex )
{
   int iR, iG, iB;
   
   if( iIndex != FL_GRAY63 ) { // hack to make printout darker
      fl_get_icm_color( iIndex, &iR, &iG, &iB );
   } else
   {
      iR = 114;
      iG = 114;
      iB = 114;
   }
      
   // set line, fill and edge colours
   outputElementCode(5, 4, 3);
   fprintf( hFile, "%c%c%c%c", iR, iG, iB, 0);
   
   outputElementCode(5, 23, 3);
   fprintf( hFile, "%c%c%c%c", iR, iG, iB, 0);
   
   outputElementCode(5, 29, 3);
   fprintf( hFile, "%c%c%c%c", iR, iG, iB, 0);
   
   outputElementCode(5, 14, 3);     // set text color
   fprintf( hFile, "%c%c%c%c", iR, iG, iB, 0);
}


void CGMPresentation::SetFgColour( float fR, float fG, float fB )
{
   fFg_red = fR;        // not sure what to do with these.. this looked right
   fFg_green = fG;
   fFg_blue = fB;
   printf("ALERT!  Unimplemented function called: SetFgColour(fr,fg,fb)\n");
}

void CGMPresentation::SetBgColour( float fR, float fG, float fB )
// wasn't specified to add, but I added this anyway 'cuz it looked right.
{
   fBg_red = fR;
   fBg_green = fG;
   fBg_blue = fB;
   printf("ALERT!  Unimplemented function called: SetBgColour(fr,fg,fb)\n");
}

void CGMPresentation::GetFgColour( float& fR, float &fG, float &fB )
{
   fR = fFg_red;
   fG = fFg_green;
   fB = fFg_blue;
   printf("ALERT!  Unimplemented function called: GetFgColour(fr,fg,fb)\n");
}

void CGMPresentation::GetBgColour( float& fR, float &fG, float &fB )
{
   fR = fBg_red;
   fG = fBg_green;
   fB = fBg_blue;
   printf("ALERT!  Unimplemented function called: GetBgColour(fr,fg,fb)\n");
}

void CGMPresentation::SetStipplePattern( eStipple_patterns esPattern )
{
   printf("ALERT!  Unimplemented function called: SetStipplePattern()\n");
}

void CGMPresentation::SetFontSize( eFont_sizes efSize )
{
   printf("ALERT!  Unimplemented function called: SetFontSize()\n");
}

// all bezier/spline curve output was disabled since it was not importing correctly in other filters
// this implementation draws straight line segments between the bezier points, but this actually doesn't
// look too bad...
void CGMPresentation::StartCurve( float fX, float fY )
{

//   outputElementCode(4, 26, 31);          // start a new PolyBezier curve
//   outputInt( 6 + 32768 );           // use long form: specify partitions of the element across these fn's
//   outputInt(1);       // 2 bytes
//   outputCoord(TX(fX));    // 2 bytes 
//   outputCoord(TY(fY));    // 2 bytes
   
}

void CGMPresentation::DrawCurve( float fX[], float fY[] )
{
//   outputInt( 16 + 32768 );          // continue this element with a new partition of 4 points (16 bytes)
   for(int i = 1; i < 4; i++ )
   {
//      outputCoord(TX(fX[i]));
//      outputCoord(TY(fY[i]));
      DrawLine(fX[i-1], fY[i-1], fX[i], fY[i]);         // straight line segments in place of splines
   }

}

void CGMPresentation::EndCurve()
{
//   outputInt(0);                      // end this element (by specifying a final 0-length partition)
}

// ****************************************************************************************************
//  End CGM generation utility functions
// ****************************************************************************************************

int CGMPresentation::GetLineWidth()
{
   return( iLine_width );
}
