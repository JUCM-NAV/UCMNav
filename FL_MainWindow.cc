
 // main window callbacks previously located in callbacks.cc

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "map.h"
#include "component_ref.h"
#include "component_mgr.h"
#include "handle.h"
#include "dynarrow.h"

#include "display.h"
#include "path.h"
#include "hyperedge_figure.h"
#include "marker_figure.h"
#include "response_time.h"
#include "devices.h"
#include "data.h"
#include "callbacks.h"
#include "xml_mgr.h"
#include "print_mgr.h"
#include "ucm_set.h"
#include "variable.h"
#include "scenario.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#ifdef LQN
#define MAINWINDOW
#include "lqngenerator.h" // lqn transform header
#undef MAINWINDOW
#endif

extern HyperedgeFigure *active_figure;
extern MarkerFigure *marker_figure;
extern DisplayManager *display_manager;
extern ComponentManager *component_manager;

extern FD_CompAttrPopup *pfdcap;
extern FD_EditRespPopup *pfderp;
extern FD_StructDynPopup *pfdsdp;
extern FD_PrintPopup *pfdpp;
extern FD_StubCreationPopup *pfdscp;
extern FD_PluginChoicePopup *pfdpcp;
extern FD_StubBindingPopup *pfdsbp;
extern FD_TimestampCreationPopup *pfdtcp;
extern FD_ResponseTimePopup *pfdtrp;
extern FD_ViewResponseTimes *pfdvrt;

FD_HelpWindow *pfdhw = NULL;
FD_DesignData *pfddes = NULL;

extern int SDPvisible, ERPvisible, PAPvisible, CAPvisible, ECPvisible,
   SCPvisible, PCPvisible, SBPvisible, TCPvisible, TRPvisible, VRTvisible;
int PPvisible;
extern float sampling_rate, user_rate;

static float resolution_values[] = { NORMAL_SAMPLING_RATE, MEDIUM_SAMPLING_RATE, MOTION_SAMPLING_RATE, STRAIGHT_LINES };
static int autosave_values[] = { 5, 10, 20, 30, EVERY_CHANGE, DISABLE_AUTOSAVE };

// global variables for tool
int showing_data_points = TRUE;
int showing_labels = FALSE;
int showing_enclosing_warning = TRUE;
int show_stub_io = TRUE;
int show_unavailable_transformations = FALSE;
int show_complete_pathname = FALSE;
int auto_map_scale = FALSE;

extern int where_description;
extern float selection_radius;
extern bool button1_down;
int iCurrent_line = 0;
int iResize_decoupled = 0;
bool FixedAllComponents = false;
bool FixedAllPositions = false;

extern Cursor curCrosshair;

extern char loaded_file[64];
extern char loaded_file_path[128];

void FileQuit( int err_code );
void ShowHelpWindow();

static int HelpWindowClose( FL_FORM *, void * );
static int DesignDialogClose( FL_FORM *, void * );

typedef enum {NO_TOOL, COMPONENT_TOOL, PATH_TOOL, SELECT_TOOL, STUB_TOOL} etTool;
extern etTool etCurrent_tool;

extern void DrawScreen();
extern void UpdateResponsibilityList();
extern void ReplaceReferences( reference_type type );

extern int PromptForSave();
extern bool PromptUserForSave;
void SetPromptUserForSave();
extern void LoadFile( const char *filename );
extern void OpenPrintDialog();
void SetFilenameDisplay();
static void SaveDesign();
static void EditDesignData();
static int options_cb( int selected );
static int spline_cb( int selected );
static int autosave_cb( int selected );

FL_PUP_ENTRY option_entries[] =
{
   {"Show Data Points", options_cb, "#D#d", FL_PUP_CHECK},
   {"Automatic Map Scaling", options_cb, "#S#s", FL_PUP_BOX},
   {"Show Path Labels", options_cb, "#L#l", FL_PUP_BOX},
   {"Show Enclosing Warnings", options_cb, "#W#w", FL_PUP_CHECK},
   {"Show Unavailable Transformations", options_cb, "#U#u", FL_PUP_BOX},
   {"Show Complete Pathname", options_cb, "#P#p", FL_PUP_BOX},
   {"Large Selection Radius", options_cb, "#R#r", FL_PUP_BOX},
   {"Show Stub I/O", options_cb, "#I#i", FL_PUP_CHECK},
   {"/Specify (Display) Spline Detail", options_cb, "" },
   {"High (Printout) Resolution", spline_cb, "", FL_PUP_CHECK},
   {"Medium Resolution", spline_cb, "", FL_PUP_BOX},
   {"Low Resolution", spline_cb, "", FL_PUP_BOX},
   {"Straight Line Segments", spline_cb, "", FL_PUP_BOX},
   {0},
   {"/Autosave Interval", autosave_cb, "" },
   {"5", autosave_cb, "", FL_PUP_BOX},
   {"10", autosave_cb, "", FL_PUP_CHECK},
   {"20", autosave_cb, "", FL_PUP_BOX},
   {"30", autosave_cb, "", FL_PUP_BOX},
   {"Every Change", autosave_cb, "", FL_PUP_BOX},
   {"Disable Autosave", autosave_cb, "", FL_PUP_BOX},
   {0},
   {0}
};

void CreateOptionsMenu()
{
   fl_set_menu_entries( pfdmMain->OptionsMenu, option_entries );
}

int options_cb( int selected )
{
   switch( selected ) {
   case 1:
      showing_data_points = 1 - showing_data_points;
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, 1, (showing_data_points) ? FL_PUP_CHECK : FL_PUP_BOX );
      break;
   case 2:
      auto_map_scale = 1 - auto_map_scale;
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, 2, (auto_map_scale) ? FL_PUP_CHECK : FL_PUP_BOX );
      break;
   case 3:
      showing_labels = 1 - showing_labels;
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, 3, (showing_labels) ? FL_PUP_CHECK : FL_PUP_BOX );
      break;
   case 4:
      showing_enclosing_warning = 1 - showing_enclosing_warning;
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, 4, (showing_enclosing_warning) ? FL_PUP_CHECK : FL_PUP_BOX );
      break;
   case 5:
      show_unavailable_transformations = 1 - show_unavailable_transformations;
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, 5, (show_unavailable_transformations) ? FL_PUP_CHECK : FL_PUP_BOX );
      break;
   case 6:
      show_complete_pathname = 1 - show_complete_pathname;
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, 6, (show_complete_pathname) ? FL_PUP_CHECK : FL_PUP_BOX );
      break;
   case 7:
      selection_radius = (selection_radius == (float)LARGE_SELECTION_RADIUS) ? SMALL_SELECTION_RADIUS : LARGE_SELECTION_RADIUS;
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, 6, (selection_radius == (float)LARGE_SELECTION_RADIUS) ? FL_PUP_CHECK : FL_PUP_BOX );
      break;
   case 8:
      show_stub_io = 1 - show_stub_io;
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, 8, (show_stub_io) ? FL_PUP_CHECK : FL_PUP_BOX );
      break;
   }

   if( selected != 5 )
      DrawScreen();
   else
      SetFilenameDisplay();  

   return 0;
}   

int spline_cb( int selected )
{
   for( int i = 9; i <= 12; i++ ) // manually reset the other items, simulating a group of radio buttons
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, i,  ( i == selected ) ? FL_PUP_CHECK : FL_PUP_BOX ); 

   user_rate = resolution_values[selected-9];
   sampling_rate = user_rate;
   DrawScreen();
   return 0;
}

int autosave_cb( int selected )
{
   for( int i = 15; i <= 20; i++ ) // manually reset the other items, simulating a group of radio buttons
      fl_set_menu_item_mode( pfdmMain->OptionsMenu, i,  ( i == selected ) ? FL_PUP_CHECK : FL_PUP_BOX ); 

   XmlManager::AutosaveInterval( autosave_values[selected-15] );
   return 0;
}

extern "C"
void FileMenu_cb( FL_OBJECT *pflo, long lData )
{
   const char *filename;
   int iWhich;
   
   switch( iWhich = fl_get_menu( pflo ) ) {
   case 1:  // "New" selected

      switch( PromptForSave() ){
      case SAVE_AND_QUIT:          // first save file, then proceed to 'new'

	 SaveDesign();

      case NOSAVE_AND_QUIT:    
	    
	 display_manager->ClearDisplay();

	 // reset file and pathnames
	 *loaded_file = 0;
	 *loaded_file_path = 0;
	 PromptUserForSave = FALSE;

	 fl_set_input( pfdmMain->Description, "" ); 
	 fl_set_input( pfdmMain->Title, "" );
	 SetFilenameDisplay();

	 // if Select tool active, switch to Component
	 if( fl_get_button( pfdmMain->SelectButton) > 0 ) {
	    fl_set_button( pfdmMain->SelectButton, 0 );
	    fl_set_button( pfdmMain->PathButton, 0 );
	    fl_set_button( pfdmMain->ComponentButton, 1 );
	    fl_call_object_callback( pfdmMain->ComponentButton );
	 }

	 //CreateOptionsMenu();
	 UpdateResponsibilityList();
	 DrawScreen();

	 break;

      case NOQUIT:
	 break;
	 
      }
      
      break;

   case 2: // Open selected

      switch( PromptForSave() ) {
      case SAVE_AND_QUIT:          // first save file, then proceed to 'load'

	 SaveDesign();

      case NOSAVE_AND_QUIT:
      
	 freeze_editor = TRUE;
	 fl_set_fselector_title( "Open File" );
	 filename = fl_show_fselector( "Filename to Open", "", "*.ucm", loaded_file );
    
	 if( filename == NULL ) {
	    freeze_editor = FALSE;
	    return;
	 }

	 PromptUserForSave = FALSE;
	 LoadFile( filename );
	 
	 fl_set_button( pfdmMain->ComponentButton, 0 );
	 fl_set_button( pfdmMain->PathButton, 0 );
	 fl_set_button( pfdmMain->SelectButton, 1 );
	 fl_call_object_callback( pfdmMain->SelectButton );

	 freeze_editor = FALSE;

	 break;
      
      case NOQUIT:
	 break;

      }

      break;

   case 4:	// for Save As operation

      loaded_file_path[0] = 0; // clear file pathname so dialog is invoked
      
   case 3:	// for Save operation
      
      SaveDesign();
      PromptUserForSave = FALSE;      
      SetFilenameDisplay();
      break;

   case 5:  // Revert from Autosave

      XmlManager::RevertFromAutosave();
      break;

   case 6:  // Print

      OpenPrintDialog();
      break;

   case 7:  // Export Maps
      
      display_manager->ExportMaps();
      break;
      
   case 8:  // Import Root Map
      
      display_manager->ImportMaps( ROOT_MAP );
      break;

   case 9:  // Export EPS

      PrintManager::CreateMapGraphics( EPS );
      break;

   case 10:  // Export MIF

      PrintManager::CreateMapGraphics( MIF );
      break;

   case 11:  // Export CGM

      PrintManager::CreateMapGraphics( CGM );
      break;

   case 12:  // Export SVG
      PrintManager::CreateMapGraphics( SVG );
      break;

   case 13:  // Edit Design Data
      
      EditDesignData();
      break;
      
   case 14:  // Export Performance Data (LQN)

     display_manager->ExportPerformanceData();
     break;

   case 15:  // Import Performance Data (LQN)

     display_manager->ImportPerformanceData();
     break; 

   case 16:  // Export DXL (DOORS)

      //fl_show_message( " Export DXL ", "To be added soon...", "Daniel." );
      display_manager->ExportDXL();
      break;

   case 17:  // Export CSM (Performance models)

      display_manager->ExportCSM();
      break;

   case 18: // Quit
      
      FileQuit( 0 );
      break;
            
   default:
      break;
   }
}

void SaveDesign()
{
   FILE *fp;
   char buffer[256] = "";
   const char *filename;
   struct stat st;
   
   if( strlen( loaded_file_path ) == 0 ) {
      freeze_editor = TRUE;
      fl_set_fselector_title( "Save File" );
      filename = fl_show_fselector( "Save as Filename", "", "*.ucm", loaded_file );
      if( filename == NULL ) {
	 freeze_editor = FALSE;
	 return;
      }

      strcpy( loaded_file, fl_get_filename() );
      strcpy( loaded_file_path, filename );
	    
      if( strcmp( (loaded_file + (strlen(loaded_file)-4)), ".ucm" ) != 0 ) {
	 strcat( loaded_file, ".ucm" );
	 strcat( loaded_file_path, ".ucm" );
      }
      fl_invalidate_fselector_cache();
   }

   // make backup version of current file, if it exists
   // use stat, link, and unlink to rename file

   if( stat( loaded_file_path, &st ) != ENOENT ) {
      sprintf( buffer, "%s.bak", loaded_file_path );
      unlink( buffer );
      link( loaded_file_path, buffer );
      unlink( loaded_file_path );
   }
   
   if( !(fp = fopen( loaded_file_path, "w" )) ) {
      sprintf( buffer, "The file %s ", loaded_file_path );
      fl_set_resource( "flAlert.title", "Error: File Save Failed" );
      fl_show_alert( buffer, "could not be opened. Check file/directory permissions.", "", 0 );
      freeze_editor = FALSE;
      return;
   }

   XmlManager::SaveFileXML( fp );
   freeze_editor = FALSE;
}

extern "C"
void DecompositionLevel_cb( FL_OBJECT *pflo, long lData )
{
   int choice = fl_get_browser( pflo );
   display_manager->InstallMap( choice-1 );
}

extern "C"
void Title_cb( FL_OBJECT *pflo, long lData )
{
   SetPromptUserForSave();
   display_manager->CurrentMap()->MapTitle( fl_get_input( pfdmMain->Title ) );
}

extern "C"
void Description_cb( FL_OBJECT *pfl, long lData )
{
   switch( where_description ) {
   case DESC_MAP:
      display_manager->CurrentMap()->MapDescription( fl_get_input( pfdmMain->Description ) );
      break;
   case DESC_PATH:
      active_figure->Edge()->Description( fl_get_input( pfdmMain->Description ) );
      break;
   case DESC_COMP:
      active_component->ReferencedComponent()->Description( fl_get_input( pfdmMain->Description ) );
      break;
   }
   SetPromptUserForSave();   
}

extern "C"
void ComponentButton_cb( FL_OBJECT *pfl, long lData )
{
   etCurrent_tool = COMPONENT_TOOL;
   if( component_manager ) component_manager->DeleteHandles();
   // delete the marker figure if it exists to cancel the create path operation
   if( marker_figure ) {
      delete marker_figure;
      marker_figure = NULL;
      DrawScreen();
   }

}

extern "C"
void PathButton_cb( FL_OBJECT *pfl, long lData )
{
   int button = fl_get_button_numb( pfdmMain->PathButton );
   if( button == FL_RIGHT_MOUSE || button == FL_MIDDLE_MOUSE )
   {
      display_manager->ResetSelectedElements();
      active_figure = NULL;
      display_manager->SetActive( NULL );
      DrawScreen();
   }
   
   etCurrent_tool = PATH_TOOL;
}

extern "C"
void SelectButton_cb( FL_OBJECT *pfl, long lData )
{
   etCurrent_tool = SELECT_TOOL;

   // delete the marker figure if it exists to cancel the create path operation
   if( marker_figure ) {
      delete marker_figure;
      marker_figure = NULL;
      DrawScreen();
   }
}

extern "C"
void ComponentMenu_cb( FL_OBJECT *pflo, long lData ) {

   switch( fl_get_menu( pflo ) ) {

   case 1:

      display_manager->SelectAllElements();
      break;
      
   case 2:
      
      iResize_decoupled = 1 - iResize_decoupled;
      fl_set_menu_item_mode( pfdmMain->ComponentMenu,
			     2, iResize_decoupled ? FL_PUP_CHECK : FL_PUP_BOX );
      break;

   case 3:

      component_manager->CutComponent();
      SetPromptUserForSave();
      break;

   case 4:
      
      component_manager->CopyComponent();
      break;

   case 5:
      
      component_manager->PasteComponent();
      SetPromptUserForSave();
      break;

   case 6:

      ReplaceReferences( COMPONENT_DEF ); // call gui for reference replacement 
      break;
   }
}

extern "C"
void PerformanceMenu_cb(FL_OBJECT *pflo, long data )
{
   ResponseTimeManager *rtm;
   DeviceDirectory *dd;
   DataStoreDirectory *dsd;
   
   switch( fl_get_menu( pflo ) ) {
      
   case 1:

      rtm = ResponseTimeManager::Instance();
      rtm->ViewAllResponseTimes();
      break;

   case 2:

      dd = DeviceDirectory::Instance();
      dd->ViewDevices();
      break;

   case 3:

      dsd = DataStoreDirectory::Instance();
      dsd->ViewDataStores();
      break;

   case 4:

      display_manager->VerifyAnnotations();
      break;

   case 5:

      display_manager->DeactivateHighlighting();
      break;

#ifdef LQN
   case 6:

      dd = DeviceDirectory::Instance();
      LqnGenerator::Instance()->Transmorgrify( display_manager->Maps(), dd->DeviceList() );
      break;
#endif
   }
}

extern "C"
void MapsMenu_cb(FL_OBJECT *pflo, long data )
{
   switch( fl_get_menu( pflo ) ) {

   case 1:

      display_manager->ChooseMap();
      break;

   case 2:

      display_manager->CreateNewRootMap();
      break;

   case 3:
      
      display_manager->ExportMaps();
      break;
      
   case 4:
      
      display_manager->ImportMaps( ROOT_MAP );
      break;
      
   case 5:
      
      display_manager->ListParentMaps();
      break;
      
   case 6:
      
      UcmSet::CreateUCMSet();
      break;
      
   case 7:
      
      UcmSet::ViewUCMSetList();
      break;

   case 8:

      UcmSet::AddCurrentMap();
      break;
      
   case 9:
      
      display_manager->SetCurrentUCMSet( NULL );
      break;
   }

}

extern "C"
void AlignMenu_cb(FL_OBJECT *pflo, long)
{
   switch( fl_get_menu( pflo ) ) {
      
   case 1:

      display_manager->AlignTop();
      break;

   case 2:

      display_manager->AlignBottom();
      break;

   case 3:
      
      display_manager->CenterVertically();
      break;
      
   case 4:
      
      display_manager->AlignLeft();
      break;

   case 5:
      display_manager->AlignRight();
      break;

   case 6:
      display_manager->CenterHorizontally();
      break;

   case 7:
      
      display_manager->DistributeHorizontally();
      break;
      
   case 8:
      
      display_manager->DistributeVertically();
      break;
   }

   SetPromptUserForSave();
   DrawScreen();
}

extern "C"
void UtilitiesMenu_cb(FL_OBJECT *pflo, long)
{
   switch( fl_get_menu( pflo ) ) {

   case 1: // Replace Component References

      ReplaceReferences( COMPONENT_DEF ); // call gui for reference replacement 
      break;

   case 2: // Replace Responsibility References
      
      ReplaceReferences( RESPONSIBILITY_DEF );
      break;
   }
}

extern "C"
void ScenariosMenu_cb(FL_OBJECT *pflo, long)
{
   switch( fl_get_menu( pflo ) ) {

   case 1: // View Boolean Variables

      BooleanVariable::ViewBooleanList();
      break;

   case 2: // View Scenario Definitions

      ScenarioList::ViewScenarioList();
      break;

   case 3: // Remove Scenario Highlighting

      display_manager->SetCurrentUCMSet( NULL );
      break;
   }
}

extern "C"
void AboutMenu_cb(FL_OBJECT *pflo, long data )
{
   switch( fl_get_menu( pflo ) ) {

   case 1: // 'Quick Help' item

      ShowHelpWindow();
      break;

   case 2: // 'About UCMNav' item
      
      fl_show_message( " UCM Navigator ", "Authors: A. Miga,  D. Petriu, D. Amyot\nContributors: J. Carriere, S. Cui, X.Y. He, K. Kaempf,\nB. Jiang, K. Lam, C. Witham, and J. Zhao", "Version 2.2.1, August 21, 2004" );
      break;
   }
}

extern "C"
void ModeChoice_cb(FL_OBJECT *, long)
{
   int choice = fl_get_choice( pfdmMain->ModeChoice );

   switch( choice ) {
   case 1: // normal editing mode
      FixedAllComponents = false;
      FixedAllPositions = false;
      break;
   case 2: // fix components
      FixedAllComponents = true;
      FixedAllPositions = false;
      break;
   case 3: // fix all positions
      FixedAllComponents = true;
      FixedAllPositions = true;
      break;
   }

   component_manager->ChangeHandleMode();
}

void FileQuit( int err_code )
{
   switch( PromptForSave() ){
   case SAVE_AND_QUIT:

      SaveDesign();

   case NOSAVE_AND_QUIT:    

      fl_hide_form( pfdmMain->Main );
      fl_free_form( pfdmMain->Main );

      if( pfdsdp ) {
	 if( SDPvisible )
	    fl_hide_form( pfdsdp->StructDynPopup );
	 fl_free_form( pfdsdp->StructDynPopup );
      }

      if( pfdcap ) {
	 if( CAPvisible )
	    fl_hide_form( pfdcap->CompAttrPopup );
	 fl_free_form( pfdcap->CompAttrPopup );
      }

      if( pfdpp ) {
	 if( PPvisible )
	    fl_hide_form( pfdpp->PrintPopup );
	 fl_free_form( pfdpp->PrintPopup );
      }

      if( pfderp ) {
	 if( ERPvisible )
	    fl_hide_form( pfderp->EditRespPopup );
	 fl_free_form( pfderp->EditRespPopup );
      }

      if( pfdscp ) {
	 if( SCPvisible )
 	    fl_hide_form( pfdscp->StubCreationPopup );
 	 fl_free_form( pfdscp->StubCreationPopup );
      }

      if( pfdpcp ) {
	 if( PCPvisible )
	    fl_hide_form( pfdpcp->PluginChoicePopup );
	 fl_free_form( pfdpcp->PluginChoicePopup );
      }

      if( pfdsbp ) {
	 if( SBPvisible )
	    fl_hide_form( pfdsbp->StubBindingPopup );
	 fl_free_form( pfdsbp->StubBindingPopup );
      }

      if( pfdtcp ) {
	 if( TCPvisible )
	    fl_hide_form( pfdtcp->TimestampCreationPopup );
	 fl_free_form( pfdtcp->TimestampCreationPopup );
      }
      
      if( pfdtrp ) {
	 if( TRPvisible )
	    fl_hide_form( pfdtrp->ResponseTimePopup );
	 fl_free_form( pfdtrp->ResponseTimePopup );
      }

      if( pfdvrt ) {
	 if( VRTvisible )
	    fl_hide_form( pfdvrt->ViewResponseTimes );
	 fl_free_form( pfdvrt->ViewResponseTimes );
      }
      
      exit( err_code );

      break;
      
   case NOQUIT:
      break;

   }
}

void SetPromptUserForSave()
{
   if( !button1_down ) // check if objects are not being moved
      XmlManager::AutosaveBackupFile(); // check if autosave time interval has elapsed

   if( PromptUserForSave == FALSE ) {
      PromptUserForSave = TRUE;
      SetFilenameDisplay();
   }
}

void SetFilenameDisplay()
{
   char filename[200];

   sprintf( filename, "Use Case Map Navigator : %s%s",
	    ((strlen( loaded_file ) == 0) ? "NoName" : ( show_complete_pathname ? loaded_file_path : loaded_file )),
	    ( PromptUserForSave ? " *" : "" ));
   fl_set_form_title( pfdmMain->Main, filename );
}

void ShowHelpWindow()
{
   char help_file[128], *ucmnav_home;
   FILE *hfile;
   
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

   sprintf( help_file, "%s/ucmnav-help.txt", ucmnav_home );
   
   if( (hfile = fopen( help_file, "r" )) == NULL ) {
      fl_set_resource( "flAlert.title", "Error: Required help file not found" );
      fl_show_alert( "The required file ucmnav-help.txt was not found.",
		     "Please install this file in the directory pointed to by UCMNAV_HOME.", "", 0 );
      return;
   }
   fclose( hfile );
   
   if( !pfdhw ) {
      pfdhw = create_form_HelpWindow();
      fl_set_form_atclose( pfdhw->HelpWindow, HelpWindowClose, NULL );
      fl_set_browser_fontsize( pfdhw->HelpDisplay, FL_NORMAL_SIZE );
      fl_load_browser( pfdhw->HelpDisplay, help_file );
   }

   fl_show_form( pfdhw->HelpWindow, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_FULLBORDER, "UCM Navigator Help" );

}

extern "C"
void HelpCloseButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdhw->HelpWindow );
}

int HelpWindowClose( FL_FORM *, void * )
{
   HelpCloseButton_cb( 0, 0 );
   return( FL_IGNORE );
}

void EditDesignData()
{
   if( pfddes == NULL ) {
      pfddes = create_form_DesignData();
      fl_set_form_atclose( pfddes->DesignData, DesignDialogClose, NULL );
   }

   fl_set_input( pfddes->DesignName, display_manager->DesignName() );
   fl_set_input( pfddes->DesignDescription, ( display_manager->DesignDescription()  ? display_manager->DesignDescription() : "" ) );

   FREEZE_TOOL();

   fl_show_form( pfddes->DesignData, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Enter Design Information" );
}

extern "C"
void DesignData_OKButton_cb(FL_OBJECT *, long)
{
   display_manager->DesignName( fl_get_input( pfddes->DesignName ) );
   display_manager->DesignDescription( fl_get_input( pfddes->DesignDescription ) );
   SetPromptUserForSave();
   DesignData_CancelButton_cb( 0, 0 );
}

extern "C"
void DesignData_CancelButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfddes->DesignData );
   UNFREEZE_TOOL();
}

int DesignDialogClose( FL_FORM *, void * )
{
   DesignData_CancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}
