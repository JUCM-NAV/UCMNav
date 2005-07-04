// printer dialog callbacks previously located in callbacks.cc

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "print_mgr.h"
#include "display.h"
#include "ucm_set.h"
#include <stdlib.h>
#include <unistd.h>

FD_PrintPopup *pfdpp = NULL;
FD_GraphicCreationDialog *pfdgcd = NULL;

extern int PPvisible;
extern char loaded_file[64];
extern DisplayManager *display_manager;

static map_list mlist;
static bool ucm_sets_exist = TRUE;
static list_operation operation;
static bool all_maps_hidden = FALSE;

static int PrintDialogClose( FL_FORM *, void * );
void ReadPreferences();
void MapGroupDialog( list_operation lo, const char *ft = NULL );

void OpenPrintDialog()
{
   if( !pfdpp ) {
      pfdpp = create_form_PrintPopup();
      fl_set_form_atclose( pfdpp->PrintPopup, PrintDialogClose, NULL );
   }

   fl_set_button( pfdpp->MapDescription, PrintManager::include_map_desc );
   fl_set_button( pfdpp->Responsibilities, PrintManager::include_responsibilities );
   fl_set_button( pfdpp->PathElements, PrintManager::include_path_elements );
   fl_set_button( pfdpp->Goals, PrintManager::include_goals );
   fl_set_button( pfdpp->StubDescriptions, PrintManager::include_stubs );
   fl_set_button( pfdpp->ComponentDescriptions, PrintManager::include_component_desc );
   fl_set_button( pfdpp->NewPage, PrintManager::start_new_page );
   fl_set_button( pfdpp->CenterHeadings, PrintManager::center_section_heading );
   fl_set_button( pfdpp->Date, PrintManager::include_date );
   fl_set_button( pfdpp->Time, PrintManager::include_time );
   fl_set_button( pfdpp->DesignDescription, PrintManager::include_design_desc );
   fl_set_button( pfdpp->Performance, PrintManager::include_performance );
   fl_set_button( pfdpp->ParentListing, PrintManager::include_parent_listing );
   fl_set_button( pfdpp->UCMSets, PrintManager::include_ucm_sets );
   fl_set_button( pfdpp->Scenarios, PrintManager::include_scenarios );
   fl_set_button( pfdpp->LargeFonts, PrintManager::large_fonts );
   fl_set_button( pfdpp->RegularFonts, (1 - PrintManager::large_fonts) );

   fl_set_button( pfdpp->AllMaps, 1 );
   fl_set_button( pfdpp->UCMSet , 0 );
   fl_set_button( pfdpp->Current , 0 );
   fl_set_button( pfdpp->CurrentSubtree , 0 );

   fl_show_form( pfdpp->PrintPopup, FL_PLACE_CENTER, FL_TRANSIENT, "Print" );

   FREEZE_TOOL(); // temporarily deactivate the main form
   PPvisible =1;
}

extern "C"
void PrintPrintButton_cb( FL_OBJECT *pflo, long lData )
{
   char command[150];
   int rc;

   ReadPreferences();

   if( strcmp( fl_get_input( pfdpp->PrinterInput ), "" ) == 0 ) {
      fl_set_resource( "flAlert.title", "Error: Undefined Printer" );
      fl_show_alert( "A valid printer name must be entered.", "", "", 0 );
      return;
   } else
      PrintManager::CreatePostScriptFile( "/tmp/temp-ucm.ps" );
   
   sprintf( command, "cat /tmp/temp-ucm.ps | lpr -P%s", fl_get_input( pfdpp->PrinterInput ) );
   rc = system( command );

   if( rc != 0 ) {
      sprintf( command, "cat /tmp/temp-ucm.ps | lp -d%s", fl_get_input( pfdpp->PrinterInput ) );
      rc = system( command );
   }
   unlink( "/tmp/temp-ucm.ps" );
   
   if( rc == 0 ) {
      fl_hide_form( pfdpp->PrintPopup );
      UNFREEZE_TOOL();
      PPvisible = 0;
   }
   else {
      sprintf( command, "The printer %s does not exist. Enter a valid printer.", fl_get_input( pfdpp->PrinterInput ) );
      fl_set_resource( "flAlert.title", "Error: Undefined Printer" );
      fl_show_alert( command, "", "", 0 );
   }

}

extern "C"
void PrintCancelButton_cb( FL_OBJECT *pflo, long lData ) {
  fl_hide_form( pfdpp->PrintPopup );
  UNFREEZE_TOOL();
  PPvisible = 0;
}

extern "C"
void PrintPrinttoFileButton_cb( FL_OBJECT *pflo, long lData )
{
   char ps_filename[64];
   const char *chosen_filename;
   
   ReadPreferences();
   fl_hide_form( pfdpp->PrintPopup );
   PPvisible = 0;

   strcpy( ps_filename, loaded_file );
   ps_filename[strlen(ps_filename)-4] = 0;
   strcat( ps_filename, ".ps" );

   fl_set_fselector_title( "Create PostScript File" );
   chosen_filename = fl_show_fselector( "PostScript File", "", "*.ps", ps_filename );

   if( chosen_filename == NULL ) {
      UNFREEZE_TOOL();
      return;
   }

   PrintManager::CreatePostScriptFile( chosen_filename );

   UNFREEZE_TOOL();
}

extern "C"
void Date_cb(FL_OBJECT *, long)
{
   fl_set_button( pfdpp->Time, 0 );
}

extern "C"
void Time_cb(FL_OBJECT *, long)
{
   fl_set_button( pfdpp->Date, 0 );
}

void ReadPreferences()
{
   PrintManager::include_map_desc = fl_get_button( pfdpp->MapDescription );
   PrintManager::include_responsibilities = fl_get_button( pfdpp->Responsibilities );
   PrintManager::include_path_elements = fl_get_button( pfdpp->PathElements );
   PrintManager::include_goals = fl_get_button( pfdpp->Goals );
   PrintManager::include_stubs = fl_get_button( pfdpp->StubDescriptions );
   PrintManager::include_component_desc = fl_get_button( pfdpp->ComponentDescriptions );
   PrintManager::start_new_page = fl_get_button( pfdpp->NewPage );
   PrintManager::center_section_heading = fl_get_button( pfdpp->CenterHeadings );
   PrintManager::include_date = fl_get_button( pfdpp->Date );
   PrintManager::include_time = fl_get_button( pfdpp->Time );
   PrintManager::include_design_desc = fl_get_button( pfdpp->DesignDescription );
   PrintManager::include_performance = fl_get_button( pfdpp->Performance );
   PrintManager::include_parent_listing = fl_get_button( pfdpp->ParentListing );
   PrintManager::include_ucm_sets = fl_get_button( pfdpp->UCMSets );
   PrintManager::include_scenarios = fl_get_button( pfdpp->Scenarios );

   if( fl_get_button( pfdpp->AllMaps ) )
      PrintManager::list_type = ALL_MAPS;
   else if( fl_get_button( pfdpp->UCMSet ) )
      PrintManager::list_type = UCM_SET;
   else if( fl_get_button( pfdpp->Current ) )
      PrintManager::list_type = CURRENT_MAP;
   else
      PrintManager::list_type = CURRENT_SUBTREE;

   if( fl_get_button( pfdpp->RegularFonts ) ) {
      PrintManager::large_fonts = FALSE;
      PrintManager::heading_font_size = REGULAR_HEADING_FONT;
      PrintManager::text_font_size = REGULAR_TEXT_FONT;
   }
   else if( fl_get_button( pfdpp->LargeFonts ) ) {
      PrintManager::large_fonts = TRUE;
      PrintManager::heading_font_size = LARGE_HEADING_FONT;
      PrintManager::text_font_size = LARGE_TEXT_FONT;
   }
}

int PrintDialogClose( FL_FORM *, void * )
{
   PrintCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}

void MapGroupDialog( list_operation lo, const char *ft )
{
   char title[50], prompt[50];
   
   operation = lo;
   FREEZE_TOOL(); // temporarily deactivate the main form

   if( pfdgcd == NULL )
      pfdgcd = create_form_GraphicCreationDialog();

   fl_set_button( pfdgcd->CurrentMap, 1 );
   fl_set_button( pfdgcd->CurrentSubtree, 0 );
   fl_set_button( pfdgcd->AllMaps, 0 );

   if( UcmSet::UcmSetsDefined() ) {
      if( !ucm_sets_exist ) {
	 ENABLE_WIDGET( pfdgcd->UCMSet );
	 fl_set_button( pfdgcd->UCMSet, 0 );
	 ucm_sets_exist = TRUE;
      }
   }
   else {
      if( ucm_sets_exist ) {
	 fl_set_button( pfdgcd->UCMSet, 0 );
	 DISABLE_WIDGET( pfdgcd->UCMSet );
	 ucm_sets_exist = FALSE;
      }
   }

   if( operation != EXPORT ) {
      if( all_maps_hidden ) {
	 fl_show_object( pfdgcd->AllMaps );
	 all_maps_hidden = FALSE;
      }
   }

   if( operation == GRAPHICS ) {
      sprintf( title, "Generate %s Graphics Files", ft );
      sprintf( prompt, "Specify Maps for %s File Generation", ft );
   }
   else if( operation == EXPORT ) {
      strcpy( title, "Export Files" );
      strcpy( prompt, "Specify Maps for File Export" );
      if( all_maps_hidden == FALSE ) {
	 fl_hide_object( pfdgcd->AllMaps );
	 all_maps_hidden = TRUE;
      }
   }
	else if( operation == EXPORTCSM ) {
      strcpy( title, "Export Files" );
      strcpy( prompt, "Specify Maps for CSM File Export" );
      if( all_maps_hidden == FALSE ) {
	 fl_hide_object( pfdgcd->AllMaps );
	 all_maps_hidden = TRUE;
      }
   }

   fl_set_object_label( pfdgcd->ChoicesBox, prompt );
   fl_show_form( pfdgcd->GraphicCreationDialog, FL_PLACE_CENTER, FL_TRANSIENT, title );
}

extern "C"
void GenerateButton_cb(FL_OBJECT *, long)
{
   export_type etype;

   CancelGenerate_cb( 0, 0 );

   if( fl_get_button( pfdgcd->CurrentMap ) )
      mlist = CURRENT_MAP;
   else if( fl_get_button( pfdgcd->CurrentSubtree ) )
      mlist = CURRENT_SUBTREE;
   else if( fl_get_button( pfdgcd->AllMaps ) )
      mlist = ALL_MAPS;
   else if( fl_get_button( pfdgcd->UCMSet ) ) { 
      mlist = UCM_SET;
      etype = ( operation == GRAPHICS ) ? SET_GRAPHICS : SET_SAVE;
      UcmSet::SelectExportSet( etype ); // invoke dialog to choose set
   }

   if( mlist != UCM_SET ) { // ucm set calls further dialog
   if( operation == GRAPHICS )
      PrintManager::GenerateMapGraphics( mlist );
   else if( operation == EXPORT )
      display_manager->ExportGroup( mlist );
	else if( operation == EXPORTCSM )
      display_manager->ExportCSMGroup( mlist );
   }
}

extern "C"
void CancelGenerate_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdgcd->GraphicCreationDialog );
   UNFREEZE_TOOL();
}
