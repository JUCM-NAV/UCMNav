/***********************************************************
 *
 * File:			mifpresentation.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

/*
 *	File: 		mifpresentation.cc
 *	Classes: 	MIFPresentation
 */

#include "mifpresentation.h"
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "collection.h"

typedef struct _ColourDesc {
   int iR, iG, iB;
   struct _ColourDesc *pcdNext;
} ColourDesc;

Cltn<ColourDesc *> lcdColours;

/*
 * Class:	MIFPresentation
 * Function:	MIFPresentation( char *szFilename, int new_width,
 *				int new_height ) 
 * Description:	
 */

MIFPresentation::MIFPresentation( FILE *file, float new_width, float new_height )
{
   fWidth = new_width;
   fHeight = new_height;
   iCurrent_group = 1;
   iGrouped = 0;

   hFinal_file = file;
   hFile = fopen( "/tmp/mif_file.tmp", "w" );

   iLine_width = 1;
   elsCurrent_line_style = LINE_SOLID;
   efsCurrent_fill_style = FILL_SOLID;
}

/*
 * Class:	MIFPresentation
 * Function:	~MIFPresentation() 
 * Description:	
 */

MIFPresentation::~MIFPresentation() {
   char acBuffer[256];

   fclose( hFile );

   fprintf( hFinal_file, "<MIFFile 4.0>\n" );

   DumpColours();

   hFile = fopen( "/tmp/mif_file.tmp", "r" );
   for( ;; ) {
      fgets( acBuffer, 256, hFile );
      if( feof( hFile ) ) {
	 break;
      }
      fprintf( hFinal_file, "%s", acBuffer );
   }
  
   fclose( hFile );
   unlink( "/tmp/mif_file.tmp" );
   fclose( hFinal_file );
}

/*
 * Class:	void MIFPresentation
 * Function:	DrawLine( float fX1, float fY1, float fX2, float fY2 ) 
 * Description:	
 */

void MIFPresentation::DrawLine( float fX1, float fY1, float fX2, float fY2 ) {
   fprintf( hFile, "<PolyLine\n <NumPoints 2>\n" );
   PrintGroupID();
   fprintf( hFile, " <Point %f\" %f\">\n", XtoInches( fX1 ), YtoInches( fY1 ) );
   fprintf( hFile, " <Point %f\" %f\">\n", XtoInches( fX2 ), YtoInches( fY2 ) );
   fprintf( hFile, ">\n" );
}

void MIFPresentation::DrawConnectedLines( float points[][2], int num_points )
{
   fprintf( hFile, "<PolyLine\n <NumPoints %d>\n", num_points );
   PrintGroupID();
   
   for( int i = 0; i < num_points; i++ )
      fprintf( hFile, " <Point %f\" %f\">\n", XtoInches( points[i][0] ), YtoInches( points[i][1] ) );

   fprintf( hFile, ">\n" );
}

/*
 * Class:	void MIFPresentation
 * Function:	DrawPoly( float fX[], float fY[], int iNum_points ) 
 * Description:	
 */

void MIFPresentation::DrawPoly( float fX[], float fY[], int iNum_points ) {
   int i;

   fprintf( hFile, "<Polygon\n <NumPoints %d>\n", iNum_points );
   PrintGroupID();

   for( i=0; i < iNum_points; i++ ) {
      fprintf( hFile, " <Point %f\" %f\">\n", XtoInches( fX[i] ), YtoInches( fY[i] ) );
   }
   fprintf( hFile, ">\n" );
}

/*
 * Class:	void MIFPresentation
 * Function:	DrawFilledPoly( float fX[], float fY[], int iNum_points ) 
 * Description:	
 */

void MIFPresentation::DrawFilledPoly( float fX[], float fY[], 
				      int iNum_points ) {
   int i;

   fprintf( hFile, "<Polygon\n" );
   PrintGroupID();
   fprintf( hFile, " <Fill %d>\n", (efsCurrent_fill_style==FILL_SOLID)?0:15 );
   fprintf( hFile, " <NumPoints %d>\n", iNum_points );

   for( i=0; i < iNum_points; i++ ) {
      fprintf( hFile, " <Point %f\" %f\">\n", XtoInches( fX[i] ), YtoInches( fY[i] ) );
   }
   fprintf( hFile, ">\n" );
   fprintf( hFile, " <Fill 15>\n" );
}

/*
 * Class:	void MIFPresentation
 * Function:	Refresh() 
 * Description:	
 */

void MIFPresentation::Refresh() {
}

/*
 * Class:	void MIFPresentation
 * Function:	Clear() 
 * Description:	
 */

void MIFPresentation::Clear() {}

void MIFPresentation::ClearRegion( float fX, float fY, float w, float h ) {}

/*
 * Class:	void MIFPresentation
 * Function:	DrawCircle( float fX, float fY, float fRadius ) 
 * Description:	
 */

void MIFPresentation::DrawCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta ) {
   fprintf( hFile, "<Arc\n <ArcRect %f\" %f\" %f\" %f\">\n", 
	    XtoInches( fX-fRadius ), YtoInches( fY-fRadius ),
	    XtoInches( fRadius*2.0 ), YtoInches( fRadius*2.0 ) );
   PrintGroupID();
   fprintf( hFile, " <ArcTheta %d>\n <ArcDTheta %d>\n", 360-iAngle_start/64, iAngle_delta/64 );
   fprintf( hFile, ">\n" );
}

/*
 * Class:	void MIFPresentation
 * Function:	DrawFilledCircle( float fX, float fY, float fRadius ) 
 * Description:	
 */

void MIFPresentation::DrawFilledCircle( float fX, float fY, float fRadius, int iAngle_start, int iAngle_delta ) {
   fprintf( hFile, "<Arc\n" );
   PrintGroupID();
   fprintf( hFile, " <Fill %d>\n", (efsCurrent_fill_style==FILL_SOLID)?0:15 );
   fprintf( hFile, " <ArcRect %f\" %f\" %f\" %f\">\n", 
	    XtoInches( fX-fRadius ), YtoInches( fY-fRadius ),
	    XtoInches( fRadius*2.0 ), YtoInches( fRadius*2.0 ) );
   fprintf( hFile, " <ArcTheta %d>\n <ArcDTheta %d>\n", 0, 360 );
   fprintf( hFile, ">\n" );
   fprintf( hFile, " <Fill 15>\n" );
}

void MIFPresentation::FillRegion( float fX, float fY, float w, float h ) {}

/*
 * Class:	float MIFPresentation
 * Function:	DrawText( float fX, float fY, char *text,
 * Description:	
 */

void MIFPresentation::DrawText( float fX, float fY, const char *text, bool italic, alignment al )
{
   static char ps_align[][7] = {
      "Left",
      "Center",
      "Right" };
   
   fprintf( hFile, "<TextLine\n" );
   PrintGroupID();
   fprintf( hFile, " <TLOrigin %f %f>\n", 
	    XtoInches( fX ), YtoInches( fY ) );

   fprintf( hFile, " <TLAlignment %s>\n", ps_align[al] );
   if( italic )
      fprintf( hFile, " <Font\n  <FAngle `Italic'>\n   <FSize  10.0 pt>\n>\n" );
   fprintf( hFile, " <String `%s'>\n", text );
   if( italic )
      fprintf( hFile, " <Font\n  <FAngle `Regular'>\n   <FSize  12.0 pt>\n>\n" );
   fprintf( hFile, ">\n" );
}

/*
 * Class:	void MIFPresentation
 * Function:	SetLinePattern( eLine_styles elsLine_style ) 
 * Description:	
 */

void MIFPresentation::SetLinePattern( eLine_styles elsLine_style ) {
   elsCurrent_line_style = elsLine_style;

   if( elsLine_style == LINE_DASHED ) {
      fprintf( hFile, "<DashedPattern\n <DashedStyle Dashed>\n" );
      fprintf( hFile, " <NumSegments 2>\n" );
      fprintf( hFile, " <DashSegment 10pt>\n" );
      fprintf( hFile, " <DashSegment 10pt>\n" );
      fprintf( hFile, ">\n" );
   } else if( elsLine_style == LINE_DOUBLE_DASHED ) {
      fprintf( hFile, "<DashedPattern\n <DashedStyle Dashed>\n" );
      fprintf( hFile, " <NumSegments 2>\n" );
      fprintf( hFile, " <DashSegment 5pt>\n" );
      fprintf( hFile, " <DashSegment 5pt>\n" );
      fprintf( hFile, ">\n" );
   } else {
      fprintf( hFile, "<DashedPattern\n <DashedStyle Solid>\n" );
      fprintf( hFile, ">\n" );
   }
}

/*
 * Class:	void MIFPresentation
 * Function:	SetFillPattern( eFill_styles efsFill_style ) 
 * Description:	
 */

void MIFPresentation::SetFillPattern( eFill_styles efsFill_style ) {
   efsCurrent_fill_style = efsFill_style;

}

/*
 * Class:	void MIFPresentation
 * Function:	SetLineWidth( int width ) 
 * Description:	
 */

void MIFPresentation::SetLineWidth( int wid )
{
   iLine_width = wid;  
   fprintf( hFile, "<PenWidth %dpt>\n", wid );
}

void MIFPresentation::SetFgColour( int iIndex )
{
   int iR, iG, iB;

   if( iIndex == WHITE ) {
      fprintf( hFile, "<ObColor `White'>\n" );
   } else if( iIndex == BLACK ) {
      fprintf( hFile, "<ObColor `Black'>\n" );
   } else {
      char *szColour;

      fl_get_icm_color( iIndex, &iR, &iG, &iB );
      fprintf( hFile, "<ObColor `%s'>\n", szColour = AddColour( iR, iG, iB ) );
      delete [] szColour;
   }
}

/*
 * Class:	void MIFPresentation
 * Function:	GetFgColour( float& fR, float &fG, float &fB ) 
 * Description:	
 */

void MIFPresentation::GetFgColour( float& fR, float &fG, float &fB ) {
   fR = fFg_red;
   fG = fFg_green;
   fB = fFg_blue;
}

/*
 * Class:	void MIFPresentation
 * Function:	GetBgColour( float& fR, float &fG, float &fB ) 
 * Description:	
 */

void MIFPresentation::GetBgColour( float& fR, float &fG, float &fB ) {
   fR = fBg_red;
   fG = fBg_green;
   fB = fBg_blue;
}

/*
 * Class:	void MIFPresentation
 * Function:	SetStipplePattern( eStipple_patterns esPattern ) 
 * Description:	
 */

void MIFPresentation::SetStipplePattern( eStipple_patterns esPattern ) {
}

/*
 * Class:	void MIFPresentation
 * Function:	SetFontSize( eFont_sizes efSize )
 * Description:	
 */

void MIFPresentation::SetFontSize( eFont_sizes efSize ) {

#if 0   
   switch( efSize ) {
   case LARGE_FONT:
      break;

   case SMALL_FONT:
      break;
   }

   Presentation::SetFontSize( efSize );
#endif
}

int MIFPresentation::GetLineWidth()
{
   return( iLine_width );
}

void MIFPresentation::SetDashPattern( char rgcPattern[], int iNum ) {}

float MIFPresentation::XtoInches( float fX )
{
   return( fX*fWidth );
}

float MIFPresentation::YtoInches( float fY )
{
   return( fY*fHeight );
}

void MIFPresentation::StartGroup() {
   iGrouped = 1;
}

void MIFPresentation::EndGroup() {
   fprintf( hFile, "<Group\n" );
   fprintf( hFile, " <ID %d>\n", iCurrent_group );
   fprintf( hFile, ">\n" );
   iCurrent_group++;
   iGrouped = 0;
}

void MIFPresentation::PrintGroupID() {
   if( iGrouped ) {
      fprintf( hFile, " <GroupID %d>\n", iCurrent_group );
   }
}

void MIFPresentation::RGBtoCMYK( float fR, float fG, float fB,
				 float &fC, float &fM, float &fY, float &fK )
{
   fK = Min( 1.0-fR, Min( 1.0-fG, 1.0-fB ) );

   fC = (1.0-fR-fK)/(1.0-fK);
   fM = (1.0-fG-fK)/(1.0-fK);
   fY = (1.0-fB-fK)/(1.0-fK);
}

char *MIFPresentation::AddColour( int iR, int iG, int iB ) {
   int i;
   char *szName = new char[16];

   for( i=0; i < lcdColours.length(); i++ ) {
      ColourDesc *pcd = lcdColours[i];

      if( pcd->iR == iR
	  && pcd->iG == iG
	  && pcd->iB == iB ) {
	 sprintf( szName, "Colour%d", i );
	 return( szName );
      }
   }

   ColourDesc *pcdNew = new ColourDesc;
   pcdNew->iR = iR;
   pcdNew->iG = iG;
   pcdNew->iB = iB;
   sprintf( szName, "Colour%d", lcdColours.length() );
   lcdColours.attachr( pcdNew );
   return( szName );
}

void MIFPresentation::DumpColours() {
   int i;

   fprintf( hFinal_file, "<ColorCatalog\n" );
   fprintf( hFinal_file, " <Color \n" );
   fprintf( hFinal_file, "  <ColorTag `Black'>\n" );
   fprintf( hFinal_file, "  <ColorCyan  0.000000>\n" );
   fprintf( hFinal_file, "  <ColorMagenta  0.000000>\n" );
   fprintf( hFinal_file, "  <ColorYellow  0.000000>\n" );
   fprintf( hFinal_file, "  <ColorBlack  100.000000>\n" );
   fprintf( hFinal_file, "  <ColorAttribute ColorIsBlack>\n" );
   fprintf( hFinal_file, "  <ColorAttribute ColorIsReserved>\n" );
   fprintf( hFinal_file, " > # end of Color\n" );
   fprintf( hFinal_file, " <Color \n" );
   fprintf( hFinal_file, "  <ColorTag `White'>\n" );
   fprintf( hFinal_file, "  <ColorCyan  0.000000>\n" );
   fprintf( hFinal_file, "  <ColorMagenta  0.000000>\n" );
   fprintf( hFinal_file, "  <ColorYellow  0.000000>\n" );
   fprintf( hFinal_file, "  <ColorBlack  0.000000>\n" );
   fprintf( hFinal_file, "  <ColorAttribute ColorIsWhite>\n" );
   fprintf( hFinal_file, "  <ColorAttribute ColorIsReserved>\n" );
   fprintf( hFinal_file, " > # end of Color\n" );

   for( i=0; i < lcdColours.length(); i++ ) {
      ColourDesc *pcd = lcdColours[i];
      float fC, fM, fY, fK;
      RGBtoCMYK( (pcd->iR)/255.0, (pcd->iG)/255.0, (pcd->iB)/255.0,
		 fC, fM, fY, fK );

      fprintf( hFinal_file, "  <Color\n" );
      fprintf( hFinal_file, "    <ColorTag Colour%d>\n", i );
      fprintf( hFinal_file, "    <ColorCyan %d>\n", (int)(fC*100.0) );
      fprintf( hFinal_file, "    <ColorMagenta %d>\n", (int)(fM*100.0) );
      fprintf( hFinal_file, "    <ColorYellow %d>\n", (int)(fY*100.0) );
      fprintf( hFinal_file, "    <ColorBlack %d>\n", (int)(fK*100.0) );
      fprintf( hFinal_file, "  >\n" );
   }
   fprintf( hFinal_file, ">\n" );
   fprintf( hFinal_file, "<PenWidth 2pt>\n" ); // set initial pen width
}

