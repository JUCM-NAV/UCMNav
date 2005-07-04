/***********************************************************
 *
 * File:			stub.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef STUB_H
#define STUB_H

#include "waiting_place.h"
#include "condition.h"
#include "service.h"
#include <fstream>

typedef enum { STATIC, DYNAMIC } stub_type;

class Map;
class StubFigure;
class Start;
class Result;

class StubBinding { // data class used by Stub to store bindings between input/output points of stubs and path endpoints in submap

public:

   StubBinding( Node *node, Hyperedge *endpoint = NULL )
   { boundary_node = node; path_endpoint = endpoint; }

   Node *boundary_node;  // input or output node of stub
   Hyperedge *path_endpoint; // path start or end in stub
   int path_identifier; // integer identifier of stub inputs, outputs

};

class PluginBinding {  // data class used by Stub which stores the plug-in bindings for a single plug-in

public:

   PluginBinding(  Stub *stub, Map *pi, const char *lcond = NULL, float prob = 1.0 );
   ~PluginBinding();

   void AddNode( Node *new_node, end_type etype ); // adds a node to the bindings list
   void RemoveNode( Node *node, end_type etype );  // removes a node from the bindings list
   void AddBinding( Hyperedge *endpoint, Hyperedge *boundary_edge, end_type etype ); // performs a binding during XML file loading
   Hyperedge * SubmapPathStart( Node *input_node );
   void GeneratePostScript( FILE *ps_file );

   char * LogicalCondition();
   void LogicalCondition( const char *new_condition );
   const char * ReferencedCondition() { return( logical_condition ); }
   float Probability() { return( probability ); }
   void Probability( float new_probability) { probability = new_probability; }

   Map *plugin;  // the plug-in map for which the bindings are made
   Cltn<StubBinding *> entry_bindings, exit_bindings;  // list of bindings for input, output points
   char *logical_condition; // text string for logical plugin selection condition
   float probability;
};

class ContinuityBinding {
public:

   ContinuityBinding( Node *in, Node *on ) { input_node = in; output_node = on; }

   Node *input_node, *output_node;
};

class Stub : public Hyperedge {

   friend class StubFigure;

public:

   Stub();  // regular constructor
   Stub( int stub_id, const char *name, float x, float y, const char *desc, stub_type st, bool shared_stub, const char *sp ); // file loading constructor
   ~Stub();

   virtual edge_type EdgeType() { return( STUB ); }  // returns identifier for subclass type
   virtual void EdgeSelected();  // installs textual conditions in main window list
   virtual void SaveXMLDetails( FILE *fp ); // implements polymorphic Save method for stubs, in XML format
   virtual void SaveDXLDetails( FILE *fp );  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ); // implements polymorphic Save method for stubs, in XML format
   virtual void OutputPerformanceData( std::ofstream &pf );
   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual void DeleteGenerationData();
   virtual void ResetGenerationState();
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   void SaveBindings( FILE *fp ); // saves plugin bindings in XML form
   void SaveContinuityBindings( FILE *fp ); // saves path continuity bindings
   virtual const char * HyperedgeName() { return( stub_label ); }  // returns user given textual identifier for stub
   virtual bool HasName() { return( TRUE ); }
   virtual int Search( search_direction dir ) { return( Hyperedge::Search( BIDIRECTIONAL ) ); }  // polymorphic method used to search for path joinings
   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir );
   virtual Path * InputPath();
   virtual bool DeleteHyperedge();
   virtual int DoubleClickAction();

   virtual void AttachSource( Node *new_node ); // adds new source node to plug-in bindings list
   virtual void AttachTarget( Node *new_node ); // adds new target node to plug-in bindings list
   virtual void DetachSource( Node *node ); // removes source node from plug-in bindings list
   virtual void DetachTarget( Node *node ); // removes target node from plug-in bindings list

   bool DeleteStub( flag execute ); // deletes the stub from the path
   bool ExpandStub( flag execute ); // allows user to create a new submap for stub
   bool ViewSubmap( flag execute ); // displays one of the stub's submaps, displays selection dialog if necessary
   bool EditStubLabel( flag execute ); // allows user to edit the stub label
   bool InstallExistingPlugin( flag execute ); // installs a previously created plug-in as a submap of the stub
   bool RemoveSubmap( flag execute ); // allows user to remove one of the stub's submaps, displays selection dialog if necessary
   bool BindPlugin( flag execute ); // allows user to specify stub bindings through a dialogs, displays selection dialog if necessary
   bool CopyExistingPlugin( flag execute ); // installs a copy of a previously created plug-in as a submap of the stub
   bool ImportPluginFile( flag execute ); // imports a plugin and its submaps from a file
   bool TransferRootPlugin( flag execute ); // transfers a root map from the root map list and its submaps to be a plugin

   bool HasSubmap() { return( submaps.Size() != 0 ); }  // returns flag if any submaps exist
   bool HasViewableSubmap();
   bool HasMultipleSubmaps() { return( submaps.Size() >= 2 ); }
   Cltn<Map *> * Submaps() { return( &submaps ); } // returns list of submap references
   Cltn<Map *> * ScenarioSubmaps() { return( &scenario_submaps ); }
   bool HasScenarioSubmap() { return( scenario_submaps.Size() != 0 ); }  // returns flag if any submaps exist
   bool ContainsSubmap( Map *map );
   bool BoundToEndpoint( Hyperedge *endpoint, Map *submap );
   bool InputBound( Map *submap, Empty *empty );
   void ViewPlugin( Empty *empty = NULL );  // displays a plug-in map
   Empty * PathContinued( Hyperedge *io_point, Map *submap );
   PluginBinding * SubmapBinding( Map *plugin );

   bool ConnectPath( Hyperedge *trigger_edge, flag execute );
   bool DisconnectPath( Hyperedge *trigger_edge, flag execute );
   bool Perform( transformation trans, execution_flag execute ); // validates or performs the single selection transformation with code trans
   bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute ); // validates or performs the double selection transformation trans

   void Install();  // invokes DisplayManager functions for installing a stub
   void SetName( const char *new_label ); // set method for stub label
   const char * StubName() { return( stub_label ); } // returns user given stub label
   void SetType( stub_type type ) { stype = type; } // set method for stub type( static/dynamic )
   stub_type GetType() { return ( stype ); }  // returns the type of the stub ( static/dynamic )
   void SetSelectedMap( Map *selected_map, Empty *empty = NULL ); // installs plug-in map selected by user in the editor window
   void InstallNewSubmap( Map *new_map );  // installs user selected submap as a plug-in to stub if it is not already one
   void InstallSubmapImage( Map *new_map ); // installs copy of user selected submap as a plug-in to stub
   void InstallRootPlugin( Map *root_map ); // transfers root map to submap list

   void RemovePlugin( Map *map ); // removes a map from the list of plug-ins of the stub
   bool RenamePlugin( flag execute );  // launches the Rename Plugin dialog box
   void InitiatePathBinding( Map *plugin );  // initiates process of binding entry/exit points for a given plug-in
   void UpdateBindingsDisplay(); // updates the display of the current bindings in the bindings dialog
   void BindEntryPoints( int stub_entry, int plugin_entry ); // records binding of stub input point and plug-in start point
   void BindExitPoints( int stub_exit, int plugin_exit ); // records binding of stub output point and plug-in end point
   void BreakBinding( int binding );  // breaks the binding selected by the user
   void RemoveEntryBinding( Start *plugin_start, Map *plugin );
   void RemoveExitBinding( Result *plugin_result, Map *plugin );

   Start * SubmapPathStart( Empty *empty ); // returns the start point in the submap bound to the input point of the stub for static stubs
   Start * SubmapPathStart( Empty *empty, Map *submap ); // returns the start point in the submap bound to the input point of the stub
   Result * SubmapPathEnd( Empty *empty ); // returns the path end point in the submap bound to the output point to the stub for static stubs
   Result * SubmapPathEnd( Empty *empty, Map *submap ); // returns the path end point in the submap bound to the output point to the stub
   Hyperedge * StubExitPath( Result *result, Map *submap ); // returns the output point of the stub corresponding to the result point in the submap

   void AddPrecondition( char *condition ) { condition_manager.AddCondition( PRECONDITION, condition ); }
   void AddPostcondition( char *condition ) { condition_manager.AddCondition( POSTCONDITION, condition ); }
   void AddPluginBinding( PluginBinding *pb );
   void ShowInputPaths();
   void ShowOutputPaths();
   void ShowInputOutputPaths() { ShowInputPaths(); ShowOutputPaths(); }

   virtual void VerifyAnnotations();
   void HighlightParentStubs();
   virtual void ResetHighlight() { annotations_complete = TRUE; }
   bool Highlight() { return( annotations_complete != TRUE ); }
   bool IsShared() { return( shared ); }
   void Shared( bool ss ) { shared = ss; }
   char * SelectionPolicy() { return( selection_policy ); }
   void SelectionPolicy( const char *new_sp );
   void AddContinuityBinding( Node *in, Node *on ) { continuity_bindings.Add( new ContinuityBinding( in, on ) ); }
   void AddContinuityBinding( Hyperedge *stub_input, Hyperedge *stub_output );
   Cltn<ContinuityBinding *> * ContinuityBindings() { return( &continuity_bindings ); }

   Cltn<ServiceRequest *> * ServiceRequests() { return( &service_requests ); }
   void AddServiceRequest( int device_id, const char *amount ); // adds new service request reference to list

   Map * getCurrentSubmap() { return current_submap; }
   Hyperedge * TraverseScenario( );
   void TraversePluginMap( Map *submap );
   Hyperedge * HighlightScenario( );

private:

   bool PathStartValid( Start *start ); // returns whether a start object stored in the bindings list still exists, error checking method
   bool PathEndValid( Result *result ); // returns whether a result object stored in the bindings list still exists, error checking method
   bool IgnoredEndingsExist(); // returns flag signifying if any start/end points in current submap are unbound
   void AddSubmap( Map *new_submap );
   Cltn<StubBinding *> * EntryBindings( Map *plugin );
   Cltn<StubBinding *> * ExitBindings( Map *plugin );
   void ScanPluginMap( Map *submap, scan_type type );

   stub_type stype;  // type of stub, static/dynamic
   bool shared;
   char stub_label[DISPLAYED_LABEL+1];  // user given label for stub
   Cltn<Map *> submaps;  // list of submaps for this stub
   Cltn<ContinuityBinding *> continuity_bindings; // list of bindings between stub inputs and outputs
   ConditionManager condition_manager;  // object which manages the textual conditions
   Cltn<PluginBinding *> plugin_bindings;  // list of plug-in bindings for all submaps
   char *selection_policy;
   PluginBinding *current_binding; // the plug-in binding that is being edited
   int annotations_complete;
   Cltn<PluginBinding *> traversal_bindings; // list of bindings per context of object
   int traversal_context; // count of contexts of this object in traversal
   bool generation_pass;
   Cltn<ServiceRequest *> service_requests; // list of ServiceRequest objects which store access to services
   Cltn<Map *> scenario_submaps;  // list of submaps for a particular scenario
   unsigned int tracing_id; // the current tracing id

   static Start **plugin_starts;  // global storage for stubs for start and end point pointers
   static Result **plugin_results;
   static int start_list_size, result_list_size;
   static int number_starts, number_results; // numbers of start and end points
   Map * current_submap;
};

#endif
