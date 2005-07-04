/***********************************************************
 *
 * File:			main.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "component.h"
#include "xfpresentation.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/cursorfont.h>

#include "callbacks.h"

#include "transformation.h"
#include "display.h"
#include "map.h"
#include "xml_mgr.h"

#define MAIN_CC
#include "bitmapmain.h"
#undef MAIN_CC

#include "templates-def.cc"

#define FONTNAME	"fixed"
#define PATHLENGTH      200

void InitializeMenus( FD_Main *pfdmMain );
extern void CreateOptionsMenu();
extern void DrawScreen();
extern void SetFilenameDisplay();
extern void FileQuit( int err_code );

Cursor curCrosshair;
Presentation *ppr;
DisplayManager *display_manager;

int default_height = 550, default_width = 660;

extern int freeze_editor;
extern int showing_data_points;
extern float left_border, right_border, top_border, bottom_border;
extern char loaded_file[64];
extern char loaded_file_path[128];

const char *scale_choice_text = " 25%% | 35%% | 50%% | 75%% | 100%% | 125%% | 150%% | 200%% | 300%%%l | Fit Virtual Area | Fit Map ";

bool PromptUserForSave = FALSE;        // to ask user if changes should be saved before quitting/loading/new

FD_Main *pfdmMain;
extern FD_PluginChoicePopup *pfdpcp;

void PrintNewXMLBlock( FILE *fpx, const char *stbp );   // Prints the line, then indents 3 spaces for a new block
void PrintEndXMLBlock( FILE *fpx, const char *stbp );   // Unindents 3 spaces, and prints the line
void PrintXML( FILE *fpx, const char *stbp );           // Just prints the line.
void PrintXMLText( FILE *fpx, const char *stbp );       // Prints only one line, but does not auto-append < > to text
void IndentNewXMLBlock( FILE *fpx );              // Indents 3 spaces for a new block
void IndentEndXMLBlock( FILE *fpx );              // Unindents 3 spaces
void LinebreakXML( FILE *fpx );                   // skips a line in the XML output
char * PrintDescription( const char *description );  // removes redundant linebreaks from description as well as escapes problem XML characters
char * removeNewlineinString( const char *description );  // removes redundant linebreaks from description for DXL
void LoadFile( const char *fn );

static char description_buffer[8192];

int XMLIndent = 0;

#define SPACES_SIZE 60
char spaces[SPACES_SIZE];    // just spaces, for correct printing of XML

#ifdef TRACE
FILE *trace_file;
#endif

int WindowClose( FL_FORM *, void * );
int ProgramClose( FL_FORM *, void * );

int main( int argc, char **argv )
{
   Pixmap pmCurve_pm;
   bool initial_load = FALSE;
   char initial_filename[256];

   if( argc == 2 )
      initial_load = TRUE;

   fl_initialize( &argc, argv, "UCMEdit", 0, 0 );

   pfdmMain = create_form_Main();

   fl_add_canvas_handler( pfdmMain->DrawingArea, Expose, DrawingArea_expose, pfdmMain );
   fl_add_canvas_handler( pfdmMain->DrawingArea, KeyPress, DrawingArea_key, pfdmMain );
   fl_add_canvas_handler( pfdmMain->DrawingArea, ButtonPress, DrawingArea_but, pfdmMain );
   fl_add_canvas_handler( pfdmMain->DrawingArea, ButtonRelease, DrawingArea_but, pfdmMain );
   fl_add_canvas_handler( pfdmMain->DrawingArea, MotionNotify, DrawingArea_motion, pfdmMain );
   fl_add_canvas_handler( pfdmMain->DrawingArea, ConfigureNotify, DrawingArea_resize, pfdmMain );

   fl_freeze_form( pfdmMain->Main );
   
   fl_set_goodies_font( FL_BOLD_STYLE, FL_NORMAL_SIZE );
   fl_set_browser_fontsize( pfdmMain->DecompositionLevel, FL_NORMAL_SIZE );
   fl_set_browser_fontstyle( pfdmMain->DecompositionLevel, FL_BOLD_STYLE );
   fl_set_browser_fontsize( pfdmMain->NavigationMode, FL_NORMAL_SIZE );
   fl_set_browser_fontstyle( pfdmMain->NavigationMode, FL_BOLD_STYLE );

   fl_set_fselector_fontsize( FL_MEDIUM_SIZE );
   fl_set_browser_fontsize( pfdmMain->Responsibilities, FL_NORMAL_SIZE );
   
   fl_deactivate_object( pfdmMain->ConditionControls );
   fl_set_object_lcol( pfdmMain->ConditionControls, FL_INACTIVE );
   fl_deactivate_object( pfdmMain->ConditionAddControls );
   fl_set_object_lcol( pfdmMain->ConditionAddControls, FL_INACTIVE );

   fl_deactivate_object( pfdmMain->EditResponsibility );
   fl_set_object_lcol( pfdmMain->EditResponsibility, FL_INACTIVE );
   fl_set_atclose( WindowClose, NULL );
   fl_set_form_atclose( pfdmMain->Main, ProgramClose, NULL );
   fl_set_input_return( pfdmMain->Title, FL_RETURN_CHANGED ); 

   fl_unfreeze_form( pfdmMain->Main );

   fl_show_form( pfdmMain->Main, FL_PLACE_CENTER|FL_FREE_SIZE,
		 FL_FULLBORDER, "Use Case Map Navigator");

   pmCurve_pm = XCreatePixmap( fl_get_display(),
			       fl_get_canvas_id( pfdmMain->DrawingArea ),
			       default_width, default_height,
			       fl_get_visual_depth() );

   ppr = new XfPresentation( fl_get_canvas_id( pfdmMain->DrawingArea ),
			     pmCurve_pm,
			     default_width,
			     default_height );

   ppr->Clear();

   InitializeMenus( pfdmMain );

   fl_set_bitmapbutton_data( pfdmMain->ComponentButton, component_width, component_height, component_bits );
   fl_set_bitmapbutton_data( pfdmMain->PathButton, path_width, path_height, path_bits );
   fl_set_bitmapbutton_data( pfdmMain->SelectButton, select_width, select_height, select_bits );

   curCrosshair = XCreateFontCursor( fl_get_display(), XC_crosshair );

   fl_set_input_return( pfdmMain->Description, FL_RETURN_CHANGED );

#ifdef CYGWIN
   fl_set_border_width( -2 );
#endif

   display_manager = new DisplayManager( TransformationManager::Instance() );
   display_manager->SetMap( new Map( "root" ) );
   SetDisplayManager( display_manager );
   freeze_editor = FALSE;

# ifdef TRACE 
   trace_file = fopen( "node_trace" , "w" );
   Trace::open_trace( trace_file );
   // Trace::start_tracing(); 
#endif

   for( int iii = 0; iii < SPACES_SIZE-1; iii++ )
      spaces[iii] = ' ';
   spaces[SPACES_SIZE-1] = 0;
   spaces[0] = 0;
   
   if( initial_load && strlen( argv[1] ) > 0 )
   {
     strcpy( initial_filename, argv[1] );

     if ( initial_filename[1] == ':' )
     {
        // MS Windows path detected. Change to Cygwin.
	strcpy(initial_filename, "/cygdrive/c/");
	if ( argv[1][0] < 'a' )
           // Fix uppercase drive name
           initial_filename[10] = argv[1][0]-'A'+'a';
	int fnlength = strlen(argv[1]);
	int indexnewfn = 12;
	for ( int indexfn = 3; indexfn < fnlength; indexfn++ )
	{
	   if (argv[1][indexfn] == '\\')
              initial_filename[indexnewfn] = '/';
	   else
              initial_filename[indexnewfn] = argv[1][indexfn];
	   indexnewfn++;
	}
	initial_filename[indexnewfn] = 0; // NULL-terminated string
     }
     else
     {
        strcpy( initial_filename, argv[1] );
     }
     
     printf("UCMNav started with file: %s\n", initial_filename);
     LoadFile( initial_filename );
   }
   
   fl_do_forms();
  
   return 0; // avoids insure error messages
}

void InitializeMenus( FD_Main *pfdmMain ) {

   fl_addto_menu( pfdmMain->FileMenu, " New | Open | Save | Save As | Revert From Autosave | Print | Export Maps | Import Root Map | Create EPS Graphics | Create MIF Graphics | Create CGM Graphics | Create SVG Graphics | Specify Design Information | Export Performance Data | Import Performance Data | Export DXL | Export CSM | Quit " );
   
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 1, "#N#n" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 2, "#O#o" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 3, "#S#s" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 4, "#A#a" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 5, "#R#r" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 6, "#P#p" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 7, "#E#e" );   
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 8, "#I#i" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 9, "#G#g" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 10, "#M#m" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 11, "#C#c" );   
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 12, "#V#v" );   
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 13, "#D#d" );   
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 16, "#X#x" );   
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 17, "#L#l" );
   fl_set_menu_item_shortcut( pfdmMain->FileMenu, 18, "#Q#q" );

   fl_addto_menu( pfdmMain->ComponentMenu, " Select All Map Elements | Decouple Resizing  | Cut | Copy | Paste | Replace References " );
   fl_set_menu_item_shortcut( pfdmMain->ComponentMenu, 1, "#S#s" );
   fl_set_menu_item_shortcut( pfdmMain->ComponentMenu, 2, "#D#d" );
   fl_set_menu_item_shortcut( pfdmMain->ComponentMenu, 3, "#U#u" );
   fl_set_menu_item_shortcut( pfdmMain->ComponentMenu, 4, "#C#c" );
   fl_set_menu_item_shortcut( pfdmMain->ComponentMenu, 5, "#P#p" );
   fl_set_menu_item_shortcut( pfdmMain->ComponentMenu, 6, "#R#r" );
   fl_set_menu_item_mode( pfdmMain->ComponentMenu, 2, FL_PUP_BOX );
   CreateOptionsMenu();

#ifdef LQN
   fl_addto_menu( pfdmMain->PerformanceMenu, " View Response Time Requirements | Edit Device Characteristics | Edit Data Store Characteristics | Verify Annotation Completeness | Deactivate Annotation Highlighting | Create LQN " );
#else
   fl_addto_menu( pfdmMain->PerformanceMenu, " View Response Time Requirements | Edit Device Characteristics | Edit Data Store Characteristics | Verify Annotation Completeness | Deactivate Annotation Highlighting " );
#endif
   fl_set_menu_item_shortcut( pfdmMain->PerformanceMenu, 1, "#R#r" );
   fl_set_menu_item_mode( pfdmMain->PerformanceMenu, 1, FL_PUP_GREY );
   fl_set_menu_item_shortcut( pfdmMain->PerformanceMenu, 2, "#D#d" );
   fl_set_menu_item_shortcut( pfdmMain->PerformanceMenu, 3, "#S#s" );
   fl_set_menu_item_shortcut( pfdmMain->PerformanceMenu, 4, "#V#v" );
   fl_set_menu_item_shortcut( pfdmMain->PerformanceMenu, 5, "#A#a" );
#ifdef LQN
   fl_set_menu_item_shortcut( pfdmMain->PerformanceMenu, 6, "#L#l" );
#endif
   
   fl_addto_menu( pfdmMain->MapsMenu, " Select Map | Create New Root Map | Export Maps | Import Root Map | List Parent Maps | Create New UCM Set | View UCM Sets | Add/Remove Current Map wrt UCM Sets | Restore Full Design Navigation " );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 1, "#S#s" );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 2, "#C#c" );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 3, "#E#e" );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 4, "#I#i" );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 5, "#L#l" );
   fl_set_menu_item_mode( pfdmMain->MapsMenu, 5, FL_PUP_GREY );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 6, "#N#n" );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 7, "#U#u" );
   fl_set_menu_item_mode( pfdmMain->MapsMenu, 7, FL_PUP_GREY );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 8, "#A#a" );
   fl_set_menu_item_mode( pfdmMain->MapsMenu, 8, FL_PUP_GREY );
   fl_set_menu_item_shortcut( pfdmMain->MapsMenu, 9, "#R#r" );

   fl_addto_menu( pfdmMain->AlignMenu, " Align Top | Align Bottom | Center Vertically | Align Left | Align Right | Center Horizontally | Distribute Horizontally | Distribute Vertically " );
   fl_set_menu_item_shortcut( pfdmMain->AlignMenu, 1, "#T#t" );
   fl_set_menu_item_shortcut( pfdmMain->AlignMenu, 2, "#B#B" );
   fl_set_menu_item_shortcut( pfdmMain->AlignMenu, 3, "#C#c" );
   fl_set_menu_item_shortcut( pfdmMain->AlignMenu, 4, "#L#l" );
   fl_set_menu_item_shortcut( pfdmMain->AlignMenu, 5, "#R#r" );
   fl_set_menu_item_shortcut( pfdmMain->AlignMenu, 6, "#E#e" );
   fl_set_menu_item_shortcut( pfdmMain->AlignMenu, 7, "#H#h" );
   fl_set_menu_item_shortcut( pfdmMain->AlignMenu, 8, "#V#v" );
   
   
   fl_addto_menu( pfdmMain->AboutMenu, " Quick Help | About UCMNav " );
   fl_set_menu_item_shortcut( pfdmMain->AboutMenu, 1, "#H#h" );
   fl_set_menu_item_shortcut( pfdmMain->AboutMenu, 2, "#A#a" );
   
   fl_addto_menu( pfdmMain->UtilitiesMenu, " Replace Component References | Replace Responsibility References " );
   fl_set_menu_item_shortcut( pfdmMain->UtilitiesMenu, 1, "#C#c" );
   fl_set_menu_item_shortcut( pfdmMain->UtilitiesMenu, 2, "#R#r" );

   fl_addto_menu( pfdmMain->ScenariosMenu, " View Boolean Variables | View Scenario Definitions | Remove Scenario Highlighting " );
   fl_set_menu_item_shortcut( pfdmMain->ScenariosMenu, 1, "#B#b" );
   fl_set_menu_item_shortcut( pfdmMain->ScenariosMenu, 2, "#S#s" );   
   fl_set_menu_item_shortcut( pfdmMain->ScenariosMenu, 3, "#R#r" );   
   
   fl_addto_choice( pfdmMain->ScaleChoice, scale_choice_text );
   fl_set_choice_fontsize( pfdmMain->ScaleChoice, FL_MEDIUM_SIZE );
   fl_set_choice_fontstyle( pfdmMain->ScaleChoice, FL_BOLD_STYLE );
   fl_set_choice_text( pfdmMain->ScaleChoice, " 100%% " );
   
   fl_addto_choice( pfdmMain->ModeChoice, " Full Editing | Fix Components | Fix All Positions " );
   fl_set_choice_fontsize( pfdmMain->ModeChoice, FL_NORMAL_SIZE );
   fl_set_choice_fontstyle( pfdmMain->ModeChoice, FL_BOLD_STYLE );
   fl_set_choice_text( pfdmMain->ModeChoice, " Full Editing " );
   
   fl_set_button( pfdmMain->SelectButton, 1 );
   fl_trigger_object( pfdmMain->SelectButton );
   fl_call_object_callback( pfdmMain->ComponentButton );
}

int WindowClose( FL_FORM *, void * )
{
   return( FL_IGNORE ); 
}

int ProgramClose( FL_FORM *, void * )
{
   FileQuit( 0 );
   return( FL_IGNORE );
}

int PromptForSave()
{
   int rc;
   
   if( PromptUserForSave == TRUE )
   {
      fl_set_resource( "flChoice.title", "Save Options" );
      switch( fl_show_choices( "Modifications to current file have not been saved!!", 3, "Save", "Don't Save", "Cancel Operation", 1 ) ) {
      case 1:
	 rc = SAVE_AND_QUIT;
	 break;
      case 2:
	 rc = NOSAVE_AND_QUIT;
	 break;
      case 3:
	 rc = NOQUIT;
	 break;
      } 
   }
   else
      rc = NOSAVE_AND_QUIT;

   return( rc );
}

void PrintNewXMLBlock( FILE *fpx, const char *stbp )   // Prints the line, then indents 3 spaces for a new block
{
   fprintf( fpx, "%s<%s>\n", spaces, stbp );

   spaces[XMLIndent] = ' ';
   XMLIndent = XMLIndent + 3;
   spaces[XMLIndent] = '\0';
}

void PrintEndXMLBlock( FILE *fpx, const char *stbp )   // Unindents 3 spaces, and prints the line
{
   spaces[XMLIndent] = ' ';
   if( XMLIndent >= 3 )
      XMLIndent = XMLIndent - 3;
   spaces[XMLIndent] = '\0';

   fprintf( fpx, "%s</%s>\n", spaces, stbp );
}

void PrintXML( FILE *fpx, const char *stbp )  // Just prints the line.
{
   fprintf( fpx, "%s<%s>\n", spaces, stbp );
}

void PrintXMLText( FILE *fpx, const char *stbp )  // Prints only one line, but does not auto-append < > to text
{
   fprintf( fpx, "%s%s", spaces, stbp );
}

void LinebreakXML( FILE *fpx )
{
   fprintf( fpx, "\n" );
}

void IndentNewXMLBlock( FILE *fpx )   // Indents 3 spaces for a new block
{
   spaces[XMLIndent] = ' ';
   XMLIndent = XMLIndent + 3;
   spaces[XMLIndent] = '\0';
}

void IndentEndXMLBlock( FILE *fpx )   // Unindents 3 spaces
{
   spaces[XMLIndent] = ' ';
   if( XMLIndent >= 3 )
      XMLIndent = XMLIndent - 3;
   spaces[XMLIndent] = '\0';
}

char * removeNewlineinString( const char *description )
{
   int i = 0, j = 0;
   description_buffer[0] = 0;
   
   // process text strings for printing to DXL by escaping newlines
   
   while( description[i] != 0 ) {
      if( description[i] == '\n' ) {
	 strcat( description_buffer, " " );
	 j +=1;
      }
      else {
	 description_buffer[j++] = description[i];
	 description_buffer[j] = 0;
      }      
      i++;
   }
   description_buffer[j] = 0; // null terminate description buffer
   return( description_buffer );
}



void AbortProgram( const char *error_message )
{
   printf( "\n%s\nProgram terminated.\n", error_message );
   fflush( stdout );
   abort();
}

char * PrintDescription( const char *description )
{
   int i = 0, j = 0;
   description_buffer[0] = 0;
   
   // process text strings for printing to XML by escaping newlines
   // and problem XML characters & < > ' "
   
   while( description[i] != 0 ) {
      if( description[i] == '\n' ) {
	 strcat( description_buffer, "&#xA;" );
	 j += 5;
      }
      else if( description[i] == '&' ) {
	 strcat( description_buffer, "&amp;" );
	 j += 5;
      }
      else if( description[i] == '>' ) {
	 strcat( description_buffer, "&gt;" );
	 j += 4;
      }
      else if( description[i] == '<' ) {
	 strcat( description_buffer, "&lt;" );
	 j += 4;
	    
      }
      else if( description[i] == '\'' ) {
	 strcat( description_buffer, "&apos;" );
	 j += 6;
      }
      else if( description[i] == '\"' ) {
	 strcat( description_buffer, "&quot;" );
	 j += 6;
      }
      else {
	 description_buffer[j++] = description[i];
	 description_buffer[j] = 0;
      }

      i++;
   }

   description_buffer[j] = 0; // null terminate description buffer
   return( description_buffer );
}

void LoadFile( const char *fn )
{
   FILE *file;
   char buffer[100] = "", cwd[100] = "", filename[128] = "", *dm,
      new_loaded_file[64], new_loaded_file_path[128];
   int rc = FILE_ERROR, i = 0;
   
   strcpy( filename, fn );

   if( strcmp( fl_get_filename(), "" ) != 0 ) {  // the file browser was used
      strcpy( new_loaded_file, fl_get_filename() );
      strcpy( new_loaded_file_path, filename );
   }
   else {  // the filename was passed as a command line argument

      if( filename[0] == '/' ) {  // the filename is absolute

	 strcpy( new_loaded_file_path, filename );

	 while( filename[i] != 0 ) {
	    if( filename[i] == '/' )
	       dm = &filename[i];
	    i++;
	 }
	 dm++; // advance to first character of filename
	 strcpy( new_loaded_file, dm );
      }
      else {  // the filename is relative

	 getcwd( cwd, 100 );
	 sprintf( new_loaded_file_path, "%s/%s", cwd, filename );

	 while( new_loaded_file_path[i] != 0 ) {
	    if( new_loaded_file_path[i] == '/' )
	       dm = &new_loaded_file_path[i];
	    i++;
	 }
	 dm++; // advance to first character of filename
	 strcpy( new_loaded_file, dm );
      }
   }

   if( !(file = fopen( new_loaded_file_path, "r" )) ) {
      sprintf( buffer, "The file %s ", new_loaded_file_path );
      fl_set_resource( "flAlert.title", "Error: File Load Failed" );
      fl_show_alert( buffer, "could not be found.", "", 0 );
      return;
   }

   // determine if this is an xml file
   fgets( buffer, 100, file );
   bool xml_file = ( ( strncmp( buffer, "<?xml version", 13 ) == 0 ) ? TRUE : FALSE );
   rewind( file );

   if( xml_file )
      rc = XmlManager::LoadFile( file );

   if( rc == FILE_ERROR ) {
      fl_set_resource( "flAlert.title", "Error: File Format Not Understood" );
      fl_show_alert( "The format of the following file was not understood:", filename, "", 0 );
      return;
   }

   strcpy( loaded_file, new_loaded_file );
   strcpy( loaded_file_path, new_loaded_file_path );
   SetFilenameDisplay();
   fl_set_object_label( pfdmMain->Description, "Description of Map" );
}
