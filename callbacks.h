/***********************************************************
 *
 * File:			callbacks.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/


class DisplayManager;
class PathInstance;
class HyperedgeFigure;
class Component;

int DrawingArea_expose( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d );
int DrawingArea_key( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d );
int DrawingArea_but( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d );
int DrawingArea_motion( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d );
int DrawingArea_resize( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d );

void SetDisplayManager( DisplayManager *cman );
void SetCurrentPath( PathInstance *new_path );
void SetActiveFigure( HyperedgeFigure *new_figure );

