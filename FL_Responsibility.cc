
// callbacks for responsibility management functions

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "resp_ref.h"
#include "resp_mgr.h"
#include "display.h"
#include "component.h"
#include "variable.h"
#include <stdlib.h>

extern void DrawScreen();
void EditResponsibilityDesc( ResponsibilityReference *new_responsibility, bool new_ref = FALSE );
void EditResponsibility( bool path_reference );
void UpdateOperationsList();

extern void StructDynPopup( DynamicArrow * dynarrow, ResponsibilityReference *ref);
extern void EditDataStoreUsage( Responsibility *resp );
extern void EditServiceRequests( Cltn<ServiceRequest *> *requests, FL_FORM *parent_form, const char *element, const char *processor );
extern void SynchronizeResponsibilityDescriptions( Responsibility *resp );
extern void EditVariableOperations( Responsibility *responsibility, FL_FORM *parent );

extern void SetPromptUserForSave();

FD_EditRespPopup *pfderp = NULL;
int iWhich = -1;
int ERPvisible;

extern DisplayManager *display_manager;
Cltn<ResponsibilityReference *> *resp_references;
static ResponsibilityReference *prev_resp = NULL, *resp_ref = NULL, *resp = NULL;
static Cltn<Responsibility *> *existing_resp;
static Responsibility *existing_selected_resp = NULL, *current_responsibility;
bool added = TRUE;
static int mode;
static bool new_reference, name_validated, path_invoked;

static void InstallResponsibility( Responsibility *new_resp );
static void ClearDialog();
static void VerifyResponsibilityExistence();
static int ERDialogClose( FL_FORM *, void * );

void UpdateResponsibilityList()
{
   int line_count = 0;
   char line_buffer[64], min_label[LABEL_SIZE+1], *rd;
   ResponsibilityReference *min_resp_ref, *current_resp_ref;
   bool print_blank;
   resp_references = display_manager->ResponsibilityList();
   
   fl_freeze_form( pfdmMain->Main );
   fl_clear_browser( pfdmMain->Responsibilities );

   for( resp_references->First(); !resp_references->IsDone(); resp_references->Next() ) {
      resp = resp_references->CurrentItem();
      if( resp->ParentResponsibility() != NULL )
	 resp->ParentResponsibility()->ResetVisited();
   }
   
   do {
      
      min_resp_ref = NULL;
      strcpy( min_label, "zzzzzzzz" );
      
      for( resp_references->First(); !resp_references->IsDone(); resp_references->Next() ) {

	current_resp_ref  = resp_references->CurrentItem();

	 if( current_resp_ref->ParentResponsibility() == NULL )
	    continue; // skip over newly created responsibility references

	 if( current_resp_ref->ParentResponsibility()->Visited() )
	    continue; // skip over responsibilities already in the list

	 if( strcasecmp( current_resp_ref->HyperedgeName(), min_label ) < 0 ) {
	    min_resp_ref = current_resp_ref;
	    strcpy( min_label, min_resp_ref->HyperedgeName() );
	 }
      }

      if( min_resp_ref != NULL ) {
	 
	 min_resp_ref->ParentResponsibility()->SetVisited(); // set the visited flag for this responsibility
      
	 sprintf( line_buffer, "@b%s   ", min_resp_ref->ParentResponsibility()->Name() );
        
	 fl_add_browser_line( pfdmMain->Responsibilities, line_buffer );
	 line_count++;

	 if( min_resp_ref == resp_ref ) // set topline to recently edited responsibility
	    fl_set_browser_topline( pfdmMain->Responsibilities, line_count );
      
	 rd = min_resp_ref->ParentResponsibility()->Description();
	 print_blank = TRUE;
      
	 if( rd ) {  // print description if nonempty
	    fl_add_browser_line( pfdmMain->Responsibilities, rd );
	    line_count++;	 
	    while( *rd ) {
	       if( *rd == '\n' )
		  line_count++;
	       rd++;
	    }
	    if( rd[strlen(rd)-1] == '\n' )
	       print_blank = FALSE;
	 }

	 if( print_blank ) {
	    fl_add_browser_line( pfdmMain->Responsibilities, "" );  // add blank line if necessary
	    line_count++;
	 }
      }
      
   } while( min_resp_ref != NULL );

   DISABLE_WIDGET( pfdmMain->EditResponsibility);
   fl_unfreeze_form( pfdmMain->Main );
}

ResponsibilityReference * GetSelectedResponsibility()
{
   int i, title_lines = 0, selected_line = 0;
   char min_label[LABEL_SIZE+1];
   ResponsibilityReference *min_resp_ref, *current_resp_ref;
   
   for( i = 1; i <= fl_get_browser_maxline( pfdmMain->Responsibilities ); i++ ) {
      if( strncmp( fl_get_browser_line( pfdmMain->Responsibilities, i ), "@b", 2 ) == 0 )
	 title_lines++;

      if( fl_isselected_browser_line( pfdmMain->Responsibilities, i ) ) {
	 selected_line = title_lines;
	 break;
      }
   }

   if( selected_line == 0 ) return( NULL ); 
   i = 1; // find responsibility reference corresponding to selected ordinal
   
   for( resp_references->First(); !resp_references->IsDone(); resp_references->Next() ) {
      current_resp_ref = resp_references->CurrentItem();
      if( current_resp_ref->ParentResponsibility() != NULL )
	 current_resp_ref->ParentResponsibility()->ResetVisited();
   }

   do {
      min_resp_ref = NULL;
      strcpy( min_label, "zzzzzzzz" );
      
      for( resp_references->First(); !resp_references->IsDone(); resp_references->Next() ) {
	current_resp_ref  = resp_references->CurrentItem();
	 if( current_resp_ref->ParentResponsibility() == NULL )
	    continue; // skip over newly created responsibility references
	 if( current_resp_ref->ParentResponsibility()->Visited() )
	    continue; // skip over responsibilities already in the list
	 if( strcasecmp( current_resp_ref->HyperedgeName(), min_label ) < 0 ) {
	    min_resp_ref = current_resp_ref;
	    strcpy( min_label, min_resp_ref->HyperedgeName() );
	 }
      }
      if( min_resp_ref != NULL ) {
	 min_resp_ref->ParentResponsibility()->SetVisited(); // set the visited flag for this responsibility
	 if( i++ == selected_line )
	    return( min_resp_ref );
      }
   } while( min_resp_ref != NULL );

   return( NULL );
}

extern "C"
void EROKButton_cb( FL_OBJECT *, long )
{

   etResponsibility_direction erd;
   Responsibility *pr;
   char buffer[150], processed_name[32];
   const char *old_name = NULL,
      *new_name = fl_get_input( pfderp->Name );
   ResponsibilityManager *rm = ResponsibilityManager::Instance();

   if( current_responsibility != NULL && name_validated )
      old_name = current_responsibility->Name();

   if( !rm->UniqueResponsibilityName( new_name, old_name, processed_name ) ) {
      sprintf( buffer, "The name \"%s\"", processed_name );
      fl_set_resource( "flAlert.title", "Error: Duplicate Responsibility Name Entered" );
      fl_show_alert( buffer, "is not a unique responsibility name. Please enter a unique name.", "", 0 );
      return;
   }
   
   if( mode == EDIT_EXISTING )
      pr = resp_ref->ParentResponsibility();
   else if( mode == CREATE_NEW )
      pr = new Responsibility();
   else // INSTALL_EXISTING
      pr = existing_selected_resp;

   if( pr == NULL ) {
      fl_set_resource( "flAlert.title", "Error: Responsibility Not Selected" );
      fl_show_alert( "A parent responsibility has not been selected ", "Please select a responsibility.", "", 0 );
      return;
   }

   if( mode != EDIT_EXISTING )
      resp_ref->ParentResponsibility( pr );
   
   fl_hide_form( pfderp->EditRespPopup );
   ERPvisible = 0;
  
   pr->Name( processed_name );
   pr->Description( fl_get_input( pfderp->Description ) );
   pr->ExecutionSequence( fl_get_input( pfderp->ExecutionSequence ) );

   SynchronizeResponsibilityDescriptions( pr ); // synchronize description with description window if selected
   
   if( fl_get_button( pfderp->REUpButton ) )
      erd = RESP_UP;
   else if( fl_get_button( pfderp->REDownButton ) )
      erd = RESP_DOWN;
   else if( fl_get_button( pfderp->RELeftButton ) )
      erd = RESP_LEFT;
   else if( fl_get_button( pfderp->RERightButton ) )
      erd = RESP_RIGHT;

   ((ResponsibilityFigure *)resp_ref->GetFigure())->Direction( erd );
   if( new_reference ) resp_ref->Install(); // install the responsibility reference at the position of the empty point
   UpdateResponsibilityList();
   UNFREEZE_TOOL();
   SetPromptUserForSave();
   DrawScreen();
}

extern "C"
void ERCancelButton_cb( FL_OBJECT *, long )
{
   if( new_reference ) resp_ref->DeleteEdge(); // cancel the reference installation
   fl_hide_form( pfderp->EditRespPopup );
   ERPvisible = 0;
   UNFREEZE_TOOL();
   DrawScreen(); //  in case a Dynamic Pointer was added/removed and Cancel is clicked
}

extern "C"
void ERAddDPButton_cb(FL_OBJECT *, long)
{
   VerifyResponsibilityExistence();
   
   DynamicArrow *dynamic_arrow = new DynamicArrow( MOVE, OUTOF );
   current_responsibility->SetDynamicArrow( dynamic_arrow );
   StructDynPopup( dynamic_arrow, resp_ref );

   DISABLE_WIDGET( pfderp->ERAddDPButton );
   DISABLE_WIDGET( pfderp->EREditDPButton );
   DISABLE_WIDGET( pfderp->ERRemoveDPButton );

   SetPromptUserForSave();
}

extern "C"
void EREditDPButton_cb(FL_OBJECT *, long)
{
   if( current_responsibility == NULL ) return;
   
   DynamicArrow *dynamic_arrow = current_responsibility->GetDynamicArrow();
   if( dynamic_arrow )
      StructDynPopup( dynamic_arrow, resp_ref );
}

extern "C"
void ERRemoveDPButton_cb(FL_OBJECT *, long)
{
   if( current_responsibility == NULL ) return;
   
   DynamicArrow *da = current_responsibility->GetDynamicArrow();
   if( da ) delete da;
   current_responsibility->SetDynamicArrow( NULL );

   ENABLE_WIDGET( pfderp->ERAddDPButton );
   DISABLE_WIDGET( pfderp->EREditDPButton );
   DISABLE_WIDGET( pfderp->ERRemoveDPButton );

   SetPromptUserForSave();
}

extern "C"
void Responsibilities_cb( FL_OBJECT *pfl, long lData )
{
   int iSelected = fl_get_browser( pfdmMain->Responsibilities );
   int i;
   bool redraw = FALSE;
   
   fl_freeze_form( pfdmMain->Main ); 
  
   for( i = 1; i <= fl_get_browser_maxline( pfdmMain->Responsibilities ); i++ )
      fl_deselect_browser_line( pfdmMain->Responsibilities, i );

   if( iSelected > 0 ) {
      
      if( strncmp( fl_get_browser_line( pfdmMain->Responsibilities, iSelected ), "@b", 2 ) == 0 ) {
	 fl_select_browser_line( pfdmMain->Responsibilities, iSelected );
	 iSelected++;
      }
    
      for( i=iSelected; i >= 1; i-- ) {
	 if( strncmp( fl_get_browser_line( pfdmMain->Responsibilities, i ), "@b", 2 ) == 0 ) {
	    fl_select_browser_line( pfdmMain->Responsibilities, i );
	    break;
	 }
	 if( strcmp( fl_get_browser_line( pfdmMain->Responsibilities, i ), "" ) != 0 ) 
	    fl_select_browser_line( pfdmMain->Responsibilities, i );
      }
    
      for( i=iSelected; i <= fl_get_browser_maxline( pfdmMain->Responsibilities ); i++ ) {
	 if( strncmp( fl_get_browser_line( pfdmMain->Responsibilities, i ), "@b", 2 ) == 0 ) {
	    break;
	 }
	 if( strcmp( fl_get_browser_line( pfdmMain->Responsibilities, i ), "" ) != 0 ) 
	    fl_select_browser_line( pfdmMain->Responsibilities, i );
      }
   }

   if( resp_ref  && !added )
      prev_resp = resp_ref;
    
   resp_ref = GetSelectedResponsibility();

#if 0
   if( (resp_ref != prev_resp)  || added ){
      if( prev_resp )
	 ((ResponsibilityFigure *)prev_resp->GetFigure())->Unhighlight();
      if( resp_ref )
	 ((ResponsibilityFigure *)resp_ref->GetFigure())->Highlight();
      redraw = TRUE;
   }
#endif
   
   added = FALSE;

   if( resp_ref == NULL ) {
      DISABLE_WIDGET( pfdmMain->EditResponsibility );
   } else {
      ENABLE_WIDGET( pfdmMain->EditResponsibility );
   }
  
   fl_unfreeze_form( pfdmMain->Main );

   if( redraw )  
      DrawScreen();
 
}


extern "C"
void EditResponsibility_cb( FL_OBJECT *pfl, long lData )
{
   new_reference = FALSE;
   if( (resp_ref = GetSelectedResponsibility()) != NULL )
      EditResponsibility( FALSE );
}

void EditResponsibility( bool path_reference )
{
   int index;
   Responsibility *pr = resp_ref->ParentResponsibility();

   FREEZE_TOOL();
  
   if( !pfderp ) {
      pfderp = create_form_EditRespPopup();
      fl_set_form_atclose( pfderp->EditRespPopup, ERDialogClose, NULL );
      fl_set_browser_fontsize( pfderp->ExistingRespBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfderp->ExistingRespBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfderp->VariableSettings, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfderp->VariableSettings, FL_BOLD_STYLE );
   }
   
   fl_set_button( pfderp->RERightButton, 0 );
   fl_set_button( pfderp->RELeftButton, 0 );
   fl_set_button( pfderp->REUpButton, 0 );
   fl_set_button( pfderp->REDownButton, 0 );

   switch( ((ResponsibilityFigure *)resp_ref->GetFigure())->Direction() ) {
   case RESP_UP:
      fl_set_button( pfderp->REUpButton, 1 );
      break;
   case RESP_DOWN:
      fl_set_button( pfderp->REDownButton, 1 );
      break;
   case RESP_LEFT:
      fl_set_button( pfderp->RELeftButton, 1 );
      break;
   case RESP_RIGHT:
      fl_set_button( pfderp->RERightButton, 1 );
      break;
   }

   // fill list of existing responsibilities
   existing_resp = ResponsibilityManager::Instance()->ResponsibilityList();
   
   fl_clear_browser( pfderp->ExistingRespBrowser );
   for( existing_resp->First(); !existing_resp->IsDone(); existing_resp->Next() )
      fl_add_browser_line( pfderp->ExistingRespBrowser, existing_resp->CurrentItem()->Name() );

   if( pr != NULL ) {
      InstallResponsibility( pr );
      mode = EDIT_EXISTING;
      DISABLE_WIDGET( pfderp->ModeButtons );
      index = existing_resp->in( pr ) + 1;   // highlight proper name in list
      fl_select_browser_line( pfderp->ExistingRespBrowser, index );
      fl_set_browser_topline( pfderp->ExistingRespBrowser, index );
      fl_set_button( pfderp->CNRButton, 0 );
      fl_set_button( pfderp->IERButton, 0 );      
      DISABLE_WIDGET( pfderp->ModeButtons );
   }
   else {
      ClearDialog();
      mode = CREATE_NEW;
      ENABLE_WIDGET( pfderp->ModeButtons );
      fl_set_button( pfderp->CNRButton, 1 );
      fl_set_button( pfderp->IERButton, 0 );      
   }

   // deactivate browser as default will be create new responsibility if responsibility nonexistent or edit existing if it exists
   DISABLE_WIDGET( pfderp->ExistingRespBrowser );

   UpdateOperationsList();
   
   fl_show_form( pfderp->EditRespPopup, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, "Edit Responsibility" );

   path_invoked = path_reference;
   ERPvisible = 1;

}

void EditResponsibilityDesc( ResponsibilityReference *new_responsibility, bool new_ref )
{
   if( resp_ref )
      prev_resp = resp_ref;
   resp_ref = new_responsibility;
   new_reference = new_ref;
   EditResponsibility( TRUE );
   added = TRUE;
}

extern "C"
void IERButton_cb(FL_OBJECT *, long)
{
   if( mode != INSTALL_EXISTING ) {
      mode = INSTALL_EXISTING;
      ENABLE_WIDGET( pfderp->ExistingRespBrowser );
      fl_deselect_browser( pfderp->ExistingRespBrowser );
      ClearDialog();
   }
}

extern "C"
void CNRButton_cb(FL_OBJECT *, long)
{
   if( mode != CREATE_NEW ) {
      mode = CREATE_NEW;
      existing_selected_resp = NULL;
      fl_deselect_browser( pfderp->ExistingRespBrowser );
      DISABLE_WIDGET( pfderp->ExistingRespBrowser );
      ClearDialog();
   }
}

extern "C"
void ExistingRespBrowser_cb(FL_OBJECT *, long)
{
   int selection = fl_get_browser( pfderp->ExistingRespBrowser );

   if( selection <= 0 ) {
      existing_selected_resp = NULL;
      return;
   }

   existing_selected_resp = existing_resp->Get( selection );
   InstallResponsibility( existing_selected_resp );
}

extern "C"
void EditSettingsButton_cb(FL_OBJECT *, long)
{
   VerifyResponsibilityExistence();
   EditVariableOperations( current_responsibility, pfderp->EditRespPopup );
}

extern "C"
void ERDataStoresButton_cb(FL_OBJECT *, long)
{
   VerifyResponsibilityExistence();
   EditDataStoreUsage( current_responsibility );
}

extern "C"
void ERServiceRequestButton_cb(FL_OBJECT *, long)
{
   char title[100];
   const char *processor = NULL;
   Component *bound_component;

   VerifyResponsibilityExistence();

   if( !path_invoked ) {
      if( !resp_ref->ParentMap()->SingleProcessorResponsibility( current_responsibility ) )
	 processor = "varied processors";
   }

   if( processor == NULL ) {
      if( (bound_component = resp_ref->GetFigure()->BoundComponent()) != NULL )
	 processor = bound_component->ProcessorName();
   }

   if( processor == NULL )
      processor = "processor unbound";

   sprintf( title, "Responsibility %s", fl_get_input( pfderp->Name ));
   EditServiceRequests( current_responsibility->ServiceRequests(), pfderp->EditRespPopup, title, processor );
}

int ERDialogClose( FL_FORM *, void * )
{
   ERCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}

void InstallResponsibility( Responsibility *new_resp )
{
   char *cp;

   current_responsibility = new_resp;
   name_validated = TRUE;
   
   if( current_responsibility->HasDynarrow() ) {
      DISABLE_WIDGET( pfderp->ERAddDPButton );
      ENABLE_WIDGET( pfderp->EREditDPButton );
      ENABLE_WIDGET( pfderp->ERRemoveDPButton );
   }
   else {
      ENABLE_WIDGET( pfderp->ERAddDPButton );
      DISABLE_WIDGET( pfderp->EREditDPButton );
      DISABLE_WIDGET( pfderp->ERRemoveDPButton );
   }
   
   fl_set_input( pfderp->Name, current_responsibility->Name() );

   cp = (char *)((current_responsibility->Description() != NULL) ? current_responsibility->Description() : "" );
   fl_set_input( pfderp->Description, cp );   
   cp = (char *)((current_responsibility->ExecutionSequence() != NULL) ? current_responsibility->ExecutionSequence() : "" );
   fl_set_input( pfderp->ExecutionSequence, cp );

}

void ClearDialog()
{
   fl_set_input( pfderp->Name, "" );
   fl_set_input( pfderp->Description, "" );   
   fl_set_input( pfderp->ExecutionSequence, "" );
   ENABLE_WIDGET( pfderp->ERAddDPButton );
   DISABLE_WIDGET( pfderp->EREditDPButton );
   DISABLE_WIDGET( pfderp->ERRemoveDPButton );

   current_responsibility = NULL;
}

void VerifyResponsibilityExistence()
{
   if( current_responsibility ) return;

   current_responsibility = new Responsibility();
   resp_ref->ParentResponsibility( current_responsibility );
   mode = EDIT_EXISTING;
   name_validated = FALSE;
}

void UpdateOperationsList()
{
   char buffer[300];
   Cltn<VariableOperation *> *vo;
   VariableOperation *cvo;
   char *logical_expression;

   fl_clear_browser( pfderp->VariableSettings );

   if( current_responsibility == NULL ) return;
   vo = current_responsibility->VariableOperations();

   for( vo->First(); !vo->IsDone(); vo->Next() ) {
      cvo = vo->CurrentItem();
      if( cvo->Value() != EVALUATED_EXPRESSION )
	 sprintf( buffer, "%s -> %s", cvo->Variable()->BooleanName(), ((cvo->Value() == TRUE) ? "T" : "F" ));
      else {
	 logical_expression = cvo->LogicalExpression();
	 sprintf( buffer, "%s -> EVAL(%s)", cvo->Variable()->BooleanName(), logical_expression );
	 free( logical_expression );
      }
      fl_add_browser_line( pfderp->VariableSettings, buffer );
   }
}

