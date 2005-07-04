/***********************************************************
 *
 * File:			display.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "defines.h"
#include "collection.h"
#include "hyperedge_figure.h"

class TransformationManager;
class Hyperedge;
class Presentation;
class Path;
class ResponsibilityReference;
class Responsibility;
class MarkerFigure;
class Synchronization;
class OrFork;
class OrJoin;
class Empty;
class Result;
class Start;
class Stub;
class Timer;
class WaitSynch;
class Loop;
class Label;
class Map;
class UcmSet;
class Scenario;

typedef struct {
   Map *map;
   Stub *stub;
} hierarchy_element;

typedef struct {
   Map *plugin;
   bool parent;
} design_plugin;

class DisplayManager {

public:

   DisplayManager( TransformationManager *trans_manager );
   ~DisplayManager() {}

   //! sets the map to be displayed
   /*! sets the displayed map as well as updates the decomposition hierarchy window */
   void SetMap( Map *new_map );  // map placement functions
   void SetFirstMap();
   void InstallMap( int map_id );
   void InstallSubmap( Map *submap, Stub *stub, Empty *empty = NULL );
   void InstallMapElements( Map *map );
   void PlaceMap( Map *new_map );
   void DisplayOnlyMode( Map *map );
   void RestoreMapNavigation();
   void RemoveStubFromDecomposition( Stub *deleted_stub );
   Map * CurrentMap() { return( current_map ); }
   void InstallParent(); // installs parent map if there is one
   void InstallSubmap(); // installs lower map in current hierarchy if it exists
   void FollowPath();
   void Submaps( design_plugin *plugins, int &num_plugins, Stub *stub = NULL ); // submap query functions
   bool HasSubmaps( Stub *stub );
   void ExportMaps();
   void ExportGroup( map_list mlist );
   void ExportCSMGroup( map_list mlist );
   void ExportMapList( Cltn<Map *> *map_list, const char *default_name );
   void ExportCSMList( Cltn<Map *> *map_list, const char *default_name );
   void ImportMaps( map_type mtype, Stub *parent_stub = NULL );
   void RenameMap( Map *root_map, const char *new_label );
   void ListParentMaps();

   void VerifyAnnotations(); // verifies existence of performance annotations for certain map elements
   void DeactivateHighlighting(); // removes highlighting for incomplete elements
   
   void Add( Hyperedge *new_edge, Hyperedge *ref_edge ); // adds new element to path at position of reference element
   void AddBL( Hyperedge *new_edge, Hyperedge *ref_edge ); // adds new element to path between reference element and previous element
   void AddBR( Hyperedge *new_edge, Hyperedge *ref_edge ); // adds new element to path between reference element and next element
   void AddAfter( Hyperedge *new_edge, Hyperedge *ref_edge ); // various positioning functions
   void AddBetween( Hyperedge *new_edge, Hyperedge *left_edge, Hyperedge *right_edge );
   void AddAtPosition(  Hyperedge *new_edge, Hyperedge *ref_edge, Hyperedge *next_edge );
   void AddRight( Hyperedge *new_edge, Hyperedge *ref_edge );
   void AddRight( Figure *new_figure, Figure *ref_figure );
   void AddLeft( Hyperedge *new_edge, Hyperedge *ref_edge );
   void AddFirst( Hyperedge *edge );
   void AddFirstToRight( Hyperedge *new_edge, Hyperedge *ref_edge );

   void Connect( Hyperedge *edge1, Hyperedge *edge2 ); // connect, disconnect two elements
   void Disconnect( Hyperedge *edge1, Hyperedge *edge2 );

   void CreateStub( Stub *stub ); // stub manipulation functions
   void DeleteStub( Stub *stub );
   void CreateWaitSynch( WaitSynch *wait_synch ); // wait_synch manipulation functions
   void DeleteWaitSynch( WaitSynch *wait_synch );
   void CreateStubJoin( Hyperedge *edge, Hyperedge *stub );
   void DisconnectStubJoin( Hyperedge *empty, Hyperedge *stub, Hyperedge *result );
   void DisconnectStubFork( Hyperedge *empty, Hyperedge *stub, Hyperedge *start );

   void CreateNewPath( float XCoord, float YCoord ); // path creation functions
   void ExtendPath( float XCoord, float YCoord );
   void CreateNewSegment();

   void SplitPaths( Hyperedge *end_edge, Hyperedge *start_edge ); // splits path into two
   void JoinPaths( Hyperedge *end_edge, Hyperedge *start_edge ); // joins a pair of paths

   void CreateAndFork( Synchronization *synch, Empty *empty ); // creates graphical elements of and fork
   void AddAndForkBranch( Synchronization *synch, Empty *empty, Result *result ); // performs graphical manipulations for adding or fork branch
   void CreateAndJoin( Synchronization *synch, Empty *empty ); // creates graphical elements of and join
   void AddAndJoinBranch( Synchronization *synch, Empty *empty ); // performs graphical manipulations for adding and join branch

   void CreateOrFork( OrFork *fork, Empty *empty ); // creates graphical elements of or fork
   void AddOrForkBranch( OrFork *fork, Empty *empty ); // performs graphical manipulations for adding or fork branch
   void AddTimeoutPath( Timer *timer, Empty *empty ); // performs graphical manipulations for adding timeout path
   void DeleteTimeoutPath( Timer *timer ); // performs graphical manipulations for deleting timeout path
   void CreateOrJoin( OrJoin *join, Empty *empty ); // creates graphical elements of or join
   void AddOrJoinBranch( OrJoin *join, Empty *empty ); // performs graphical manipulations for adding or join branch
   void DisconnectJoin( Hyperedge *empty, Hyperedge *result ); // performs graphical manipulations for disconnecting joins
   void CreateLoop( Loop *loop, Empty *in_empty, Empty *out_empty ); // creates graphical elements of ucm loop

   void DeleteEndNull( Hyperedge *edge ); // deletes the end element of path
   void DeleteStartNull( Hyperedge *edge ); // deletes the start element of path

   void TransApplicableTo( Hyperedge *edge ); // determines applicable transformations for hyperedge 
   void TransApplicableToPair( Hyperedge *edge1, Hyperedge *edge2 ); // determines applicable transformations for pair of hyperedge

   HyperedgeFigure *FindFigure( float XCoord, float YCoord ); // performs hit detection for hyperedge figures
   void HandleDoubleClick( float XCoord, float YCoord ); // performs handling of double clicks in the editing area

   void RegisterMap( Map *new_map ); // registration and deregistration functions for maps, plug-ins, figures, paths, and responsibilities
   void PurgeMap( Map *map );
   void ReorderInMapList( Map *map ) { PurgeMap( map ); RegisterMap( map ); }
   void RegisterStubExpansion( Map *new_plugin );
   void PurgeStubExpansion( Map *plugin );
   void RegisterFigure( HyperedgeFigure *new_figure );
   void PurgeFigure( HyperedgeFigure *figure );
   void RegisterPath( Path *new_path );
   void PurgePath( Path *path );

   Cltn<ResponsibilityReference *> *ResponsibilityList() { return( responsibility_pool ); }
   Cltn<Map *> * Maps()  { return( map_pool ); }
   void AddMap( Map *new_map ) { map_pool->Add( new_map ); }
   Cltn<HyperedgeFigure *> * Figures() { return( figure_pool ); }
   TransformationManager *GetTransformationManager() { return( trans_manager ); }  
   void Draw( Presentation *ppr ); // draws screen

   void DrawSelectionBox( Presentation *ppr, float x1, float y1, float x2, float y2  ); // draws temporary rectangle and selects enclosed elements
   void AddSelectedFigure( HyperedgeFigure *new_figure );
   bool TransformationsApplicable();
   bool SingleSelection() { return( selected_figures.Size() == 1 ? TRUE : FALSE ); }
   bool SingleComponentSelection();
   HyperedgeFigure * FirstSelection() { return( selected_figures.Get( 1 ) ); }
   HyperedgeFigure * SecondSelection() { return( selected_figures.Get( 2 ) ); }
   void ResetSelectedElements(); // deselects multiple selected elements
   void SelectAllElements(); // selects all points and components
   void ShiftSelectedElements( float x_offset, float y_offset );

   void ClearDisplay( bool create_map = TRUE );
   Path *CurrentPath() {  return( current_path ); }
   void Update();
   void SetPath( Path *path ) { current_path = path; }
   void SetActive( HyperedgeFigure *figure );
   void ResetCurrentPath() { current_path = NULL; }
   const char * UniqueMapName( const char *prompt, const char *default_name );
   void CreateNewRootMap();
   void ChooseMap();
   Map * CurrentTopLevelMap() { return( hierarchy[0].map ); }
   bool TransferableRootMaps();
   
   const char *DesignName() { return( design_name ); }
   void DesignName( const char *new_name );
   const char *DesignDescription() { return( design_description ); }
   void DesignDescription( const char *new_description );
   int DesignId() { return( design_id++ ); }
   void DesignId( int new_id ) { design_id = new_id+1; }

   void AlignTop();
   void AlignBottom();
   void CenterVertically();
   void AlignLeft();
   void AlignRight();
   void CenterHorizontally();
   void DistributeHorizontally();
   void DistributeVertically();

   void SetCurrentUCMSet( UcmSet *new_set, bool update = TRUE );
   void SetScenarioHighlight( Scenario *new_scenario );
   bool MapViewable( Stub *stub, Map *map );
   Map * ViewableMap( Stub *stub, int user_choice );
   bool HasViewableSubmaps( Stub *stub );
   Map * SoleViewableSubmap( Stub *stub, Empty *empty = NULL );
   void ExpandUCMSet( Map *map );
   void ReplaceComponent( Component *old_component, Component *new_component );
   void ReplaceResponsibility( Responsibility *old_resp, Responsibility *new_resp );
   void DeleteSelectedElements();

   Empty * PathContinuedInParent( Hyperedge *io_point );
   void ContinuePathInParent( Empty *bound_empty, edge_type type );

   void ExportPerformanceData();
   void ImportPerformanceData();
   void ExportDXL();  // export DXL for the current design
   void ExportCSM();  // export Core Scenario Model for the current design

private:

   void PropagateForward( Hyperedge *split_edge, Path *new_path, Path *old_path, Label *new_label = NULL );
   void PropagateBackward( Hyperedge *split_edge, Path *new_path, Path *old_path, Label *new_label = NULL );
   bool MapNameExists( const char *map_name );
   Map * FindMap( const char *map_name );
   void FillDecompositionList();

   void LoadStartPFData( Start *start, FILE *fp );
   void LoadOrForkPFData( OrFork *fork, FILE *fp );
   void LoadStubPFData( Stub *stub, FILE *fp );
   void LoadRespRefPFData( ResponsibilityReference *resp, FILE *fp );

   Cltn<Map *> *map_pool;  // lists of maps, figures, pools, and responsibilities
   Cltn<HyperedgeFigure *> *figure_pool, selected_figures;
   Cltn<Path *> *path_pool;
   Cltn<ResponsibilityReference *> *responsibility_pool;

   const float XSpacing, YSpacing;
   Map *current_map;
   //! pointer to currently selected graphical path element
   HyperedgeFigure *active_figure;
    //! pointer to currently selected UCM path segment
   Path *current_path;
    //! pointer to current ucm set, used for restricted navigation
   UcmSet *current_ucm_set;
   TransformationManager *trans_manager;
   Cltn<Hyperedge *> *edges;
   hierarchy_element hierarchy[MAXIMUM_HIERARCHY_LEVELS];
   int decomposition_levels, displayed_map_level, design_id;
   char design_name[LABEL_SIZE+1]; // user given design name
   char *design_description; // user given design description
   float sbx, sby, ebx, eby;
   Map *base_map;
   Scenario *current_scenario;
   
};

#endif
