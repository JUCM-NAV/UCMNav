/***********************************************************
 *
 * File:			defines.h
 * Author:			Andrew Miga
 * Created:			July 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef DEFINES_H
#define DEFINES_H

#ifdef TRACE
#include "../trace.h"
#endif

typedef unsigned int transformation;
typedef unsigned int edge_type;
typedef unsigned int execution_flag;
typedef unsigned int flag;
typedef unsigned int figure_orientation;
typedef int map_type;

#define NEWLINE 10

typedef enum { INPUT, OUTPUT } path_direction;
typedef enum { FORWARD_SEARCH, BACKWARD_SEARCH, BIDIRECTIONAL } search_direction;
typedef enum { TRIGGERING, RESULTING, PRECONDITION, POSTCONDITION } etype;
typedef enum { PATH_ENTRY, PATH_EXIT } end_type;
typedef enum { CURRENT_MAP, CURRENT_SUBTREE, ALL_MAPS, UCM_SET } map_list;
typedef enum { GRAPHICS, EXPORT,EXPORTCSM, REPORT } list_operation;
typedef enum { COMPONENT_DEF, RESPONSIBILITY_DEF } reference_type;
typedef enum { MSC_FIRST_PASS, MSC_GENERATION_PASS, SCENARIO_TRACE, ADVANCE_SCAN } scan_type;

extern void AbortProgram( const char * );

#define strequal( x , y ) ( strcmp( x, y ) == 0 )

#ifndef NULL
#define NULL   0
#endif

#define NONEXISTENT 0

#ifndef PI
#define PI 3.14159265358979
#endif

#define forever for(;;)

#define Min(a, b) ( (a < b) ? a : b )
#define Max(a, b) ( (a > b) ? a : b )

// common declarations for bool functions

//#define TRUE		 1
//#define FALSE		 0
#if !defined(TRUE) && !defined(FALSE)
#define FALSE        0
#define TRUE        (!FALSE)
#endif

#define MODIFIED         1
#define UNMODIFIED       2
#define NA              -1
#define UNINITIALIZED    2
#define NOT_CONTAINED   -1
#define FILE_LOAD	 0
#define FILE_ERROR       1
#define ERROR            1
#define FINE             0
#define EXECUTE          1
#define COMPONENTS       1
#define POOLS            0
#define EDIT_EXISTING    0
#define CREATE_NEW       1
#define NO_MODE          2
#define INSTALL_EXISTING 2
#define DESC_MAP         0  // used by where_description, says current location is from map, path or component
#define DESC_PATH        1
#define DESC_COMP        2

#define FORWARD_FIG     0
#define VERTICAL_RIGHT  0
#define BACKWARD_FIG    180
#define VERTICAL_LEFT   180
#define UP_FIG          90
#define HORIZONTAL_UP   90
#define DOWN_FIG        270
#define HORIZONTAL_DOWN 270

#define MAP_ELEMENT_CONDITIONS  0
#define GOAL_CONDITIONS         1
#define PRECONDITIONS           0
#define POSTCONDITIONS          1

#define AVAILABLE       0
#define UNAVAILABLE     1
#define INVALID         2

#define QUIT            0
#define CONTINUE        1
#define JOINED_PATHS    2

#define STUB_INPUT           0
#define STUB_OUTPUT          1

#define SAVE_AND_QUIT    1     // used by prompt-for-save routines
#define NOSAVE_AND_QUIT  2
#define NOQUIT           3


const unsigned int DUPLICATE_ALLOWED = 1;
const unsigned int UNIQUE =  0;

// figure sizes

#define EMPTY_FIGURE_SIZE	0.00741
#define WAIT_FIGURE_SIZE	0.0111
#define RESULT_BAR		0.0222
#define TIMER_FIG_SIZE		0.0111
#define SYNCH_TIMER_SIZE        0.01
#define START_BAR_SIZE		0.0148
#define RESP_BAR		0.009
#define AND_FORK_HEIGHT		0.035
#define AND_FORK_PATH	        0.013
#define AND_FORK_PATH_BOTTOM	0.013
#define AND_FORK_PATH_TOP	-0.013
#define STRAIGHT_PATH_SEGMENT	0.025
#define CENTERED                0

#define OR_FORK_PATH            0.015
#define STUB_BOX_WIDTH          0.02
#define STUB_BOX_HEIGHT         0.03
#define STUB_CIRCLE_SIZE        0.01
#define MINIMUM_COMPONENT_DIMENSION  0.08
#define RADIUS_FACTOR           0.2
#define SELECTION_MARKER        0.014
#define LARGE_SELECTION_RADIUS        0.044
#define SMALL_SELECTION_RADIUS        0.03
#define WINDOW_MAX		1.985
#define WINDOW_MIN		0.015
#define VIRTUAL_WIDTH           1320
#define VIRTUAL_HEIGHT          1100
#define DEFAULT_WIDTH           660
#define DEFAULT_HEIGHT          550
#define MAP_BORDER              0.015
#define VIRTUAL_SCALING         2

#define BLUE	FL_BLUE  // colour for selected elements
#define RED     FL_RED   // colour for highlighted paths and elements with missing annotations
#define ORANGE  FL_DARKORANGE // highlight colour for multiply traversed path sections
#define BLACK	FL_BLACK
#define WHITE	FL_WHITE

#define NORMAL_SAMPLING_RATE 0.05
#define MEDIUM_SAMPLING_RATE 0.25
#define MOTION_SAMPLING_RATE 0.5
#define STRAIGHT_LINES 1.0

#define DOUBLE_CLICK_THRESHOLD 300
#define MAXIMUM_HIERARCHY_LEVELS 100

#define ST_STUB    -2
#define DYN_STUB   -3
#define ROOT_MAP -2
#define PLUGIN   -3

#define CHOOSE_SUBMAP    1
#define INSTALL_EXISTING 2
#define REMOVE_PLUGIN    3
#define COPY_EXISTING    4
#define POOL_CHOICE      5
#define LABEL_SIZE       64
#define DISPLAYED_LABEL  40
#define DISABLE_AUTOSAVE 1000
#define EVERY_CHANGE     1
#define FORK_OFFSET      1000

#endif
