/*********************************************************************
 *
 * File:			print_mgr.cc
 * Author:			Andrew Miga
 * Created:			February 1999
 *
 * Modification history:
 *
 *******************************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "print_mgr.h"
#include "display.h"
#include "map.h"
#include "resp_figure.h"
#include "pspresentation.h"
#include "mifpresentation.h"
#include "cgmpresentation.h"
#include "svgpresentation.h"
#include "collection.h"
#include "devices.h"
#include "data.h"
#include "response_time.h"
#include "ucm_set.h"
#include "variable.h"
#include "scenario.h"
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

extern Presentation *ppr;
extern DisplayManager *display_manager;
extern int showing_data_points;
extern float sampling_rate, user_rate;
extern char loaded_file[64], loaded_file_path[128];
extern float left_border, right_border, top_border, bottom_border;
extern void DrawScreen();
extern void MapGroupDialog( list_operation lo, const char *ft = NULL );

bool postscript_output = FALSE; // global variable to avoid highlighting selected components and figures

char PrintManager::text_buffer[8096];
const char *PrintManager::ps_filename;
FILE *PrintManager::ps_file;
Cltn<Map *> *PrintManager::maps_list;
bool PrintManager::include_map_desc = TRUE;
bool PrintManager::include_responsibilities = TRUE;
bool PrintManager::include_path_elements = TRUE;
bool PrintManager::include_goals = TRUE;
bool PrintManager::include_stubs = TRUE;
bool PrintManager::include_component_desc = TRUE;
bool PrintManager::start_new_page = FALSE;
bool PrintManager::center_section_heading = FALSE;
bool PrintManager::pdf_output = FALSE;
bool PrintManager::include_date = FALSE;
bool PrintManager::include_time = FALSE;
bool PrintManager::include_design_desc = TRUE;
bool PrintManager::include_performance = TRUE;
bool PrintManager::include_parent_listing = TRUE;
bool PrintManager::include_ucm_sets = TRUE;
bool PrintManager::include_scenarios = TRUE;
bool PrintManager::large_fonts = FALSE;
int PrintManager::heading_font_size = 12;
int PrintManager::text_font_size = 10;
map_list PrintManager::list_type;
graphics_type PrintManager::gtype;

static char month[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun" , "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char graphics[][4] = { "EPS", "MIF", "CGM", "SVG", "eps", "mif", "cgm", "svg" };

void PrintManager::CreatePostScriptFile( const char *ps_name )
{
   Cltn<Map *> *map_list = NULL;
   Map *displayed_map;

   ps_filename = ps_name;

   if( list_type == CURRENT_MAP || list_type == CURRENT_SUBTREE ) {
      map_list = new Cltn<Map *>;
      displayed_map = display_manager->CurrentMap();
   }
   
   if( list_type == CURRENT_MAP )
      map_list->Add( displayed_map );
   else if( list_type == CURRENT_SUBTREE )
      displayed_map->ListSubmaps( map_list );      
   else if( list_type == UCM_SET )
      UcmSet::SelectExportSet( SET_REPORT ); // invoke dialog to choose set
      
   if( list_type != UCM_SET ) // ucm set choice dialog will call generation procedure separately
      GeneratePostScriptFile( map_list ); 
}

void PrintManager::GeneratePostScriptFile( Cltn<Map *> *list )
{
   Map *current_map, *displayed_map;
   char header_file[128], design_name[64], buffer[256], *ucmnav_home;
   FILE *hfile;
   bool first_map = TRUE, first_rp;
   int section_count;
   char pdf_index[500];
   char pdf_buffer[500]; // DA: changed from std::ostringstream pdf_buffer;
        
   Cltn<Map *> *map_pool;
   time_t ct;
   char *date;
   struct tm *ctd;
   const char *dn;

   maps_list = list;

   if( list != NULL )
      map_pool = list;
   else
      map_pool = display_manager->Maps();
   
   time(&ct); // store current time and date
   date = ctime( &ct );
   ctd = localtime( &ct );
   
   if( strequal( loaded_file_path, "" ) ) {
      fl_set_resource( "flAlert.title", "Error: Printing of Report Failed" );
      fl_show_alert( "The design has not yet been saved.",
		     "Please save the design before printing.", "", 0 );
      return;
   }

   if( !strequal( display_manager->DesignName(), "" ) )
      dn = display_manager->DesignName();
   else {
      strcpy( design_name, loaded_file );
      design_name[strlen(design_name)-4] = 0; // remove extension
      dn = design_name;
   }
   
   // add header information to PostScript file
   ucmnav_home = getenv( "UCMNAV_HOME" );

   if( ucmnav_home == NULL ) {
	ucmnav_home = UCMNAV_HOME;
/*
      fl_set_resource( "flAlert.title", "Error: UCMNAV_HOME directory not defined" );
      fl_show_alert( "Install required files in directory pointed to by",
		     "environment variable UCMNAV_HOME.", "", 0 );
      return;
*/
   }

   sprintf( header_file, "%s/ps-header.ps", ucmnav_home );
   
   if( (hfile = fopen( header_file, "r" )) == NULL ) {
      fl_set_resource( "flAlert.title", "Error: Required File Not Found" );
      fl_show_alert( "The required file ps-header.ps was not found.",
		     "Please install this file in the directory pointed to by UCMNAV_HOME.", "", 0 );
      return;
   }

   if( (ps_file = fopen( ps_filename, "w" )) == NULL ) {
      fl_set_resource( "flAlert.title", "Error: Print to File Failed" );
      fl_show_alert( "The postscript file could not be opened.",
		     "Check the permissions of the current directory", "", 0 );
      return;
   }

   displayed_map = display_manager->CurrentMap();
   
   fprintf( ps_file, "%%!PS-Adobe-2.0\n%%%%Title: %s\n%%%%Creator: UCMNav 1.13.3\n%%%%CreationDate: %s", dn, date );
   fprintf( ps_file, "%%%%BeginResource:tinydict\nuserdict begin\n" );
   fprintf( ps_file, "/pdfmark where { pop } { userdict /pdfmark /cleartomark load put } ifelse\n" );
   fprintf( ps_file, "/title (UCM Navigator - %s) def\n", PrintPostScriptText( dn ) );
   if( PrintManager::include_time )
      fprintf( ps_file, "/date (%s) def\n", date );
   else if( PrintManager::include_date )
      fprintf( ps_file, "/date (%s %d %d) def\n", month[ctd->tm_mon], ctd->tm_mday, (1900 + ctd->tm_year) );
   else
      fprintf( ps_file, "/date () def\n" );
   
   // insert pdf information
   fprintf( ps_file, "[/Title (%s)\n", PrintPostScriptText( dn ) );
   fprintf( ps_file, "/Creator (UCMNav 2.3)\n" );
   fprintf( ps_file, "/Subject (%s)\n", display_manager->DesignDescription() );
   fprintf( ps_file, "/Author (%s)\n", getenv( "USERNAME" ) );
   fprintf( ps_file, "/ModDate (D:%d%.2d%.2d%.2d%.2d%.2d)\n", (1900 + ctd->tm_year), (ctd->tm_mon)+1, ctd->tm_mday,
	    ctd->tm_hour, ctd->tm_min, ctd->tm_sec );
   fprintf( ps_file, "/DOCINFO pdfmark\n" );
   
   // set line spacing according to chosen text font size
   fprintf( ps_file, "/LG { /lg exch def } def %d LG\n", PrintManager::text_font_size );

   for( ;; ) {
      fgets( buffer, 256, hfile );
      if( feof( hfile ) )
	 break;
      fprintf( ps_file, "%s", buffer );
   }
   fclose( hfile );

   // output map graphics and then textual annotations, for root maps and then plugin maps in alphabetical order
   PrintManager::pdf_output = TRUE;
   
   // output design description if it exists and is selected for inclusion
   if( PrintManager::include_design_desc ) {
      if( PrintManager::TextNonempty( display_manager->DesignDescription() ) ) {
	 PrintManager::PrintHeading( "Design Description" );
	 fprintf( ps_file, "[/Title(Design Description) /OUT pdfmark\n" );
	 fprintf( ps_file, "%d rom\n", PrintManager::text_font_size );
	 PrintManager::PrintDescription( display_manager->DesignDescription(), 36 );
      }
   }

   for( int i = 0; i <= 1; i++ ) {
      first_rp = TRUE;
      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
	 current_map = map_pool->CurrentItem();
	 if( current_map->MapType(i)) {
	    if( PrintManager::start_new_page && !first_map )
	       fprintf( ps_file, "newpage\n" );
	    first_map = FALSE;

	    // DA: Changed from ostringstream implementation;
	    strcpy ( pdf_buffer, "" );
	    
	    if( first_rp ) {
	       sprintf( pdf_buffer, "[/Count %d /Title(%s) /OUT pdfmark\n", 
			       ((i == 0) ? Map::NumberRootMaps() : Map::NumberPluginMaps()), 
			       ((i == 0) ? "Root Maps" : "Plugin Maps" ) );
	       first_rp = FALSE;
	    }
	    strcpy (pdf_index, ( strcat( pdf_buffer, "[" ) ) );

	    if( (section_count = current_map->DeterminePSSectionCount()) > 0 ) {
  	       sprintf ( pdf_buffer, "/Count %d /Title(%s) /OUT pdfmark\n[/Dest /%s%s /DEST pdfmark\n",
			       -1*section_count,
			       PrintPostScriptText( current_map->MapLabel() ),
			       ((i == 0) ? "Root" : "Plugin" ),
			       PrintPostScriptText( current_map->MapLabel(), FALSE )
		      );
	       
	       strcat ( pdf_index, pdf_buffer ); 
	    }
	    
	    pdf_index[499] = 0;
	    PrintManager::InsertEPSFile( current_map, pdf_index );
	    current_map->GeneratePostscriptDescription( ps_file );
	 }
      }
   }

   if( list == NULL ) { // print global data only for complete designs

      // output UCM Sets if selected for inclusion
      if( PrintManager::include_ucm_sets )
	 UcmSet::GeneratePostScript( ps_file );

      // output scenario definitions and control data if selected for inclusion
      if( PrintManager::include_scenarios ) {
	 PathVariableList::GeneratePostScript( ps_file );
	 ScenarioList::GeneratePostScript( ps_file );
      }

      // output global performance data if selected for inclusion
      if( PrintManager::include_performance ) {
	 ResponseTimeManager::Instance()->GeneratePostscriptDescription( ps_file );
	 DeviceDirectory::Instance()->GeneratePostScriptDescription( ps_file );
	 DataStoreDirectory::Instance()->GeneratePostScriptDescription( ps_file );
      }
   }
   
   fprintf( ps_file, "close\n%%%%EOF\n" ); // close postscript file
   fclose( ps_file );
   if( list != NULL ) delete list; // free temporary map list
   
   PrintManager::pdf_output = FALSE;
   display_manager->PlaceMap( displayed_map );
}

void PrintManager::InsertEPSFile( Map *map, const char *pdf_index )
{
   Presentation *pprOrig = ppr;
   float left, right, top, bottom;
   
   display_manager->PlaceMap( map );
   map->CalculatePathEndings();
   
   display_manager->CurrentMap()->DetermineMapBoundaries( top, bottom, left, right );
   ppr = new PSPresentation( ps_file, map->MapLabel(), 6.5, left, right, top, bottom, FALSE, map->MapTitle(),
			     ((map->IsRootMap()) ? "Root" : "Plugin" ), pdf_index );
   PrintManager::DrawPostscript();
   fprintf( ps_file, "text\n" );
   delete ppr;
   ppr = pprOrig;
}

bool PrintManager::ReportIncludesMap( Map *map )
{
   if( maps_list == NULL )
      return TRUE;

   return( maps_list->Includes( map ) );
}

void PrintManager::CreateMapGraphics( graphics_type type )
{
   char message1[100], message2[100];

   // check if file is saved
   if( strequal( loaded_file_path, "" ) ) {
      sprintf( message1, " Error: Creation of %s Files Failed  ", graphics[type] );
      sprintf( message2, "Please save the design before creating the %s graphics files.", graphics[type] );
      fl_set_resource( "flAlert.title", message1 );
      fl_show_alert( "The design has not yet been saved.", message2, "", 0 );
      return;
   }

   // invoke dialog to determine list of maps for which graphics should be generated
   gtype = type;
   MapGroupDialog( GRAPHICS, graphics[type] );
}

void PrintManager::GenerateMapGraphics( map_list list, Cltn<Map *> *map_list )
{
   Presentation *pprOrig = ppr;
   Map *map, *displayed_map;
   float left, right, top, bottom;
   char design_name[128], filename[200], message[100];
   FILE *file;
   Cltn<Map *> *map_pool;
   bool temp_list = TRUE;

   displayed_map = display_manager->CurrentMap();
   
   // create list of maps based on parameter passed from dialog
   if( list == ALL_MAPS ) {
      map_pool = display_manager->Maps();
      temp_list = FALSE;
   }
   else if( list == UCM_SET )
      map_pool = map_list;
   else {
      map_pool = new Cltn<Map *>;

      if( list == CURRENT_MAP )
	 map_pool->Add( displayed_map );
      else if( list == CURRENT_SUBTREE )
	 displayed_map->ListSubmaps( map_pool );
   }
   
   strcpy( design_name, loaded_file_path );
   design_name[strlen(design_name)-4] = 0;
   
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {

      map = map_pool->CurrentItem();
      sprintf( filename, "%s-%s.%s", design_name, map->MapLabel(), graphics[gtype+4] );
      
      if( (file = fopen( filename, (( gtype == CGM ) ? "wb" : "w" ))) == NULL ) {
	 sprintf( message, " Error: Creation of %s Files Failed  ", graphics[gtype] );
	 fl_set_resource( "flAlert.title", message );
	 fl_show_alert( "The graphics files could not be opened.",
			"Check the permissions of the current directory", "", 0 );
	 return;
      }

      display_manager->PlaceMap( map );
      display_manager->CurrentMap()->DetermineMapBoundaries( top, bottom, left, right );
      map->CalculatePathEndings();

      if( gtype == EPS )
	 ppr = new PSPresentation( file, filename, 6.5, left, right, top, bottom );
      else if( gtype == MIF )
	 ppr = new MIFPresentation( file );
      else if( gtype == CGM )
	 ppr = new CGMPresentation( file, filename, left, right, top, bottom );
      else if( gtype == SVG )
	 ppr = new SVGPresentation( file, filename );
      
      PrintManager::DrawPostscript();
      delete ppr;
   }

   display_manager->PlaceMap( displayed_map );
   ppr = pprOrig;
   if( temp_list ) delete map_pool;
}

void PrintManager::DrawPostscript()
{
   float old_left, old_right, old_top, old_bottom;
   bool showing_points_old;

   // disable the showing of empty points
   showing_points_old = showing_data_points;
   showing_data_points = FALSE;

   // turn off drawing limiting borders
   old_left = left_border; left_border = 0;
   old_right = right_border; right_border = 2.0;
   old_top = top_border; top_border = 0;
   old_bottom = bottom_border; bottom_border = 2.0;
   postscript_output = TRUE;
    
   DrawScreen();

   // reset visible area borders
   left_border = old_left;
   right_border = old_right;
   top_border = old_top;
   bottom_border = old_bottom;
   postscript_output = FALSE;
   showing_data_points = showing_points_old; // set the show points flag back to the original value
}

void PrintManager::PrintHeading( const char *heading )
{
   if( PrintManager::center_section_heading )
      fprintf( ps_file, "60 PS L %d bol\n1.5 rule (%s) centre s L 1.5 rule L\n", heading_font_size, heading );
   else
      fprintf( ps_file, "60 PS L %d bol\n(%s) s L 1.5 rule L\n", heading_font_size, heading );
}

char * PrintManager::PrintPostScriptText( const char *text, bool spaces )
{
   int i = 0, j = 0;
   
   // escape PostScript reserved characters for printable text

   if( spaces ) {  // spaces are allowed for printable text and escaping is needed
      
      while( text[i] != 0 ) {
	 if( strchr( "()<>[]{}/%%\\", text[i] ) != NULL )
	    text_buffer[j++] = '\\'; // insert backslash, must be escaped itself
	 text_buffer[j++] = text[i];
	 i++;
      }
   }
   else { // spaces are disallowed for PDF links and escaping causes problems

      while( text[i] != 0 ) {

	 if( text[i] == ' ' )
	    text_buffer[j++] = '-';
	 else if( strchr( "()<>[]{}/%%\\", text[i] ) == NULL ) // skip over problem characters
	    text_buffer[j++] = text[i];

	 i++;
      }
   }

   text_buffer[j] = 0;
   return( text_buffer );
}

bool PrintManager::TextNonempty( const char *text )
{
   int length, i;
   
   if( text == NULL ) return FALSE;

   length = strlen( text );

   for( i = 0; i < length; i++ ) {
      if( !isspace( text[i] ) )
	 return TRUE;
   }

   return FALSE;
}

void PrintManager::PrintSequence( const char *text )
{
   char line[1024] = "";
   int i = 0, j = 0;
   
   while( text[j] ) {
      if( text[j] == '\n' ) {
	 fprintf( ps_file, "(%s ) P\n", line );
	 i = 0;
	 line[0] = 0;
      }
      else {
	 if( strchr( "()<>[]{}/%%\\", text[j] ) != NULL ) // escape PostScript reserved characters for printable text
	    line[i++] = '\\'; // insert backslash, must be escaped itself
	 line[i++] = text[j];
	 line[i] = 0;
      }
      j++;
   }

   if( PrintManager::TextNonempty( line ) )
      fprintf( ps_file, "(%s ) P\n", line ); // print last line
}

void PrintManager::PrintDescription( const char *text, int tab )
{
   char line[16384] = "";
   int i = 0, j = 0;
   bool return_found = FALSE, first_line = TRUE;
   
   while( text[j] ) {
      if( text[j] == '\n' ) {
	 if( !return_found ) {       // toggle behavior, start new paragraph for double newlines
	    return_found = TRUE;
	    if( (text[j-1] != ' ') && (text[j+1] != ' ') && (text[j+1] != '\n') && (text[j+1] != '\0') ) {
	       line[i++] = ' ';
	       line[i] = 0;
	    }
	 }
	 else {
	    if( !first_line )
	       fprintf( ps_file, "5 a " );
	    else
	       first_line = FALSE;
	    if( tab != 0 )
	       fprintf( ps_file, "%d T ", tab );
	    
	    fprintf( ps_file, "(%s ) P\n", line );
	    i = 0;
	    return_found = FALSE;
	 }
      }
      else {
	 return_found = FALSE;
	 if( strchr( "()<>[]{}/%%\\", text[j] ) != NULL ) // escape PostScript reserved characters for printable text
	    line[i++] = '\\'; // insert backslash, must be escaped itself
	 line[i++] = text[j];
	 line[i] = 0;
      }
      j++;
   }

   if( PrintManager::TextNonempty( line ) ) {  // print last line
      if( !first_line )
	 fprintf( ps_file, "5 a " );
      if( tab != 0 )
	 fprintf( ps_file, "%d T ", tab );
      fprintf( ps_file, "(%s ) P\n", line );
   }
}
