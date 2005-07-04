 // callbacks for component and responsibility reference replacement dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "display.h"
#include "component.h"
#include "component_mgr.h"
#include "responsibility.h"
#include "resp_mgr.h"

extern DisplayManager *display_manager;
extern ComponentManager *component_manager;
extern void DrawScreen();
extern void UpdateResponsibilityList();
extern void SetPromptUserForSave();

static FD_ReferenceReplacement *pfdrr = NULL;
static reference_type rtype;
static int selected_count;
static Cltn<Component *> *existing_components;
static Cltn<Responsibility *> *existing_resp;

void UpdateReferenceDisplay();
static int RRDialogClose( FL_FORM *, void * );

static char dialog_prompt[][35] = {
   "Replace Component References",
   "Replace Responsibility References" };

void ReplaceReferences( reference_type type )
{
   rtype = type;
   FREEZE_TOOL();

   if( !pfdrr ) {
      pfdrr = create_form_ReferenceReplacement();
      fl_set_form_atclose( pfdrr->ReferenceReplacement, RRDialogClose, NULL );
      fl_set_browser_fontsize( pfdrr->ReplaceList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdrr->ReplaceList, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdrr->ReplacingReference, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdrr->ReplacingReference, FL_BOLD_STYLE );
   }

   UpdateReferenceDisplay();
   fl_show_form( pfdrr->ReferenceReplacement, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, dialog_prompt[rtype] );
}

void UpdateReferenceDisplay()
{
   fl_clear_browser( pfdrr->ReplaceList );
   fl_clear_browser( pfdrr->ReplacingReference );
   DISABLE_WIDGET( pfdrr->Replace );
   DISABLE_WIDGET( pfdrr->Deselect );
   selected_count = 0;

   if( rtype == COMPONENT_DEF ) {
      existing_components = component_manager->ComponentList();
      for( existing_components->First(); !existing_components->IsDone(); existing_components->Next() ) {
	 fl_add_browser_line( pfdrr->ReplaceList, existing_components->CurrentItem()->GetLabel() );
	 fl_add_browser_line( pfdrr->ReplacingReference, existing_components->CurrentItem()->GetLabel() );
      }
   }
   else { // RESPONSIBILITY_DEF
      existing_resp = ResponsibilityManager::Instance()->ResponsibilityList();
      for( existing_resp->First(); !existing_resp->IsDone(); existing_resp->Next() ) {
	 fl_add_browser_line( pfdrr->ReplaceList, existing_resp->CurrentItem()->Name() );
	 fl_add_browser_line( pfdrr->ReplacingReference, existing_resp->CurrentItem()->Name() );
      }
   }
}

extern "C"
void ReplacingReference_cb(FL_OBJECT *, long)
{
   if( selected_count > 0 ) {
      ENABLE_WIDGET( pfdrr->Replace );
   }
}

extern "C"
void ReplaceList_cb(FL_OBJECT *list, long)
{
   if( fl_get_browser( list ) > 0 )
      selected_count++;
   else
      selected_count--;

   if( selected_count > 0 ) {   
      ENABLE_WIDGET( pfdrr->Deselect );
      if( fl_get_browser( pfdrr->ReplacingReference ) > 0 ) {
	 ENABLE_WIDGET( pfdrr->Replace );
      }
   } else {
      DISABLE_WIDGET( pfdrr->Replace );
      DISABLE_WIDGET( pfdrr->Deselect );
   }
}

extern "C"
void Deselect_cb(FL_OBJECT *, long)
{ // deselect all of the selected lines
   fl_deselect_browser( pfdrr->ReplaceList );
   DISABLE_WIDGET( pfdrr->Deselect );
   selected_count = 0;   
}

extern "C"
void Replace_cb(FL_OBJECT *, long)
{
   int i, j = 0, line_count = fl_get_browser_maxline( pfdrr->ReplaceList );

   if( rtype == COMPONENT_DEF ) {

      Component *replacing_component = existing_components->Get( fl_get_browser( pfdrr->ReplacingReference ) );
      Component **replaced_components = (Component**)malloc(selected_count*sizeof(Component *));

      for( i = 1; i <= line_count; i++ ) {
	 if( fl_isselected_browser_line( pfdrr->ReplaceList, i ) )     // fill list of components to be replaced
	     replaced_components[j++] = existing_components->Get( i );
      }

      for( i = 0; i < selected_count; i++ )
	 display_manager->ReplaceComponent( replaced_components[i], replacing_component );

      free( replaced_components );
   }
   else { // RESPONSIBILITY_DEF

      Responsibility *replacing_responsibility = existing_resp->Get( fl_get_browser( pfdrr->ReplacingReference ) );
      Responsibility **replaced_responsibilities = (Responsibility**)malloc(selected_count*sizeof(Responsibility *));

      for( i = 1; i <= line_count; i++ ) {
	 if( fl_isselected_browser_line( pfdrr->ReplaceList, i ) )     // fill list of components to be replaced
	     replaced_responsibilities[j++] = existing_resp->Get( i );
      }

      for( i = 0; i < selected_count; i++ )
	 display_manager->ReplaceResponsibility( replaced_responsibilities[i], replacing_responsibility );

      free( replaced_responsibilities );
   }

   UpdateReferenceDisplay();
   SetPromptUserForSave();
}

extern "C"
void Dismiss_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdrr->ReferenceReplacement );
   UNFREEZE_TOOL();
   DrawScreen();
   if( rtype == RESPONSIBILITY_DEF )
      UpdateResponsibilityList();
}

int RRDialogClose( FL_FORM *, void * )
{
   Dismiss_cb( 0, 0 );
   return( FL_IGNORE );
}
