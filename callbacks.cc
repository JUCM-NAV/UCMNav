/*****************************************************************
 *
 * File:			callbacks.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:	Nov. 1996 - Load method changed
 *                              June 1997 - Load method removed
 ****************************************************************/

#include <iostream>

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "display.h"
#include "hyperedge.h"
#include "condition.h"
#include "map.h"
#include "component_mgr.h"
#include "xfpresentation.h"
#include "hypergraph.h"
#include "timestamp.h"
#include "resp_ref.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef TIME
#include <sys/time.h>
#include <sys/resource.h>
#endif

extern void UpdateResponsibilityList();
extern void UpdatePathMenu();
extern void ResetConditions();
extern void EditComponentAttributes();

#define MOUSE_EVENT 0
#define KEY_INCREMENT 1
#define KEY_DECREMENT 2
#define STEP_SIZE 0.1
#define SHIFT_INCREMENT 0.01
#define FIT_VIRTUAL_AREA 10
#define FIT_MAP 11

// character arrays for description titles

static char hyperedge_description[][32] = {

   "Description of Empty Point",
   "Description of OR Fork",
   "Description of OR Join",
   "Description of Responsibility",
   "Description of End Point",
   "Description of Start Point",
   "Description of Stub",
   "Description of Synchronization",
   "Description of Timer",
   "Description of Waiting Place",
   "Description of Timestamp Point",
   "Description of Goal Tag",
   "Description of Loop" };

char component_description[][24] = {

   "Description of Team",
   "Description of Object",
   "Description of Process",
   "Description of ISR",
   "Description of Pool",
   "Description of Agent",
   "Description of Other" };

extern GC gcCanvas_gc;
extern FD_Main *pfdmMain;

extern Cursor curCrosshair;

int SDPvisible;

extern Presentation *ppr;

float x_scroll, y_scroll;  // global variables for scrolling offsets
extern int default_width, default_height;

int actual_width = default_width,
   actual_height = default_height,
   window_width = default_width,
   window_height = default_height;

int left_boundary = 0,
   top_boundary = 0;

float left_border = 0, // global variables for drawing area borders
   right_border = 1,
   top_border = 0,
   bottom_border = 1,
   scale = 1;  // global variable for zoom scale

bool fit_virtual_area = FALSE;
bool fit_map = FALSE;
int current_scale = 5;

static float x_coord, y_coord, start_selection_x, start_selection_y;
static int xc, yc;
static double horizontal_length = 1.0, vertical_length = 1.0; // lengths of scrollbars

extern bool FixedAllComponents;
extern bool FixedAllPositions;

bool button2_down = FALSE, button1_down = FALSE, initial_button1_press;

int iMotion_direction = -1, iLast_x = -1, iLast_y = -1;
float sampling_rate = NORMAL_SAMPLING_RATE,
   user_rate = NORMAL_SAMPLING_RATE;

extern ComponentReference *active_component;
extern ComponentManager *component_manager;

extern void SetPromptUserForSave();

int start_component_x, start_component_y, current_component_x, current_component_y;

float fR=.5, fG=.5, fB=.5; /* temporary */

typedef enum {NO_TOOL, COMPONENT_TOOL, PATH_TOOL, SELECT_TOOL} etTool;
etTool etCurrent_tool = NO_TOOL;

char loaded_file[64] = "";
char loaded_file_path[128] = "";

int GetSelectedResponsibilityNumber();
void SetCurrentScaleSelection( float scale );

HyperedgeFigure *active_figure = NULL, *first_figure = NULL;
MarkerFigure *marker_figure = NULL;
Timestamp *last_timestamp = NULL,
   *previous_timestamp = NULL;

bool multiple_selection = FALSE;
bool timestamp_pair_selected;

int freeze_editor = FALSE;   // global variable used to freeze editor window during dialog operations
int where_description = DESC_MAP;  // global var, says where to get current desc from

TransformationManager *trans_manager;
extern DisplayManager *display_manager;

#ifdef TIME
long time_difference( struct timeval time1, struct timeval time2 );
int get_cpu_time( void );
#endif

void SetDisplayManager( DisplayManager *manager )
{
   display_manager = manager;
   trans_manager = display_manager->GetTransformationManager();
}

void SetActiveFigure( HyperedgeFigure *new_figure )
{
   active_figure = new_figure;
}

// static int draw_count = 0; /* DA, removed August 2004 */

void DrawScreen()
{
#ifdef TIME
   struct timeval time1, time2;
   int cputime1, cputime2;
   void *dummy;
   gettimeofday( &time1, dummy );
   cputime1 = get_cpu_time();
#endif

   fl_freeze_form( pfdmMain->Main ); 
   ppr->Clear();
   component_manager->Draw( ppr );
   display_manager->Draw( ppr );
   ppr->Refresh();
   fl_unfreeze_form( pfdmMain->Main );

#ifdef TIME
   gettimeofday( &time2, dummy );
   long td = time_difference( time1, time2 );
   cputime2 = get_cpu_time();  
   printf( "Screen redraw CPU time: %d Elapsed time: %d microseconds\n", cputime2-cputime1, td );
#endif
}

int DrawingArea_expose( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d )
{
   FD_Main *pfdmMain = (FD_Main *)d;
   if (pfdmMain == NULL) {} // Makes GCC happy... pfdmMain is now used!  
   ppr->Refresh();
   return 0;
}

void ResizeDrawingArea( int new_width, int new_height )
{
   char choice_text[10];
   float x_scale, y_scale, original_scale, top, bottom, left, right;

   window_width = new_width;
   window_height = new_height;
   ((XfPresentation *)ppr)->SetWindowDimensions( new_width, new_height );
   
   if( (fit_virtual_area == TRUE) || (fit_map == TRUE) ) { // scale needs to be recalculated

      if( fit_virtual_area == TRUE ) {
	 x_scale = (float)new_width/VIRTUAL_WIDTH;
	 y_scale = (float)new_height/VIRTUAL_HEIGHT;
      }
      else {

	 display_manager->CurrentMap()->DetermineMapBoundaries( top, bottom, left, right );

	 left = ( (left-MAP_BORDER) > 0.0 ) ? left-MAP_BORDER : 0.0;
	 right = ( (right+MAP_BORDER) < 2.0 ) ? right+MAP_BORDER : 2.0;
	 top = ( (top-MAP_BORDER) > 0.0 ) ? top-MAP_BORDER : 0.0;
	 bottom = ( (bottom+MAP_BORDER) < 2.0 ) ? bottom+MAP_BORDER : 2.0;
	 
	 x_scale = (float)new_width/((right-left)*VIRTUAL_WIDTH/2);
	 y_scale = (float)new_height/((bottom-top)*VIRTUAL_HEIGHT/2);
      }

      original_scale = Min( x_scale, y_scale );      
      scale = (  original_scale*original_scale > 3.0 ) ? sqrt( 3.0 ) : original_scale; // limit scale at 300 %

      if( fit_map == TRUE ) {
	 x_scroll = 0;
	 y_scroll = 0;
	 if( original_scale == scale ) {
	    left_boundary = (int)(((left+(((left +((float)new_width/(float)default_width)/scale)-right)/2))*VIRTUAL_WIDTH*scale)/VIRTUAL_SCALING);
	    top_boundary = (int)(((top+((bottom-(top +((float)new_height/(float)default_height)/scale))/2))*VIRTUAL_HEIGHT*scale)/VIRTUAL_SCALING);
	 }
	 else {
	    left_boundary = (int)((left*VIRTUAL_WIDTH*scale)/VIRTUAL_SCALING);
	    top_boundary = (int)((top*VIRTUAL_HEIGHT*scale)/VIRTUAL_SCALING);
	 }
      }
      
      sprintf( choice_text, "%d%%%%", (int)(scale*scale*100) );
      if( fl_get_choice_maxitems( pfdmMain->ScaleChoice ) == 12 )
	 fl_replace_choice( pfdmMain->ScaleChoice, 12, choice_text );
      else
	 fl_addto_choice( pfdmMain->ScaleChoice, choice_text );
      fl_set_choice( pfdmMain->ScaleChoice, 12 );
      ((XfPresentation *)ppr)->SetScaleFactor( scale );
      SetCurrentScaleSelection( scale );
   }
   
   // limit width and height if they surpass virtual drawing area
   if( new_width > (VIRTUAL_WIDTH*scale) ) new_width = (int)(VIRTUAL_WIDTH*scale);
   if( new_height > (VIRTUAL_HEIGHT*scale) ) new_height = (int)(VIRTUAL_HEIGHT*scale);

   // adjust range and size of scrollbars
   horizontal_length = (((VIRTUAL_WIDTH*scale)/(double)new_width) - 1 );
   vertical_length = (((VIRTUAL_HEIGHT*scale)/(double)new_height) - 1 );
   
   fl_set_scrollbar_bounds( pfdmMain->HorizontalScrollbar, 0, horizontal_length );
   fl_set_scrollbar_size( pfdmMain->HorizontalScrollbar, (double)new_width/(VIRTUAL_WIDTH*scale) );

   fl_set_scrollbar_bounds( pfdmMain->VerticalScrollbar, 0, vertical_length );
   fl_set_scrollbar_size( pfdmMain->VerticalScrollbar, (double)new_height/(VIRTUAL_HEIGHT*scale) );

   // adjust scrolling positions

   if( x_scroll > horizontal_length ) {
      fl_set_scrollbar_value( pfdmMain->HorizontalScrollbar, horizontal_length );
      x_scroll = horizontal_length;
      left_boundary = (int)(x_scroll*new_width);
   }
   else {
      x_scroll = (float)left_boundary/(float)new_width;
      fl_set_scrollbar_value( pfdmMain->HorizontalScrollbar, x_scroll );
   }  

   if( y_scroll > vertical_length ) {
      fl_set_scrollbar_value( pfdmMain->VerticalScrollbar, vertical_length );
      y_scroll = vertical_length;
      top_boundary = (int)(y_scroll*new_height);
   }
   else {
      y_scroll = (float)top_boundary/(float)new_height;
      fl_set_scrollbar_value( pfdmMain->VerticalScrollbar, y_scroll );
   }      

   // recalculate drawing area borders
   left_border = ((float)left_boundary/(VIRTUAL_WIDTH*scale))*VIRTUAL_SCALING;
   right_border = left_border +((float)new_width/(float)default_width)/scale;
   top_border = ((float)top_boundary/(VIRTUAL_HEIGHT*scale))*VIRTUAL_SCALING;
   bottom_border = top_border +((float)new_height/(float)default_height)/scale;

   if( (window_width != new_width) || (window_height != new_height) || (new_width != actual_width) || (new_height != actual_height) ) {
      actual_width = new_width;
      actual_height = new_height;
      ((XfPresentation *)ppr)->CreateNewPixmap( actual_width, actual_height );
   }
   
   DrawScreen();
}

int DrawingArea_resize( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d )
{
   FL_Coord x, y, new_width, new_height;
   
   fl_get_object_geometry( pfdmMain->DrawingArea, &x, &y, &new_width, &new_height );
   ResizeDrawingArea( new_width, new_height );
   
   return 0;
}

void HorizontalScrollbar_cb(FL_OBJECT *scrollbar, long increment )
{
   float x_new;
   
   x_scroll = fl_get_scrollbar_value( scrollbar );

   if( increment != MOUSE_EVENT ) {

      if( increment == KEY_INCREMENT )
	 x_new = (((x_scroll+STEP_SIZE) < horizontal_length) ? (x_scroll+STEP_SIZE) : horizontal_length );
      else
	 x_new = (((x_scroll-STEP_SIZE) > 0 ) ? (x_scroll-STEP_SIZE) : 0 ); 

      if( x_new == x_scroll ) // if value didn't change as it was at the boundary return immediately
	 return;
      else
	 x_scroll = x_new;

      fl_set_scrollbar_value( scrollbar, x_scroll );
   }
   
   left_boundary = (int)(x_scroll*actual_width);
   left_border = ((float)left_boundary/(VIRTUAL_WIDTH*scale))*VIRTUAL_SCALING;
   right_border = left_border +((float)actual_width/(float)default_width)/scale;
   DrawScreen();
}

void VerticalScrollbar_cb(FL_OBJECT *scrollbar, long increment )
{
   float y_new;
   
   y_scroll = fl_get_scrollbar_value( scrollbar );

   if( increment != MOUSE_EVENT ) {

      if( increment == KEY_INCREMENT )
	 y_new = (((y_scroll+STEP_SIZE) < vertical_length) ? (y_scroll+STEP_SIZE) : vertical_length );
      else
	 y_new = (((y_scroll-STEP_SIZE) > 0 ) ? (y_scroll-STEP_SIZE) : 0 ); 

      if( y_new == y_scroll ) // if value didn't change as it was at the boundary return immediately
	 return;
      else
	 y_scroll = y_new;
      
      fl_set_scrollbar_value( scrollbar, y_scroll );
   }
   
   top_boundary = (int)(y_scroll*actual_height);
   top_border = ((float)top_boundary/(VIRTUAL_HEIGHT*scale))*VIRTUAL_SCALING;
   bottom_border = top_border +((float)actual_height/(float)default_height)/scale;
   DrawScreen();
}

void ChangeScale( int scale_selection )
{   
   char choice_text[10];
   
   if( fit_virtual_area == TRUE ) {
      fit_virtual_area = FALSE;
      fl_delete_choice( pfdmMain->ScaleChoice, 12 );
   }

   if( fit_map == TRUE ) {
      fit_map = FALSE;
      fl_delete_choice( pfdmMain->ScaleChoice, 12 );
   }

   if( scale_selection < 10 ) {
      strcpy( choice_text, fl_get_choice_item_text( pfdmMain->ScaleChoice, scale_selection ) );
      if( scale_selection != fl_get_choice( pfdmMain->ScaleChoice ) ) // check if keyboard was used
	 fl_set_choice( pfdmMain->ScaleChoice, scale_selection );
      choice_text[strlen(choice_text)-1] = 0;
      scale = sqrt( atof( choice_text )/100 );
      ((XfPresentation *)ppr)->SetScaleFactor( scale );   
   }
   else if( scale_selection == 10 ) // Fit Virtual Area was chosen
      fit_virtual_area = TRUE;
   else  // Fit Map was chosen
      fit_map = TRUE;

   ResizeDrawingArea( window_width, window_height );
}

void ResetScrollbars()
{
   x_scroll = 0;
   y_scroll = 0;

   fl_set_scrollbar_value( pfdmMain->HorizontalScrollbar, 0 );
   fl_set_scrollbar_value( pfdmMain->VerticalScrollbar, 0 );

   left_boundary = 0;
   left_border = 0;
   right_border = ((float)actual_width/(float)default_width)/scale;
   
   top_boundary = 0;
   top_border = 0;
   bottom_border = ((float)actual_height/(float)default_height)/scale;
}

void ScaleChoice_cb(FL_OBJECT *, long)
{
   current_scale = fl_get_choice( pfdmMain->ScaleChoice );
   ChangeScale( current_scale );
}

void ZoomIn()
{
   if( (fit_virtual_area == TRUE) || (fit_map == TRUE) )
      current_scale--;
   
   if( current_scale < 9 )
      ChangeScale( ++current_scale );
}

void ZoomOut()
{
   if( current_scale > 1 )
      ChangeScale( --current_scale ); 
}

void FitVirtualArea()
{
   ChangeScale( FIT_VIRTUAL_AREA );
}

void FitMap()
{
   ChangeScale( FIT_MAP );
}

void SetCurrentScaleSelection( float scale )
{
   float factor = scale*scale;

   if( factor < 0.25 )
      current_scale = 1;
   else if( factor < 0.35 )
      current_scale = 2;
   else if( factor < 0.5 )
      current_scale = 3;
   else if( factor < 0.75 )
      current_scale = 4;
   else if( factor < 1.0 )
      current_scale = 5;
   else if( factor < 1.25 )
      current_scale = 6;
   else if( factor < 1.5 )
      current_scale = 7;
   else if( factor < 2.0 )
      current_scale = 8;
   else
      current_scale = 9;
}

void FullEditMode()
{
   FixedAllComponents = false;
   FixedAllPositions = false;
   fl_set_choice( pfdmMain->ModeChoice, 1 );
}

void FixComponentsMode()
{
   FixedAllComponents = true;
   FixedAllPositions = false;
   fl_set_choice( pfdmMain->ModeChoice, 2 );
}

void FixAllPositionsMode()
{
   FixedAllComponents = true;
   FixedAllPositions = true;
   fl_set_choice( pfdmMain->ModeChoice, 3 );
}

void SelectPathTool()
{
   if( etCurrent_tool != PATH_TOOL ){
    
      fl_set_button( pfdmMain->PathButton, 1 );
      fl_trigger_object( pfdmMain->PathButton );
      fl_set_button( pfdmMain->SelectButton, 0 );
      fl_set_button( pfdmMain->ComponentButton, 0 );
      display_manager->ResetSelectedElements();
   }
}

void SelectSelectTool()
{
   if( etCurrent_tool != SELECT_TOOL ){
    
      fl_set_button( pfdmMain->SelectButton, 1 );
      fl_trigger_object( pfdmMain->SelectButton );
      fl_set_button( pfdmMain->PathButton, 0 );
      fl_set_button( pfdmMain->ComponentButton, 0 );
   }
}

void SelectComponentTool()
{
   if( etCurrent_tool != COMPONENT_TOOL ){
    
      fl_set_button( pfdmMain->ComponentButton, 1 );
      fl_trigger_object( pfdmMain->ComponentButton );
      fl_set_button( pfdmMain->SelectButton, 0 );
      fl_set_button( pfdmMain->PathButton, 0 );
      display_manager->ResetSelectedElements();
   }
}

int DrawingArea_key( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d )
{
   FD_Main *pfdmMain = (FD_Main *)d;
   char buffer[10];
   KeySym ksKey;
   XComposeStatus xcsCompose;

   XLookupString( &ev->xkey, buffer, 10, &ksKey, &xcsCompose );

   switch( ksKey ) {

   case XK_Delete:  // delete key was pressed, delete selected path elements and components
      
      display_manager->DeleteSelectedElements();
      component_manager->DeleteSelectedComponents();
      DrawScreen();
      break;
      
   case XK_Down: // down arrow was pressed, increment vertical scrollbar value through callback

      if( ev->xkey.state & ShiftMask )
	 display_manager->ShiftSelectedElements( 0, SHIFT_INCREMENT );
      else
	 VerticalScrollbar_cb( pfdmMain->VerticalScrollbar, KEY_INCREMENT );
      break;

   case XK_Up: // up arrow was pressed, decrement vertical scrollbar value through callback

      if( ev->xkey.state & ShiftMask )
	 display_manager->ShiftSelectedElements( 0, -SHIFT_INCREMENT );
      else
	 VerticalScrollbar_cb( pfdmMain->VerticalScrollbar, KEY_DECREMENT );
      break;

   case XK_Right: // right arrow was pressed, increment horizontal scrollbar value through callback

      if( ev->xkey.state & ShiftMask )
	 display_manager->ShiftSelectedElements( SHIFT_INCREMENT, 0 );
      else
	 HorizontalScrollbar_cb( pfdmMain->HorizontalScrollbar, KEY_INCREMENT );
      break;

   case XK_Left: // right arrow was pressed, increment horizontal scrollbar value through callback

      if( ev->xkey.state & ShiftMask )
	 display_manager->ShiftSelectedElements( -SHIFT_INCREMENT, 0 );
      else
	 HorizontalScrollbar_cb( pfdmMain->HorizontalScrollbar, KEY_DECREMENT );
      break;

   case XK_plus: // '+'

      ZoomIn();
      break;

   case XK_minus: // '-'

      ZoomOut();
      break;

   case XK_equal: // '='

      FitMap();
      break;

   case XK_Escape:
      
      display_manager->ResetSelectedElements();
      DrawScreen();
      break;
      
   case XK_a: // 'a'
   case XK_A: // 'A'

      FixAllPositionsMode();
      break;
      
   case XK_b: // 'b'
   case XK_B: // 'B'

      display_manager->FollowPath();
      break;

   case XK_c: // 'c'
   case XK_C: // 'C'
  
      SelectComponentTool();
      break;

   case XK_d: // 'd'
   case XK_D: // 'D'
   case XK_Page_Down: // PageDown key
      
      display_manager->InstallSubmap();
      break;

   case XK_e: // 'e'
   case XK_E: // 'E'

      FullEditMode();
      break;

   case XK_f: // 'f'
   case XK_F: // 'F'

      FixComponentsMode();
      break;
      
#ifdef DEBUG
   case XK_H: // 'H'  hidden debugging option
      
      display_manager->CurrentMap()->MapHypergraph()->Output();
      break;
#endif

   case XK_l: // 'l'
   case XK_L: // 'L'
   case XK_less: // '<'

      component_manager->LowerLastComponent();
      break;

   case XK_m:
   case XK_M:

      display_manager->SelectAllElements();
      break;

   case XK_p: // 'p'
   case XK_P: // 'P'
  
      SelectPathTool();
      break;

   case XK_s: //'s'
   case XK_S: // 'S'
  
      SelectSelectTool();
      break;

   case XK_u: // 'u'
   case XK_U: // 'U'
   case XK_Page_Up: // PageUp key

      display_manager->InstallParent();
      break;
      
   case XK_v: // 'v'
   case XK_V: // 'V'

      FitVirtualArea();
      break;
      
   default:

      break;

   }
   
   return 0;
}

void CalculateCoordinates( int iXloc, int iYloc )
{
   x_coord = ((float)(iXloc+((int)(x_scroll*actual_width)))/(float)default_width)/scale;
   y_coord = ((float)(iYloc+((int)(y_scroll*actual_height)))/(float)default_height)/scale;
   xc = (int)((iXloc + (x_scroll*actual_width))/scale);
   yc = (int)((iYloc + (y_scroll*actual_height))/scale);
}

float translate_diff_x( int xp )
{
   return( ((float)xp/(float)default_width) );
}

float translate_diff_y( int yp )
{
   return( ((float)yp/(float)default_height) );
}

void SynchronizeResponsibilityDescriptions( Responsibility *resp )
{
   if( active_figure ) {
      if( active_figure->Edge()->EdgeType() == RESPONSIBILITY_REF ) {
	 if( ((ResponsibilityReference *)active_figure->Edge())->ParentResponsibility() == resp )
	    fl_set_input( pfdmMain->Description, ( (resp->Description()) ? resp->Description() : "" ) );
      }
   }
}

void DisplayMapDescription()
{
   where_description = DESC_MAP;
   char *description = display_manager->CurrentMap()->MapDescription();
   fl_set_input( pfdmMain->Description, description ? description : "" );
   fl_set_object_label( pfdmMain->Description, "Description of Map" );
}

int DrawingArea_but( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d )
{
   FD_Main *pfdmMain = (FD_Main *)d;
   int iPopup, iWhich_popup;
   // float fMin_dist = 10000.0, fSecond_dist = 10000.0;
   // float fMin_width = 10000.0, fMin_height = 10000.0;
   const char *mn;
   char menu[300];
   int iXloc = ev->xbutton.x, iYloc = ev->xbutton.y;
   static unsigned long int click1_time, click2_time;
   static Figure *prev_active_figure = NULL;
   static ComponentReference *prev_active_component = NULL;
   bool redraw = FALSE;

   // check if editor disabling flag has been set by any of the dialog boxes, if so return immediately
   if( freeze_editor == TRUE )
      return 0;

   CalculateCoordinates( iXloc, iYloc );
   
   switch( ev->type ) {
   case ButtonPress:
      switch( ev->xbutton.button ) {
      case Button1:
	 
	 if( etCurrent_tool == COMPONENT_TOOL ) {
	    button1_down = TRUE;
	    start_component_x = xc;
	    start_component_y = yc;
	    current_component_x = xc;
	    current_component_y = yc;

	    SetPromptUserForSave();
	
	    DrawScreen();
	    
	 } else if( etCurrent_tool == PATH_TOOL ) {      
	    // ADD_SIMPLE_PATH
	    if ( display_manager->CurrentPath() != NULL )
	       display_manager->ExtendPath( x_coord, y_coord );
	    else {	       
	       if( marker_figure )
		  display_manager->ExtendPath( x_coord, y_coord );
	       else
		  display_manager->CreateNewPath( x_coord, y_coord );
	    }

	    SetPromptUserForSave();
	    DrawScreen();	    
	 } 
      
	 else if( etCurrent_tool == SELECT_TOOL ) {

	    // check for double click

	    click2_time = ev->xbutton.time;

	    if( click2_time - click1_time <= DOUBLE_CLICK_THRESHOLD ) {
	       // double click has occurred, search for stub with submap defined or responsibility
	       display_manager->HandleDoubleClick( x_coord, y_coord );
	       if( active_component )  // if no hyperedge was found edit component if a component was selected
		  EditComponentAttributes();
	       break;
	    }
	    else
	       click1_time = click2_time;
	
	    iLast_x = xc;
	    iLast_y = yc;
	    start_selection_x = x_coord;
	    start_selection_y = y_coord;

	    button1_down = TRUE;
	    initial_button1_press = TRUE;
	    button2_down = FALSE;
	    
	    if( sampling_rate < MOTION_SAMPLING_RATE )
	       sampling_rate = MOTION_SAMPLING_RATE;

	    if( component_manager->FindHandle( x_coord, y_coord ) )  // if point is inside a handle simply return
	       return 0;

	    // FIGURE_HIT_DETECT
	    timestamp_pair_selected = FALSE; // reset timestamp pair selection flag
	    if( ev->xbutton.state & ShiftMask ){
	       multiple_selection = TRUE;
	    }
	    else if( ev->xbutton.state & ControlMask ){
	       display_manager->ResetSelectedElements();
	       active_figure = NULL;
	       active_component = NULL;
	       return 0;
	    }
	    else {
	       display_manager->ResetSelectedElements();
	       multiple_selection = FALSE;
	    }

	    active_figure = display_manager->FindFigure( x_coord, y_coord );

	    if( active_figure != prev_active_figure )
	       redraw = TRUE;
	    if( active_figure )
	       display_manager->AddSelectedFigure( active_figure );

	    // check if figure is that of a timestamp and if so store timestamp pointer
	    if( active_figure ) {
	       if( active_figure->Edge()->EdgeType() == TIMESTAMP ) {
		  if( last_timestamp != NULL )
		     previous_timestamp = last_timestamp;
		  last_timestamp = (Timestamp *)(active_figure->Edge());
		  if( last_timestamp == previous_timestamp )
		     previous_timestamp = NULL; // clear previous timestamp pointer if the same timestamp is reselected
		  if((previous_timestamp != NULL) && (multiple_selection == TRUE))
		     timestamp_pair_selected = TRUE; // set timestamp pair selection flag
	       }
	    }
	    
	    prev_active_figure = active_figure;
	    
	    if( active_figure ) {
	       if( redraw ) DrawScreen();
	       active_figure->Edge()->EdgeSelected();
	       where_description = DESC_PATH;
	       fl_set_input( pfdmMain->Description, ( (active_figure->Edge()->Description()) ? active_figure->Edge()->Description() : "" ) );
	       fl_set_object_label( pfdmMain->Description, hyperedge_description[active_figure->Edge()->EdgeType()-2] );
	       return 0;
	    }
	    else
	       ResetConditions();

	    active_component = component_manager->FindComponent( x_coord, y_coord );

	    if( (active_component != prev_active_component) || (ev->xbutton.state & ShiftMask) )
	       redraw = TRUE;

	    prev_active_component = active_component;

	    if( !active_figure )       // no path element selected
	    {
	       if( !active_component )       // no component selected
	       {
		  display_manager->ResetSelectedElements();
		  DisplayMapDescription();
	       }
	       else       // component is selected
	       {
		  component_manager->AddSelectedComponent( active_component );
		  where_description = DESC_COMP;
		  const char *comp_desc = NULL;
		  etComponent_type component_type = TEAM;
		  if( active_component->ReferencedComponent() ) {
		     comp_desc = active_component->ReferencedComponent()->Description();
		     component_type = active_component->ReferencedComponent()->GetType();
		  }
		  fl_set_input( pfdmMain->Description, ( (comp_desc) ? comp_desc : "" ) );
		  fl_set_object_label( pfdmMain->Description, component_description[component_type] );
	       }
	    }
	 }

	 if( redraw ) {
	    DrawScreen();
	    redraw = FALSE;
	 }

	 break;
      
      case Button2:
      case Button3:

	 if( ( etCurrent_tool == SELECT_TOOL ) && display_manager->TransformationsApplicable()
	     && !( ev->xbutton.state & ControlMask ) ) { // TRANSFORMATION_MENU_CODE
	    if( timestamp_pair_selected ) // check for timestamp pair selections
	       display_manager->TransApplicableToPair( previous_timestamp, last_timestamp );
	    else {  // perform regular processing
	       if( display_manager->SingleSelection() )
		  display_manager->TransApplicableTo( display_manager->FirstSelection()->Edge() );
	       else
		  display_manager->TransApplicableToPair( display_manager->FirstSelection()->Edge(), display_manager->SecondSelection()->Edge() );

	       if( !freeze_editor ) DrawScreen();
	    }

	    break;  // breaks the rest of the Button2 code	
	 }
	 else {
	    button2_down = TRUE;
	    iLast_x = xc;
	    iLast_y = yc;
	 }
	 
	 iWhich_popup = -1;

	 if( etCurrent_tool == SELECT_TOOL ) {
	    if( display_manager->SingleComponentSelection() )
	       iWhich_popup = 1;
	 }
	 else if( etCurrent_tool == PATH_TOOL )
	    iWhich_popup = 0;
	 else if( etCurrent_tool == COMPONENT_TOOL &&  display_manager->SingleComponentSelection() )
	    iWhich_popup = 1;
      
	 switch( iWhich_popup ) {

	 case 0:
  	
	    break;
	
	 case 1:

	    sprintf( menu, " Component %%t | Attributes | Cut Component | Copy Component | Paste Component | Bind Enclosed Path Elements | Unbind Enclosed Path Elements | Bind Enclosed Components | Unbind Enclosed Components " );	       
	    mn = menu;
      
	    iPopup = fl_defpup( FL_ObjWin( ob ), mn );

	    switch( fl_dopup( iPopup ) ) {
	    case 1:

	       EditComponentAttributes();
	       break;

	    case 2:

	       component_manager->CutComponent();
	       SetPromptUserForSave();
	       DrawScreen();
	       break;

	    case 3:

	       component_manager->CopyComponent();
	       break;

	    case 4:

	       component_manager->PasteComponent();	       
	       SetPromptUserForSave();
	       break;

	    case 5:

	       component_manager->BindEnclosedFigures();
	       SetPromptUserForSave();
	       break;

	    case 6:

	       component_manager->UnbindEnclosedFigures();
	       SetPromptUserForSave();
	       break;
	       
	    case 7:

	       component_manager->BindEnclosedComponents();
	       SetPromptUserForSave();
	       break;

	    case 8:

	       component_manager->UnbindEnclosedComponents();
	       SetPromptUserForSave();
	       break;
	       
	    default:
	       break;
	    }	// END switch( fl_dopup( iPopup ) )_show

	    button2_down = FALSE;
	    fl_freepup( iPopup );

	    break;

	 default:
	    break;
      
	 }  // END switch( iWhich_popup ) 
      
	 // end Button 3 code to be transferred to Button 2 --------------------------

   
	 break;
   
      
      default:
	 break;
      }		// END switch(ev.xbutton->button)
    
 
    
      break;

   case ButtonRelease:
      iMotion_direction = -1;

      switch( ev->xbutton.button ) {
      case Button1:
	 
	 button1_down = FALSE;
	 sampling_rate = user_rate;

	 if( etCurrent_tool == SELECT_TOOL )
	    component_manager->ResizeComponent();
	 else if( etCurrent_tool == COMPONENT_TOOL ) {
	    new_component nc;

	    if( xc > start_component_x ) {
	       nc.sx = start_component_x;
	       nc.x = xc;
	    } else {
	       nc.sx = xc;
	       nc.x = start_component_x;
	    }

	    if( start_component_y < yc ) {
	       nc.sy = start_component_y;
	       nc.y = yc;
	    } else {
	       nc.sy = yc;
	       nc.y = start_component_y;
	    }

	    SetPromptUserForSave();
	    component_manager->CreateNewComponent( nc );
	    
	 }
	 DrawScreen();  // new
	 break;

      case Button2:
      case Button3:

	 button2_down = FALSE;
	 break;

      default:
	 break;
      }
      break;
   }
    

   return 0;
}

int DrawingArea_motion( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d )
{
   int iXloc = ev->xbutton.x, iYloc = ev->xbutton.y;
   
   CalculateCoordinates( iXloc, iYloc );
   
   if( button1_down ) {
      if( etCurrent_tool == SELECT_TOOL ) {
	 if( ev->xbutton.state & ShiftMask ) {
	    if( iMotion_direction == -1 ) {
	       if( abs( iLast_x - xc ) > abs( iLast_y - yc ) )
		  iMotion_direction = 0;
	       else
		  iMotion_direction = 1;
	    } 
	 } 

	 if( iMotion_direction == 0 )
	    iYloc = iLast_y;
	 else if( iMotion_direction == 1 )
	    iXloc = iLast_x;

	 //	FIGURE_MOTION
  
	 if( (active_figure != NULL) && (active_component == NULL) && (FixedAllPositions == false) )
	 {
	    if( initial_button1_press ) {
	       button1_down = FALSE; 
	       SetPromptUserForSave();
	       button1_down = TRUE;
	       initial_button1_press = FALSE;
	    }
	    active_figure->SetPosition( x_coord, y_coord );
	    DrawScreen();
	 }

	 if( FixedAllPositions == false ) {
	    if( FixedAllComponents == false ) {
	       if( (active_component != NULL) && (active_figure == NULL) ) {
		  if( active_component->Fixed() == FALSE ) {
		     SetPromptUserForSave();
		     component_manager->MoveComponent( x_coord, y_coord );
		  }
	       }
	    }
	 }
	 
	 if( (active_figure == NULL) && (active_component == NULL) )
	    display_manager->DrawSelectionBox( ppr, start_selection_x, start_selection_y, x_coord, y_coord );
	 
      } else if( etCurrent_tool == COMPONENT_TOOL ) {

	 new_component nc;
	 
	 nc.sx = start_component_x;     nc.sy = start_component_y;
	 nc.cx = current_component_x;   nc.cy = current_component_y;
	 nc.x = xc;                     nc.y = yc;
	 
	 current_component_x = xc;
	 current_component_y = yc;

	 SetPromptUserForSave();
	 component_manager->DrawNewComponent( ppr, nc );
	 
      }
      
   }
   else if( button2_down ) {
      display_manager->ShiftSelectedElements( translate_diff_x( xc-iLast_x ), translate_diff_y( yc-iLast_y ) );
      iLast_x = xc;
      iLast_y = yc;
   }
   
   return 0;
}

extern "C"
void dummy_cb( FL_OBJECT *, long ){}

#ifdef TIME
long time_difference( struct timeval time1, struct timeval time2 )
{  
   if( time1.tv_sec == time2.tv_sec )
      return( time2.tv_usec - time1.tv_usec );
   else
      return( (time2.tv_sec - time1.tv_sec -1)*1000000 + time2.tv_usec + ( 1000000 - time1.tv_usec ) );
}

int get_cpu_time( void )
{
   struct rusage rusage;
   int cputime;
  
   if ( getrusage( RUSAGE_SELF, &rusage ) == -1 ) {
      printf( "\nUnable to get rusage.\n");
      exit( 1 );
   }
   cputime = (rusage.ru_utime.tv_sec + rusage.ru_stime.tv_sec) * 1e6
      + (rusage.ru_utime.tv_usec + rusage.ru_stime.tv_usec);

   return cputime;

}
#endif
