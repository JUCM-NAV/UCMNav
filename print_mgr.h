/*********************************************************************
 *
 * File:			print_mgr.h
 * Author:			Andrew Miga
 * Created:			February 1999
 *
 * Modification history:
 *
 *******************************************************************/

#ifndef PRINT_MANAGER_H
#define PRINT_MANAGER_H

#include "defines.h"
#include "collection.h"
#include <stdio.h>

typedef enum { EPS, MIF, CGM, SVG } graphics_type;

#define REGULAR_HEADING_FONT 12
#define REGULAR_TEXT_FONT    10
#define LARGE_HEADING_FONT   14
#define LARGE_TEXT_FONT      12

class Map;

class PrintManager {

public:

   static void CreatePostScriptFile( const char *ps_name );
   static void GeneratePostScriptFile( Cltn<Map *> *list = NULL );
   static void CreateMapGraphics( graphics_type type );
   static void GenerateMapGraphics( map_list list, Cltn<Map *> *map_list = NULL );
   static char * PrintPostScriptText( const char *text, bool spaces = TRUE );
   static void PrintSequence( const char *text );
   static void PrintDescription( const char *text, int tab = 0 );
   static void PrintHeading( const char *heading );
   static bool TextNonempty( const char *text );
   static bool ReportIncludesMap( Map *map );
   
   static bool include_map_desc, include_responsibilities, include_path_elements, include_goals,
      include_stubs, include_component_desc, start_new_page, center_section_heading, pdf_output,
      include_date, include_time, include_design_desc, include_performance, include_parent_listing,
      include_ucm_sets, include_scenarios, large_fonts;
   static int heading_font_size, text_font_size;
   static map_list list_type;

private:

   static graphics_type gtype;
   
   static void InsertEPSFile( Map *map, const char *pdf_index );
   static void DrawPostscript();
   
   static char text_buffer[8096];
   static const char *ps_filename;
   static FILE *ps_file;
   static Cltn<Map *> *maps_list;
};

#endif
