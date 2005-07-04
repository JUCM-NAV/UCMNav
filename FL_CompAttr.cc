// component attribute callbacks previously located in callbacks.cc

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "component_ref.h"
#include "component_mgr.h"
#include "defines.h"
#include "stub.h"
#include "devices.h"
#include <stdlib.h>

void EditComponentAttributes();

FD_CompAttrPopup *pfdcap = NULL;
int CAPvisible;
extern ComponentManager *component_manager;
extern int showing_enclosing_warning;
extern FD_PoolPluginChoicePopup *pfdppcp;
extern ComponentReference *active_component;
extern char component_description[][24];

extern void DrawScreen();
extern void LaunchPoolPluginChoice();
extern void SetPromptUserForSave();

static void InstallComponent( Component *new_component );
static int ColourIndex( int colour_value );
static int ComponentDialogClose( FL_FORM *, void * );
static void ClearDialog();
static void ComponentChoice( int index );

static FL_OBJECT *ComponentColourButtons[18];
static Component *active_component_def;
static int mode;
static int colour_matrix[] = { 0, 17, 1, 2, 3, 4, 5, 6, 8, 9, 10, 18, 19, 20, 21, 22, 23, 24 };
static Cltn<Component *> *existing_components;

void EditComponentAttributes()
{
   int i, index;
   DeviceDirectory *dd = DeviceDirectory::Instance();

   active_component_def = active_component->ReferencedComponent();
   FREEZE_TOOL(); // temporarily deactivate the main form

   if( !pfdcap ) {
      pfdcap = create_form_CompAttrPopup();
      fl_set_form_atclose( pfdcap->CompAttrPopup, ComponentDialogClose, NULL );
      fl_addto_choice( pfdcap->ComponentType, " Team | Object | Process | ISR | Pool | Agent | Other " );
      fl_set_choice_fontsize( pfdcap->ComponentType, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfdcap->ComponentType, FL_BOLD_STYLE );
      fl_addto_choice( pfdcap->PoolType, " None | Component | Plug-in " );
      fl_set_browser_fontsize( pfdcap->ExistingComponentsBrowser, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdcap->ExistingComponentsBrowser, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdcap->ProcessorList, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdcap->ProcessorList, FL_BOLD_STYLE );
      fl_set_choice_fontsize( pfdcap->PoolType, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfdcap->PoolType, FL_BOLD_STYLE );
      fl_set_choice_fontsize( pfdcap->PoolComponentType, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfdcap->PoolComponentType, FL_BOLD_STYLE );
      fl_activate_object( pfdcap->ReplicationFactor );

      ComponentColourButtons[0] = pfdcap->ColourButton0;
      ComponentColourButtons[1] = pfdcap->ColourButton1;
      ComponentColourButtons[2] = pfdcap->ColourButton2;
      ComponentColourButtons[3] = pfdcap->ColourButton3;
      ComponentColourButtons[4] = pfdcap->ColourButton4;
      ComponentColourButtons[5] = pfdcap->ColourButton5;
      ComponentColourButtons[6] = pfdcap->ColourButton6;
      ComponentColourButtons[7] = pfdcap->ColourButton7;
      ComponentColourButtons[8] = pfdcap->ColourButton8;
      ComponentColourButtons[9] = pfdcap->ColourButton9;
      ComponentColourButtons[10] = pfdcap->ColourButton10;
      ComponentColourButtons[11] = pfdcap->ColourButton11;
      ComponentColourButtons[12] = pfdcap->ColourButton12;
      ComponentColourButtons[13] = pfdcap->ColourButton13;
      ComponentColourButtons[14] = pfdcap->ColourButton14;
      ComponentColourButtons[15] = pfdcap->ColourButton15;
      ComponentColourButtons[16] = pfdcap->ColourButton16;
      ComponentColourButtons[17] = pfdcap->ColourButton17;

      for( i=0; i < 18; i++ )
	 fl_set_object_color( ComponentColourButtons[i], colour_matrix[i], colour_matrix[i] );
   }

   fl_hide_object( pfdcap->PoolType );
   fl_hide_object( pfdcap->PoolPluginButton );
   fl_hide_object( pfdcap->PoolComponentType );
   fl_set_choice( pfdcap->PoolComponentType, 0 );

   fl_set_button( pfdcap->InstallExistingButton, 0 );
   	    
   // fill list of existing components
   fl_clear_browser( pfdcap->ExistingComponentsBrowser );
   existing_components = component_manager->ComponentList();
   if( existing_components->Size() == 0 ) {
      DISABLE_WIDGET( pfdcap->InstallExistingButton );
   } else {
      ENABLE_WIDGET( pfdcap->InstallExistingButton );
      for( existing_components->First(); !existing_components->IsDone(); existing_components->Next() )
	 fl_add_browser_line( pfdcap->ExistingComponentsBrowser, existing_components->CurrentItem()->GetLabel() );
   }

   dd->ListProcessors( pfdcap->ProcessorList );

   if( active_component_def ) {
      InstallComponent( active_component_def );
      mode = EDIT_EXISTING;
      index = existing_components->in( active_component_def ) + 1;   // highlight proper name in list
      fl_select_browser_line( pfdcap->ExistingComponentsBrowser, index );
      fl_set_browser_topline( pfdcap->ExistingComponentsBrowser, index );
      fl_set_button( pfdcap->CreateNewButton, 0 );      
   }
   else { // component has not yet been defined
      ClearDialog();
      mode = CREATE_NEW;
      fl_set_button( pfdcap->CreateNewButton, 1 );      
   } 

   if( active_component->Anchored() ) {
      fl_set_button( pfdcap->AnchoredButton, 1 );
      fl_set_button( pfdcap->NonAnchoredButton, 0 );
   }
   else {
      fl_set_button( pfdcap->NonAnchoredButton, 1 );
      fl_set_button( pfdcap->AnchoredButton, 0 );
   }

   fl_set_input( pfdcap->ComponentRole, active_component->ComponentRole() );

   // deactivate browser as default will be create new component if component nonexistent or edit existing if it exists
   DISABLE_WIDGET( pfdcap->ExistingComponentsBrowser );

   fl_show_form( pfdcap->CompAttrPopup, FL_PLACE_CENTER | FL_FREE_SIZE, FL_TRANSIENT, "Component Attributes" );
   CAPvisible = 1;
}

extern "C"
void InstallExistingButton_cb(FL_OBJECT *, long)
{
   if( existing_components->Size() == 0 ) {
      fl_set_button( pfdcap->InstallExistingButton, 0 );
      return; // if there are no existing components to select don't select mode
   }

   if( mode != INSTALL_EXISTING ) {
      mode = INSTALL_EXISTING;
      ENABLE_WIDGET( pfdcap->ExistingComponentsBrowser );
      fl_set_button( pfdcap->CreateNewButton, 0 );
   }
}

extern  "C"
void CreateNewButton_cb(FL_OBJECT *, long)
{
   if( mode != CREATE_NEW ) {
      mode = CREATE_NEW;
      active_component_def = NULL;
      DISABLE_WIDGET( pfdcap->ExistingComponentsBrowser );
      ClearDialog();
      fl_set_button( pfdcap->InstallExistingButton, 0 );
   }
}

extern "C"
void ExistingComponentsBrowser_cb(FL_OBJECT *, long)
{
   int selection = fl_get_browser( pfdcap->ExistingComponentsBrowser );

   if( selection <= 0 ) {
      active_component_def = NULL;
      return;
   }

   active_component_def = existing_components->Get( selection );
   InstallComponent( active_component_def );
}

extern "C"
void Colour_cb(FL_OBJECT *button, long index )
{
   fl_set_object_color( pfdcap->ColourDisplay, colour_matrix[index], colour_matrix[index] );
}

extern "C"
void CAOKButton_cb( FL_OBJECT *pflo, long lData)
{
   int dynamic, stack, is_protected;
   int i, pct, selected_processor;
   char buffer[150], processed_name[LABEL_SIZE+1];
   Component *cd;
   const char *old_name = NULL,
      *new_name = fl_get_input( pfdcap->Label );

   if( active_component_def != NULL )
      old_name = active_component_def->GetLabel();

   if( !component_manager->UniqueComponentName( new_name, old_name, processed_name ) ) {
      sprintf( buffer, "The name \"%s\"", processed_name );
      fl_set_resource( "flAlert.title", "Error: Duplicate Component Name" );
      fl_show_alert( "Duplicate component name entered.", buffer, "is not a unique component name. Please enter a unique name.", 0 );
      return;
   }
   
   if( mode == EDIT_EXISTING )
      cd = active_component->ReferencedComponent();
   else if( mode == CREATE_NEW )
      cd = new Component();
   else {// INSTALL_EXISTING
      if( active_component_def == NULL ) {
	 fl_set_resource( "flAlert.title", "Error: No Existing Component Selected" );
	 fl_show_alert( "No existing component definition has been selected.", "Please select a definition.", "", 0 );
	 return;
      }
      else
	 cd = active_component_def;
   }
   if( mode != EDIT_EXISTING )
      active_component->ReferencedComponent( cd );

   etComponent_type component_type = (etComponent_type)(fl_get_choice( pfdcap->ComponentType ) - 1);
   cd->SetType( component_type );

   if( component_type == POOL ) {

      cd->SetPoolType( (etPool_type)(fl_get_choice( pfdcap->PoolType ) - 1) );
  
      switch ( fl_get_choice( pfdcap->PoolType ) ) {
      case 1:
	 cd->PoolComponentType( NO_TYPE );
	 break;
      case 2:
	 pct = fl_get_choice( pfdcap->PoolComponentType );
	 if( pct > 0 ) cd->PoolComponentType( (etComponent_type)(pct-1) );
	 break;
      case 3:
	 cd->PoolComponentType( PLUGIN_MAP );
	 break;
      }
   }
   
   dynamic = fl_get_button( pfdcap->DynamicButton );
   stack = fl_get_button( pfdcap->StackButton );
   is_protected = fl_get_button( pfdcap->ProtectedButton );

   cd->SetAttributes( dynamic, stack, is_protected );
   active_component->Fixed( (bool)fl_get_button( pfdcap->FixedButton ) );
   cd->SetLabel( processed_name );

   cd->Actual( fl_get_button( pfdcap->ActualButton ) );
   active_component->Anchored( fl_get_button( pfdcap->AnchoredButton ) );
   active_component->ComponentRole( fl_get_input( pfdcap->ComponentRole ) );

   for( i=0; i < 18; i++ ) {
      if( fl_get_button( ComponentColourButtons[i] ) ) {
	 cd->SetColour( colour_matrix[i] );
	 break;
      }
   }

   // etComponent_type type = cd->GetType();
   if( (selected_processor = fl_get_browser( pfdcap->ProcessorList )) != 0 )
      cd->ProcessorId( DeviceDirectory::Instance()->DeviceId( PROCESSOR, selected_processor ) );

   fl_set_object_label( pfdmMain->Description, component_description[ cd->GetType() ] );

   if( stack != 0 )
      cd->SetReplNumber( fl_get_input( pfdcap->ReplicationFactor ) );
  
   active_component->BindEnclosedFigures();

   fl_hide_form( pfdcap->CompAttrPopup );
   UNFREEZE_TOOL();
   CAPvisible = 0;

   SetPromptUserForSave();
   DrawScreen();
}

extern "C"
void CACancelButton_cb( FL_OBJECT *pflo, long lData) {
   fl_hide_form( pfdcap->CompAttrPopup );
   UNFREEZE_TOOL();
   CAPvisible = 0;
}

extern "C"
void ComponentType_cb( FL_OBJECT *pflo, long lData )
{
   int index = fl_get_choice( pflo );
   ComponentChoice( index );

   if( (active_component->CheckTypeContainment( (etComponent_type)(index-1) ) == 0) && (showing_enclosing_warning) ) {
      fl_set_resource( "flAlert.title", "Warning: Invalid Component Containment" );
      fl_show_alert( "Active components cannot be placed inside passive components.", "", "", 0 );
   }
} 

void ComponentChoice( int index )
{
   etPool_type pool_type = P_NONE;

   if( index == 5 ) { // choice is POOL
      DISABLE_WIDGET( pfdcap->StackButton );
      DISABLE_WIDGET( pfdcap->DynamicButton );
      DISABLE_WIDGET( pfdcap->ProtectedButton );
      fl_hide_object( pfdcap->ReplicationFactor );
      fl_show_object( pfdcap->PoolType );

      if( active_component_def )
	 pool_type = active_component_def->GetPoolType();

      switch( pool_type ) {
      case P_NONE:
	 DISABLE_WIDGET( pfdcap->PoolComponentType );
	 fl_set_object_label( pfdcap->PoolComponentType, "" );
	 fl_hide_object( pfdcap->PoolPluginButton );
	 fl_show_object( pfdcap->PoolComponentType );
	 break;
      case P_COMPONENT:
	 fl_set_object_label( pfdcap->PoolComponentType, "Type" );
	 fl_clear_choice( pfdcap->PoolComponentType );
	 fl_addto_choice( pfdcap->PoolComponentType, " Team | Object | Process | ISR | Agent | Other " );
	 fl_set_choice( pfdcap->PoolComponentType, active_component_def->PoolComponentType()+1 );
	 fl_hide_object( pfdcap->PoolPluginButton );
	 fl_show_object( pfdcap->PoolComponentType );
	 break;
      case P_PLUGIN:
	 fl_hide_object( pfdcap->PoolComponentType );
	 fl_show_object( pfdcap->PoolPluginButton );
	 break;
      }
      fl_set_button( pfdcap->StackButton, 0 );
      fl_set_button( pfdcap->DynamicButton, 0 );
      fl_set_button( pfdcap->ProtectedButton, 0 );
   } else {
      fl_hide_object( pfdcap->PoolType );
      fl_hide_object( pfdcap->PoolComponentType );
      fl_hide_object( pfdcap->PoolPluginButton );
      ENABLE_WIDGET( pfdcap->StackButton );
      ENABLE_WIDGET( pfdcap->DynamicButton );
      if( index == 3 || index == 4 ) {
	 fl_set_button( pfdcap->ProtectedButton, 0 );
	 DISABLE_WIDGET( pfdcap->ProtectedButton );
      } else {
	 ENABLE_WIDGET( pfdcap->ProtectedButton );
      }
   }
}

void InstallComponent( Component *new_component )
{
   int dynamic, stack = 0, is_protected, i, new_colour, processor_id, index;
   etComponent_type component_type;

   for( i=0; i < 18; i++ )
      fl_set_button( ComponentColourButtons[i], 0 );

   component_type = new_component->GetType();
   fl_set_choice( pfdcap->ComponentType, component_type+1 );
   fl_set_choice( pfdcap->PoolType, new_component->GetNPoolType()+1 );   
   fl_set_input( pfdcap->Label, new_component->GetLabel() );
   new_colour = new_component->GetColour();
   fl_set_button( ComponentColourButtons[ColourIndex(new_colour)], 1 );
   fl_set_object_color( pfdcap->ColourDisplay, new_colour, new_colour );
   new_component->GetAttributes( dynamic, stack, is_protected );
   fl_set_button( pfdcap->DynamicButton, dynamic );
   fl_set_button( pfdcap->StackButton, stack );
   fl_set_button( pfdcap->ProtectedButton, is_protected );
   fl_set_button( pfdcap->FixedButton, (int)active_component->Fixed() );
   fl_set_input( pfdcap->ReplicationFactor, new_component->GetReplNumber() );
   
   if( new_component->Actual() ) {
      fl_set_button( pfdcap->ActualButton, 1 );
      fl_set_button( pfdcap->FormalButton, 0 );
   }
   else {
      fl_set_button( pfdcap->FormalButton, 1 );
      fl_set_button( pfdcap->ActualButton, 0 );
   }

   if( component_type == POOL ) {
      fl_show_object( pfdcap->PoolType );
      fl_show_object( pfdcap->PoolComponentType );
      fl_clear_choice( pfdcap->PoolComponentType );

      switch( new_component->GetPoolType() ) {
      case P_NONE:
	 fl_set_object_label( pfdcap->PoolComponentType, "" );
	 DISABLE_WIDGET( pfdcap->PoolComponentType );
	 break;
      case P_COMPONENT:   
	 fl_set_object_label( pfdcap->PoolComponentType, "Type" );
	 fl_addto_choice( pfdcap->PoolComponentType, " Team | Object | Process | ISR | Agent | Other " );
	 fl_set_choice( pfdcap->PoolComponentType, new_component->PoolComponentType()+1 );
	 break;
      case P_PLUGIN:
	 fl_hide_object( pfdcap->PoolComponentType );
	 fl_show_object( pfdcap->PoolPluginButton );
	 break;
      }
   } else {
      fl_hide_object( pfdcap->PoolType );
      fl_hide_object( pfdcap->PoolComponentType );
      fl_hide_object( pfdcap->PoolPluginButton );
      ENABLE_WIDGET( pfdcap->StackButton );
      ENABLE_WIDGET( pfdcap->DynamicButton );
      if( (component_type == PROCESS) || (component_type == ISR) ) {
	 fl_set_button( pfdcap->ProtectedButton, 0 );
	 DISABLE_WIDGET( pfdcap->ProtectedButton );
      }
      else
	 ENABLE_WIDGET( pfdcap->ProtectedButton );
   }

   if ( stack != 0 )
      fl_show_object( pfdcap->ReplicationFactor );
   else
      fl_hide_object( pfdcap->ReplicationFactor );

   if( (processor_id = new_component->ProcessorId()) != NA ) {
      if( (index = DeviceDirectory::Instance()->DeviceIndex( PROCESSOR, processor_id )) != 0 ) {
	 fl_select_browser_line( pfdcap->ProcessorList, index );
	 fl_set_browser_topline( pfdcap->ProcessorList, index );
      }
      else
	 new_component->ProcessorId( NA ); // set the component's processor id to null as it does not exist anymore
   }
   else
      fl_deselect_browser( pfdcap->ProcessorList );
}

void ClearDialog()
{
   fl_set_choice( pfdcap->ComponentType, 1 ); // set default to TEAM
   ComponentChoice( 1 );
   fl_set_choice( pfdcap->PoolType, 1 ); // set default to NONE
   fl_set_input( pfdcap->Label, "" );
   fl_set_button( ComponentColourButtons[0], 1 ); // set default to black
   fl_set_object_color( pfdcap->ColourDisplay, 0, 0 );
   fl_set_button( pfdcap->DynamicButton, 0 );
   fl_set_button( pfdcap->StackButton, 0 );
   fl_set_button( pfdcap->ProtectedButton, 0 );
   fl_set_button( pfdcap->FixedButton, 0 );
   fl_set_button( pfdcap->ActualButton, 1 );
   fl_set_button( pfdcap->FormalButton, 0 );
   fl_hide_object( pfdcap->ReplicationFactor );
   fl_deselect_browser( pfdcap->ProcessorList );
}

int ComponentDialogClose( FL_FORM *, void * )
{
   CACancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}

void StackButton_cb( FL_OBJECT *pflo, long lData )
{

   if ( fl_get_button( pfdcap->StackButton ) != 0 )
      fl_show_object( pfdcap->ReplicationFactor );
   else
      fl_hide_object( pfdcap->ReplicationFactor );
}

void PoolType_cb( FL_OBJECT *pflo, long lData )
{
   switch( fl_get_choice( pflo ) ) {
   case 1:
      fl_set_object_label( pfdcap->PoolComponentType, "" );
      fl_clear_choice( pfdcap->PoolComponentType );
      DISABLE_WIDGET( pfdcap->PoolComponentType );
      fl_hide_object( pfdcap->PoolPluginButton );
      break;
   case 2:
      fl_set_object_label( pfdcap->PoolComponentType, "Type" );
      fl_clear_choice( pfdcap->PoolComponentType );
      fl_addto_choice( pfdcap->PoolComponentType, " Team | Object | Process | ISR | Agent | Other " );
      fl_set_choice( pfdcap->PoolComponentType, ((active_component_def) ? active_component_def->PoolComponentType()+1 : 1) );
      ENABLE_WIDGET( pfdcap->PoolComponentType );
      fl_hide_object( pfdcap->PoolPluginButton );
      fl_show_object( pfdcap->PoolComponentType );
      break;
   case 3:
      fl_hide_object( pfdcap->PoolComponentType );
      fl_show_object( pfdcap->PoolPluginButton );
      break;
   }
}

void PoolPluginButton_cb( FL_OBJECT *pflo, long lData)
{
   LaunchPoolPluginChoice();
}

int ColourIndex( int colour_value )
{
   for( int i = 0; i < 18; i++ ) {
      if( colour_matrix[i] == colour_value )
	 return( i );
   }
   return (0); // DA: Added August 2004 (just in case)
}
