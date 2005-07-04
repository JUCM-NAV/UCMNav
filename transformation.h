/***********************************************************
 *
 * File:			transformation.h
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "node.h"
#include "defines.h"
#include "collection.h"
#include "display.h"

class Hypergraph;

class TransformationManager
{

public:

   ~TransformationManager();
   static TransformationManager * Instance(); // returns single instance of object

   Cltn<Hyperedge *> *CreateNewPath(); // creates a new simple path and returns elements
   Hypergraph *CurrentGraph() { return( current_graph ); } // access methods for current hypergraph
   DisplayManager *GetDisplayManager() {  return( connect_manager ); }
   void InstallDisplayManager( DisplayManager *connect ) { connect_manager = connect; }
   void InstallCurrentGraph( Hypergraph *new_graph ) { current_graph = new_graph; }
   void TransApplicableTo( Hyperedge *edge ); // determines applicable transformations for hyperedge
   void TransApplicableToPair( Hyperedge *edge1, Hyperedge *edge2 ); // determines applicable transformations for pair of hyperedges

private:

   TransformationManager(); // protected constructor enforces single instance pattern
   void CreateDictionaries(); // creates lists of applicable transformations for all hyperedges at initialization

   DisplayManager *connect_manager;
   Hypergraph *current_graph; // pointer to the current hypergraph being displayed
   Cltn<transformation> transformation_dictionary[15]; // array of lists of single selection transformations for each hyperedge type
   Cltn<transformation> transformation_dictionary2[15]; // array of lists of double selection transformations for each hyperedge type

};

#if 0

typedef enum {
   CONNECTION,	ABORT,			EMPTY,	       	OR_FORK,	OR_JOIN,		RESPONSIBILITY_REF,	RESULT,	START,
   STUB,	SYNCHRONIZATION,	TIMER,		WAIT,		TIMESTAMP,		GOAL_TAG,		LOOP, RESOURCEACQUIRE, RESOURCERELEASE, DUMMY
} hyperedges;

#endif

// hyperedge types
#define CONNECTION		0
#define ABORT                   1
#define	EMPTY			2
#define	OR_FORK			3
#define	OR_JOIN			4
#define	RESPONSIBILITY_REF      5
#define	RESULT			6
#define	START			7
#define	STUB			8
#define SYNCHRONIZATION         9
#define	TIMER			10
#define	WAIT			11
#define TIMESTAMP               12
#define GOAL_TAG                13
#define LOOP                    14
#define RESOURCEACQUIRE			15
#define RESOURCERELEASE			16
#define DUMMY                   17

// single path element transformations
#define EDIT_LABEL                      0
#define	DECOMPOSE_FROM_JOIN		1
#define	ADD_RESPONSIBILITY		2
#define	ADD_WAIT			3
#define	ADD_STUB			4
#define	ADD_TIMER			5
#define	ADD_OR_FORK			6
#define	ADD_AND_FORK			7
#define	CUT				8
#define	DELETE_RESPONSIBILITY		9
#define	DELETE_STUB			10
#define	DELETE_TIMER			11
#define	EXPAND_STUB			12
#define	DELETE_WAIT			13
#define DELETE_POINT			14
#define ADD_POINT			15
#define DELETE_PATH			16
#define EDIT_RESPONSIBILITY		17
#define ADD_BRANCH			18
#define DELETE_BRANCH			19
#define ENABLE_PATH_LABEL               20
#define DISABLE_PATH_LABEL              21
#define VIEW_SUB_MAP                    22
#define EDIT_STUB_LABEL                 23
#define DECOMPOSE_FROM_STUB             24
#define ADD_FAILURE_POINT               25
#define REMOVE_FAILURE_POINT            26
#define ADD_SHARED_RESPONSIBILITY       27
#define REMOVE_SHARED_RESPONSIBILITY    28
#define ROTATE                          29
#define INSTALL_EXISTING_PLUGIN         30
#define REMOVE_SUBMAP                   31
#define BIND_STUB                       32
#define ADD_TIMESTAMP                   33
#define DELETE_TIMESTAMP                34
#define EDIT_TIMESTAMP                  35
#define EDIT_PATH_INITIATION            36
#define EDIT_START_NAME                 37
#define EDIT_TIMER_CHARACTERISTICS      38
#define EDIT_WAIT_CHARACTERISTICS       39
#define EDIT_END_NAME                   40
#define VIEW_RESPONSE_TIMES             41
#define COPY_EXISTING_PLUGIN            42
#define RENAME_SUBMAP                   43
#define ADD_GOAL_TAG                    44
#define DELETE_GOAL_TAG                 45
#define RENAME_GOAL_TAG                 46
#define IMPORT_PLUGIN_FILE              47
#define ADD_DIRECTION_ARROW             48
#define REMOVE_DIRECTION_ARROW          49
#define CENTER_LABEL                    50
#define FLUSH_LABEL                     51
#define ADD_TIMEOUT_PATH                52
#define DELETE_TIMEOUT_PATH             53
#define EDIT_FORK_SPECIFICATION         54
#define TRANSFER_ROOT_PLUGIN            55
#define GENERATE_MSC                    56
#define ADD_LOOP                        57
#define EDIT_LOOP_CHARACTERISTICS       58
#define DELETE_LOOP                     59
#define FOLLOW_PATH_SUBMAP              60
#define FOLLOW_PATH_PARENT_MAP          61
#define LIST_BOUND_PARENT_MAPS          62
#define SET_SCENARIO_START              63
#define LIST_SCENARIOS                  64
#define EDIT_LOGICAL_CONDITION          65

// double path element transformations
#define	AND_COMPOSE		       0
#define	OR_COMPOSE		       1
#define	STUB_COMPOSE		       2
#define	MERGE			       3
#define	CONNECT			       4
#define	DISCONNECT		       5
#define ADD_INCOMING_SYNCHRONIZATION   6
#define CREATE_TIME_RESPONSE           7
#define EDIT_TIME_RESPONSE             8
#define DELETE_TIME_RESPONSE           9
#define CREATE_GOAL                    10
#define EDIT_GOAL                      11

//Weight and Height for graph define in DXL for DOORS, added by Bo Jiang, Sept 2004
#define WIDTH                       1320
#define HEIGHT                     1100

#endif
