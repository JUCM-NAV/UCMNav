/***********************************************************
 *
 * File:			map.cc
 * Author:			Andrew Miga
 * Created:			May 1997
 *
 * Modification history:
 *
 ***********************************************************/

#include "map.h"
#include "hypergraph.h"
#include "figure.h"
#include "path.h"
#include "component.h"
#include "component_mgr.h"
#include "transformation.h"
#include "resp_ref.h"
#include "responsibility.h"
#include "resp_mgr.h"
#include "or_null_figure.h"
#include "synch_null_figure.h"
#include "loop.h"
#include "loop_null_figure.h"
#include "stub_figure.h"
#include "xml_mgr.h"
#include "print_mgr.h"
#include "goal.h"
#include "goal_tag.h"
#include "stub.h"
#include "start.h"
#include "empty.h"
#include "result.h"
#include "ucm_set.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

extern ComponentManager *component_manager;
extern char loaded_file_path[128];
extern char loaded_file[64];

int Map::number_maps = 0;
int Map::number_root_maps = 0;
int Map::number_plugin_maps = 0;

#define FIGURE_BOUNDARY 0.05 // rough extension for text strings

int map_deletion = 0; // global flag which denotes deletion of all map objects

Map::Map( const char *label, map_type type )
{
   mtype = type;
   map_label[LABEL_SIZE] = '\0';
   if( label ) strncpy( map_label, label, LABEL_SIZE );

   graph = new Hypergraph;
   figures = new Cltn<HyperedgeFigure *>;
   paths = new Cltn<Path *>;
   components = new Cltn< ComponentReference *>;
   responsibilities = new Cltn<ResponsibilityReference *>;
   labels = new Cltn<Label *>;
   parent_stubs = new Cltn<Stub *>;
   
   if( mtype == ROOT_MAP )
      number_root_maps++;
   else
      number_plugin_maps++;
      
      
   scanning_parent_stub = NULL;
   map_title = NULL;
   map_description = NULL;
   reference_count = 0;
   map_number = number_maps++;   
   display_manager->RegisterMap( this );
}

Map::Map( int map_id, const char *label, map_type type, const char *title, const char *description )
{
   load_number = map_id;
 
   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file  
   map_number = map_id;
   if (number_maps <= map_number) number_maps = map_number+1; 
   //End of the modification by Bo Jiang, April, 2005   
   
   mtype = type;
   map_label[LABEL_SIZE] = '\0';
   if( label ) strncpy( map_label, VerifyUniqueMapName(label ), LABEL_SIZE );
   map_title = ( strcmp( title, "" ) != 0 ) ? strdup( title ) : NULL;
   map_description = ( strcmp( description, "" ) != 0 ) ? strdup( description ) : NULL;

   graph = new Hypergraph;
   figures = new Cltn<HyperedgeFigure *>;
   paths = new Cltn<Path *>;
   components = new Cltn< ComponentReference *>;
   responsibilities = new Cltn<ResponsibilityReference *>;
   labels = new Cltn<Label *>;
   parent_stubs = new Cltn<Stub *>;

   reference_count = 0;
   // map_number = number_maps++; //Modified by Bo Jiang. April,2005, fixed that ID changes when load a .ucm file
   if( mtype == ROOT_MAP )
      number_root_maps++;
   else
      number_plugin_maps++;
   
   scanning_parent_stub = NULL;
   display_manager->RegisterMap( this );
   display_manager->PlaceMap( this );
}

Map::~Map()
{
   map_deletion++;

   Map *old_map = display_manager->CurrentMap();
   display_manager->PlaceMap( this );   // place data structures in DM so destructors function properly
   display_manager->PurgeMap( this );
   delete graph;
   graph = NULL;

   delete figures;
   figures = NULL;
   
   while( !paths->is_empty() )
      delete paths->Detach();
   delete paths;
   paths = NULL;

   while( !parent_stubs->is_empty() )
      parent_stubs->Detach();
   delete parent_stubs;
   parent_stubs = NULL;
   
   for( ucm_sets.First(); !ucm_sets.IsDone(); ucm_sets.Next() )
      ucm_sets.CurrentItem()->RemoveSetMap( this );
   
   while( !components->is_empty() )
      delete components->Detach();
   delete components;
   components = NULL;
   
   while( !labels->is_empty() )
      delete labels->Detach();
   delete labels;
   labels = NULL;
   
   delete responsibilities;
   responsibilities = NULL;
   
   if( map_title != NULL )
      free( map_title );
   map_title = NULL;
   
   if( map_description != NULL )
      free( map_description );
   map_description = NULL;

   display_manager->PlaceMap( old_map );

   if( mtype == ROOT_MAP )
      number_root_maps--;
   else
      number_plugin_maps--;

   map_deletion--;
}

void Map::MapTitle( const char *new_title )
{
   if( map_title != NULL ) {
      free( map_title );
      map_title = NULL;
   }
   map_title = strdup( new_title );
}

void Map::MapDescription( const char *new_description )
{
   if( map_description != NULL ) {
      free( map_description );
      map_description = NULL;
   }
   map_description = strdup( new_description );
}

void Map::MapLabel( const char *new_label )
{
   map_label[0] = '\0';
   strncpy( map_label, new_label, LABEL_SIZE );
   display_manager->ReorderInMapList( this );

   for( ucm_sets.First(); !ucm_sets.IsDone(); ucm_sets.Next() )
      ucm_sets.CurrentItem()->ReorderInMapList( this );
}

void Map::SetMapType( int type )
{
   if( type != mtype ) {
      if( mtype == ROOT_MAP ) {
	 number_root_maps--;
	 number_plugin_maps++;
      } else {
	 number_root_maps++;
	 number_plugin_maps--;
      }
      mtype = type;
   }
}

void Map::RemoveParentStub( Stub *stub )
{
   if( parent_stubs) parent_stubs->Delete( stub );
   this->DecrementCount();
}

void Map::RegisterLabel( Label *new_label )
{
   if( !labels->Includes( new_label ) )
      labels->Add( new_label );
}

void Map::PurgeLabel( Label *label )
{
   labels->Delete( label );
}

Label * Map::NewLabel( Hyperedge *edge )
{
   const char *entered_label;

   if( (entered_label = LabelPrompt( " Enter label for new path ", "" )) != NULL )
      edge->PathLabel()->TextLabel( entered_label );
   return( edge->PathLabel() );
}

int Map::EditLabel( Hyperedge *edge, const char *message )
{
   const char *entered_label;

   if( (entered_label = LabelPrompt( message, edge->PathLabel()->TextLabel() )) != NULL ) {
      edge->PathLabel()->TextLabel( entered_label );
      return( MODIFIED );
   } else
      return( UNMODIFIED );
}
     
char * Map::LabelPrompt( const char *message, const char *default_label )
{
   const char *entered_label = NULL;
   static char label[DISPLAYED_LABEL+1];
   int count;
   bool quit = FALSE;

   label[DISPLAYED_LABEL] = 0; // null terminate array
   
   while( quit == FALSE ) {
   
      do {
	 if( (entered_label = fl_show_input( message, default_label )) == NULL )
	    return( NULL ); // allow user to cancel operation
	 if( PrintManager::TextNonempty( entered_label ) )
	    quit = TRUE;
	 else {
	    fl_show_message( " Label Is Empty ",
			     "A path label must be given.\n Please enter a new path label.", "" );
	 }
      } while( quit == FALSE );

      quit = FALSE;
      
      // strip any spaces out of the entered text
      count = 0;
      while( *entered_label != 0 && count < DISPLAYED_LABEL ) {
	 if( *entered_label != ' ' )
	    label[count++] = *entered_label;
	 entered_label++;
      }
      label[count] = 0;  // null terminate the string

      if( strequal( label, default_label ) )
	 return( NULL ); // as label hasn't changed, no need to change it

      // scan the rest of the labels in the pool for a conflict
      for( labels->First(); !labels->IsDone(); labels->Next() ) {
	 if( strcmp( label, labels->CurrentItem()->TextLabel()) == 0 ) {
	    fl_show_message( " Label Already Exists ",
			     "The label you have entered already exists on the map.\n Please choose a new path label.", "" );
	    quit = FALSE;
	    break;
	 }
	 else {
	    quit = TRUE;
	 }
      }
   }

   return( label );
}

void Map::SaveXML( FILE *fp )
{
   char buffer[1000];

   buffer[0] = 0;
   
   sprintf( buffer, "<model model-id=\"m%d\" model-name=\"%s\"", map_number, PrintDescription( map_label ) );
   PrintXMLText( fp, buffer );
   fprintf( fp, " title=\"%s\"", map_title ? PrintDescription( map_title ) : "" );
   fprintf( fp, " description=\"%s\" >\n", map_description ? PrintDescription( map_description ) : "" );   
   LinebreakXML( fp );
   IndentNewXMLBlock( fp );

   // save hyperedges, path-label-list

   if( graph->HypergraphExists() ) {  // save path spec only if paths exist

      PrintNewXMLBlock( fp, "path-spec" );
      PrintNewXMLBlock( fp, "path-label-list" );
   
      for( labels->First(); !labels->IsDone(); labels->Next() )
	 labels->CurrentItem()->SaveXML( fp );

      PrintEndXMLBlock( fp, "path-label-list" );
   
      PrintNewXMLBlock( fp, "hypergraph" );
      graph->SaveXML( fp );
      PrintEndXMLBlock( fp, "hypergraph" );   
      LinebreakXML( fp );
   
      PrintEndXMLBlock( fp, "path-spec" );
      LinebreakXML( fp );
   }

   if( components->Size() != 0 ) {
      PrintNewXMLBlock( fp, "structure-spec" );

      for( components->First(); !components->IsDone(); components->Next() )
	 components->CurrentItem()->SaveXML( fp );
   
      PrintEndXMLBlock( fp, "structure-spec" );
      LinebreakXML( fp );
   }

   PrintEndXMLBlock( fp, "model" );
   LinebreakXML( fp );
}

void Map::SaveCSMXML( FILE *fp )
{
   char buffer[1000];

   buffer[0] = 0;

   if( components->Size() != 0 ) {
     for( components->First(); !components->IsDone(); components->Next() )
	 components->CurrentItem()->SaveCSMXML( fp );
      LinebreakXML( fp );
	  
   }
   
   sprintf( buffer, "<Scenario id=\"m%d\" name=\"%s\"", map_number, PrintDescription( map_label ) );
   PrintXMLText( fp, buffer );
  // fprintf( fp, " title=\"%s\"", map_title ? PrintDescription( map_title ) : "" );
   fprintf( fp, " description=\"%s\" >\n", map_description ? PrintDescription( map_description ) : "" );   
   LinebreakXML( fp );
   IndentNewXMLBlock( fp );


	
   //Save responsibilities
  // ResponsibilityManager::Instance()->SaveCSMResponsibilityList(fp);

   // save hyperedges, path-label-list

   if( graph->HypergraphExists() ) {  // save path spec only if paths exist

     //PrintNewXMLBlock( fp, "path-spec" );
     //PrintNewXMLBlock( fp, "path-label-list" );
   
     //for( labels->First(); !labels->IsDone(); labels->Next() )
	   //labels->CurrentItem()->SaveXML( fp );

       //PrintEndXMLBlock( fp, "path-label-list" );
  
      //PrintNewXMLBlock( fp, "PathConnection" );
      graph->SaveCSMXML( fp );
      //PrintEndXMLBlock( fp, "PathConnection" );   
      LinebreakXML( fp );
   
      //PrintEndXMLBlock( fp, "path-spec" );
     // LinebreakXML( fp );
   }

   

   PrintEndXMLBlock( fp, "Scenario" );
   LinebreakXML( fp );
}

void Map::SaveDXL( FILE *fp )
{
   char buffer[500], design_name[128], filename[200];
   buffer[0] = 0;
   
   strcpy( design_name, loaded_file );
   design_name[strlen(design_name)-4] = 0;
   sprintf( filename, "%s-%s.bmp", design_name, removeNewlineinString( map_label ) );   
   sprintf( buffer, "map( \"m%d\", \"%s\", \"%s\"", map_number, removeNewlineinString( map_label ), filename );
   PrintXMLText( fp, buffer );
   fprintf( fp, ", \"%s\"", map_title ? removeNewlineinString( map_title ) : "" );
   fprintf( fp, ", \"%s\" )\n", map_description ? removeNewlineinString( map_description ) : "" );   
 
   IndentNewXMLBlock( fp );
   if( graph->HypergraphExists() ) {  // save path spec only if paths exist
      graph->SaveDXL( fp );
   }
   if( components->Size() != 0 ) {
      for( components->First(); !components->IsDone(); components->Next() ) {
    	 components->CurrentItem()->SaveGraphDXL( fp );
      }
   }
   IndentEndXMLBlock( fp ); 
}

void Map::OutputPerformanceData( std::ofstream &pf )
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   pf << "\nMap\t\"" << map_label << "\"\n";

   pf << "\nStart Points\n\n"; // output data for start points
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == START )
	 edge_pool->CurrentItem()->OutputPerformanceData( pf );
   }

   pf << "\nOr Forks\n\n"; // output data for or fork branches
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == OR_FORK )
	 edge_pool->CurrentItem()->OutputPerformanceData( pf );
   }

   pf << "\n\nDynamic Stubs\n"; // output data for plugin choices
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB )
	 edge_pool->CurrentItem()->OutputPerformanceData( pf );
   }

   pf << "\nLoops\n\n"; // output data for loop counts
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == LOOP )
	 edge_pool->CurrentItem()->OutputPerformanceData( pf );
   }

   pf << "\nResponsibilities\n"; // output data for responsibilities
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == RESPONSIBILITY_REF )
	 edge_pool->CurrentItem()->OutputPerformanceData( pf );
   }
}

Hyperedge * Map::FindElement( edge_type type, const char *name )
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == type ){
	 if( strequal( edge_pool->CurrentItem()->HyperedgeName(), name ) )
	    return( edge_pool->CurrentItem() );
      }
   }

   return( NULL );
}

bool Map::HasComponents()
{
   Component *parent_component;

   if( components->Size() == 0 )
      return FALSE;

   for( components->First(); !components->IsDone(); components->Next() ) {
      if( (parent_component = components->CurrentItem()->ReferencedComponent()) != NULL )
	 parent_component->SetVisited();
   }

   return TRUE;
}

bool Map::HasResponsibilities()
{
   Responsibility *parent_responsibility;

   if( responsibilities->Size() == 0 ) return FALSE;

   for( responsibilities->First(); !responsibilities->IsDone(); responsibilities->Next() ) {
      if( (parent_responsibility = responsibilities->CurrentItem()->ParentResponsibility()) != NULL )
	 parent_responsibility->SetVisited();
   }

   return TRUE;
}

bool Map::SingleProcessorResponsibility( Responsibility *responsibility )
{ // return flag if responsibility is referenced in map multiple times with different hardware processor bindings
   int processor_id, previous_processor;
   bool processor_found = FALSE;
   Component *enclosing_component;

   for( responsibilities->First(); !responsibilities->IsDone(); responsibilities->Next() ) {
      if( responsibilities->CurrentItem()->ParentResponsibility() == responsibility ) {
	 if((enclosing_component = responsibilities->CurrentItem()->GetFigure()->BoundComponent()) != NULL ) {
	    processor_id = enclosing_component->ProcessorId();
	    if( processor_found ) {
	       if( previous_processor != processor_id )
		  return FALSE;
	    }
	    previous_processor = processor_id;       
	    processor_found = TRUE;
	 }  
      }
   }

   return TRUE;
}

bool Map::HasGoals()
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == GOAL_TAG ) {
	 if( ((GoalTag *)edge_pool->CurrentItem())->FormsGoal() )
	    return TRUE;
      }
   }

   return FALSE;
}

void Map::SaveGoals( FILE *fp )
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   GoalList::Instance()->ResetGenerationState();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == GOAL_TAG )
	 ((GoalTag *)edge_pool->CurrentItem())->SaveGoal( fp );
   }
}

int Map::DeterminePSSectionCount()
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();
   int section_count = 0;
   
   // reset include flags for this map
   include_map_description = FALSE;   
   include_responsibilities = FALSE;
   include_component_descriptions = FALSE;
   include_conditions = FALSE;
   include_goals = FALSE;
   include_stubs = FALSE;
   include_parent_listing = FALSE;
   
   if( PrintManager::include_map_desc ) { // output map description, if it exists
      if( PrintManager::TextNonempty( map_description ) ) {
	 include_map_description = TRUE;
	 section_count++;
      }
   }

   if( PrintManager::include_responsibilities ) { // create lists of responsibilities, if they exist
      if( responsibilities->Size() > 0 ) {
	 include_responsibilities = TRUE;
	 section_count++;
      }	 
   }

   if( PrintManager::include_path_elements ) { // create list of map element condition lists and descriptions, if necessary
      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 if( edge_pool->CurrentItem()->HasTextualAnnotations() ) {
	    include_conditions = TRUE;
	    section_count++;
	    break;
	 }
      }
   }   

   if( PrintManager::include_goals ) { // create list of goal specifications, if necessary     
      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 if( edge_pool->CurrentItem()->EdgeType() == GOAL_TAG ) {
	    if( ((GoalTag *)edge_pool->CurrentItem())->FormsGoal() ) {
	       include_goals = TRUE;
	       section_count++;
	       break;
	    }
	 }
      }
   }

   if( PrintManager::include_stubs ) { // create list of stub descriptions and plugin bindings
      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 if( edge_pool->CurrentItem()->EdgeType() == STUB ) {
	    include_stubs = TRUE;
	    section_count++;
	    break;
	 }
      }
   }

   if( PrintManager::include_component_desc ) { // create list of component descriptions, if necessary
      for( components->First(); !components->IsDone(); components->Next() ) {
	 if( components->CurrentItem()->ReferencedComponent() != NULL ) {
	    if( PrintManager::TextNonempty( components->CurrentItem()->ReferencedComponent()->Description() ) ) {
	       include_component_descriptions = TRUE;
	       section_count++;
	       break;
	    }
	 }
      }
   }
   
   if( PrintManager::include_parent_listing ) {
      if( mtype == PLUGIN ) {
	 include_parent_listing = TRUE;
	 section_count++;
      }
   }
   
   return( section_count );
}

void Map::GeneratePostscriptDescription( FILE *ps_file )
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();
   edge_type type;
   char min_label[LABEL_SIZE+1];
   ResponsibilityReference *min_resp_ref, *current_resp_ref;
   Stub *current_stub;
   
   map_generated = TRUE;

   if( include_map_description ) {
      PrintManager::PrintHeading( "Map Description" );
      fprintf( ps_file, "[/Title(Map Description) /OUT pdfmark\n" );
      fprintf( ps_file, "%d rom\n", PrintManager::text_font_size );
      PrintManager::PrintDescription( map_description, 36 );
   }

   if( include_parent_listing ) {
      PrintManager::PrintHeading( "Parent Map Listing" );
      fprintf( ps_file, "[/Title(Parent Map Listing) /OUT pdfmark\n" );
      for( parent_stubs->First(); !parent_stubs->IsDone(); parent_stubs->Next() ) {
	 current_stub = parent_stubs->CurrentItem();
	 fprintf( ps_file, "%d rom (%s Map -  ) S %d bol (%s ) S \n", PrintManager::text_font_size,
		  ( current_stub->ParentMap()->IsRootMap() ? "Root" : "Plugin" ), PrintManager::text_font_size,
		  PrintManager::PrintPostScriptText( current_stub->ParentMap()->MapLabel() ));
	 if( PrintManager::ReportIncludesMap( current_stub->ParentMap() )) {
	    fprintf( ps_file, "[/Dest /%s%s /Rect [72 tm 3 sub cpp tm 10 add]\n", ( current_stub->ParentMap()->IsRootMap() ? "Root" : "Plugin" ),
		     PrintManager::PrintPostScriptText( current_stub->ParentMap()->MapLabel(), FALSE ) );
	    fprintf( ps_file, "/Border [0 0 0] /Color [1 0 0] /InvisibleRect /I /Subtype /Link /ANN pdfmark\n" );
	 }
	 fprintf( ps_file, "%d rom (  Stub -  ) S %d bol(%s ) P\n", PrintManager::text_font_size, PrintManager::text_font_size,
		  PrintManager::PrintPostScriptText( current_stub->HyperedgeName() ));
      }
   }
   
   if( include_responsibilities ) {

      ResponsibilityManager::Instance()->ResetGenerate();
      
      PrintManager::PrintHeading( "Responsibilities" );
      fprintf( ps_file, "[/Title(Responsibilities) /OUT pdfmark\n" );

      do {
	 min_resp_ref = NULL;
	 strcpy( min_label, "zzzzzzzz" );
	 for( responsibilities->First(); !responsibilities->IsDone(); responsibilities->Next() ) {
	    current_resp_ref = responsibilities->CurrentItem();
	    if( !current_resp_ref->ParentResponsibility()->Visited() ) {
	       if( strcasecmp( current_resp_ref->HyperedgeName(), min_label ) < 0 ) {
		  min_resp_ref = current_resp_ref;
		  strcpy( min_label, min_resp_ref->HyperedgeName() );
	       }
	    }
	 }
      
	 if( min_resp_ref != NULL )
	    min_resp_ref->GeneratePostScriptDescription( ps_file );
      } while( min_resp_ref != NULL );
   }

   if( include_conditions ) {
   
      PrintManager::PrintHeading( "Map Elements" );
      fprintf( ps_file, "[/Title(Map Elements) /OUT pdfmark\n" );

      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 if( edge_pool->CurrentItem()->EdgeType() == START )
	    edge_pool->CurrentItem()->GeneratePostScriptDescription( ps_file );
      }

      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 type = edge_pool->CurrentItem()->EdgeType();
	 if( type == TIMER || type == WAIT )
	    edge_pool->CurrentItem()->GeneratePostScriptDescription( ps_file );
      }

      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 if( edge_pool->CurrentItem()->EdgeType() == RESULT )
	    edge_pool->CurrentItem()->GeneratePostScriptDescription( ps_file );
      }

      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 type = edge_pool->CurrentItem()->EdgeType();
	 if( type == OR_FORK || type == OR_JOIN || type == SYNCHRONIZATION || type == TIMESTAMP || type == EMPTY || type == LOOP )
	    edge_pool->CurrentItem()->GeneratePostScriptDescription( ps_file );
      }
   }

   if( include_goals ) {
      
      GoalList::Instance()->ResetGenerationState();
      PrintManager::PrintHeading( "Goals" );
      fprintf( ps_file, "[/Title(Goals) /OUT pdfmark\n" );

      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 if( edge_pool->CurrentItem()->EdgeType() == GOAL_TAG )
	    edge_pool->CurrentItem()->GeneratePostScriptDescription( ps_file );
      }
   }

   if( include_stubs ) {

      PrintManager::PrintHeading( "Stubs" );
      fprintf( ps_file, "[/Title(Stubs) /OUT pdfmark\n" );

      for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
	 if( edge_pool->CurrentItem()->EdgeType() == STUB )
	    edge_pool->CurrentItem()->GeneratePostScriptDescription( ps_file );
      }
   }

   if( include_component_descriptions ) {

      PrintManager::PrintHeading( "Component Structure" );
      fprintf( ps_file, "[/Title(Component Structure) /OUT pdfmark\n" );
      
      for( components->First(); !components->IsDone(); components->Next() )
	 components->CurrentItem()->GeneratePostScriptDescription( ps_file );
   }
}

void Map::SavePluginBindings( FILE *fp )
{
   graph->SavePluginBindings( fp );
}

void Map::SavePoolPlugins( FILE *fp )
{	
   for( components->First(); !components->IsDone(); components->Next() ) {
      if( components->CurrentItem()->ReferencedComponent() )
	 components->CurrentItem()->ReferencedComponent()->SavePoolPlugins( fp );
   }
}

void Map::CreatePaths()
{
   Hyperedge *path_starts[200], *next_edge, *current_edge, *timer;
   int i, j, num_path_starts = 0;
   edge_type type;
   Path *new_path;
   Figure *figure;
   HyperedgeFigure *trigger, *wait;
   Cltn<Node *> *target;
   
   // rebuild all path splines from hypergraph
   // search for all spline starting hyperedges, start, or fork, or join, synchronization, stub, abort, loop, timer, wait
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      type = edge_pool->CurrentItem()->EdgeType();
      if( type == START || type == OR_FORK || type == OR_JOIN || type == SYNCHRONIZATION || type == STUB 
	  || type == ABORT || type == LOOP || type == TIMER || type == WAIT )
	 path_starts[num_path_starts++] = edge_pool->CurrentItem();
   }

   for( i = 0; i < num_path_starts; i++ ) {

      if( path_starts[i]->EdgeType() != ABORT )
	 figure = path_starts[i]->GetFigure();
      
      switch( path_starts[i]->EdgeType() ) {

      case START:
      case OR_JOIN:

	 new_path = new Path( display_manager );
	 new_path->SetPathStart( path_starts[i] );
	 figure->SetPath( new_path );
	 new_path->AddFigure( figure );
	 next_edge = path_starts[i]->FirstOutput();
	 this->BuildPath( new_path, next_edge );
	 break;

      case TIMER:
      case WAIT:

	 new_path = new Path( display_manager );
	 new_path->SetPathStart( path_starts[i] );
	 new_path->AddFigure( figure );
	 next_edge = path_starts[i]->FirstOutput();
	 this->BuildPath( new_path, next_edge );
	 break;

      case OR_FORK:

	 target = path_starts[i]->TargetSet();
	 for( target->First(); !target->IsDone(); target->Next() ) {
	    new_path = new Path( display_manager );
	    new_path->SetPathStart( path_starts[i] );
	    new_path->AddFigure( new OrNullFigure( (OrFigure *)figure, new_path, FALSE ) );
	    next_edge = target->CurrentItem()->NextEdge();
	    this->BuildPath( new_path, next_edge );
	 }
	 break;

      case SYNCHRONIZATION:

	 target = path_starts[i]->TargetSet();
	 for( target->First(); !target->IsDone(); target->Next() ) {
	    new_path = new Path( display_manager );
	    new_path->SetPathStart( path_starts[i] );
	    new_path->AddFigure( new SynchNullFigure( (SynchronizationFigure *)figure, new_path, OUTPUT, FALSE ) );
	    next_edge = target->CurrentItem()->NextEdge();
	    this->BuildPath( new_path, next_edge );
	 }
	 break;

      case STUB:

	 target = path_starts[i]->TargetSet();
	 for( target->First(); !target->IsDone(); target->Next() ) {
	    new_path = new Path( display_manager );
	    new_path->SetPathStart( path_starts[i] );
	    new_path->AddFigure( figure );
	    ((StubFigure *)figure)->AddDependentPath( new_path );
	    next_edge = target->CurrentItem()->NextEdge();
	    this->BuildPath( new_path, next_edge );
	 }
	 break;

      case ABORT:

	 new_path = new Path( display_manager );
	 timer = path_starts[i]->FirstInput();
	 new_path->SetPathStart( timer );
	 new_path->AddFigure( timer->GetFigure() );
	 next_edge = path_starts[i]->FirstOutput();
	 this->BuildPath( new_path, next_edge );
	 break;

      case LOOP:

	 for( j = 1; j <= 2; j++ ) {
	    new_path = new Path( display_manager );
	    new_path->SetPathStart( path_starts[i] );
	    new_path->AddFigure( new LoopNullFigure( (LoopFigure *)figure, new_path, ((j == 1) ? MAIN_OUT : LOOP_OUT ) ) );
	    next_edge = path_starts[i]->TargetSet()->Get( j )->NextEdge();
	    this->BuildPath( new_path, next_edge );
	 }
	 break;
      }     
   }

   // reorder input/output paths at forks and joins
   for( figures->First(); !figures->IsDone(); figures->Next() )
      figures->CurrentItem()->OrderPaths();

   // rebuild dependent figure relationships in asynchronous triggering connections
   // cause input and output paths of stubs to be labelled

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      current_edge = edge_pool->CurrentItem();
      if( current_edge->EdgeType() == CONNECTION ) {
	 trigger = current_edge->FirstInput()->GetFigure();
	 wait = current_edge->FirstOutput()->GetFigure();
	 wait->DependentFigure( trigger );
	 trigger->DependentFigure( wait );
      }
      else if( current_edge->EdgeType() == STUB ) {
	 ((Stub *)current_edge)->ShowInputPaths();
	 ((Stub *)current_edge)->ShowOutputPaths();
      }
   }
}

void Map::BuildPath( Path *new_path, Hyperedge *edge )
{
   Hyperedge *previous_edge, *current_edge = edge;
   Figure *current_figure = edge->GetFigure();
   edge_type type;
   
   do {

      current_figure->SetPath( new_path );
      new_path->AddFigure( current_figure );
      previous_edge = current_edge;
      current_edge = current_edge->PrimaryTarget()->NextEdge();
      current_figure = current_edge->GetFigure();
      type = current_edge->EdgeType();
      
   } while( (type != RESULT) && (type != OR_FORK) && (type != OR_JOIN) && (type != STUB) && (type != SYNCHRONIZATION) 
	    && (type != LOOP) && (type != TIMER) && (type != WAIT) );

   switch( type ) {

   case RESULT:
   case OR_FORK:
   case TIMER:
   case WAIT:

      current_figure->SetPath( new_path );
      new_path->AddFigure( current_figure ); 
      break;

   case STUB:

      ((StubFigure *)current_figure)->AddDependentPath( new_path );
      new_path->AddFigure( current_figure );
      break;

   case OR_JOIN:

      new_path->AddFigure( new OrNullFigure( (OrFigure *)current_figure, new_path, FALSE ) );
      break;

   case SYNCHRONIZATION:

      new_path->AddFigure( new SynchNullFigure( (SynchronizationFigure *)current_figure, new_path, INPUT, FALSE ) );
      break;

   case LOOP:

      new_path->AddFigure( new LoopNullFigure( (LoopFigure *)current_figure, new_path, 
					       ((((Loop *)current_edge)->LoopInput( previous_edge ) == MAIN_IN) ? MAIN_IN : LOOP_IN ) ) );
      break;      
   }

   new_path->SetPathEnd( current_edge );  
}

Map * Map::Copy( const char *new_label )
{
   // create temporary file
   FILE *fp = fopen( "/tmp/temp-file.ucm", "w" ); 

   for( responsibilities->First(); !responsibilities->IsDone(); responsibilities->Next() )
      responsibilities->CurrentItem()->ParentResponsibility()->ResetLoadIdentifier();

   this->SaveXML( fp );
   fclose( fp );

   fp = fopen( "/tmp/temp-file.ucm", "r" );
   Map *new_map = XmlManager::LoadSingleMap( fp, new_label );

   unlink( "/tmp/temp-file.ucm" );
   new_map->MapLabel( new_label );
   
   return( new_map );   
}

void Map::ListSubmaps( Cltn<Map *> *submap_list )
{
   Cltn<Map *> *map_pool = display_manager->Maps();
   
   for( map_pool->First(); !map_pool->IsDone(); map_pool->Next() )
      map_pool->CurrentItem()->ResetGenerated();

   this->DetermineSubmaps( submap_list );
}

void  Map::DetermineSubmaps( Cltn<Map *> *submap_list )
{
   Stub *stub;
   Cltn<Map *> *sublist;
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   if( map_generated ) return; // avoid including map and its submaps twice

   submap_list->Add( this ); // add this map to the list
   map_generated = TRUE;

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) { // search for all stubs and 
      if( edge_pool->CurrentItem()->EdgeType() == STUB ) {              // their associated pluginsin map
	 stub = (Stub *)(edge_pool->CurrentItem());
	 if( stub->HasSubmap() ) {
	    sublist = stub->Submaps();
	    for( sublist->First(); !sublist->IsDone(); sublist->Next() ) 
	       sublist->CurrentItem()->DetermineSubmaps( submap_list );
	 }
      }
   }
}

bool Map::HasBoundParents( Hyperedge *endpoint )
{
   for( parent_stubs->First(); !parent_stubs->IsDone(); parent_stubs->Next() ) {
      if( parent_stubs->CurrentItem()->BoundToEndpoint( endpoint, this ) )
	 return TRUE;
   }

   return FALSE;
}

Label * Map::FindLabel( int label_id )
{
   for( labels->First(); !labels->IsDone(); labels->Next() ) {
      if( labels->CurrentItem()->LoadIdentifier() == label_id )
	 return( labels->CurrentItem() );
   }
   return( NULL );
}

ComponentReference *Map::FindComponent( int component_id )
{
   for( components->First(); !components->IsDone(); components->Next() ) {
      if( components->CurrentItem()->LoadIdentifier() == component_id )
	 return( components->CurrentItem() );
   }
   AbortProgram( "Component Reference not found." );
   return( NULL ); // DA: Added August 2004   
}

void Map::DetermineMapBoundaries( float& top, float& bottom, float& left, float& right )
{
   float min_y = 2.0, max_y = 0.0, min_x = 2.0, max_x = 0.0, lb, rb, tb, bb;
   // determine boundaries of map by scanning positions of figures and geometries of components

   // check if map is empty, if so return default screen size
   if( (figures->Size() == 0) && (components->Size() == 0) ) {
      top = 0.0;
      bottom = 1.0-MAP_BORDER;
      left = 0.0;
      right = 1.0-MAP_BORDER;
      return;
   }
   
   for( figures->First(); !figures->IsDone(); figures->Next() ) {
      figures->CurrentItem()->DetermineBoundingBox( lb, rb, tb, bb );
      if( lb < min_x ) min_x = lb;
      if( rb > max_x ) max_x = rb;
      if( tb < min_y ) min_y = tb;
      if( bb > max_y ) max_y = bb;
   }

   for( components->First(); !components->IsDone(); components->Next() ) {
      if( components->CurrentItem()->GetParent() == NULL ) {
	 components->CurrentItem()->GetBoundaries( tb, bb, lb, rb );
	 if( lb < min_x ) min_x = lb;
	 if( rb > max_x ) max_x = rb;
	 if( tb < min_y ) min_y = tb;
	 if( bb > max_y ) max_y = bb;
      }
   }

   top = min_y;
   bottom = max_y;
   left = min_x;
   right = max_x;
}

void Map::CalculatePathEndings()
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == RESULT )
	 edge_pool->CurrentItem()->GetFigure()->GetPath()->Interpolate();
   }
}

const char * Map::VerifyUniqueMapName( const char *name )
{
   int extension = 1;
   static char new_name[LABEL_SIZE+1];
   bool duplicate_name;
   Cltn<Map *> *maps = display_manager->Maps();

   strcpy( new_name, name );
   
   forever {

      duplicate_name = FALSE;
      
      for( maps->First(); !maps->IsDone(); maps->Next() ) {
	 if( strequal( maps->CurrentItem()->MapLabel(), new_name ) ) {
	    duplicate_name = TRUE;
	    break;
	 }
      }

      if( duplicate_name )
	 sprintf( new_name, "%s-%d", name, extension++ );
      else
	 return( new_name );
   }
}

void Map::VerifyAnnotations()
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() )
      edge_pool->CurrentItem()->ResetHighlight();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() )
      edge_pool->CurrentItem()->VerifyAnnotations();
}

void Map::DeactivateHighlighting()
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() )
      edge_pool->CurrentItem()->ResetHighlight();
}

void Map::HighlightParentStubs()
{
   for( parent_stubs->First(); !parent_stubs->IsDone(); parent_stubs->Next() )
      parent_stubs->CurrentItem()->HighlightParentStubs();
}

void Map::HighlightEntryPoint( Empty *empty, Stub *stub )
{
   Start *bound_start;
   Result *bound_endpoint;
   Cltn<Node *> *stub_io;
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();
   Hyperedge *current_edge;
   edge_type etype;
   
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) { // reset highlights on start points in plugin
      current_edge = edge_pool->CurrentItem();
      etype = current_edge->EdgeType();
      if( etype == START )
	 ((Start *)current_edge)->ResetHighlight();
      else if( etype == RESULT )
	 ((Result *)current_edge)->Unhighlight();
   }

   edge_pool = stub->ParentMap()->MapHypergraph()->Hyperedges(); // reset highlights on input paths to stub in parent map
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB ) {
	 stub_io = edge_pool->CurrentItem()->SourceSet();
	 for( stub_io->First(); !stub_io->IsDone(); stub_io->Next() ) 
	    ((Empty *)(stub_io->CurrentItem()->PreviousEdge()))->Unhighlight();
	 stub_io = edge_pool->CurrentItem()->TargetSet();
	 for( stub_io->First(); !stub_io->IsDone(); stub_io->Next() ) 
	    ((Empty *)(stub_io->CurrentItem()->NextEdge()))->Unhighlight();
      }
   }

   if( empty ) {
      if( empty->PrimaryTarget()->NextEdge()->EdgeType() == STUB ) {
	 if( (bound_start = stub->SubmapPathStart( empty, this )) != NULL ) {
	    bound_start->HighlightEntry();
	    empty->Highlight();
	 }
      } else { // empty is an output of a stub
	 if( (bound_endpoint = stub->SubmapPathEnd( empty, this )) != NULL ) {
	    bound_endpoint->Highlight();
	    empty->Highlight();
	 }
      }
   }
}

void Map::HighlightSubmapExitPoint( Result *result )
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();
   
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) { // reset highlights on start points in plugin
      if( edge_pool->CurrentItem()->EdgeType() == RESULT )
	((Result *)(edge_pool->CurrentItem()))->Unhighlight();
   }

   result->Highlight();
}

void Map::HighlightSubmapEntryPoint( Start *start )
{
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();
   
   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) { // reset highlights on start points in plugin
      if( edge_pool->CurrentItem()->EdgeType() == START )
	((Start *)(edge_pool->CurrentItem()))->ResetHighlight();
   }

   start->HighlightEntry();
}

void Map::HighlightExitPoint( Empty *empty )
{
   Cltn<Node *> *stub_target;
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB ) {
	 stub_target = edge_pool->CurrentItem()->TargetSet();
	 for( stub_target->First(); !stub_target->IsDone(); stub_target->Next() ) 
	    ((Empty *)(stub_target->CurrentItem()->NextEdge()))->Unhighlight();
      }
   }

   empty->Highlight();
}

void Map::HighlightEntryPoint( Empty *empty )
{
   Cltn<Node *> *stub_source;
   Cltn<Hyperedge *> *edge_pool = graph->Hyperedges();

   for( edge_pool->First(); !edge_pool->IsDone(); edge_pool->Next() ) {
      if( edge_pool->CurrentItem()->EdgeType() == STUB ) {
	 stub_source = edge_pool->CurrentItem()->SourceSet();
	 for( stub_source->First(); !stub_source->IsDone(); stub_source->Next() ) 
	    ((Empty *)(stub_source->CurrentItem()->PreviousEdge()))->Unhighlight();
      }
   }

   empty->Highlight();
}

void Map::ReplaceComponent( Component *old_component, Component *new_component )
{
   for( components->First(); !components->IsDone(); components->Next() ) {
      if( components->CurrentItem()->ReferencedComponent() == old_component )
	 components->CurrentItem()->ReferencedComponent( new_component );
   }
}

void Map::ReplaceResponsibility( Responsibility *old_resp, Responsibility *new_resp )
{
   for( responsibilities->First(); !responsibilities->IsDone(); responsibilities->Next() ) {
      if( responsibilities->CurrentItem()->ParentResponsibility() == old_resp )
	 responsibilities->CurrentItem()->ParentResponsibility( new_resp );
   }
}
