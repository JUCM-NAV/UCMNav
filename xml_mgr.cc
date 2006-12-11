/*********************************************************************
 *
 * File:			xml_mgr.cc
 * Author:			Andrew Miga
 * Created:			October 1998
 *
 * Modification history:
 *
 *******************************************************************/
/* Needed for compatibility of gettimeofday under MacOS */
#ifdef __DARWIN__
#undef _POSIX_SOURCE
#endif

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "xml_mgr.h"
#include "component.h"
#include "component_mgr.h"
#include "component_ref.h" // Apr2005 gM: required for map import to avoid conflict with constant IDs
#include "display.h"
#include "hypergraph.h"
#include "devices.h"
#include "data.h"
#include "response_time.h"
#include "goal.h"
#include "map.h"
#include "path.h"
#include "resp_mgr.h"

#include "synchronization.h"
#include "connect.h"
#include "empty.h"
#include "start.h"
#include "resp_ref.h"
#include "result.h"
#include "or_fork.h"
#include "or_join.h"
#include "timer.h"
#include "stub.h"
#include "wait.h"
#include "timestamp.h"
#include "goal_tag.h"
#include "loop.h"
#include "ucm_set.h"
#include "utilities.h"
#include "variable.h"
#include "scenario.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>

#define READ_SIZE 8192
#define SEARCH_SIZE 50

extern ComponentManager *component_manager; // global reference to sole component manager
extern char loaded_file[64];
extern char loaded_file_path[128];
extern int freeze_editor;

// declarations of static variables
StartHandlerTreeElement XmlManager::start_handler_tree[NUMBER_START_HANDLERS];
StartHandlerTreeElement *XmlManager::start_handler_root = NULL;
int XmlManager::start_handler_tree_size = 0;
EndHandlerTreeElement XmlManager::end_handler_tree[NUMBER_END_HANDLERS];
EndHandlerTreeElement *XmlManager::end_handler_root = NULL;
int XmlManager::end_handler_tree_size = 0;
bool XmlManager::root_maps;
int XmlManager::version_number;
bool XmlManager::tree_constructed = FALSE;
Map *XmlManager::current_map;
Map *XmlManager::submap;
Map *XmlManager::parent_map;

Responsibility *XmlManager::responsibility;
Start *XmlManager::start;
WaitingPlace *XmlManager::waiting_place;
Result *XmlManager::result;
Stub *XmlManager::stub;
PluginBinding *XmlManager::plugin_binding;
Empty *XmlManager::empty;
OrFork *XmlManager::current_fork;
Component *XmlManager::component;
ComponentReference *XmlManager::component_reference;
Goal *XmlManager::goal;

hyper_data XmlManager::hyperedge_data;
etype XmlManager::event_type;
char *XmlManager::event_data[4][30];
char XmlManager::text_buffer[2][8096];
int XmlManager::event_number[4] = { 0, 0, 0, 0 };
char XmlManager::direction[10];

Cltn<StubEntryExitList *> XmlManager::stub_boundaries;
Cltn<Component *> XmlManager::plugin_pools;
Cltn<Timestamp *> XmlManager::timestamps;
Cltn<GoalTag *> XmlManager::goal_tags;
Cltn<Map *> XmlManager::import_map_list;
Cltn<Map *> *XmlManager::export_list;
StubEntryExitList *XmlManager::current_stub_list;
Map *XmlManager::main_import_map;
UcmSet *XmlManager::current_ucm_set;
Scenario *XmlManager::current_scenario;
ScenarioGroup *XmlManager::current_scenario_group;

const char *XmlManager::single_map_label;
bool XmlManager::attributes_valid;

int XmlManager::current_source_id;
ref_state XmlManager::reference_state;
float XmlManager::scaling_factor;
bool XmlManager::import_maps;
bool XmlManager::copy_map = false;						// Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
bool XmlManager::import_substitution;
bool XmlManager::mode_selected;
bool XmlManager::responsibility_list;
int XmlManager::resp_load_offset;
int XmlManager::comp_load_offset;
int XmlManager::comp_ref_load_offset;			// Apr2005 gM: required for map import to avoid conflict with constant IDs
int XmlManager::device_load_offset;    			// Apr2005 gM: required for map import to avoid conflict with constant IDs
int XmlManager::hyperedge_load_offset; 			// Apr2005 gM: required for map import to avoid conflict with constant IDs
int XmlManager::model_load_offset;				// Apr2005 gM: required for map import to avoid conflict with constant IDs
int XmlManager::datastoredirectory_load_offset;		// Apr2005 gM: required for map import to avoid conflict with constant IDs
int XmlManager::boolean_load_offset;			// Apr2005 gM: required for map import to avoid conflict with constant IDs
int XmlManager::autosave_interval = 10;

#define flag( x ) ( strequal( x, "yes" ) ?  TRUE : FALSE )
#define coordinate( x ) ( atof( attributes[x] )*XmlManager::scaling_factor )

static unsigned long hash( const char *s )
{
  unsigned long h = 0;
  while (*s)
    h = (h << 5) + h + (unsigned char)*s++;
  return h;
}

int StubEntryExitList::FindHyperedgeId( int eid, end_type etype )
{
   Cltn<BoundaryListItem> &boundary_list = ( (etype == PATH_ENTRY) ? entry_list : exit_list ); 

   // find input hyperedge corresponding to entry_id
   for( boundary_list.First(); !boundary_list.IsDone(); boundary_list.Next() ) {
      if( boundary_list.CurrentItem().stub_boundary_id == eid )
	 return( boundary_list.CurrentItem().hyperedge_id );
   }

   AbortProgram( "Input/output hyperedge id to stub not found" );
   return (0); // DA: Added August 2004 (just in case) 
}

void XmlManager::SaveFileXML( FILE *file, Cltn<Map *> *sublist, const char *subdesign_name )
{
   Cltn<Map *> *map_pool;
   char buffer[300], design_name[64];
   bool has_plugins = FALSE;
   const char *dn;
   export_list = sublist;
   
   map_pool = ((export_list != NULL) ? export_list : display_manager->Maps());

   PrintXML( file, "?xml version='1.0' standalone='no'?" );
   PrintXML( file, "!DOCTYPE ucm-design SYSTEM \"ucm23xml.dtd\"" );  //  DTD version needs to be changed now and again
   LinebreakXML( file );

   if( subdesign_name == NULL ) {
      if( !strequal( display_manager->DesignName(), "" ) )
	 dn = display_manager->DesignName();
      else {
	 strcpy( design_name, loaded_file );
	 design_name[strlen(design_name)-4] = 0; // remove .ucm extension
	 dn = design_name;
      }
   }
   else
      dn = subdesign_name;
      
   sprintf( buffer, "<ucm-design design-id=\"design%d\" dtd-version=\"23\" design-name=\"%s\"", display_manager->DesignId(), PrintDescription( dn ) );
   PrintXMLText( file, buffer );
   fprintf( file, " description=\"%s\" notation=\"Buhr-UCM\" width=\"%d\" height=\"%d\">\n",
	    ( display_manager->DesignDescription()  ? PrintDescription( display_manager->DesignDescription() ) : "" ), VIRTUAL_WIDTH, VIRTUAL_HEIGHT );
   IndentNewXMLBlock( file );
   LinebreakXML( file );

   component_manager->SaveComponentList( file, export_list );
   PathVariableList::SaveXML( file );
   ResponsibilityManager::Instance()->SaveResponsibilityList( file, export_list ); // save all of the responsibilities of the entire design or subdesign
   
   DeviceDirectory::Instance()->SaveXML( file ); // save the device and data store directories
   DataStoreDirectory::Instance()->SaveXML( file );
   
   PrintNewXMLBlock( file, "root-maps" );
   
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      if( map_pool->CurrentItem()->IsRootMap() )
	  map_pool->CurrentItem()->SaveXML( file );
   }

   PrintEndXMLBlock( file, "root-maps" );
   LinebreakXML( file );

   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      if( map_pool->CurrentItem()->IsPlugIn() ) {
	 has_plugins = TRUE;
	 break;
      }
   }

   if( has_plugins ) {

      PrintNewXMLBlock( file, "plugin-maps" );

      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
	 if( map_pool->CurrentItem()->IsPlugIn() )
	    map_pool->CurrentItem()->SaveXML( file );

      PrintEndXMLBlock( file, "plugin-maps" );
      LinebreakXML( file );

      PrintNewXMLBlock( file, "plugin-bindings" );
   
      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
	 map_pool->CurrentItem()->SavePluginBindings( file );

      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
	 map_pool->CurrentItem()->SavePoolPlugins( file );

      PrintEndXMLBlock( file, "plugin-bindings" );
      LinebreakXML( file );

   }

   ResponseTimeManager::Instance()->SaveXML( file ); // save response time requirements

   if( export_list == NULL ) { // save goal lists for entire designs   
      GoalList::Instance()->SaveXML( file ); // save agent annotations
   }
   else { // save goal lists for goals in export_list, if they exist

      bool has_goals = FALSE;

      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
	 if( map_pool->CurrentItem()->HasGoals() ) {
	    has_goals = TRUE;
	    break;
	 }
      }

      if( has_goals ) {

	 PrintNewXMLBlock( file, "agent-annotations" );
	 PrintNewXMLBlock( file, "goal-list" );

	 for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
	    map_pool->CurrentItem()->SaveGoals( file );
	 
	 PrintEndXMLBlock( file, "goal-list" );
	 PrintEndXMLBlock( file, "agent-annotations" );
      }
   }

   UcmSet::SaveUcmSets( file, export_list );
   ScenarioList::SaveXML( file, export_list );

   PrintEndXMLBlock( file, "ucm-design" );
   fclose( file );

   if( export_list != NULL ) delete export_list; // free temporary map list
}

void XmlManager::SaveFileGraphDXL( FILE *file )
{
   Cltn<Map *> *map_pool = display_manager->Maps();
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      if( map_pool->CurrentItem()->IsRootMap() )
	     map_pool->CurrentItem()->SaveDXL( file );
   }
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
      if( map_pool->CurrentItem()->IsPlugIn() )
         map_pool->CurrentItem()->SaveDXL( file );
   LinebreakXML( file ); 
}

void XmlManager::SaveCSMXML( FILE *file, Cltn<Map *> *sublist, const char *subdesign_name )
{
   Cltn<Map *> *map_pool;
   char buffer[300], design_name[64];
   bool has_plugins = FALSE;
   const char *dn;
   export_list = sublist;
   
   map_pool = ((export_list != NULL) ? export_list : display_manager->Maps());

   PrintXML( file, "?xml version=\"1.0\" encoding=\"utf-8\"?" );
   LinebreakXML( file );

   if( subdesign_name == NULL ) {
      if( !strequal( display_manager->DesignName(), "" ) )
	 dn = display_manager->DesignName();
      else {
	 strcpy( design_name, loaded_file );
	 design_name[strlen(design_name)-4] = 0; // remove .ucm extension
	 dn = design_name;
      }
   }
   else
      dn = subdesign_name;

   sprintf( buffer, "<CSM:CSMType xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:CSM=\"platform:/resource/edu.carleton.sce.puma/CSM.xsd\"  name=\"%s\"", PrintDescription( dn ) );
   PrintXMLText( file, buffer );
   fprintf( file, " description=\"%s\" >\n",
	    ( display_manager->DesignDescription()  ? PrintDescription( display_manager->DesignDescription() ) : "" ));
   IndentNewXMLBlock( file );
   LinebreakXML( file );

	//save devices which corrospond to CSM processingResource
   DeviceDirectory::Instance()->SaveCSMXML( file ); // save the device and data store directories

   //save UCM component to CSM component
   //component_manager->SaveCSMComponentList( file, export_list );

   //save scenario information for CSM
   //ScenarioList::SaveCSMXML( file, export_list );
   
   
  // PathVariableList::SaveXML( file );
  //ResponsibilityManager::Instance()->SaveResponsibilityList( file, export_list ); // save all of the responsibilities of the entire design or subdesign
   
   
   //DataStoreDirectory::Instance()->SaveXML( file );
   
   // scenario in CSM corresponding to Maps in UCM
   //PrintNewXMLBlock( file, "Scenario" );
   
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      if( map_pool->CurrentItem()->IsRootMap() )
	     map_pool->CurrentItem()->SaveCSMXML( file );
   }

   //PrintEndXMLBlock( file, "Scenario" );
   LinebreakXML( file );

   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
      if( map_pool->CurrentItem()->IsPlugIn() ) {
	 has_plugins = TRUE;
	 break;
      }
   }

   if( has_plugins ) {

     // PrintNewXMLBlock( file, "plugin-maps" );

      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
	 if( map_pool->CurrentItem()->IsPlugIn() )
	    map_pool->CurrentItem()->SaveCSMXML( file );
/*
      //PrintEndXMLBlock( file, "plugin-maps" );
      LinebreakXML( file );

      PrintNewXMLBlock( file, "plugin-bindings" );
   
      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
	 map_pool->CurrentItem()->SavePluginBindings( file );

      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
	 map_pool->CurrentItem()->SavePoolPlugins( file );

      PrintEndXMLBlock( file, "plugin-bindings" );
  */
      LinebreakXML( file );

   }
/*
   ResponseTimeManager::Instance()->SaveXML( file ); // save response time requirements

   if( export_list == NULL ) { // save goal lists for entire designs   
      GoalList::Instance()->SaveXML( file ); // save agent annotations
   }
   else { // save goal lists for goals in export_list, if they exist

      bool has_goals = FALSE;

      for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() ) {
	 if( map_pool->CurrentItem()->HasGoals() ) {
	    has_goals = TRUE;
	    break;
	 }
      }

      if( has_goals ) {

	 PrintNewXMLBlock( file, "agent-annotations" );
	 PrintNewXMLBlock( file, "goal-list" );

	 for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
	    map_pool->CurrentItem()->SaveGoals( file );
	 
	 PrintEndXMLBlock( file, "goal-list" );
	 PrintEndXMLBlock( file, "agent-annotations" );
      }
   }

   UcmSet::SaveUcmSets( file, export_list );
   
*/
   PrintEndXMLBlock( file, "CSM:CSMType" );
   fclose( file );

   if( export_list != NULL ) delete export_list; // free temporary map list
}

bool XmlManager::LoadFile( FILE *file, bool import )
{
   char buffer[READ_SIZE];
   int done, len, size, i = 1;
   struct stat fs;
   bool file_complete = FALSE;

   XmlManager::import_maps = import;
   XmlManager::import_substitution = FALSE; // reset back to default mode of name extensions in case of collisions
   XmlManager::mode_selected = FALSE;
   XmlManager::scaling_factor = 1.0; // reset scaling factor to default value
   
   if( import == IMPORT_MAPS ) {
      XmlManager::main_import_map = NULL;
      XmlManager::resp_load_offset = ResponsibilityManager::Instance()->MaximumLoadNumber();
      XmlManager::comp_load_offset = component_manager->MaximumLoadNumber();
	//begin Apr2005 gM: required for map import to avoid conflict with constant IDs
	XmlManager::comp_ref_load_offset = ComponentReference::NumberComponents();
	XmlManager::device_load_offset = Device::NumberDevices();
	XmlManager::hyperedge_load_offset = Hyperedge::NumberHyperedges();
	XmlManager::model_load_offset = Map::NumberMaps();
	XmlManager::datastoredirectory_load_offset = DataStoreItem::GetItemCount();
	XmlManager::boolean_load_offset = BooleanVariable::GetNumberBooleans();
      //end Apr2005 gM
   }

   if( !XmlManager::tree_constructed )
      XmlManager::ConstructHandlerTree();

   // verify if file is complete
   fstat( fileno( file ), &fs );
   size = fs.st_size;
   do { // loop is required to handle fseek() problems with extra carriage returns on windows
      fseek( file, -i*SEARCH_SIZE, SEEK_END );
      buffer[0] = 0;
      buffer[SEARCH_SIZE] = 0;
      fread( buffer, 1, SEARCH_SIZE, file );
      if( strstr( buffer, "</ucm-design>" ) != NULL ) {
	 file_complete = TRUE;
	 break;
      }
   } while( size > (++i)*SEARCH_SIZE );

   if( !file_complete ) {
      fclose( file );
      return( FILE_ERROR );
   }
  
   fseek( file, 0L, SEEK_SET);
   XML_Parser parser = XML_ParserCreate(NULL);

   // perform validation pass
   XmlManager::attributes_valid = TRUE;
   
   XML_SetElementHandler( parser, XmlManager::TestStartElementHandler, XmlManager::TestEndElementHandler );
   
   do {
      len = fread( buffer, 1, READ_SIZE, file );
      done = len < READ_SIZE;
      if (!XML_Parse( parser, buffer, len, done )) {
	 fprintf(stderr, "%s at line %d\n",
		 XML_ErrorString(XML_GetErrorCode(parser)),
		 XML_GetCurrentLineNumber(parser));
	 fclose( file );
	 XML_ParserFree(parser);
	 return( FILE_ERROR );
      }
   } while (!done);

   XML_ParserFree(parser);

   if( XmlManager::attributes_valid == FALSE ) {
      fclose( file );
      return( FILE_ERROR );
   }
   
   // if successful create objects

   fseek( file, 0L, SEEK_SET);
   parser = XML_ParserCreate(NULL);
   XML_SetElementHandler( parser, XmlManager::StartElementHandler, XmlManager::EndElementHandler );
   
   do {
      len = fread( buffer, 1, READ_SIZE, file );
      done = len < READ_SIZE;
      XML_Parse( parser, buffer, len, done );
   } while (!done);

   fclose( file );
   XML_ParserFree(parser);

   if( import == IMPORT_MAPS ) {
      if( XmlManager::import_substitution == TRUE ) {
	 component_manager->DeleteMappings();
	 ResponsibilityManager::Instance()->DeleteMappings();
      }
   } else
      display_manager->SetFirstMap();
      
   return( FINE );
}

void XmlManager::AutosaveBackupFile()
{
   struct timeval tv;
   static int previous_time = 0;
   int current_time;
   FILE *asf;
   struct stat st;
   char autosave_file_path[128], autosave_past[128];

   if( XmlManager::autosave_interval == DISABLE_AUTOSAVE )
      return;

   if( freeze_editor == TRUE ) // do not autosave while a dialog is open, state may be corrupt
      return;

   if( XmlManager::autosave_interval != EVERY_CHANGE ) {
      // determine if time interval has elapsed
      gettimeofday( &tv, 0 );
      current_time = tv.tv_sec;

      if( previous_time == 0 ) {
	 previous_time = current_time;
	 // return;
      } else {
	 if( (current_time - previous_time) < XmlManager::autosave_interval*60 )
	    return;
      }
   
      previous_time = current_time; // perform autosave and reset previous time
   }

   if( strlen( loaded_file_path ) == 0 )
      strcpy( autosave_file_path, "NoName" );
   else {
      strcpy( autosave_file_path, loaded_file_path );
      autosave_file_path[strlen(autosave_file_path)-4] = 0; // remove extension
   }

   if( XmlManager::autosave_interval == EVERY_CHANGE ) {  // make back version of autosave file
      sprintf( autosave_past, "%s1.uas", autosave_file_path );
      if( stat( autosave_past, &st ) != ENOENT )
	 unlink( autosave_past );
      strcat( autosave_file_path, ".uas" );
      if( stat( autosave_file_path, &st ) != ENOENT ) {
	 link( autosave_file_path, autosave_past );
	 unlink( autosave_file_path );
      }
   }
   else
      strcat( autosave_file_path, ".uas" );

   if( !(asf = fopen( autosave_file_path, "w" )) ) {
      fl_set_resource( "flAlert.title", "Error: Autosave of Design Failed" );
      fl_show_alert( "An autosave of the design could not be performed.",
		     "Check the permissions of the current directory",
		     "and edit designs in a writable directory.", 0 );
      return;
   }

   if( strlen( loaded_file_path ) == 0 )
      XmlManager::SaveFileXML( asf, NULL, "NoName" );
   else
      XmlManager::SaveFileXML( asf );

}

void XmlManager::RevertFromAutosave()
{
   char autosave_file_path[128], autosave_past[128];
   FILE *asf = NULL;
   //struct stat st;
   // determine if an autosave file exists for the current file

   if( strlen( loaded_file_path ) == 0 )
      strcpy( autosave_file_path, "NoName" );
   else {
      strcpy( autosave_file_path, loaded_file_path );
      autosave_file_path[strlen(autosave_file_path)-4] = 0; // remove extension
   }

   if( XmlManager::autosave_interval == EVERY_CHANGE ) {  // use back version of autosave file
      sprintf( autosave_past, "%s1.uas", autosave_file_path );
      asf = fopen( autosave_past, "r" );
//        if( stat( autosave_past, &st ) != ENOENT )
//  	 strcpy( autosave_file_path, autosave_past );
//        else
//  	 strcat( autosave_file_path, ".uas" );
   }
//   else
  
   if( asf == NULL ) {
      strcat( autosave_file_path, ".uas" );
      asf = fopen( autosave_file_path, "r" );
   }

   if( asf != NULL )
      XmlManager::LoadFile( asf );
}

Map * XmlManager::LoadSingleMap( FILE *file, const char *new_label )
{
   char buffer[READ_SIZE];
   int done, len;
   XmlManager::root_maps = FALSE; // reset root maps flag
   XmlManager::single_map_label = new_label;
   XmlManager::scaling_factor = 2.0; // reset scaling factor to default value

   // begin Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   XmlManager::copy_map = TRUE;
   XmlManager::hyperedge_load_offset = Hyperedge::NumberHyperedges();
   XmlManager::model_load_offset = Map::NumberMaps();
   XmlManager::comp_ref_load_offset = ComponentReference::NumberComponents();
   // end Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   			
   if( !XmlManager::tree_constructed )
      XmlManager::ConstructHandlerTree();
   
   XML_Parser parser = XML_ParserCreate(NULL);
   
   XML_SetElementHandler( parser, XmlManager::StartElementHandler, XmlManager::EndElementHandler );
   
   do {
      len = fread( buffer, 1, READ_SIZE, file );
      done = len < READ_SIZE;
      XML_Parse( parser, buffer, len, done );
   } while (!done);
   
   fclose( file );
   XML_ParserFree(parser);

   XmlManager::single_map_label = NULL;
   XmlManager::copy_map = FALSE;		// Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   return( XmlManager::current_map );
}

void XmlManager::StartElementHandler( void *data, const char *name, const char **attributes )
{
   // look up element name in binary tree to find start element handler pointer
   (XmlManager::FindStartHandler( name ))( attributes );
}

void XmlManager::EndElementHandler( void *data, const char *name )
{
   // look up element name in binary tree to find end element handler pointer
   // if not found do nothing
   void (*handler)();

   if( (handler = XmlManager::FindEndHandler( name )) != NULL )
      handler();
}

void XmlManager::TestStartElementHandler( void *data, const char *name, const char **attributes )
{
   // look up element name in binary tree to find start element handler pointer
   void (*handler)(const char**);

   handler = XmlManager::FindStartHandler( name );
   if( handler == NULL ) {
      XmlManager::attributes_valid = FALSE;
      std::cout << "\nLoad File Error: element \"" << name << "\" not understood.\n" << std::flush;
   }
}

void XmlManager::TestEndElementHandler( void *data, const char *name ) {}

void XmlManager::UCMDesignElement( const char **attributes )
{   
   int i, id = 0;
   const char *name, *description = NULL;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"design-id"))
	 id = atoi( attributes[i+1]+6 );
      else if(strequal(attributes[i],"design-name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"description"))
	 description = attributes[i+1];
      // read in width of drawing area to calculate scaling factor
      // if aspect ratio changes in future height will have to be read in as well
      // and separate scaling factors used for x and y coordinates
      else if(strequal(attributes[i],"width"))
	 XmlManager::scaling_factor = atoi( attributes[i+1] )/DEFAULT_WIDTH;
      else if(strequal(attributes[i],"dtd-version"))
	 XmlManager::version_number = atoi( attributes[i+1] );
   }

   if( XmlManager::import_maps != IMPORT_MAPS ) {  // if maps are not being imported into current design
      display_manager->ClearDisplay( FALSE );      // delete all maps, clear all directories
      display_manager->DesignId( id ); // set the new design id number if specified
      display_manager->DesignName( name );
      display_manager->DesignDescription( XmlManager::AddNewlines( description, 0 ) );
   }
}

void XmlManager::EndDesign()
{
   while( !import_map_list.is_empty() )
      import_map_list.Detach();

   ResponsibilityManager::Instance()->CleanList();
   component_manager->CleanList();
   Timer::ValidateTimerList();
}

void XmlManager::DeviceDirectoryElement( const char **attributes )
{
   // do nothing
}

void XmlManager::DeviceElement( const char **attributes )
{ // read in device-id, name, type, op-time, and description and create Device object
   float op_time = 0.0;
   int i, device_id;
   const char *name, *description;
   DeviceDirectory *dd = DeviceDirectory::Instance();
   device_type dtype;
    
   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"device-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 // skip over 'dv' prefix on first attribute
	 device_id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + device_load_offset : atoi( attributes[i+1]+2 ));
      else if(strequal(attributes[i],"device-name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"description"))
	 description = attributes[i+1];
      else if(strequal(attributes[i],"op-time"))
	 op_time = atof( attributes[i+1] );
      else if(strequal(attributes[i],"device-type"))
	 dtype = dd->DeviceType( attributes[i+1] ); // convert string into enumerated type
   }

   dd->AddDevice( device_id, dtype, name, op_time, XmlManager::AddNewlines( description, 0 ) );
}

void XmlManager::DataStoreDirectoryElement( const char **attributes )
{
   // do nothing
}

void XmlManager::DataStoreElement( const char **attributes )
{  // read in data store id and item and create data store
   int i, id;
   const char *item;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"data-store-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + datastoredirectory_load_offset : atoi( attributes[i+1]+2 ));
      else if(strequal(attributes[i],"data-store-item"))
	 item = attributes[i+1];
   }

   DataStoreDirectory::Instance()->AddItem( DATA_STORES, item, id );   
}

void XmlManager::AccessModeElement( const char **attributes )
{   // read in access mode id and item and create access mode
   int i, id;
   const char *item;
    
   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"access-mode-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + datastoredirectory_load_offset : atoi( attributes[i+1]+2 ));
      else if(strequal(attributes[i],"access-mode-item"))
	 item = attributes[i+1];
   }

   DataStoreDirectory::Instance()->AddItem( ACCESS_MODES, item, id );   
}
   
void XmlManager::RootMapsElement( const char **attributes )
{
   XmlManager::root_maps = TRUE; // set root maps flag
}

void XmlManager::PluginMapsElement( const char **attributes )
{
   XmlManager::root_maps = FALSE; // reset root maps flag
}

void XmlManager::ModelElement( const char **attributes )
{
   // read in model id, name, title, and description and create new map, set type according to root_maps flag
   const char *name, *description, *title="No title";
   int i, id;
    
   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"model-id"))
	 id = atoi( attributes[i+1]+1 );
      else if(strequal(attributes[i],"model-name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"description"))
	 description = attributes[i+1];
      else if(strequal(attributes[i],"title"))
	 title = attributes[i+1];;
   }

   // Apr2005 gM: required for map import to avoid conflict with constant IDs
   // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   XmlManager::current_map = new Map( (( import_maps == IMPORT_MAPS || copy_map ) ? id + model_load_offset : id), 
					((XmlManager::single_map_label == NULL) ? name : XmlManager::single_map_label ),
				      ( root_maps ? ROOT_MAP :PLUGIN ), title, XmlManager::AddNewlines( description, 0 ) );

   if( XmlManager::import_maps == IMPORT_MAPS ) {
      XmlManager::import_map_list.Add( current_map );
      if( XmlManager::main_import_map == NULL )
	 XmlManager::main_import_map = current_map;
   }
}

void XmlManager::StartResponsibilitySpecElement( const char **attributes )
{
   XmlManager::responsibility_list = TRUE;
}

void XmlManager::EndResponsibilitySpecElement()
{
   XmlManager::responsibility_list = FALSE;
}

void XmlManager::StartResponsibilityElement( const char **attributes )
{
   // read in responsibility id, name, direction for v12, and optionally execution sequence and description
   const char *name, *es = NULL, *desc = NULL;
   int existing_index, id, i;
   bool duplicate_name;
      
   for( i=0; attributes[i]!=NULL; i += 2 ) {
      if(strequal(attributes[i],"resp-id"))
	 id = atoi( attributes[i+1]+1 );
      else if(strequal(attributes[i],"resp-name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"description"))
	 desc = XmlManager::AddNewlines( attributes[i+1], 1 );
      else if(strequal(attributes[i],"exec-sequence"))
	 es = XmlManager::AddNewlines( attributes[i+1], 0 );
      else if(strequal(attributes[i],"direction"))
	 strcpy( XmlManager::direction, attributes[i+1] );
   }

   if( XmlManager::import_maps == IMPORT_MAPS ) {
      if( !((XmlManager::mode_selected == TRUE) && (XmlManager::import_substitution == FALSE)) )
	 duplicate_name = ResponsibilityManager::Instance()->DuplicateName( name, existing_index );

      if( duplicate_name == TRUE && XmlManager::mode_selected == FALSE ) {
	 XmlManager::import_substitution = XmlManager::PromptImportMode();
	 XmlManager::mode_selected = TRUE;
      }

      // if substitution import mode is turned on determine if there is a name clash between this and a previously defined responsibility
      if( XmlManager::import_substitution == TRUE ) {
	 if( duplicate_name == TRUE ) { // store mapping between existing identifier and this identifier
	    ResponsibilityManager::Instance()->CreateMapping( id+resp_load_offset, existing_index );
	    return; // do not create a new responsibility object
	 }
      }
   }
   XmlManager::responsibility = new Responsibility( (( import_maps == IMPORT_MAPS ) ? id + resp_load_offset : id), name, es, desc );
}

void XmlManager::EndResponsibilityElement()
{
   int i;
   // install preconditions and postconditions, if any

   for( i = 0; i < event_number[PRECONDITION]; i++ )
      responsibility->AddPrecondition( event_data[PRECONDITION][i] );

   event_number[PRECONDITION] = 0;

   for( i = 0; i < event_number[POSTCONDITION]; i++ )
      responsibility->AddPostcondition( event_data[POSTCONDITION][i] );

   event_number[POSTCONDITION] = 0;   
}

void XmlManager::DataStoreSpecElement( const char **attributes )
{
   // do nothing
}

void XmlManager::DataStoreAccessElement( const char **attributes )
{
   // read ids for data store and access mode and add to last responsibility
   int i, id, item;
    
   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"data-store-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + datastoredirectory_load_offset : atoi( attributes[i+1]+2 ));
      else if(strequal(attributes[i],"access-mode-id")) // Apr2005 gM: bug fix - undefined xml attribute
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 item = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + datastoredirectory_load_offset : atoi( attributes[i+1]+2 ));
   }

   XmlManager::responsibility->AddDataReference( id, item );
}

void XmlManager::ServiceRequestListElement( const char **attributes )
{
   // do nothing
}

void XmlManager::ServiceRequestElement( const char **attributes )
{ // read in device-id and amount string
   int i, device_id;
   const char *number;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"service-type")) {
	 if( strncmp( attributes[i+1], "pr", 2 ) == 0 )
	    device_id = PROCESSOR_DEVICE;
	 else
	    // Apr2005 gM: required for map import to avoid conflict with constant IDs	
	    device_id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + device_load_offset : atoi( attributes[i+1]+2 ));
      }
      else if(strequal(attributes[i],"request-number"))
	 number = attributes[i+1];
   }


   if( XmlManager::responsibility_list == TRUE )
      XmlManager::responsibility->AddServiceRequest( device_id, number );
   else
      XmlManager::stub->AddServiceRequest( device_id, number ); 
}

void XmlManager::DynamicRespElement( const char **attributes )
{
   // read in dynamic arrow type, direction, sourcepool, and length and pass to previous responsibility
   const char *type, *direction, *source;
   float length;

   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"type"))
	 type = attributes[i+1];
      else if(strequal(attributes[i],"direction"))
	 direction = attributes[i+1];
      else if(strequal(attributes[i],"sourcepool"))
	 source = attributes[i+1];
      else if(strequal(attributes[i],"arrow-length"))
	 length = atof( attributes[i+1] );
   }
   
   XmlManager::responsibility->AddDynamicArrow( type, direction, source, length );
}

void XmlManager::PathSpecElement( const char **attributes )
{
   // do nothing
}

void XmlManager::PathLabelListElement( const char **attributes )
{
   // do nothing
}

void XmlManager::PathLabelElement( const char **attributes )
{
   // read label id and text, create label object, constructor will register object in current map
   int id;
   const char *label;

   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"path-label-id"))
	 id = atoi( attributes[i+1]+2 );
      else if(strequal(attributes[i],"label"))
	 label = attributes[i+1];
   }

   new Label( id, label, XmlManager::current_map );
}

void XmlManager::StartHypergraphElement( const char **attributes )
{
   XmlManager::reference_state = CONNECTIONS;
}
  
void XmlManager::EndHypergraphElement()
{
   // rebuild path spline objects
   XmlManager::current_map->CreatePaths();
}

void XmlManager::HyperedgeElement( const char **attributes )
{
   // store common hyperedge information for later use
   int i = 2;
   bool name_found = FALSE, fx_found = FALSE, fy_found = FALSE, desc_found = FALSE, label_found = FALSE;
   // reset common hyperedge data
   hyperedge_data.name[0] = 0;
   hyperedge_data.description[0] = 0;
   
   // Apr2005 gM: required for map import to avoid conflict with constant IDs
   // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   hyperedge_data.id = (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[1]+1 ) + hyperedge_load_offset : atoi( attributes[1]+1 ));
     
   while( attributes[i] != 0 ) {

      if( !name_found ) {
	 if( strequal( attributes[i], "hyperedge-name" ) ) {
	    strncpy( hyperedge_data.name, attributes[i+1], 29 );
	    name_found = TRUE;
	    continue;
	 }
      }

      if( !fx_found ) {
	 if( strequal( attributes[i], "fx" ) ) {
	    hyperedge_data.fx = coordinate(i+1);
	    fx_found = TRUE;
	    continue;
	 }
      }

      if( !fy_found ) {
	 if( strequal( attributes[i], "fy" ) ) {
	    hyperedge_data.fy = coordinate(i+1);
	    fy_found = TRUE;
	    continue;
	 }
      }

      if( !desc_found ) {
	 if( strequal( attributes[i], "description" ) ) {
	    strncpy( hyperedge_data.description, XmlManager::AddNewlines( attributes[i+1], 0 ), 8095 );
	    desc_found = TRUE;
	    continue;
	 }
      }

      if( !label_found ) {  // kept for compatibility with ucm10xml.dtd
	 if( strequal( attributes[i], "path-label-id" ) ) {
	    hyperedge_data.label = current_map->FindLabel( atoi( attributes[i+1]+2 ) );
	    label_found = TRUE;
	    continue;
	 }
      }

      i += 2;
   }
}

void XmlManager::StartStartElement( const char **attributes )
{
   float input[2] = { 0, 0 };
   const char *expert = NULL;
   const char *logical_expression = NULL;
   char newcondition[1000];			// Apr2005 gM: required for map import to avoid conflict with constant IDs
   const char *alignment = NULL;
   const char *arrival_name = NULL;
   float mean, value, low, high, kernel;
   arrival_type atype = UNDEFINED;
   stream_type stype = OPEN_ARRIVAL;
   int population_size = 0;

   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"arrival"))
	 arrival_name = attributes[i+1];
      else if(strequal(attributes[i],"stream-type"))
	 stype = ((strequal(attributes[i+1], "open")) ? OPEN_ARRIVAL : CLOSED_ARRIVAL);
      else if(strequal(attributes[i],"population-size"))
	 population_size = atoi( attributes[i+1] );
      else if(strequal(attributes[i],"mean"))
	 mean = atof( attributes[i+1] );
      else if(strequal(attributes[i],"value"))
	 value = atof( attributes[i+1] );
      else if(strequal(attributes[i],"low"))
	 low = atof( attributes[i+1] );
      else if(strequal(attributes[i],"high"))
	 high = atof( attributes[i+1] );
      else if(strequal(attributes[i],"kernel"))
	 kernel = atof( attributes[i+1] );
      else if(strequal(attributes[i],"expert-distribution"))
	 expert = attributes[i+1];
      else if( strequal( attributes[i], "logical-condition" ) ) {
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 if( import_maps == IMPORT_MAPS ) {
	  AddOffsetToBoolean( attributes[i+1], newcondition );
	 logical_expression = newcondition;
	 }
	 else
	 logical_expression = attributes[i+1];
	}
      else if( strequal( attributes[i], "label-alignment" ) )
	 alignment = attributes[i+1];
   }

   if( arrival_name != NULL ) {
      if( strequal( arrival_name, "exponential" ) ) {
	 atype = EXPONENTIAL;
	 input[0] = mean;
      }
      else if ( strequal( arrival_name, "deterministic" ) ) {
	 atype = DETERMINISTIC;
	 input[0] = value;
      }
      else if(strequal( arrival_name, "uniform" ) ) {
	 atype = UNIFORM;
	 input[0] = low;
	 input[1] = high;
      }      
      else if(strequal( arrival_name, "erlang" ) ) {
	 atype = ERLANG;
	 input[0] = mean;
	 input[1] = kernel;
      }      
      else if(strequal( arrival_name, "expert" ) )
	 atype = EXPERT;
   }

   XmlManager::start = new Start( hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy, hyperedge_data.description,
				  logical_expression, atype, stype, population_size, input, expert, alignment );
}

void XmlManager::EndStartElement()
{
   int i;
   // install triggering events and preconditions, if any

   for( i = 0; i < event_number[TRIGGERING]; i++ )
      start->AddTriggeringEvent( event_data[TRIGGERING][i] );

   event_number[TRIGGERING] = 0;

   for( i = 0; i < event_number[PRECONDITION]; i++ )
      start->AddPrecondition( event_data[PRECONDITION][i] );

   event_number[PRECONDITION] = 0;
}

void XmlManager::StartWaitingPlaceElement( const char **attributes )
{
   const char *type = NULL, *timer="no";;
   const char *logical_expression = NULL;
   char newcondition[1000];			// Apr2005 gM: required for map import to avoid conflict with constant IDs
   BooleanVariable *timer_variable = NULL;
   int i;

   for( i = 0; attributes[i]; i+=2 ) {
      if( strequal( attributes[i], "timer" ) )
	 timer = attributes[i+1];
      if( strequal( attributes[i], "timeout-variable" ) )
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 timer_variable = BooleanVariable::ReferencedVariable( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + boolean_load_offset : atoi( attributes[i+1]+2 )) );
      else if( strequal( attributes[i], "wait-type" ) )
	 type = attributes[i+1];
      else if( strequal( attributes[i], "logical-condition" ) ) {
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 if( import_maps == IMPORT_MAPS ) {
	  AddOffsetToBoolean( attributes[i+1], newcondition );
	 logical_expression = newcondition;
	 }
	 else
	 logical_expression = attributes[i+1];
	}
   }

   if( strequal( timer, "yes" ) ) {
      XmlManager::waiting_place = new Timer( hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy, 
					     hyperedge_data.description, timer_variable, type, logical_expression );
   } else {
      XmlManager::waiting_place = new Wait(  hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy, 
					     hyperedge_data.description, type, logical_expression );
   }
}

void XmlManager::EndWaitingPlaceElement()
{
   int i;
   // install triggering events and preconditions, if any

   for( i = 0; i < event_number[TRIGGERING]; i++ )
      waiting_place->AddTriggeringEvent( event_data[TRIGGERING][i] );

   event_number[TRIGGERING] = 0;

   for( i = 0; i < event_number[PRECONDITION]; i++ )
      waiting_place->AddPrecondition( event_data[PRECONDITION][i] );

   event_number[PRECONDITION] = 0;
}

void XmlManager::StartEndPointElement( const char **attributes )
{   
   XmlManager::result = new Result( hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy, hyperedge_data.description,
				    ( ( attributes[0] != NULL ) ? attributes[1] : NULL ) );
}

void XmlManager::EndEndPointElement()
{
   int i;
   // install resulting events and postconditions, if any

   for( i = 0; i < event_number[RESULTING]; i++ )
      result->AddResultingEvent( event_data[RESULTING][i] );

   event_number[RESULTING] = 0;

   for( i = 0; i < event_number[POSTCONDITION]; i++ )
      result->AddPostcondition( event_data[POSTCONDITION][i] );

   event_number[POSTCONDITION] = 0;
}

void XmlManager::ResponsibilityRefElement( const char **attributes )
{

   const char *ldir = NULL, *position = NULL;
   int parent_id, i, id;
    
   for( i=0; attributes[i]!=NULL; i += 2 ) {
      if(strequal(attributes[i],"resp-id"))
	 id = atoi( attributes[i+1]+1 );
      else if(strequal(attributes[i],"direction"))
	 ldir = attributes[i+1];
      else if(strequal(attributes[i],"arrow-position"))
	 position = attributes[i+1];
   }
   
   if( ldir != NULL )
      parent_id = id;
   else {
      ldir = XmlManager::direction;
      parent_id = NA;
   }

   if( import_maps == IMPORT_MAPS ) {
      if( parent_id != NA )
	 parent_id += resp_load_offset;
      else
	 parent_id = hyperedge_data.id + resp_load_offset;
   }
   new ResponsibilityReference( hyperedge_data.id, hyperedge_data.fx, hyperedge_data.fy, ldir, parent_id, position );
}

void XmlManager::JoinElement( const char **attributes )
{
   new OrJoin( hyperedge_data.id, hyperedge_data.fx, hyperedge_data.fy, hyperedge_data.description, atoi( attributes[1] ) );
}

void XmlManager::ForkElement( const char **attributes )
{
   new OrFork( hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy, hyperedge_data.description, atoi( attributes[1] ) );
}

void XmlManager::PathBranchingSpecElement( const char **attributes )
{
   // Apr2005 gM: required for map import to avoid conflict with constant IDs
   // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   XmlManager::current_fork = (OrFork *)(current_map->MapHypergraph()->FindHyperedge( (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[1]+1 ) + hyperedge_load_offset : atoi( attributes[1]+1 )) ));
}

void XmlManager::BranchingCharacteristicElement( const char **attributes )
{
   float probability = 0;
   const char *condition = NULL, *characteristic = NULL;
   char newcondition[1000];			// Apr2005 gM: required for map import to avoid conflict with constant IDs
   Hyperedge *branch;
    
   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if( strequal( attributes[i], "probability" ))
	 probability = atof( attributes[i+1] );
      else if( strequal( attributes[i], "branch-condition" )) {
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 if( import_maps == IMPORT_MAPS ) {
	  AddOffsetToBoolean( attributes[i+1], newcondition );
	  condition = newcondition;
	 }
	 else
	  condition = attributes[i+1];
	}
      else if( strequal( attributes[i], "empty-segment" ))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
	 branch = current_map->MapHypergraph()->FindHyperedge( (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 )) );
      else if( strequal( attributes[i], "characteristic" ))
	 characteristic = attributes[i+1];
   }

   XmlManager::current_fork->AddBranchingCharacteristic( branch, condition, characteristic, probability );
}

void XmlManager::SynchronizationElement( const char **attributes )
{
   int i, orientation;
    
   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal( attributes[i], "orientation" )){ 
	 orientation = atoi( attributes[i+1] );
	 break;
      }
   }

   new Synchronization( hyperedge_data.id, hyperedge_data.fx, hyperedge_data.fy, hyperedge_data.description, orientation );
}

void XmlManager::StartStubElement( const char **attributes )
{
   const char *shared="no", *policy = NULL;
   stub_type type = STATIC;

   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if( strequal( attributes[i], "type" ))
	 type = (strequal( attributes[i+1], "static" ) ? STATIC : DYNAMIC);
      else if( strequal( attributes[i], "shared" ))
	 shared = attributes[i+1];
      else if( strequal( attributes[i], "selection-policy" ))
	 policy = XmlManager::AddNewlines( attributes[i+1], 0 );
   }

   XmlManager::stub = new Stub( hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy, hyperedge_data.description,
				type, flag( shared ), policy );
   XmlManager::current_stub_list = new StubEntryExitList( stub, hyperedge_data.id );
   XmlManager::stub_boundaries.Add( current_stub_list );
}

void XmlManager::EndStubElement()
{
   int i;
   // install preconditions and postconditions, if any

   for( i = 0; i < event_number[PRECONDITION]; i++ )
      stub->AddPrecondition( event_data[PRECONDITION][i] );

   event_number[PRECONDITION] = 0;

   for( i = 0; i < event_number[POSTCONDITION]; i++ )
      stub->AddPostcondition( event_data[POSTCONDITION][i] );

   event_number[POSTCONDITION] = 0;   
}

void XmlManager::StartEmptySegmentElement( const char **attributes )
{
   int characteristics = NONE;
   bool show_label = FALSE;
   Label *label = NULL;
   int i = 0;
   
   while( attributes[i] != NULL ){
      if( strequal( attributes[i], "path-label-id" ) ) 
	 label = current_map->FindLabel( atoi( attributes[i+1]+2 ) );
      else if( strequal( attributes[i], "characteristics" ) ) {
	 if( strequal( attributes[i+1], "direction-arrow" ) )
	    characteristics = DIRECTION_ARROW;
	 else if( strequal( attributes[i+1], "failure-point" ) )
	    characteristics = FAILURE_POINT;
	 else
	    characteristics = SHARED_RESPONSIBILITY;
      }
      else if( strequal( attributes[i], "show-label" ) )
	 show_label = TRUE;
      i += 2;
   }
   
   XmlManager::empty = new Empty( hyperedge_data.id, hyperedge_data.fx, hyperedge_data.fy, ((label != NULL) ? label : hyperedge_data.label),
				  hyperedge_data.description, characteristics, show_label );
}

void XmlManager::EndEmptySegmentElement()
{
   int i;
   // install preconditions and postconditions, if any

   for( i = 0; i < event_number[PRECONDITION]; i++ )
      empty->AddPrecondition( event_data[PRECONDITION][i] );

   event_number[PRECONDITION] = 0;

   for( i = 0; i < event_number[POSTCONDITION]; i++ )
      empty->AddPostcondition( event_data[POSTCONDITION][i] );

   event_number[POSTCONDITION] = 0;   
}

void XmlManager::TimestampPointElement( const char **attributes )
{
   int i, orientation;
   timestamp_reference refer;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"orientation"))
	 orientation = atoi( attributes[i+1] );
      else if(strequal(attributes[i],"reference"))
	 refer = ( strequal( attributes[i+1], "previous" ) ? PREVIOUS : NEXT );
   }

   Timestamp * timestamp = new Timestamp( hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy, hyperedge_data.description,
					  orientation, refer );
   XmlManager::timestamps.Add( timestamp );
}

void XmlManager::GoalTagElement( const char **attributes )
{
   GoalTag *goal_tag = new GoalTag( hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy,
				    hyperedge_data.description );
   XmlManager::goal_tags.Add( goal_tag );
}

void XmlManager::ConnectElement( const char **attributes )
{
   new Connect( hyperedge_data.id );
}

void XmlManager::AbortElement( const char **attributes )
{
   new Abort( hyperedge_data.id );
}

void XmlManager::LoopElement( const char **attributes )
{
   const char *orientation, *count = NULL;

   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"orientation"))
	 orientation = attributes[i+1];
      else if(strequal(attributes[i],"exit-condition"))
	 count = attributes[i+1];
   }

   new Loop( hyperedge_data.id, hyperedge_data.name, hyperedge_data.fx, hyperedge_data.fy, hyperedge_data.description, orientation, count );
}

void XmlManager::TriggeringEventListElement( const char **attributes )
{
   XmlManager::event_type = TRIGGERING;
}

void XmlManager::ResultingEventListElement( const char **attributes )
{
   XmlManager::event_type = RESULTING;
}

void XmlManager::EventElement( const char **attributes )
{
   XmlManager::CreateCondition( attributes );
}

void XmlManager::PreconditionListElement( const char **attributes )
{
   XmlManager::event_type = PRECONDITION; 
}

void XmlManager::PostconditionListElement( const char **attributes )
{
   XmlManager::event_type = POSTCONDITION; 
}

void XmlManager::ConditionElement( const char **attributes )
{
   XmlManager::CreateCondition( attributes );
}

void XmlManager::StubEntryListElement( const char **attributes )
{
   // do nothing
}

void XmlManager::StubEntryElement( const char **attributes )
{
   int i, entry, hyper;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"stub-entry-id"))
	 entry = atoi( attributes[i+1]+2 );
      else if(strequal(attributes[i],"hyperedge-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
	 hyper = (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 ));
   }
   
   current_stub_list->AddStubEntry( hyper, entry );
}

void XmlManager::StubExitListElement( const char **attributes )
{
   // do nothing
}

void XmlManager::StubExitElement( const char **attributes )
{
   int i, exit, hyper;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"stub-exit-id")) 
	 exit = atoi( attributes[i+1]+2 );
      else if(strequal(attributes[i],"hyperedge-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
	 hyper = (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 ));
   }

   current_stub_list->AddStubExit( hyper, exit );
}

void XmlManager::HyperedgeConnectionElement( const char **attributes )
{
   // Apr2005 gM: required for map import to avoid conflict with constant IDs
   // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   XmlManager::current_source_id = (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[1]+1 ) + hyperedge_load_offset : atoi( attributes[1]+1 ));
}

void XmlManager::HyperedgeRefElement( const char **attributes )
{
   int id;					
   // Apr2005 gM: required for map import to avoid conflict with constant IDs	
   // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   id = (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[1]+1 ) + hyperedge_load_offset : atoi( attributes[1]+1 ));

   if( reference_state == CONNECTIONS )
      XmlManager::current_map->MapHypergraph()->ConnectHyperedges( current_source_id, id );
   else
      XmlManager::component_reference->AddContainedElement( current_map->MapHypergraph()->FindHyperedge( id )->GetFigure() );

}
   
void XmlManager::StructureSpecElement( const char **attributes )
{
   XmlManager::reference_state = COMPONENT_REFERENCE;
}

void XmlManager::ComponentElement( const char **attributes )
{
   ComponentReference *parent = NULL;
   const char *name, *description = NULL, *formal="no";
   bool duplicate_name;
   int i, existing_index, id, colour;
   float x, y, width, height;
    
   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"component-id"))
	 id = atoi( attributes[i+1]+1 );
      else if(strequal(attributes[i],"component-name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"description"))
	 description = attributes[i+1];
      else if(strequal(attributes[i],"formal"))
	 formal = attributes[i+1];
      else if(strequal(attributes[i],"colour"))
	 colour = atoi( attributes[i+1] );
      else if(strequal(attributes[i],"component-parent"))
	 parent = XmlManager::current_map->FindComponent( atoi( attributes[i+1]+1 ) );
      else if(strequal(attributes[i],"fx"))
	 x = coordinate(i+1);
      else if(strequal(attributes[i],"fy"))
	 y = coordinate(i+1);
      else if(strequal(attributes[i],"width"))
	 width= coordinate(i+1);
      else if(strequal(attributes[i],"height"))
	 height= coordinate(i+1);
   }
 
   if( XmlManager::import_maps == IMPORT_MAPS ) {
      if( !((XmlManager::mode_selected == TRUE) && (XmlManager::import_substitution == FALSE)) )
	 duplicate_name = component_manager->DuplicateName( name, existing_index );

      if( duplicate_name == TRUE && XmlManager::mode_selected == FALSE ) {
	 XmlManager::import_substitution = XmlManager::PromptImportMode();
	 XmlManager::mode_selected = TRUE;
      }

      // if substitution import mode is turned on determine if there is a name clash between this and a previously defined component
      if( XmlManager::import_substitution == TRUE ) {
	 if( duplicate_name == TRUE ) { // store mapping between existing identifier and this identifier
	    component_manager->CreateMapping( id+comp_load_offset, existing_index );
	    return; // do not create a new component object
	 }
      }
   }

   if( XmlManager::version_number >= 17 ) { // new format with separation between component definitions and references, create component object only
      XmlManager::component = new Component( (( import_maps == IMPORT_MAPS ) ? id + comp_load_offset : id),
					     name, flag( formal ), colour, description );
   }    
   else { // old combined format, must create both reference and definition
      // read in component id, name, formal/anchored flags, fx, fy, width, height, colour and create objects
      if( description != NULL )
	 description = XmlManager::AddNewlines( description, 0 );
       
      XmlManager::component = new Component( id, name, flag( formal ), colour, description );
      XmlManager::component_reference = new ComponentReference( id, x, y, width, height, parent, flag( formal ), NA, component );
      XmlManager::current_map->AddComponentReference( component_reference );
   }
    
}

void XmlManager::ComponentReferenceElement( const char **attributes )
{
   ComponentReference *parent = NULL;
   const char *role = NULL, *anchored="no", *fixed="no";
   int i, id = NA, ref_index = NA;
   float x, y, width, height;
    
   for(i=0;attributes[i]!=NULL;i+=2) {
      if(strequal(attributes[i],"component-ref-id"))
	 id = atoi( attributes[i+1]+2 );
      else if(strequal(attributes[i],"referenced-component"))
	 ref_index = atoi( attributes[i+1]+1 );
      else if(strequal(attributes[i],"component-parent")) 
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
	 parent = XmlManager::current_map->FindComponent( (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[i+1]+2 ) + comp_ref_load_offset : atoi( attributes[i+1]+2 )) );  
      else if(strequal(attributes[i],"role"))
	 role = attributes[i+1];
      else if(strequal(attributes[i],"anchored"))
	 anchored= attributes[i+1];
      else if(strequal(attributes[i],"fixed"))
	 fixed= attributes[i+1];
      else if(strequal(attributes[i],"fx"))
	 x = coordinate(i+1);
      else if(strequal(attributes[i],"fy"))
	 y = coordinate(i+1);
      else if(strequal(attributes[i],"width"))
	 width= coordinate(i+1);
      else if(strequal(attributes[i],"height"))
	 height= coordinate(i+1);
   }
   if( import_maps == IMPORT_MAPS ) {
      if( ref_index != NA )
	 ref_index += comp_load_offset;
      if( id != NA )				   // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id += comp_ref_load_offset;
   }
   else if( copy_map ) {			// Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   	  id += comp_ref_load_offset;
   }

   XmlManager::component_reference = new ComponentReference( id, x, y, width, height, parent, flag( anchored ), ref_index, NULL, role );
   XmlManager::current_map->AddComponentReference( component_reference );
}

void XmlManager::ComponentSpecificationElement( const char **attributes )
{
   // do nothing
}

void XmlManager::ResponsibilityListElement( const char **attributes )
{
   // do nothing
}

void XmlManager::OtherHyperedgeListElement( const char **attributes )
{
   // do nothing
}

void XmlManager::RegularElement( const char **attributes )
{
   const char *type="team", *protect="no", *slot="no", *replicated="no", *rep_factor="1";
   int i, processor_id = NA;
    
   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"type"))
	 type = attributes[i+1];
      else if(strequal(attributes[i],"protected"))
	 protect = attributes[i+1];
      else if(strequal(attributes[i],"slot"))
	 slot = attributes[i+1];
      else if(strequal(attributes[i],"replicated"))
	 replicated = attributes[i+1];
      else if(strequal(attributes[i],"replication-factor"))
	 rep_factor = attributes[i+1];
      else if(strequal(attributes[i],"processor-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs	
	 processor_id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + device_load_offset : atoi( attributes[i+1]+2 ));
   }

   XmlManager::component->SetCharacteristics( type, flag( protect ), flag( replicated ), rep_factor,
					      flag( slot ), processor_id );
}

void XmlManager::PoolElement( const char **attributes )
{
   XmlManager::component->SetType( POOL );
   XmlManager::component->PoolComponentType( attributes[1] );

   if( strequal( attributes[1], "plugin" ) )
      plugin_pools.Add( component );
}
   
void XmlManager::StartPluginBindingsElement( const char **attributes )
{
   // do nothing
}

void XmlManager::EndPluginBindingsElement()
{
   // empty temporary data structures
   while( !stub_boundaries.is_empty() )
      delete stub_boundaries.Detach();

   while( !plugin_pools.is_empty() )
      plugin_pools.Detach();
}

void XmlManager::PluginBindingElement( const char **attributes )
{
   const char *condition = NULL;
   char newcondition[1000];	// Apr2005 gM: required for map import to avoid conflict with constant IDs
   int i, stub_id;
   float probability = 1.0;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"parent-map-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 XmlManager::parent_map = XmlManager::FindMap( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + model_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal(attributes[i],"submap-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
	 XmlManager::submap = XmlManager::FindMap( (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[i+1]+1 ) + model_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal(attributes[i],"stub-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 stub_id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 ));
      else if( strequal( attributes[i], "branch-condition" )) {
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 if( import_maps == IMPORT_MAPS ) {
	  AddOffsetToBoolean( attributes[i+1], newcondition );
	  condition = newcondition;
	 }
	 else
	  condition = attributes[i+1];
	}
      else if(strequal(attributes[i],"probability"))
	 probability = atof( attributes[i+1] );
   }

   XmlManager::current_stub_list = NULL;
  
   // find StubEntryExitList object corresponding to stub id
   for( stub_boundaries.First(); !stub_boundaries.IsDone(); stub_boundaries.Next() ) {
      if( stub_boundaries.CurrentItem()->stub_id == stub_id ) {
	 current_stub_list = stub_boundaries.CurrentItem();
	 break;
      }
   }

   if( current_stub_list == NULL ) AbortProgram( "Stub entry/exit list corresponding to plugin binding not found" );

   Stub *source_stub = current_stub_list->stub;

   XmlManager::plugin_binding = new PluginBinding( source_stub, submap, condition, probability );
   source_stub->AddPluginBinding( XmlManager::plugin_binding );
}

void XmlManager::InConnectionListElement( const char **attributes ) {} // do nothing

void XmlManager::InConnectionElement( const char **attributes )
{
   Hyperedge *plugin_start, *input_hyperedge;
   
   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"stub-entry-id"))
	 input_hyperedge = XmlManager::parent_map->MapHypergraph()->FindHyperedge(current_stub_list->FindHyperedgeId( atoi( attributes[i+1]+2 ), PATH_ENTRY));
      else if(strequal(attributes[i],"hyperedge-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 plugin_start = XmlManager::submap->MapHypergraph()->FindHyperedge( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 )) ); // find start point in plugin
   }

   plugin_binding->AddBinding( plugin_start, input_hyperedge, PATH_ENTRY ); // perform binding   
}

void XmlManager::OutConnectionListElement( const char **attributes ) {} // do nothing

void XmlManager::OutConnectionElement( const char **attributes )
{
   Hyperedge *plugin_end, *output_hyperedge;
   
   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if( strequal( attributes[i], "stub-exit-id" ))
	 output_hyperedge = XmlManager::parent_map->MapHypergraph()->FindHyperedge(current_stub_list->FindHyperedgeId( atoi( attributes[i+1]+2 ), PATH_EXIT));
      else if( strequal( attributes[i], "hyperedge-id" ))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 plugin_end = XmlManager::submap->MapHypergraph()->FindHyperedge( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 )) ); // find start point in plugin
   }

   plugin_binding->AddBinding( plugin_end, output_hyperedge, PATH_EXIT ); // perform binding   
}

void XmlManager::PluginPoolElement( const char **attributes )
{
   int i, component_id, model_id;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"model-id"))
	 model_id = atoi( attributes[i+1]+1 );
      else if(strequal(attributes[i],"pool-id"))
	 component_id = atoi( attributes[i+1]+1 );
   }

   for( plugin_pools.First(); !plugin_pools.IsDone(); plugin_pools.Next() ) {
      if( plugin_pools.CurrentItem()->LoadIdentifier() == component_id ) {
	 plugin_pools.CurrentItem()->AddPoolPlugin( XmlManager::FindMap( model_id ) );
	 return;
      }
   }

   AbortProgram( "Plugin pool not found" );
}

void XmlManager::StartResponseTimeRequirementsElement( const char **attributes ) {} // do nothing

void XmlManager::EndResponseTimeRequirementsElement()
{
   while( !timestamps.is_empty() )
      timestamps.Detach();
}

void XmlManager::ResponseTimeReqElement( const char **attributes )
{
   Timestamp *tp1, *tp2;
   int i, time, percent;
   const char *name;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"timestamp1"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 tp1 = XmlManager::FindTimestampPoint( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal(attributes[i],"timestamp2"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 tp2 = XmlManager::FindTimestampPoint( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal(attributes[i],"resptime-name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"response-time"))
	 time = atoi( attributes[i+1] );
      else if(strequal(attributes[i],"percentage"))
	 percent = atoi( attributes[i+1] );
   }

   ResponseTimeManager::Instance()->AddNewResponseTime( tp1, tp2, name, time, percent );
}

void XmlManager::AgentAnnotationsElement( const char **attributes )
{
   // do nothing
}

void XmlManager::StartGoalListElement( const char **attributes )
{
   // do nothing
}

void XmlManager::EndGoalListElement()
{
   while( !goal_tags.is_empty() )
      goal_tags.Detach();
}

void XmlManager::StartGoalElement( const char **attributes )
{
   GoalTag *gt1, *gt2;
   const char *name, *description = NULL;

   for( int i=0; attributes[i]!=NULL; i+=2) {
      if(strequal(attributes[i],"start-point"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 gt1 = XmlManager::FindGoalTag( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal(attributes[i],"end-point"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 gt2 = XmlManager::FindGoalTag( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal(attributes[i],"goal-name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"description"))
	 description = attributes[i+1];
   }

   XmlManager::goal = new Goal( gt1, gt2, name, description );
   GoalList::Instance()->AddGoal( goal );
}

void XmlManager::EndGoalElement()
{
   int i;
   // install preconditions and postconditions, if any

   for( i = 0; i < event_number[PRECONDITION]; i++ )
      goal->AddPrecondition( event_data[PRECONDITION][i] );

   event_number[PRECONDITION] = 0;

   for( i = 0; i < event_number[POSTCONDITION]; i++ )
      goal->AddPostcondition( event_data[POSTCONDITION][i] );

   event_number[POSTCONDITION] = 0;   

}

void XmlManager::EnforceBindingsElement( const char **attributes )
{
  // Apr2005 gM: required for map import to avoid conflict with constant IDs
  // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
  XmlManager::stub = (Stub *)(current_map->MapHypergraph()->FindHyperedge( (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[1]+1 ) + hyperedge_load_offset : atoi( attributes[1]+1 )) )); 
}

void XmlManager::PathBindingElement( const char **attributes )
{
   Hyperedge *stub_input, *stub_output;
   
   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"stub-entry-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
	 stub_input = current_map->MapHypergraph()->FindHyperedge((( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[i+1]+2 ) + hyperedge_load_offset : atoi( attributes[i+1]+2 )) );
      else if(strequal(attributes[i],"stub-exit-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 // Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
	 stub_output = current_map->MapHypergraph()->FindHyperedge( (( import_maps == IMPORT_MAPS || copy_map ) ? atoi( attributes[i+1]+2 ) + hyperedge_load_offset : atoi( attributes[i+1]+2 )) );
   }

   XmlManager::stub->AddContinuityBinding( stub_input, stub_output ); 
}

void XmlManager::UcmSetsElement( const char **attributes )
{
   // do nothing
}

void XmlManager::UcmSetElement( const char **attributes )
{
   const char *set_name, *set_description = NULL;
   Map *primary_map = NULL;

   for( int i = 0; attributes[i]; i+=2 ) {
      if( strequal( attributes[i], "primary-map-id" ) )
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 primary_map = XmlManager::FindMap( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + model_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal( attributes[i], "name" ) )
      	 set_name = attributes[i+1];
      else if( strequal( attributes[i], "description" ) )
	 set_description = attributes[i+1]; 
   }

   current_ucm_set = new UcmSet( set_name,  primary_map, set_description );
   UcmSet::AddUcmSet( current_ucm_set );
}

void XmlManager::UcmSetItemElement( const char **attributes )
{
   Map *map;
   const char *comment = NULL;

   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"map-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 map = XmlManager::FindMap( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + model_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal(attributes[i],"comment"))
	 comment = attributes[i+1]; 
   }

   XmlManager::current_ucm_set->AddMap( map, comment );
}

void XmlManager::PathVariableListElement( const char **attributes ) {} // do nothing

void XmlManager::BooleanVariableElement( const char **attributes )
{
   int i, id, ref;
   const char *name;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"boolvar-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + boolean_load_offset : atoi( attributes[i+1]+2 ));
      else if(strequal(attributes[i],"ref-count"))
	 ref = atoi( attributes[i+1] );	    
      else if(strequal(attributes[i],"id")) // old name, kept for compatibility
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + boolean_load_offset : atoi( attributes[i+1]+2 ));
   }

   BooleanVariable::AddBooleanVariable( id, name, ref );
}

void XmlManager::ScenarioListElement( const char **attributes ) {} // do nothing

void XmlManager::ScenarioGroupElement( const char **attributes )
{
   const char *name, *description = NULL;
   int i;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"description"))
	 description=attributes[i+1];
   }
   current_scenario_group = ScenarioList::AddScenarioGroup( name, description );
}

void XmlManager::ScenarioElement( const char **attributes )
{
   Start *start_point = NULL;
   const char *name, *description = NULL;
   Map *map;

   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal(attributes[i],"name"))
	 name = attributes[i+1];
      else if(strequal(attributes[i],"map-id"))
	 map = XmlManager::FindMap( atoi( attributes[i+1]+1 ) );
      else if(strequal(attributes[i],"start-id"))
	 start_point = (Start *)map->MapHypergraph()->FindHyperedge( atoi( attributes[i+1]+1 ) );
      else if(strequal(attributes[i],"description"))
	 description=attributes[i+1];	    
   }

   current_scenario = current_scenario_group->AddScenario( name, description );
   if( start_point != NULL )
      current_scenario->AddStartingPoint( start_point );
}

void XmlManager::ScenarioStartElement( const char **attributes )
{
   Map *map = NULL;
   int hyperedge_id;

   for( int i=0; attributes[i]!=NULL; i+=2 ) {
      if( strequal( attributes[i], "map-id" ))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 map = XmlManager::FindMap( (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + model_load_offset : atoi( attributes[i+1]+1 )) );
      else if(strequal(attributes[i],"start-id"))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 hyperedge_id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+1 ) + hyperedge_load_offset : atoi( attributes[i+1]+1 ));
   }

   if( map != NULL )
      current_scenario->AddStartingPoint( (Start *)map->MapHypergraph()->FindHyperedge( hyperedge_id ) );
}

void XmlManager::VariableInitElement( const char **attributes )
{
   int i, id;
   bool value;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal( attributes[i], "variable-id" ))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + boolean_load_offset : atoi( attributes[i+1]+2 ));
       else if( strequal( attributes[i], "value" ))
	 value = ((attributes[i+1][0] == 'T') ? TRUE : FALSE);
   }

   current_scenario->AddVariableInitialization( id, value );
}

void XmlManager::ScenarioPostconditionElement( const char **attributes )
{
   int i, id;
   bool value;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if(strequal( attributes[i], "variable-id" ))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + boolean_load_offset : atoi( attributes[i+1]+2 ));
      else if( strequal( attributes[i], "value" ))
	 value = ((attributes[i+1][0] == 'T') ? TRUE : FALSE);
   }

   current_scenario->AddScenarioPostcondition( id, value );
}

void XmlManager::VariableOperationListElement( const char **attributes ) {} // do nothing

void XmlManager::VariableOperationElement( const char **attributes )
{
   int i, id, value;
   const char *expression = NULL;

   for( i=0; attributes[i]!=NULL; i+=2 ) {
      if( strequal( attributes[i], "variable-id" ))
	 // Apr2005 gM: required for map import to avoid conflict with constant IDs
	 id = (( import_maps == IMPORT_MAPS ) ? atoi( attributes[i+1]+2 ) + boolean_load_offset : atoi( attributes[i+1]+2 ));
      else if( strequal( attributes[i], "value" )) {
	 if( strequal( attributes[i+1], "T" ) )
	    value = TRUE;
	 else if( strequal( attributes[i+1], "F" ) )
	    value = FALSE;
	 else {
	    value = EVALUATED_EXPRESSION;
	    expression = attributes[3];
	 }
      }
   }

   XmlManager::responsibility->AddVariableOperation( id, value, expression );
}

void XmlManager::AddStartHandler( const char *element_name, void (*handler)( const char ** ) )
{
   StartHandlerTreeElement *current_element, *next_element;
   StartHandlerTreeElement *new_element = &XmlManager::start_handler_tree[XmlManager::start_handler_tree_size++];
   unsigned long key = hash( element_name );
   
   new_element->key = key;
   new_element->handler = handler;

   if( XmlManager::start_handler_root == NULL ) { // install first element as root element
      XmlManager::start_handler_root = new_element;
      return;
   }
   
   // find appropriate place for new element in tree
   current_element = XmlManager::start_handler_root;

   forever {
      
      if( key > current_element->key ) {
	 
	 next_element = current_element->right;
	 if( next_element == NULL ) { // install new element to right of current element
	    current_element->right = new_element;
	    return;
	 }
	 current_element = next_element;
      }
      else if ( key < current_element->key ) {
	 
	 next_element = current_element->left;
	 if( next_element == NULL ) { // install new element to left of current element
	    current_element->left = new_element;
	    return;
	 }
	 current_element = next_element;
      }
      else         // if key == current_element->key two element names have hashed to the same key value
	 AbortProgram( "Two XML elements have hashed to the same key" );
   }
}

void XmlManager::AddEndHandler( const char *element_name, void (*handler)() )
{
   EndHandlerTreeElement *current_element, *next_element;
   EndHandlerTreeElement *new_element = &XmlManager::end_handler_tree[XmlManager::end_handler_tree_size++];
   unsigned long key = hash( element_name );
   
   new_element->key = key;
   new_element->handler = handler;

   if( XmlManager::end_handler_root == NULL ) { // install first element as root element
      XmlManager::end_handler_root = new_element;
      return;
   }
   
   // find appropriate place for new element in tree
   current_element = XmlManager::end_handler_root;

   forever {
      
      if( key > current_element->key ) {
	 
	 next_element = current_element->right;
	 if( next_element == NULL ) { // install new element to right of current element
	    current_element->right = new_element;
	    return;
	 }
	 current_element = next_element;
      }
      else if ( key < current_element->key ) {
	 
	 next_element = current_element->left;
	 if( next_element == NULL ) { // install new element to left of current element
	    current_element->left = new_element;
	    return;
	 }
	 current_element = next_element;
      }
      else         // if key == current_element->key two element names have hashed to the same key value
	 AbortProgram( "Two XML elements have hashed to the same key" );
   }
}

StartHandler XmlManager::FindStartHandler( const char *element_name )
{
   StartHandlerTreeElement *current_element = XmlManager::start_handler_root;
   unsigned long key = hash( element_name );

   forever {
      if( key > current_element->key ) {
	 current_element = current_element->right;
	 if( current_element == NULL )
	    return( NULL );
      }
      else if ( key < current_element->key ) {
	 current_element = current_element->left;
	 if( current_element == NULL )
	    return( NULL );
      }
      else  // key == current_element->key
	 return( current_element->handler );
   }
}

EndHandler XmlManager::FindEndHandler( const char *element_name )
{
   EndHandlerTreeElement *current_element = XmlManager::end_handler_root;
   unsigned long key = hash( element_name );

   forever {
      if( key > current_element->key ) {
	 current_element = current_element->right;
	 if( current_element == NULL )
	    return( NULL );
      }
      else if ( key < current_element->key ) {
	 current_element = current_element->left;
	 if( current_element == NULL )
	    return( NULL );
      }
      else  // key == current_element->key
	 return( current_element->handler );
   }
}

void XmlManager::ConstructHandlerTree()
{
   // add all element name, handler pairs to the tree
   // add start element handlers to start element handler tree
   XmlManager::AddStartHandler( "ucm-design", XmlManager::UCMDesignElement );
   XmlManager::AddStartHandler( "device-directory", XmlManager::DeviceDirectoryElement );
   XmlManager::AddStartHandler( "device", XmlManager::DeviceElement );
   XmlManager::AddStartHandler( "data-store-directory", XmlManager::DataStoreDirectoryElement );
   XmlManager::AddStartHandler( "data-store", XmlManager::DataStoreElement );
   XmlManager::AddStartHandler( "access-mode", XmlManager::AccessModeElement );
   XmlManager::AddStartHandler( "root-maps", XmlManager::RootMapsElement );
   XmlManager::AddStartHandler( "plugin-maps", XmlManager::PluginMapsElement );
   XmlManager::AddStartHandler( "model", XmlManager::ModelElement );
   XmlManager::AddStartHandler( "responsibility-specification", XmlManager::StartResponsibilitySpecElement );
   XmlManager::AddStartHandler( "responsibility-spec", XmlManager::StartResponsibilitySpecElement ); // keep old name for backward compatibility
   XmlManager::AddStartHandler( "responsibility", XmlManager::StartResponsibilityElement );
   XmlManager::AddStartHandler( "data-store-spec", XmlManager::DataStoreSpecElement );
   XmlManager::AddStartHandler( "data-store-access", XmlManager::DataStoreAccessElement );
   XmlManager::AddStartHandler( "dynamic-resp", XmlManager::DynamicRespElement );
   XmlManager::AddStartHandler( "path-spec", XmlManager::PathSpecElement );
   XmlManager::AddStartHandler( "path-label-list", XmlManager::PathLabelListElement );
   XmlManager::AddStartHandler( "path-label", XmlManager::PathLabelElement );
   XmlManager::AddStartHandler( "hypergraph", XmlManager::StartHypergraphElement );
   XmlManager::AddStartHandler( "hyperedge", XmlManager::HyperedgeElement );
   XmlManager::AddStartHandler( "start", XmlManager::StartStartElement );
   XmlManager::AddStartHandler( "waiting-place", XmlManager::StartWaitingPlaceElement );
   XmlManager::AddStartHandler( "end-point", XmlManager::StartEndPointElement );
   XmlManager::AddStartHandler( "responsibility-ref", XmlManager::ResponsibilityRefElement );
   XmlManager::AddStartHandler( "join", XmlManager::JoinElement );
   XmlManager::AddStartHandler( "fork", XmlManager::ForkElement );
   XmlManager::AddStartHandler( "synchronization", XmlManager::SynchronizationElement );
   XmlManager::AddStartHandler( "stub", XmlManager::StartStubElement );
   XmlManager::AddStartHandler( "empty-segment", XmlManager::StartEmptySegmentElement );
   XmlManager::AddStartHandler( "timestamp-point", XmlManager::TimestampPointElement );
   XmlManager::AddStartHandler( "connect", XmlManager::ConnectElement );
   XmlManager::AddStartHandler( "goal-tag", XmlManager::GoalTagElement );
   XmlManager::AddStartHandler( "abort", XmlManager::AbortElement );
   XmlManager::AddStartHandler( "loop", XmlManager::LoopElement );
   XmlManager::AddStartHandler( "triggering-event-list", XmlManager::TriggeringEventListElement );
   XmlManager::AddStartHandler( "resulting-event-list", XmlManager::ResultingEventListElement );
   XmlManager::AddStartHandler( "event", XmlManager::EventElement );
   XmlManager::AddStartHandler( "precondition-list", XmlManager::PreconditionListElement );
   XmlManager::AddStartHandler( "postcondition-list", XmlManager::PostconditionListElement );
   XmlManager::AddStartHandler( "condition", XmlManager::ConditionElement );
   XmlManager::AddStartHandler( "stub-entry-list", XmlManager::StubEntryListElement );
   XmlManager::AddStartHandler( "stub-entry", XmlManager::StubEntryElement );
   XmlManager::AddStartHandler( "stub-exit-list", XmlManager::StubExitListElement );
   XmlManager::AddStartHandler( "stub-exit", XmlManager::StubExitElement );
   XmlManager::AddStartHandler( "hyperedge-connection", XmlManager::HyperedgeConnectionElement );
   XmlManager::AddStartHandler( "hyperedge-ref", XmlManager::HyperedgeRefElement );
   XmlManager::AddStartHandler( "structure-spec", XmlManager::StructureSpecElement );
   XmlManager::AddStartHandler( "component", XmlManager::ComponentElement );
   XmlManager::AddStartHandler( "component-ref", XmlManager::ComponentReferenceElement );
   XmlManager::AddStartHandler( "component-specification", XmlManager::ComponentSpecificationElement );
   XmlManager::AddStartHandler( "responsibility-list", XmlManager::ResponsibilityListElement );
   XmlManager::AddStartHandler( "other-hyperedge-list", XmlManager::OtherHyperedgeListElement );
   XmlManager::AddStartHandler( "regular", XmlManager::RegularElement );
   XmlManager::AddStartHandler( "pool", XmlManager::PoolElement );
   XmlManager::AddStartHandler( "plugin-bindings", XmlManager::StartPluginBindingsElement );
   XmlManager::AddStartHandler( "plugin-binding", XmlManager::PluginBindingElement );
   XmlManager::AddStartHandler( "in-connection-list", XmlManager::InConnectionListElement );
   XmlManager::AddStartHandler( "in-connection", XmlManager::InConnectionElement );
   XmlManager::AddStartHandler( "out-connection-list", XmlManager::OutConnectionListElement );
   XmlManager::AddStartHandler( "out-connection", XmlManager::OutConnectionElement );
   XmlManager::AddStartHandler( "plugin-pool", XmlManager::PluginPoolElement );
   XmlManager::AddStartHandler( "response-time-requirements", XmlManager::StartResponseTimeRequirementsElement );
   XmlManager::AddStartHandler( "response-time-req", XmlManager::ResponseTimeReqElement );
   XmlManager::AddStartHandler( "agent-annotations", XmlManager::AgentAnnotationsElement );
   XmlManager::AddStartHandler( "goal-list", XmlManager::StartGoalListElement );
   XmlManager::AddStartHandler( "goal", XmlManager::StartGoalElement );
   XmlManager::AddStartHandler( "service-request-spec", XmlManager::ServiceRequestListElement );
   XmlManager::AddStartHandler( "service-request", XmlManager::ServiceRequestElement );
   XmlManager::AddStartHandler( "path-branching-spec", XmlManager::PathBranchingSpecElement );
   XmlManager::AddStartHandler( "path-branching-characteristic", XmlManager::BranchingCharacteristicElement );
   XmlManager::AddStartHandler( "enforce-bindings", XmlManager::EnforceBindingsElement );
   XmlManager::AddStartHandler( "path-binding", XmlManager::PathBindingElement );
   XmlManager::AddStartHandler( "ucm-sets", XmlManager::UcmSetsElement );
   XmlManager::AddStartHandler( "ucm-set", XmlManager::UcmSetElement );
   XmlManager::AddStartHandler( "ucm-set-elem", XmlManager::UcmSetItemElement );
   XmlManager::AddStartHandler( "path-variable-list", XmlManager::PathVariableListElement );
   XmlManager::AddStartHandler( "control-variable-list", XmlManager::PathVariableListElement ); // keep old name for backward compatibility
   XmlManager::AddStartHandler( "boolean-variable", XmlManager::BooleanVariableElement );
   XmlManager::AddStartHandler( "scenario-list", XmlManager::ScenarioListElement );
   XmlManager::AddStartHandler( "scenario-group", XmlManager::ScenarioGroupElement );
   XmlManager::AddStartHandler( "scenario-definition", XmlManager::ScenarioElement );
   XmlManager::AddStartHandler( "scenario-start", XmlManager::ScenarioStartElement );
   XmlManager::AddStartHandler( "variable-init", XmlManager::VariableInitElement );
   XmlManager::AddStartHandler( "variable-operation-list", XmlManager::VariableOperationListElement );
   XmlManager::AddStartHandler( "variable-operation", XmlManager::VariableOperationElement );
   XmlManager::AddStartHandler( "variable-operations", XmlManager::VariableOperationListElement ); // keep old name for backward compatibility
   XmlManager::AddStartHandler( "variable-set", XmlManager::VariableOperationElement ); // keep old name for backward compatibility
   XmlManager::AddStartHandler( "postcondition", XmlManager::ScenarioPostconditionElement );

   // add end element handlers to end element handler tree
   XmlManager::AddEndHandler( "ucm-design", XmlManager::EndDesign );
   XmlManager::AddEndHandler( "responsibility-specification", XmlManager::EndResponsibilitySpecElement );
   XmlManager::AddEndHandler( "responsibility-spec", XmlManager::EndResponsibilitySpecElement ); // keep old name for backward compatibility
   XmlManager::AddEndHandler( "hypergraph", XmlManager::EndHypergraphElement );
   XmlManager::AddEndHandler( "responsibility", XmlManager::EndResponsibilityElement );
   XmlManager::AddEndHandler( "start", XmlManager::EndStartElement );
   XmlManager::AddEndHandler( "waiting-place", XmlManager::EndWaitingPlaceElement );
   XmlManager::AddEndHandler( "end-point", XmlManager::EndEndPointElement );
   XmlManager::AddEndHandler( "stub", XmlManager::EndStubElement );
   XmlManager::AddEndHandler( "empty-segment", XmlManager::EndEmptySegmentElement );
   XmlManager::AddEndHandler( "plugin-bindings", XmlManager::EndPluginBindingsElement );
   XmlManager::AddEndHandler( "response-time-requirements", XmlManager::EndResponseTimeRequirementsElement );
   XmlManager::AddEndHandler( "goal-list", XmlManager::EndGoalListElement );
   XmlManager::AddEndHandler( "goal", XmlManager::EndGoalElement );

   XmlManager::tree_constructed = TRUE;
   XmlManager::hyperedge_data.name[29] = 0;
   XmlManager::hyperedge_data.description[8095] = 0;
   XmlManager::single_map_label = NULL;
}

void XmlManager::CreateCondition( const char **attributes )
{
   char buffer[4096];
   *buffer = 0;
  
   if( attributes[2] != NULL )
      sprintf( buffer, "%s&&%s", attributes[1], XmlManager::AddNewlines( attributes[3], 0 ) );
   else
      sprintf( buffer, "%s&&", attributes[1] );
      
   event_data[event_type][event_number[event_type]] = strdup( buffer );
   event_number[event_type]++;
}

Timestamp * XmlManager::FindTimestampPoint( int ts_id )
{
   for( timestamps.First(); !timestamps.IsDone(); timestamps.Next() ) {
      if( timestamps.CurrentItem()->LoadIdentifier() == ts_id )
	 return( timestamps.CurrentItem() );
   }
   AbortProgram( "Timestamp point not found" );
   return ( NULL ); // DA: Added August 2004 (just in case)
}

Map * XmlManager::FindMap( int map_id )
{
   Cltn<Map *> *map_list = ( XmlManager::import_maps == IMPORT_MAPS ) ? &import_map_list : display_manager->Maps();

   for( map_list->First(); !map_list->IsDone(); map_list->Next() ) {
      if( map_list->CurrentItem()->LoadIdentifier() == map_id )
	 return( map_list->CurrentItem() );
   }

   return( NULL );
   //AbortProgram( "Map not found." );
}

bool XmlManager::MapExportable( Map *map )
{
   if( export_list == NULL )
      return TRUE;

   return( export_list->Includes( map ) );
}

GoalTag * XmlManager::FindGoalTag( int gt_id )
{
   for( goal_tags.First(); !goal_tags.IsDone(); goal_tags.Next() ) {
      if( goal_tags.CurrentItem()->LoadIdentifier() == gt_id )
	 return( goal_tags.CurrentItem() );
   }
   AbortProgram( "Goal tag not found" );
   return ( NULL ); // DA: Added August 2004 (just in case) 
}
   
const char * XmlManager::AddNewlines( const char *text, int bid )
{
   int i = 0, j = 0;
   bool space_found = FALSE;

   if( XmlManager::version_number >= 15 )
      return( text );
   
   // search for consecutive spaces and replace with a newline, obsolete, kept for compatibility

   while( text[i] != 0 ) {
      if( text[i] != ' ' ) {
	 text_buffer[bid][j++] = text[i];
	 space_found = FALSE;
      }
      else if ( space_found == FALSE ) {
	 text_buffer[bid][j++] = ' ';
	 space_found = TRUE;
      }
      else {
	 text_buffer[bid][j-1] = '\n';
	 space_found = FALSE;
      }
      i++;
   }
   
   text_buffer[bid][j] = 0;
   return( text_buffer[bid] );
}

bool XmlManager::PromptImportMode()
{
   return ( QuestionDialog( "Choose Import Mode", "Name clashes between existing/importing elements have been detected.",
			    "Do you wish to substitute existing elements in place of imported elements ?",
			    "Substitute Conflicting Elements", "Extend Names of Conflicting Elements" ) );
}

void XmlManager::RemoveWhitespace( char *processed, char *unprocessed )
{ // remove all leading and trailing spaces from a string as well as extra interior spaces
   int i = 0, j = 0;
   bool char_found = FALSE;
   
   while( unprocessed[i] ) {
      if( unprocessed[i] != ' ' ) {
	 processed[j++] = unprocessed[i];
	 char_found = TRUE;
      }
      else if( char_found ) {
	 processed[j++] = ' ';
	 char_found = FALSE;
      }
      i++;
   }

   if( processed[j-1] == ' ' )
      processed[j-1] = 0; // remove terminating space and null terminate
   else
      processed[j] = 0; // null terminate output string
}

void XmlManager::AddOffsetToBoolean( const char *condition, char *newcondition ) // Apr2005 gM: required for map import to avoid conflict with constant IDs
{   
   char l[100];
   int len = strlen( condition ), len2 = sprintf( l, "%d", boolean_load_offset) + 1, i, j = 0, k = 0;
   char s[len*len2], n[len], m[len+len2];
   bool foundDigit = FALSE;

   for( i=0; i<len; i++ ) {
	if( isdigit( condition[i] ) != 0 )
	   if( foundDigit )
		n[k++] = condition[i];
	   else {
		k = 0;
		n[k++] = condition[i];
		foundDigit = TRUE;
	   }
	else
	   if( foundDigit ) {
		foundDigit = FALSE;
		s[j] = 0;
		n[k] = 0;
		sprintf( m, "%d",  atoi( n ) + boolean_load_offset );
		strcat( s, m );
		j = strlen ( s );
		s[j++] = condition[i];
	   }
	   else
		s[j++] = condition[i];
   }
   s[j] = 0;
   n[k] = 0;
   if( foundDigit ) {
	sprintf( m, "%d",  atoi( n ) + boolean_load_offset );
	strcat( s, m );
   }
   strcpy( newcondition, s);
}
