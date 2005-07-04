/***********************************************************
 *
 * File:			map.h
 * Author:			Andrew Miga
 * Created:			May 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef MAP_H
#define MAP_H

#include "collection.h"
#include "defines.h"
#include "definitions.h"
#include <stdio.h>
#include <fstream>

class Hypergraph;
class Hyperedge;
class HyperedgeFigure;
class Path;
class ComponentReference;
class Component;
class ResponsibilityReference;
class Responsibility;
class Label;
class Stub;
class Empty;
class Start;
class Result;
class UcmSet;

class Map {

public:

   Map( const char *label, map_type type = ROOT_MAP );  // constructor, contains label and map type
   Map( int map_id, const char *label, map_type type, const char *title, const char *description );  // constructor, contains label and map type
   ~Map();

   Hypergraph * MapHypergraph() { return( graph ); }  // returns hypergraph object for map
   Cltn<HyperedgeFigure *> * Figures() { return( figures ); }  // returns list of hyperedge figures
   Cltn<Path *> * Paths() { return( paths ); } // returns list of paths

   Cltn<ComponentReference *> * Components() { return( components ); } // returns list of components
   ComponentReference *FindComponent( int component_id ); // returns component reference object with load number component_id
   void AddComponentReference( ComponentReference *new_cr ) { components->Add( new_cr ); } // adds newly created component reference to list

   Cltn<ResponsibilityReference *> * Responsibilities()  { return( responsibilities ); } // returns list of responsibilities

   Cltn<Label *> * Labels() { return( labels ); } // returns list of path labels
   Label *FindLabel( int label_id ); // returns label object with load number label_id
   void RegisterLabel( Label *new_label );
   void PurgeLabel( Label *label );
   Label * NewLabel( Hyperedge *edge );
   int EditLabel( Hyperedge *edge, const char *message );

   void MapHypergraph( Hypergraph *new_graph ) { graph = new_graph; }
   void Figures( Cltn<HyperedgeFigure *> *new_figures ) { figures = new_figures; }
   void Paths( Cltn<Path *> *new_paths ) { paths = new_paths; }
   void Components( Cltn<ComponentReference *> *new_components ) { components = new_components; }
   void Responsibilities( Cltn<ResponsibilityReference *> *new_responsibilities ) { responsibilities = new_responsibilities; }

   char * MapLabel() { return( map_label ); } // access methods for map label, title and description
   void MapLabel( const char *new_label );
   char * MapTitle() { return( map_title ); }
   void MapTitle( const char *new_title );
   char * MapDescription()  { return( map_description ); }
   void MapDescription( const char *new_description );
   Map *Copy( const char *new_label ); // returns a map object which is a copy of itself

   bool IsRootMap() { return( mtype == ROOT_MAP ); } // bool functions which identify type of map
   bool IsPlugIn() { return( mtype == PLUGIN ); }
   bool IsTopLevelMap() { return( parent_stubs->Size() == 0 ); }
   bool MapType( int i ) { return( i == 0 ? mtype == ROOT_MAP : mtype == PLUGIN ); }
   map_type MapType() { return( mtype ); }
   void SetMapType( int type );

   void IncrementCount() { reference_count++; } // increments reference count
   void DecrementCount() { reference_count--; } // decrements reference count
   bool IsNotReferenced() { return( reference_count == 0 ); } // determines whether a map is deletable

   void SaveXML( FILE *fp );  // saves all components of the map, in XML format
   void SaveDXL( FILE *fp );  //added by Bo Jiang for graph info exporting in DXL for DOORS, August, 2004
   void SaveCSMXML( FILE *fp );
   void SavePluginBindings( FILE *fp );  // saves the bindings of all stubs to their plugin maps
   void SavePoolPlugins( FILE *fp );  // saves the bindings of all plugins that are part of pools
   void CreatePaths(); // rebuild all path splines
   void CalculatePathEndings(); // calculate splines for path endings
   void DetermineMapBoundaries( float& top, float& bottom, float& left, float& right );
   void OutputPerformanceData( std::ofstream &pf );
   void GeneratePostscriptDescription( FILE *ps_file );
   int DeterminePSSectionCount();
   void ListSubmaps( Cltn<Map *> *submap_list );
   bool HasComponents();
   bool HasResponsibilities();
   bool SingleProcessorResponsibility( Responsibility *responsibility );
   bool HasGoals();
   void SaveGoals( FILE *fp );

   void VerifyAnnotations(); // verifies existence of performance annotations for certain map elements
   void DeactivateHighlighting(); // removes higlighting for incomplete elements

   void HighlightParentStubs();
   void HighlightEntryPoint( Empty *empty, Stub *stub );
   void HighlightSubmapExitPoint( Result *result );
   void HighlightSubmapEntryPoint( Start *start );
   void HighlightExitPoint( Empty *empty );
   void HighlightEntryPoint( Empty *empty );
   void AddParentStub( Stub *stub ) { parent_stubs->Add( stub ); reference_count++; }
   void RemoveParentStub( Stub *stub );
   Cltn<Stub *> * ParentStubs() { return( parent_stubs ); }
   bool HasBoundParents( Hyperedge *endpoint );
   Hyperedge * FindElement( edge_type type, const char *name );

   void RegisterUCMSet( UcmSet *new_set ) { ucm_sets.Add( new_set ); }
   void DeregisterUCMSet( UcmSet *set ) { ucm_sets.Delete( set ); }

   int GetNumber() { return( map_number ); } // returns integer identifier of map
   int LoadIdentifier() { return( load_number ); }

   void ReplaceComponent( Component *old_component, Component *new_component );
   void ReplaceResponsibility( Responsibility *old_resp, Responsibility *new_resp );

   void SetGenerated() { map_generated = TRUE; }
   void ResetGenerated() { map_generated = FALSE; }
   bool IsGenerated() { return( map_generated ); }

   static void ResetMapCount() { number_maps = 0; }
   static int NumberMaps() { return( number_maps ); } // Apr2005 gM: required for map import to avoid conflict with constant IDs
   static int NumberRootMaps() { return( number_root_maps ); }
   static int NumberPluginMaps() { return( number_plugin_maps ); }

   void setScanningParentStub(Stub * stub) { scanning_parent_stub = stub; }
   Stub * getScanningParentStub( ) { return scanning_parent_stub; }

   //void setScanningParentStub( int stub_id) { scanning_parent_stub_id = stub_id; }
   //int getScanningParentStub( ) { return scanning_parent_stub_id; }


private:

   void BuildPath( Path *new_path, Hyperedge *edge );
   const char * VerifyUniqueMapName( const char *name );
   void DetermineSubmaps( Cltn<Map *> *submap_list );
   char * LabelPrompt( const char *message, const char *default_label );

   map_type mtype; // flag for type of map ( root/plug-in )
   Hypergraph *graph;  // the hypergraph of the map
   Cltn<HyperedgeFigure *> *figures; // list of hyperedge figures
   Cltn<Path *> *paths; // list of paths
   Cltn<ComponentReference *> *components; // list of component references in map
   Cltn<ResponsibilityReference *> *responsibilities;
   Cltn<Label *> *labels; // list of path labels in map
   Cltn<Stub *> *parent_stubs;
   Cltn<UcmSet *> ucm_sets;
   char *map_title, *map_description; // user given map title and description
   char map_label[LABEL_SIZE+1]; // user given map name

   int map_number, load_number, reference_count; // integer identifiers of map
   bool map_generated, include_map_description, include_responsibilities, include_component_descriptions,
   include_conditions, include_goals, include_stubs, include_parent_listing;
   static int number_maps, number_root_maps, number_plugin_maps; // global counter for number of maps
   Stub * scanning_parent_stub;
   //int scanning_parent_stub_id;

};

#endif

