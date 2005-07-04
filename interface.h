
// common includes for XForms GUI C files and files accesing gui

#ifndef INTERFACE_H
#define INTERFACE_H

extern FD_Main *pfdmMain;
extern int freeze_editor;

#define FREEZE_TOOL() \
fl_deactivate_form( pfdmMain->Main ); \
freeze_editor = TRUE \

#define UNFREEZE_TOOL() \
fl_activate_form( pfdmMain->Main ); \
freeze_editor = FALSE \

#define DISABLE_WIDGET(w) \
fl_set_object_lcol( w, FL_INACTIVE ); \
fl_deactivate_object( w ) \

#define ENABLE_WIDGET(w) \
fl_activate_object( w ); \
fl_set_object_lcol( w, FL_BLACK ) \

#endif
