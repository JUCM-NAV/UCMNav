/*********************************************************************
 *
 * File:			xml_mgr.h
 * Author:			Andrew Miga
 * Created:			October 1998
 *
 * Modification history:
 *
 *******************************************************************/

#ifndef XML_MANAGER_H
#define XML_MANAGER_H

#include "collection.h"
#include "xmlparse.h"
#include <stdio.h>

#define NUMBER_START_HANDLERS 90
#define NUMBER_END_HANDLERS 14
#define IMPORT_MAPS 1

typedef enum { CONNECTIONS, COMPONENT_REFERENCE } ref_state;

class Timestamp;
class Responsibility;
class Label;
class Map;
class Start;
class WaitingPlace;
class Result;
class Stub;
class GoalTag;
class Goal;
class PluginBinding;
class Empty;
class OrFork;
class Component;
class ComponentReference;
class UcmSet;
class Scenario;
class ScenarioGroup;
class StartHandlerTreeElement;
class EndHandlerTreeElement;
class Action;

class StartHandlerTreeElement {
public:

   StartHandlerTreeElement() { key = 0; handler = NULL; left = NULL; right = NULL; }

   unsigned long key;
   void (*handler)( const char ** );
   StartHandlerTreeElement *left, *right;
};

class EndHandlerTreeElement {
public:

   EndHandlerTreeElement() { key = 0; handler = NULL; left = NULL; right = NULL; }

   unsigned long key;
   void (*handler)();
   EndHandlerTreeElement *left, *right;
};

class StubEntryExitList {
public:

   StubEntryExitList( Stub *s, int id ) { stub = s; stub_id = id; }

   bool operator==( StubEntryExitList &s )
   {
      if( (s.stub == stub) && (s.stub_id == stub_id) )
	 return 1;
      else
	 return 0;
   }

   void AddStubEntry( int hid, int eid ) { entry_list.Add( BoundaryListItem( hid, eid ) ); }
   void AddStubExit( int hid, int eid )  { exit_list.Add( BoundaryListItem( hid, eid ) ); }
   int FindHyperedgeId( int eid, end_type etype );

   Stub *stub;
   int stub_id;
   Cltn<BoundaryListItem> entry_list, exit_list;
};

typedef void (*StartHandler)( const char **attributes );
typedef void (*EndHandler)();

typedef struct {
   int id;
   char name[30];
   float fx;
   float fy;
   char description[8096];
   Label *label;
} hyper_data;

class XmlManager {

public:

   static void SaveFileXML( FILE *file, Cltn<Map *> *sublist = NULL, const char *subdesign_name = NULL );
   static void SaveCSMXML( FILE *file, Cltn<Map *> *sublist = NULL, const char *subdesign_name = NULL );
   static void SaveFileGraphDXL( FILE *file);  // Added by Bo Jiang, for DXL exporting.  July, 2004
   static void AutosaveBackupFile();
   static bool LoadFile( FILE *file, bool import = FALSE );
   static void RevertFromAutosave();
   static Map * LoadSingleMap( FILE *file, const char *new_label );
   static Map * MainImportMap() { return( main_import_map ); }
   static bool MapExportable( Map *map );

   static void StartElementHandler( void *data, const char *name, const char **attributes );
   static void EndElementHandler( void *data, const char *name );
   static void TestStartElementHandler( void *data, const char *name, const char **attributes );
   static void TestEndElementHandler( void *data, const char *name );

   static void AutosaveInterval( int new_interval ) { autosave_interval = new_interval; }
   static bool ImportSubstitution() { return( import_substitution ); }
   static void RemoveWhitespace( char *processed, char *unprocessed );

private:

   // start element handlers
   static void UCMDesignElement( const char **attributes );
   static void DeviceDirectoryElement( const char **attributes );
   static void DeviceElement( const char **attributes );
   static void DataStoreDirectoryElement( const char **attributes );
   static void DataStoreElement( const char **attributes );
   static void AccessModeElement( const char **attributes );

   static void RootMapsElement( const char **attributes );
   static void PluginMapsElement( const char **attributes );
   static void ModelElement( const char **attributes );

   static void StartResponsibilitySpecElement( const char **attributes );
   static void StartResponsibilityElement( const char **attributes );
   static void DataStoreSpecElement( const char **attributes );
   static void DataStoreAccessElement( const char **attributes );
   static void DynamicRespElement( const char **attributes );
   static void ServiceRequestListElement( const char **attributes );
   static void ServiceRequestElement( const char **attributes );

   static void PathSpecElement( const char **attributes );
   static void PathLabelListElement( const char **attributes );
   static void PathLabelElement( const char **attributes );
   static void StartHypergraphElement( const char **attributes );

   static void HyperedgeElement( const char **attributes );
   static void StartStartElement( const char **attributes );
   static void StartWaitingPlaceElement( const char **attributes );
   static void StartEndPointElement( const char **attributes );
   static void ResponsibilityRefElement( const char **attributes );
   static void JoinElement( const char **attributes );
   static void ForkElement( const char **attributes );
   static void PathBranchingSpecElement( const char **attributes );
   static void BranchingCharacteristicElement( const char **attributes );
   static void SynchronizationElement( const char **attributes );
   static void StartStubElement( const char **attributes );
   static void StartEmptySegmentElement( const char **attributes );
   static void TimestampPointElement( const char **attributes );
   static void GoalTagElement( const char **attributes );
   static void ConnectElement( const char **attributes );
   static void AbortElement( const char **attributes );
   static void LoopElement( const char **attributes );

   static void EnforceBindingsElement( const char **attributes );
   static void PathBindingElement( const char **attributes );

   static void TriggeringEventListElement( const char **attributes );
   static void ResultingEventListElement( const char **attributes );
   static void EventElement( const char **attributes );
   static void PreconditionListElement( const char **attributes );
   static void PostconditionListElement( const char **attributes );
   static void ConditionElement( const char **attributes );

   static void StubEntryListElement( const char **attributes );
   static void StubEntryElement( const char **attributes );
   static void StubExitListElement( const char **attributes );
   static void StubExitElement( const char **attributes );

   static void HyperedgeConnectionElement( const char **attributes );
   static void HyperedgeRefElement( const char **attributes );

   static void StructureSpecElement( const char **attributes );
   static void ComponentElement( const char **attributes );
   static void ComponentReferenceElement( const char **attributes );
   static void ComponentSpecificationElement( const char **attributes );
   static void ResponsibilityListElement( const char **attributes );
   static void OtherHyperedgeListElement( const char **attributes );
   static void RegularElement( const char **attributes );
   static void PoolElement( const char **attributes );

   static void StartPluginBindingsElement( const char **attributes );
   static void PluginBindingElement( const char **attributes );
   static void InConnectionListElement( const char **attributes );
   static void InConnectionElement( const char **attributes );
   static void OutConnectionListElement( const char **attributes );
   static void OutConnectionElement( const char **attributes );
   static void PluginPoolElement( const char **attributes );
   static void StartResponseTimeRequirementsElement( const char **attributes );
   static void ResponseTimeReqElement( const char **attributes );

   static void AgentAnnotationsElement( const char **attributes );
   static void StartGoalListElement( const char **attributes );
   static void StartGoalElement( const char **attributes );

   static void UcmSetsElement( const char **attributes );
   static void UcmSetElement( const char **attributes );
   static void UcmSetItemElement( const char **attributes );

   static void PathVariableListElement( const char **attributes );
   static void BooleanVariableElement( const char **attributes );

   static void ScenarioListElement( const char **attributes );
   static void ScenarioGroupElement( const char **attributes );
   static void ScenarioElement( const char **attributes );
   static void ScenarioStartElement( const char **attributes );
   static void VariableInitElement( const char **attributes );
   static void VariableOperationListElement( const char **attributes );
   static void VariableOperationElement( const char **attributes );
   static void ScenarioPostconditionElement( const char **attributes );

   // end element handlers
   static void EndDesign();
   static void EndResponsibilitySpecElement();
   static void EndHypergraphElement();
   static void EndResponsibilityElement();
   static void EndStartElement();
   static void EndWaitingPlaceElement();
   static void EndEndPointElement();
   static void EndStubElement();
   static void EndEmptySegmentElement();
   static void EndPluginBindingsElement();
   static void EndResponseTimeRequirementsElement();
   static void EndGoalListElement();
   static void EndGoalElement();

   static Map * FindMap( int map_id );
   static Timestamp * FindTimestampPoint( int ts_id );
   static GoalTag * FindGoalTag( int gt_id );
   static void CreateCondition( const char **attributes );
   static const char *AddNewlines( const char *text, int bid );
   static bool PromptImportMode();
   static void AddOffsetToBoolean( const char *condition, char *newcondition ); // Apr2005 gM: required for map import to avoid conflict with constant IDs

   static void ConstructHandlerTree();
   static void AddStartHandler( const char *element_name, void (*handler)( const char ** ) );
   static StartHandler FindStartHandler( const char *element_name );

   static void AddEndHandler( const char *element_name, void (*handler)() );
   static EndHandler FindEndHandler( const char *element_name );

   static StartHandlerTreeElement start_handler_tree[NUMBER_START_HANDLERS];
   static StartHandlerTreeElement *start_handler_root;
   static int start_handler_tree_size;
   static EndHandlerTreeElement end_handler_tree[NUMBER_END_HANDLERS];
   static EndHandlerTreeElement *end_handler_root;
   static int end_handler_tree_size;
   static bool tree_constructed;
   static bool root_maps;
   static int version_number;
   static Map *current_map;
   static Map *submap;
   static Map *parent_map;

   static Responsibility *responsibility;
   static Start *start;
   static Stub *stub;
   static WaitingPlace *waiting_place;
   static Result *result;
   static Empty *empty;
   static OrFork *current_fork;
   static hyper_data hyperedge_data;
   static Component *component;
   static ComponentReference *component_reference;
   static Goal *goal;

   static etype event_type;
   static char *event_data[4][30];
   static char text_buffer[2][8096];
   static int event_number[4];
   static char direction[10];

   static Cltn<StubEntryExitList *> stub_boundaries;
   static Cltn<Component *> plugin_pools;
   static Cltn<Timestamp *> timestamps;
   static Cltn<GoalTag *> goal_tags;
   static Cltn<Map *> import_map_list;
   static Cltn<Map *> *export_list;
   static StubEntryExitList *current_stub_list;
   static PluginBinding *plugin_binding;
   static Map *main_import_map;
   static UcmSet *current_ucm_set;
   static Scenario *current_scenario;
   static ScenarioGroup *current_scenario_group;

   static const char *single_map_label;
   static bool attributes_valid;

   static int current_source_id;
   static ref_state reference_state;
   static float scaling_factor;
   static bool import_maps, import_substitution, mode_selected, responsibility_list;
   static bool copy_map;			// Jun2005 gM: required for "image of plugin" to avoid conflict with constant IDs
   static int resp_load_offset, comp_load_offset;
   // Apr2005 gM: required for map import to avoid conflict with constant IDs
   static int comp_ref_load_offset, device_load_offset, hyperedge_load_offset, model_load_offset, datastoredirectory_load_offset, boolean_load_offset; 
   static int autosave_interval;

};

#endif
