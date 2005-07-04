
// callbacks for stub management functions

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "utilities.h"
#include "stub.h"
#include "map.h"
#include "display.h"
#include "component.h"

extern DisplayManager *display_manager;
extern const char * EditLogicalCondition( const char *condition );

typedef struct {
   Node *node;
   int identifier;
   bool bound;
} boundary;

extern void DrawScreen();
extern void SetPromptUserForSave();
extern void EditServiceRequests( Cltn<ServiceRequest *> *requests, FL_FORM *parent_form, const char *element, const char *processor );

void EditStubDescription( Stub *new_stub, bool ns = FALSE );
void SelectStubSubmap( Stub *stub, int operation, Empty *empty = NULL );
void InstallExistingStubExpansion( Stub *stub, bool copy = FALSE );
void BindStubPlugin( Stub *stub, Map *plugin );
void UpdateBindingsList();
void ValidateEntryOperation();
void ValidateExitOperation();
void UpdateContinuityList();

void StubLabelOKButton_cb(FL_OBJECT *, long);
void StubLabelCancelButton_cb(FL_OBJECT *, long);

FD_StubCreationPopup *pfdscp = NULL;
FD_PluginChoicePopup *pfdpcp = NULL;
FD_StubBindingPopup *pfdsbp = NULL;

static Stub *current_stub = NULL;
static bool new_install;
static design_plugin plugins[50];
static boundary stub_inputs[20], stub_outputs[20];
static Map *current_plugin;
static PluginBinding *current_binding;
static char *current_lcondition = NULL;
static int stub_operation, num_inputs, num_outputs;
static bool bind_hidden = FALSE;
static bool unbound_input;
static Empty *current_input = NULL;
static int prev_choice = 0;
static bool probability_modified = FALSE;

int SCPvisible, PCPvisible, SBPvisible;

static int StubDialogClose( FL_FORM *, void * );
static int PluginSelectorDialogClose( FL_FORM *, void * );
static int StubBindingsDialogClose( FL_FORM *, void * );

extern Component *active_component;

void EditStubDescription( Stub *new_stub, bool ns )
{
   Cltn<Node *> *source, *target;
   
   current_stub = new_stub;
   new_install = ns;
   source = current_stub->SourceSet();
   target = current_stub->TargetSet();
   
   FREEZE_TOOL();

   if( !pfdscp ) {
      pfdscp = create_form_StubCreationPopup();
      fl_set_form_atclose( pfdscp->StubCreationPopup, StubDialogClose, NULL );
      fl_set_browser_fontsize( pfdscp->ContinuityBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdscp->ContinuityBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdscp->InputsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdscp->InputsBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdscp->OutputsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdscp->OutputsBrowser, FL_BOLD_STYLE );
   }

   SCPvisible = 1;
   
   fl_set_input( pfdscp->StubLabel, current_stub->StubName() );
   fl_set_input( pfdscp->SelectionPolicy, current_stub->SelectionPolicy() != NULL ? current_stub->SelectionPolicy() : "" );
   fl_set_button( pfdscp->SharedButton, current_stub->IsShared() );
   
   if( current_stub->GetType() == STATIC ) {
      fl_set_button( pfdscp->StaticButton, 1 );
      fl_set_button( pfdscp->DynamicButton, 0 );
   } else {  // DYNAMIC
      fl_set_button( pfdscp->DynamicButton, 1);
      fl_set_button( pfdscp->StaticButton, 0 );
   }

   if( current_stub->HasMultipleSubmaps() ) {
      DISABLE_WIDGET( pfdscp->StubTypeButtons );
   } else {
      ENABLE_WIDGET( pfdscp->StubTypeButtons );
   }
   
   num_inputs = 0;
   num_outputs = 0;
   
   for( source->First(); !source->IsDone(); source->Next() ) {
      stub_inputs[num_inputs].node = source->CurrentItem();
      stub_inputs[num_inputs].identifier = num_inputs+1;
      stub_inputs[num_inputs++].bound = FALSE;
   }

   for( target->First(); !target->IsDone(); target->Next() ) {
      stub_outputs[num_outputs].node = target->CurrentItem();
      stub_outputs[num_outputs].identifier = num_outputs+1;
      stub_outputs[num_outputs++].bound = FALSE;
   }

   UpdateContinuityList();
   fl_show_form( pfdscp->StubCreationPopup, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT,
		 (ns == TRUE) ? "Create Stub" : "Edit Stub" );
}

void UpdateContinuityList()
{
   Node *in, *on;
   int input_index, output_index, i;
   char buffer[50];
   Cltn<ContinuityBinding *> *cb = current_stub->ContinuityBindings();
 
   fl_freeze_form( pfdscp->StubCreationPopup );

   fl_clear_browser( pfdscp->ContinuityBrowser );
   fl_clear_browser( pfdscp->InputsBrowser );
   fl_clear_browser( pfdscp->OutputsBrowser );

   for( cb->First(); !cb->IsDone(); cb->Next() ) {
       
      in = cb->CurrentItem()->input_node;  // create list of continuity bindings
      on = cb->CurrentItem()->output_node; // and determine unbound input/output points

      for( i = 0; i < num_inputs; i++ ) {
	 if( stub_inputs[i].node == in ) {
	    input_index = i;
	    break;
	 }
      }
      
      for( i = 0; i < num_outputs; i++ ) {
	 if( stub_outputs[i].node == on ) {
	    output_index = i;
	    break;
	 }
      }
      
      stub_inputs[input_index].bound = TRUE;
      stub_outputs[output_index].bound = TRUE;
      
      sprintf( buffer, "IN%d <-> OUT%d", stub_inputs[input_index].identifier, stub_outputs[output_index].identifier );
      fl_addto_browser( pfdscp->ContinuityBrowser, buffer );
   }

   unbound_input = FALSE;
   for( i = 0; i < num_inputs; i++ ) {  // create unbound inputs list
      if( stub_inputs[i].bound == FALSE ) {
	 sprintf( buffer, "IN%d", stub_inputs[i].identifier );
	 fl_addto_browser( pfdscp->InputsBrowser, buffer );
	 unbound_input = TRUE;
      }
   }
      
   for( i = 0; i < num_outputs; i++ ) {  // create unbound outputs list
      if( stub_outputs[i].bound == FALSE || unbound_input ) {
	 sprintf( buffer, "OUT%d", stub_outputs[i].identifier );
	 fl_addto_browser( pfdscp->OutputsBrowser, buffer );
      }
   }
   
   DISABLE_WIDGET( pfdscp->Bind );
   DISABLE_WIDGET( pfdscp->Unbind );
   
   fl_unfreeze_form( pfdscp->StubCreationPopup );
}

extern "C"
void ContinuityBrowser_cb(FL_OBJECT *, long)
{
   if( fl_get_browser( pfdscp->ContinuityBrowser ) != 0) {
      ENABLE_WIDGET(  pfdscp->Unbind );
   }
}

extern "C"
void InputOutputBrowsers_cb(FL_OBJECT *, long)
{
   if( (fl_get_browser( pfdscp->InputsBrowser ) != 0) &&
       (fl_get_browser( pfdscp->OutputsBrowser ) != 0 )) {
      ENABLE_WIDGET(  pfdscp->Bind );
   }
}

extern "C"
void Unbind_cb(FL_OBJECT *, long)
{
   int binding, i;
   ContinuityBinding *selected_binding;
   Cltn<ContinuityBinding *> *cb = current_stub->ContinuityBindings();

   if( (binding = fl_get_browser( pfdscp->ContinuityBrowser )) == 0 )
      return;

   selected_binding = cb->Get( binding );

   for( i = 0; i < num_inputs; i++ ) {
      if( stub_inputs[i].node ==  selected_binding->input_node ) {
	 stub_inputs[i].bound = FALSE;
	 break;
      }
   }

   for( i = 0; i < num_outputs; i++ ) {
      if( stub_outputs[i].node ==  selected_binding->output_node ) {
	 stub_outputs[i].bound = FALSE;
	 break;
      }
   }


   cb->Delete( selected_binding );
   UpdateContinuityList();
   SetPromptUserForSave();

}

extern "C"
void Bind_cb(FL_OBJECT *, long)
{
   int input_selection, output_selection, i, j;
   Node *in, *on;

   input_selection = fl_get_browser( pfdscp->InputsBrowser );
   output_selection = fl_get_browser( pfdscp->OutputsBrowser );

   j = 0;
   for( i = 0; i < num_inputs; i++ ) {
      if( stub_inputs[i].bound == FALSE ) {
	 j++;
	 if( j == input_selection  ) { 
	    in = stub_inputs[i].node;
	    break;
	 }
      }
   }
      
   j = 0;
   for( i = 0; i < num_outputs; i++ ) {
      if( stub_outputs[i].bound == FALSE || unbound_input  ) {
	 j++;
	 if( j == output_selection  ) { 
	    on = stub_outputs[i].node;
	    break;
	 }
      }
   }
      
   current_stub->AddContinuityBinding( in, on );
   UpdateContinuityList();
   SetPromptUserForSave();
}

extern "C"
void StubLabelOKButton_cb(FL_OBJECT *, long)
{
   stub_type type;
   
   fl_hide_form( pfdscp->StubCreationPopup );
   SCPvisible = 0;
   
   current_stub->SetName( fl_get_input( pfdscp->StubLabel ) );   
   type = fl_get_button( pfdscp->DynamicButton ) ? DYNAMIC : STATIC;
   current_stub->SetType( type );
   current_stub->Shared( fl_get_button( pfdscp->SharedButton ) );
   current_stub->SelectionPolicy( fl_get_input( pfdscp->SelectionPolicy ) );
   
   UNFREEZE_TOOL();
   SetPromptUserForSave();
   DrawScreen();

}

extern "C"
void StubLabelCancelButton_cb(FL_OBJECT *, long)
{
   if( new_install ) { // cancel the stub installation
      current_stub->DeleteStub( TRUE );
      DrawScreen();
   }

   fl_hide_form( pfdscp->StubCreationPopup );
   UNFREEZE_TOOL();
   SCPvisible = 0;
}

extern "C"
void SDServiceRequestButton_cb(FL_OBJECT *, long)
{
   char title[100];
   const char *processor = NULL;
   Component *bound_component;

   if( (bound_component = current_stub->GetFigure()->BoundComponent()) != NULL )
      processor = bound_component->ProcessorName();

   if( processor == NULL )
      processor = "processor unbound";

   sprintf( title, "Stub %s", fl_get_input( pfdscp->StubLabel ));
   EditServiceRequests( current_stub->ServiceRequests(), pfdscp->StubCreationPopup, title, processor );
}

void CreatePluginChoicePopup()
{
   if( !pfdpcp ) {
      pfdpcp = create_form_PluginChoicePopup();
      fl_set_browser_fontsize( pfdpcp->PluginSelector, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdpcp->PluginSelector, FL_BOLD_STYLE );
      fl_set_browser_dblclick_callback( pfdpcp->PluginSelector, PluginChoiceOKButton_cb, 0 );
      fl_set_browser_fontsize( pfdpcp->SelectionPolicy, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdpcp->SelectionPolicy, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdpcp->LogicalCondition, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdpcp->LogicalCondition, FL_BOLD_STYLE );
      fl_set_form_atclose( pfdpcp->PluginChoicePopup, PluginSelectorDialogClose, NULL );
      fl_set_input_return( pfdpcp->Probability, FL_RETURN_CHANGED );
   }

   // fill selection policy display
   fl_clear_browser( pfdpcp->SelectionPolicy );
   if( current_stub->SelectionPolicy() != NULL )
      fl_add_browser_line( pfdpcp->SelectionPolicy, current_stub->SelectionPolicy() );
   fl_clear_browser( pfdpcp->LogicalCondition );
   fl_set_input( pfdpcp->Probability, "" );
   probability_modified = FALSE;
   current_binding = NULL;
}

void SelectStubSubmap( Stub *stub, int operation, Empty *empty )
{
   Cltn<Map *> *submaps;
   
   prev_choice = 0;
   current_stub = stub;
   current_input = empty;
   submaps = stub->Submaps();
   
   FREEZE_TOOL();
   stub_operation = operation;
   
   CreatePluginChoicePopup();
   fl_clear_browser( pfdpcp->PluginSelector );

   if( stub_operation == CHOOSE_SUBMAP ) {
      fl_set_object_label( pfdpcp->PluginChoiceOKButton, "Install" );
      if( bind_hidden ) {
	 fl_show_object( pfdpcp->BindPluginButton );
	 bind_hidden = FALSE;
      }
      DISABLE_WIDGET( pfdpcp->BindPluginButton );
   }
   else {
      fl_set_object_label( pfdpcp->PluginChoiceOKButton, "Remove" );
      if( !bind_hidden ) {
	 fl_hide_object( pfdpcp->BindPluginButton );
	 bind_hidden = TRUE;
      }
   }
	 
   for( submaps->First(); !submaps->IsDone(); submaps->Next() ) {
      if( !display_manager->MapViewable( stub, submaps->CurrentItem() ) )
	 continue; // implement restricted navigation for ucm sets and scenario trace modes     
      fl_add_browser_line( pfdpcp->PluginSelector, submaps->CurrentItem()->MapLabel() );
   }
   
   // disable buttons initially until something is selected
   DISABLE_WIDGET( pfdpcp->PluginChoiceOKButton );
   DISABLE_WIDGET( pfdpcp->RenameButton );
   
   fl_show_form( pfdpcp->PluginChoicePopup, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT, "Choose Plugin" );
   PCPvisible = 1;
}

extern "C"
void PluginChoiceOKButton_cb(FL_OBJECT *, long)
{
   int choice;

   fl_hide_form( pfdpcp->PluginChoicePopup );
   PCPvisible = 0;

   if( (choice = fl_get_browser( pfdpcp->PluginSelector )) == 0 ) {   
      UNFREEZE_TOOL();
      return;
   }

   if( stub_operation == INSTALL_EXISTING || stub_operation == COPY_EXISTING ) {
      if( plugins[choice-1].parent == TRUE ) {
	 if( !QuestionDialog( "Confirm Plugin Recursion", "The selected submap is a parent of the current map.",
			      "Installing it will result in recursion. Do you wish to do this ?", "Yes, Install Submap", "No, Cancel Install", FALSE )) {
	    UNFREEZE_TOOL();
	    return;
	 }
      }
   }
   
   if( stub_operation == INSTALL_EXISTING )
   {
      Map *selected_map = plugins[choice-1].plugin;
      current_stub->InstallNewSubmap( selected_map );
   }
   else if( stub_operation == COPY_EXISTING )
   {
      Map *selected_map = plugins[choice-1].plugin;
      current_stub->InstallSubmapImage( selected_map );
   }
   else if( stub_operation == CHOOSE_SUBMAP )
      current_stub->SetSelectedMap( display_manager->ViewableMap( current_stub, choice ), current_input ); // add restricted navigation
   else if( stub_operation == POOL_CHOICE )
   {
//      active_component->
   }
   else   // remove submap operation
      current_stub->RemovePlugin( display_manager->ViewableMap( current_stub, choice ) );
   
   if( probability_modified ) {
      current_binding->probability = atof( fl_get_input( pfdpcp->Probability ) );
      probability_modified = FALSE;
   }

   UNFREEZE_TOOL();
   SetPromptUserForSave();
}

void PICCancelButton_cb(FL_OBJECT *, long)
{
   if( current_lcondition != NULL ) {
      free( current_lcondition );
      current_lcondition = NULL;
   }

   if( probability_modified ) {
      current_binding->probability = atof( fl_get_input( pfdpcp->Probability ) );
      probability_modified = FALSE;
      SetPromptUserForSave();
   }

   fl_hide_form( pfdpcp->PluginChoicePopup );
   PCPvisible = 0;
   UNFREEZE_TOOL();
}

extern "C"
void PluginSelector_cb(FL_OBJECT *pflo, long)
{
   int choice;
   char *condition, probability[10];

   if( (choice = fl_get_browser( pfdpcp->PluginSelector )) == 0 ) return;
   if( choice == prev_choice )
      return;
   else
      prev_choice = choice;

   fl_clear_browser( pfdpcp->LogicalCondition );

   if( stub_operation == CHOOSE_SUBMAP ) {

      if( probability_modified ) {
	 current_binding->probability = atof( fl_get_input( pfdpcp->Probability ) );
	 probability_modified = FALSE;
	 SetPromptUserForSave();   
      }

      current_plugin = display_manager->ViewableMap( current_stub, choice );
      current_binding = current_stub->SubmapBinding( current_plugin );

      if( (condition = current_binding->LogicalCondition()) != NULL )
	 fl_add_browser_line( pfdpcp->LogicalCondition, condition );

      if( current_lcondition != NULL ) free( current_lcondition );
      current_lcondition = condition;

      sprintf( probability, "%f", current_binding->probability );
      fl_set_input( pfdpcp->Probability, probability );
   }

   // enable buttons as something was selected
   ENABLE_WIDGET( pfdpcp->PluginChoiceOKButton );
   if( stub_operation != COPY_EXISTING ) { // keep rename disabled for image operations
      ENABLE_WIDGET( pfdpcp->RenameButton );
   }

   if( !bind_hidden ) {
      ENABLE_WIDGET( pfdpcp->BindPluginButton );
   }
}

extern "C"
void Probability_cb(FL_OBJECT *, long)
{
   if( current_binding != NULL )
      probability_modified = TRUE;
}

extern "C"
void PICEditConditionButton_cb(FL_OBJECT *, long)
{
   const char *edited_condition;

   if( fl_get_browser( pfdpcp->PluginSelector ) == 0 ) return;

   if( (edited_condition = EditLogicalCondition( current_lcondition )) != NULL ) {
      if( current_lcondition != NULL ) free( current_lcondition );
      current_lcondition = strdup( edited_condition );
      current_binding->LogicalCondition( edited_condition );
      fl_clear_browser( pfdpcp->LogicalCondition );
      fl_add_browser_line( pfdpcp->LogicalCondition, edited_condition );
      SetPromptUserForSave();
   }
}

extern "C"
void PICRenameButton_cb( FL_OBJECT *, long )
{
   Map *selected_map;
   
   int choice = fl_get_browser( pfdpcp->PluginSelector );
   if( choice == 0 ) return;
   
   if( stub_operation == INSTALL_EXISTING )
      selected_map = plugins[choice-1].plugin;
   else if( stub_operation == CHOOSE_SUBMAP )
      selected_map = current_stub->Submaps()->Get( choice );
   else
      return;
   
   const char *entered_label = display_manager->UniqueMapName( "Enter the new name for the plugin. ", selected_map->MapLabel() );
   if( entered_label )     //  if == NULL, then the Cancel button was hit and we don't want to change anything
   {
      selected_map->MapLabel( entered_label );
      fl_replace_browser_line( pfdpcp->PluginSelector, fl_get_browser( pfdpcp->PluginSelector ), entered_label );
      SetPromptUserForSave();
   }

}

void BindPluginButton_cb(FL_OBJECT *, long)
{
   if( fl_get_browser( pfdpcp->PluginSelector ) == 0 ) return;

   fl_hide_form( pfdpcp->PluginChoicePopup );
   PCPvisible = 0;
   UNFREEZE_TOOL();
   current_stub->InitiatePathBinding( current_plugin );
   DrawScreen();
   BindStubPlugin( current_stub, current_plugin );
}

void InstallExistingStubExpansion( Stub *stub, bool copy )
{
   int number;
   
   prev_choice = 0;
   current_stub = stub;
   display_manager->Submaps( plugins, number, stub );

   FREEZE_TOOL();
   stub_operation = copy ? COPY_EXISTING : INSTALL_EXISTING;
   
   CreatePluginChoicePopup();
   fl_clear_browser( pfdpcp->PluginSelector );
   fl_set_object_label( pfdpcp->PluginChoiceOKButton, "Install" );

   for( int i = 0; i < number; i++ )
      fl_addto_browser( pfdpcp->PluginSelector, plugins[i].plugin->MapLabel());

   if( !bind_hidden ) {
      fl_hide_object( pfdpcp->BindPluginButton );
      bind_hidden = TRUE;
   }

   // disable buttons initially until something is selected
   DISABLE_WIDGET( pfdpcp->PluginChoiceOKButton );
   DISABLE_WIDGET( pfdpcp->RenameButton );
   
   fl_show_form( pfdpcp->PluginChoicePopup, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT, "Choose Plugin" );
   PCPvisible = 1;
}

void BindStubPlugin( Stub *stub, Map *plugin )
{
   char buffer[100], enforced[50];
   Cltn<ContinuityBinding *> *cb;
   
   current_stub = stub;

   FREEZE_TOOL();

   if( !pfdsbp ) {
      pfdsbp = create_form_StubBindingPopup();
      fl_set_form_atclose( pfdsbp->StubBindingPopup, StubBindingsDialogClose, NULL );
      fl_set_browser_fontsize( pfdsbp->StubBindingsList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsbp->StubBindingsList, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdsbp->StubEntryPointsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsbp->StubEntryPointsBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdsbp->StubExitPointsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsbp->StubExitPointsBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdsbp->PluginEntryPointsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsbp->PluginEntryPointsBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdsbp->PluginExitPointsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsbp->PluginExitPointsBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdsbp->EnforcedBindings, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdsbp->EnforcedBindings, FL_BOLD_STYLE );
   }

   sprintf( buffer, "Plug-in Name: %s", plugin->MapLabel() );
   fl_set_object_label( pfdsbp->PluginName, buffer );
   
   // deactivate all buttons initially
   DISABLE_WIDGET( pfdsbp->UnbindButton );
   DISABLE_WIDGET( pfdsbp->EntryPointsBindButton );
   DISABLE_WIDGET( pfdsbp->ExitPointsBindButton );

   UpdateBindingsList();

   // display enforced bindings for stub if they exist
   fl_clear_browser( pfdsbp->EnforcedBindings );
   cb = current_stub->ContinuityBindings();

   for( cb->First(); !cb->IsDone(); cb->Next() ) {
      sprintf( enforced, "IN%d <-> OUT%d",
	       current_stub->SourceSet()->in( cb->CurrentItem()->input_node )+1,
	       current_stub->TargetSet()->in( cb->CurrentItem()->output_node )+1 );
      fl_addto_browser( pfdsbp->EnforcedBindings, enforced );
   }
   
   fl_show_form( pfdsbp->StubBindingPopup, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT, "Bind Plugin to Stub" );
   SBPvisible = 1;
}

void UpdateBindingsList()
{

   fl_clear_browser( pfdsbp->StubBindingsList );
   fl_clear_browser( pfdsbp->StubEntryPointsBrowser );
   fl_clear_browser( pfdsbp->StubExitPointsBrowser );
   fl_clear_browser( pfdsbp->PluginEntryPointsBrowser );
   fl_clear_browser( pfdsbp->PluginExitPointsBrowser );

   current_stub->UpdateBindingsDisplay();
}

extern "C"
void StubBindingOKButton_cb(FL_OBJECT *, long)
{
   fl_hide_form( pfdsbp->StubBindingPopup );   
   SBPvisible = 0;
   UNFREEZE_TOOL();
   DrawScreen();
}

extern "C"
void UnbindButton_cb(FL_OBJECT *, long)
{
   int binding = fl_get_browser( pfdsbp->StubBindingsList );

   if( binding != 0 )
   {
      DISABLE_WIDGET( pfdsbp->UnbindButton );
      current_stub->BreakBinding( binding );
      UpdateBindingsList();
   }

   SetPromptUserForSave();
}

extern "C"
void EntryPointsBindButton_cb(FL_OBJECT *, long)
{
   int stub_entry = fl_get_browser( pfdsbp->StubEntryPointsBrowser );
   int plugin_entry = fl_get_browser( pfdsbp->PluginEntryPointsBrowser );

   if( (stub_entry != 0) && (plugin_entry != 0) )
   {
      DISABLE_WIDGET( pfdsbp->EntryPointsBindButton );
      current_stub->BindEntryPoints( stub_entry, plugin_entry );
      UpdateBindingsList();
   }

   SetPromptUserForSave();
}

extern "C"
void ExitPointsBindButton_cb(FL_OBJECT *, long)
{
   int stub_exit = fl_get_browser( pfdsbp->StubExitPointsBrowser );
   int plugin_exit = fl_get_browser( pfdsbp->PluginExitPointsBrowser );

   if( (stub_exit != 0) && (plugin_exit != 0) )
   {
      DISABLE_WIDGET( pfdsbp->ExitPointsBindButton );
      current_stub->BindExitPoints( stub_exit, plugin_exit );
      UpdateBindingsList();
   }

   SetPromptUserForSave();
}

extern "C"
void StubBindingsList_cb(FL_OBJECT *, long){

   int choice = fl_get_browser( pfdsbp->StubBindingsList );

   if( choice == 0 ) {
      DISABLE_WIDGET( pfdsbp->UnbindButton );
   }
   else {
      ENABLE_WIDGET( pfdsbp->UnbindButton );
   }

}

extern "C"
void StubEntryPointsBrowser_cb(FL_OBJECT *, long)
{
  ValidateEntryOperation(); 
}

extern "C"
void PluginEntryPointsBrowser_cb(FL_OBJECT *, long)
{
  ValidateEntryOperation(); 
}

void ValidateEntryOperation()
{
   if( (fl_get_browser( pfdsbp->StubEntryPointsBrowser ) != 0) &&
       (fl_get_browser( pfdsbp->PluginEntryPointsBrowser ) != 0 )) {
      ENABLE_WIDGET( pfdsbp->EntryPointsBindButton );
   }
}

extern "C"
void StubExitPointsBrowser_cb(FL_OBJECT *, long)
{
   ValidateExitOperation();
}

extern "C"
void PluginExitPointsBrowser_cb(FL_OBJECT *, long)
{
   ValidateExitOperation();
}

void ValidateExitOperation()
{
   if( (fl_get_browser( pfdsbp->StubExitPointsBrowser ) != 0 ) &&
       (fl_get_browser( pfdsbp->PluginExitPointsBrowser ) != 0 )) {
      ENABLE_WIDGET( pfdsbp->ExitPointsBindButton );
   }
}

int StubDialogClose( FL_FORM *, void * )
{
   StubLabelCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}

int PluginSelectorDialogClose( FL_FORM *, void * )
{
   PICCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}

int StubBindingsDialogClose( FL_FORM *, void * )
{
   StubBindingOKButton_cb( 0, 0 );
   return( FL_IGNORE );
}
