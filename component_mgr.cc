/***********************************************************
 *
 * File:			component_mgr.cc
 * Author:			Andrew Miga
 * Created:			May 1997
 *
 * Modification history:        Added handling of global
 *                              component pool
 *
 ***********************************************************/

#include "component_mgr.h"
#include "xml_generator.h"
#include "xml_mgr.h"
#include "print_mgr.h"

ComponentReference *active_component;  // global active component pointer

extern int default_width, default_height, iResize_decoupled;
extern void DrawScreen();
extern void SetPromptUserForSave();
extern int showing_enclosing_warning;
extern bool FixedAllComponents;

ComponentManager::ComponentManager()
{
   handles = new Cltn<Handle *>;
   active_component = NULL;
   copy_component = NULL;
   active_handle = NULL;
}

ComponentManager::~ComponentManager()
{
   while( !handles->is_empty() )
      delete handles->Detach();
   delete handles;
}

void ComponentManager::Reset()
{
    while( !selected_components.is_empty() )
      selected_components.Detach();
    
   copy_component = NULL;
}

void ComponentManager::AddComponent( Component *new_component )
{
   Component *cc; // add new Component to list in alphabetical order

   if( component_pool.Includes( new_component ) )
      return;

   if( component_pool.Size() == 0 )
      component_pool.Add( new_component );
   else {
      for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
	 cc = component_pool.CurrentItem();
	 if( strcmp( new_component->GetLabel(), cc->GetLabel() ) < 0 ) {
	    component_pool.AddBefore( new_component, cc );
	    return;
	 }
      }
      component_pool.Add( new_component ); // add new component at end of list  
   }
}

bool ComponentManager::DuplicateName(const char *name, int &index )
{
   for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
      if( strequal( component_pool.CurrentItem()->GetLabel(), name ) ) {
	 index = component_pool.CurrentItem()->GetComponentNumber();
	 return TRUE;
      }
   }

   return FALSE;
}

Component * ComponentManager::FindComponent( int parent_id )
{
   if( XmlManager::ImportSubstitution() ) { // search list of mappings from replaced components to existing components
      for( import_mappings.First(); !import_mappings.IsDone(); import_mappings.Next() ) {
	 if( import_mappings.CurrentItem().replaced_identifier == parent_id ) {
	    for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
	       if( component_pool.CurrentItem()->GetComponentNumber() == import_mappings.CurrentItem().existing_identifier )
		  return( component_pool.CurrentItem() );
	    }
	 }
      }
   }

   // search list of existing components to match up identifiers
   for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
      if( component_pool.CurrentItem()->LoadIdentifier() == parent_id )
	 return( component_pool.CurrentItem() );
   }

   AbortProgram( "Referenced Component not found." );
   return( NULL ); // DA: Added August 2004   
}

void ComponentManager::DeleteMappings()
{
   while( !import_mappings.is_empty() )
      import_mappings.Detach();
}

int ComponentManager::MaximumLoadNumber()
{
   int max_load = 0;

   for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
      if( component_pool.CurrentItem()->LoadIdentifier() > max_load )
	 max_load = component_pool.CurrentItem()->LoadIdentifier();
   }

   return( max_load+1 );
}

void ComponentManager::ClearComponentList()
{
   while( !component_pool.is_empty() )
      delete component_pool.Detach();
}

void ComponentManager::CleanList()
{
   Component *cp;

   for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
      cp = component_pool.CurrentItem();
      if( cp->IsNotReferenced() ) {
	 component_pool.RemoveCurrentItem();
	 delete cp;
      }
   }
}

bool ComponentManager::UniqueComponentName( const char *new_name, const char *old_name, char *name )
{
   char processed_name[LABEL_SIZE+1], unprocessed_name[LABEL_SIZE+1];
   
   unprocessed_name[LABEL_SIZE] = 0;
   strncpy( unprocessed_name, new_name, LABEL_SIZE );
   XmlManager::RemoveWhitespace( processed_name, unprocessed_name );
   strcpy( name, processed_name );

   if( strequal( processed_name, "" ) )
      return( FALSE );

   if( old_name ) {
      if( strequal( processed_name, old_name ) )
	 return( TRUE );
   }

   for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
      if( strequal( component_pool.CurrentItem()->GetLabel(), processed_name ) )
	 return( FALSE );
   }

   return( TRUE );
}

const char * ComponentManager::VerifyUniqueComponentName( const char *name )
{
   int extension = 1;
   static char new_name[LABEL_SIZE+1];
   char old_name[LABEL_SIZE+1];
   bool duplicate_name;

   strcpy( new_name, name );
   strcpy( old_name, name );
   
   forever {

      duplicate_name = FALSE;
      
      for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
	 if( strequal( component_pool.CurrentItem()->GetLabel(), new_name ) ) {
	    duplicate_name = TRUE;
	    break;
	 }
      }

      if( duplicate_name ) {
	 if( strlen( old_name ) > (LABEL_SIZE-3) )
	    old_name[LABEL_SIZE-3] = 0;
	 sprintf( new_name, "%s-%d", old_name, extension++ );
      }
      else
	 return( new_name );
   }
}

void ComponentManager::SaveComponentList( FILE *fp, Cltn<Map *> *map_list )
{
   if( component_pool.Size() == 0 ) return;

   CleanList();
   
   if( map_list ) {

      bool components_exist = FALSE;
      ResetGenerate();
      
      for( map_list->First(); !map_list->IsDone(); map_list->Next() ) {
	 if( map_list->CurrentItem()->HasComponents() )
	    components_exist = TRUE;
      }
      
      if( !components_exist ) return;
   }
   
   PrintNewXMLBlock( fp, "component-specification" );

   if( !map_list ) {
      for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() )
	 component_pool.CurrentItem()->SaveXML( fp );
   } else {
      for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
	 if( component_pool.CurrentItem()->Visited() )
	    component_pool.CurrentItem()->SaveXML( fp );
      }
   }

   PrintEndXMLBlock( fp, "component-specification" );
   LinebreakXML( fp );
}

void ComponentManager::SaveCSMComponentList( FILE *fp, Cltn<Map *> *map_list )
{
   if( component_pool.Size() == 0 ) return;

   CleanList();
   
   if( map_list ) {

      bool components_exist = FALSE;
      ResetGenerate();
      
      for( map_list->First(); !map_list->IsDone(); map_list->Next() ) {
	 if( map_list->CurrentItem()->HasComponents() )
	    components_exist = TRUE;
      }
      
      if( !components_exist ) return;
   }
   
   //PrintNewXMLBlock( fp, "component-specification" );

   if( !map_list ) {
      for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() )
	 component_pool.CurrentItem()->SaveCSMXML( fp );
   } else {
      for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
	 if( component_pool.CurrentItem()->Visited() )
	    component_pool.CurrentItem()->SaveCSMXML( fp );
      }
   }

  // PrintEndXMLBlock( fp, "component-specification" );
   LinebreakXML( fp );
}
void ComponentManager::ResetGenerate()
{
   for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() )
      component_pool.CurrentItem()->ResetVisited();
}

void ComponentManager::Draw( Presentation *ppr )
{
   for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {
      if( component_references->CurrentItem()->GetParent() == NULL )
	 component_references->CurrentItem()->Draw( ppr );
   }

   for( handles->First(); !handles->IsDone(); handles->Next() )
      handles->CurrentItem()->Draw( ppr );

}

bool ComponentManager::FindHandle( float XCoord, float YCoord )
{
   active_handle = NULL;
   for( handles->First(); !handles->IsDone(); handles->Next() ) {
      if( handles->CurrentItem()->PointinHandle( XCoord, YCoord ) ) {
	 active_handle = handles->CurrentItem();
	 return( TRUE );
      }
   }

   return( FALSE );
}

#if 0
bool ComponentManager::FindBorderSelection( float XCoord, float YCoord )
{
   for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {
      if( component_references->CurrentItem()->PointOnBorder( XCoord, YCoord ) ) {
	 cout << "\nComponent border hit detected" << flush;
	 return( TRUE );	 
      }
   }
   return( FALSE );
}
#endif

ComponentReference * ComponentManager::FindComponent( float XCoord, float YCoord )
{
   ComponentReference *last_component, *current_component;
   
   float min_width = 10000.0, min_height = 10000.0, x, y;
   last_component = active_component;
   active_component = NULL;

   for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {

      current_component = component_references->CurrentItem();
      if( current_component->PointInBox( XCoord, YCoord ) ) {
	 if( current_component->GetWidth() < min_width  &&
	     current_component->GetHeight() < min_height ) {
	    active_component = current_component;
	    min_width = current_component->GetWidth();
	    min_height = current_component->GetHeight();
	    current_component->GetPosition( x, y );
	    xoffset = x - XCoord;
	    yoffset = y - YCoord;
	 }
      }
   }

   if( (active_component != last_component) || (active_component == NULL) ) {  // free old handles if necessary
      while( !handles->is_empty() )  
	 delete handles->Detach();
   }

   if( active_component )
      active_component->SetSelected();
   
   if( (active_component != last_component) && active_component ) {   // create handles for new active component
      if(!( active_component->Fixed() || FixedAllComponents == true )) // add handles if component is not fixed in position
	 AddHandles();
   }

   return( active_component );
}

void ComponentManager::DetermineParent( ComponentReference *component )
{
   ComponentReference *new_component = NULL, *current_component, *old_parent;
   float x, y, min_width = 10000.0, min_height = 10000.0;

   component->GetPosition( x, y );
   
   for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {

      current_component = component_references->CurrentItem();
      if( current_component->PointInBox( x, y ) ) {
	 if( current_component->GetWidth() < min_width  &&
	     current_component->GetHeight() < min_height &&
	     current_component != component ) {

	    new_component = current_component;
	    min_width = current_component->GetWidth();
	    min_height = current_component->GetHeight();
	 }
      }
   }

   if( (new_component != component->GetParent()) 
       && (!component->IsComponentDescendent( new_component ) ) ) {

      old_parent = component->GetParent();
	
      if( old_parent )
	 old_parent->RemoveChild( component );
	
      if( component->SetParent( new_component ) )
	 component->SetPosition( x, y );
      else
	 old_parent->AddChild( component );

      if( (component->CheckTypeContainment( component->ComponentType() ) == 0) && (showing_enclosing_warning) )
	 fl_show_message( "Warning:", "Invalid component containment.", "" );

   }
}

void ComponentManager::DetermineSelectedComponents( float lb, float rb, float tb, float bb )
{
   ComponentReference *current_component;
   float tlx, tly, brx, bry;
   
   while( !selected_components.is_empty() )
      selected_components.Detach();

   for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {

      current_component = component_references->CurrentItem();
      current_component->GetPosition( tlx, tly );
      brx = tlx + current_component->GetWidth();
      bry = tly + current_component->GetHeight();
      if( tlx > lb && brx < rb && tly > tb && bry < bb ) {
	 selected_components.Add( current_component );
	 current_component->SetSelected();
      }
      else
	current_component->ResetSelected(); 
   }
}

void ComponentManager::ResetSelectedComponents()
{
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() )
      selected_components.CurrentItem()->ResetSelected();

   while( !selected_components.is_empty() )
      selected_components.Detach();

   while( !handles->is_empty() )
      delete handles->Detach();

   active_component = NULL;
   active_handle = NULL;
}

void ComponentManager::SelectAllComponents()
{
   ComponentReference *cc;

    for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {
       cc = component_references->CurrentItem();
       cc->SetSelected();
       selected_components.Add( cc );
    }
}

void ComponentManager::AddSelectedComponent( ComponentReference *new_component )
{
   DeleteHandles();

   if( selected_components.Includes( new_component ) ) {
      selected_components.Remove( new_component );
      new_component->ResetSelected();
   }
   else {
      selected_components.Add( new_component );
      if( selected_components.Size() == 1 ) {
	 active_component = new_component;
	 AddHandles();
      }
   }
}

bool ComponentManager::SingleSelection()
{
   if( selected_components.Size() == 1 ) {
      active_component = selected_components.Get( 1 );
      return TRUE;
   }
   else
      return FALSE;
}

void ComponentManager::ValidatePositionShifts( float x_offset, float y_offset, float& x_limit, float& y_limit )
{
   float x_lim, y_lim, max_x_limit = 0, max_y_limit = 0;
   
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->ValidatePositionShift( x_offset, y_offset, x_lim, y_lim );
      if( x_lim > max_x_limit ) max_x_limit = x_lim;
      if( y_lim > max_y_limit ) max_y_limit = y_lim;
   }

   x_limit = max_x_limit;
   y_limit = max_y_limit;
}

void ComponentManager::ShiftSelectedComponents( float x_offset, float y_offset )
{
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() )
      selected_components.CurrentItem()->ShiftPosition( x_offset, y_offset );

   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() )
      DetermineParent( selected_components.CurrentItem() );
}

void ComponentManager::AddHandles()
{
   handles->Add( new Handle( TOP_LEFT, active_component ) );
   handles->Add( new Handle( TOP_MIDDLE, active_component ) );
   handles->Add( new Handle( TOP_RIGHT, active_component ) );
   handles->Add( new Handle( MIDDLE_LEFT, active_component ) );
   handles->Add( new Handle( MIDDLE_RIGHT, active_component ) );
   handles->Add( new Handle( BOTTOM_LEFT, active_component ) );
   handles->Add( new Handle( BOTTOM_MIDDLE, active_component ) );
   handles->Add( new Handle( BOTTOM_RIGHT, active_component ) );
}

void ComponentManager::DeleteHandles()
{
   while( !handles->is_empty() )  
      delete handles->Detach();
}

void ComponentManager::ChangeHandleMode()
{
   if( active_component ) {
      if( !active_component->Fixed() ) {
	 if( FixedAllComponents == true )
	    DeleteHandles();
	 else
	    AddHandles();
	 DrawScreen();
      }
   }
}

ComponentReference * ComponentManager::FindEnclosingComponent( float XCoord, float YCoord, Map *map )
{
   ComponentReference *encl_component, *current_component;
   Cltn<ComponentReference *> *component_list;
   float min_width = 10000.0, min_height = 10000.0;
   
   if( map != NULL )
      component_list = map->Components();
   else
      component_list = component_references;
   
   encl_component = NULL;
  
   for( component_list->First(); !component_list->IsDone(); component_list->Next() ) {

      current_component = component_list->CurrentItem();
      if( current_component->PointInBox( XCoord, YCoord ) ) {

	 if( current_component->GetWidth() < min_width 
	     && current_component->GetHeight() < min_height ) {

	    min_width = current_component->GetWidth();
	    min_height = current_component->GetHeight();

	    encl_component = current_component;
	 }
      }
   }
      
   return( encl_component );
}

void ComponentManager::MoveComponent( float XCoord, float YCoord )
{

   ComponentReference *new_component = NULL, *current_component, *old_parent;
   float min_width = 10000.0, min_height = 10000.0;
   
   if( active_handle ) {

      active_handle->MoveHandle( XCoord, YCoord, iResize_decoupled );
      DrawScreen();
      
   }
   else if( active_component ) {

      for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {

	 current_component = component_references->CurrentItem();
	 if( current_component->PointInBox( XCoord, YCoord ) ) {
	    if( current_component->GetWidth() < min_width  &&
		current_component->GetHeight() < min_height &&
	        current_component != active_component ) {

	       new_component = current_component;
	       min_width = current_component->GetWidth();
	       min_height = current_component->GetHeight();
	    }
	 }
      }

      if( (new_component != active_component->GetParent()) 
	  && (!active_component->IsComponentDescendent( new_component ) ) ) {

	 old_parent = active_component->GetParent();
	
	 if( old_parent )
	    old_parent->RemoveChild( active_component );
	
	 if( !active_component->SetParent( new_component ) )
	    old_parent->AddChild( active_component );
      }

      active_component->SetPosition( XCoord+xoffset, YCoord+yoffset );
      DrawScreen();

   }

}

void ComponentManager::BindEnclosedComponents()
{
   ComponentReference *current_component;
   float x, y;

   for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {
      
      current_component = component_references->CurrentItem();
      if( current_component != active_component ) { 
	 current_component->GetPosition( x, y );
	 if( active_component->PointInBox( x, y ) ) {
	    current_component->SetParent( active_component );
	    current_component->SetPosition( x, y );
	 }
      }
   }

}

void ComponentManager::DrawNewComponent( Presentation *ppr, new_component & nc )
{

   float X[4] = { (float)nc.sx/(float)default_width, 
		  (float)nc.cx/(float)default_width,
		  (float)nc.cx/(float)default_width, 
		  (float)nc.sx/(float)default_width};
			 
   float Y[4] = {(float)nc.sy/(float)default_height, 
		 (float)nc.sy/(float)default_height, 
		 (float)nc.cy/(float)default_height,
		 (float)nc.cy/(float)default_height};

   ppr->SetFgColour( WHITE );
   ppr->DrawPoly( X, Y, 4 );

   float X2[4] = {(float)nc.sx/(float)default_width, 
		  (float)nc.x/(float)default_width,
		  (float)nc.x/(float)default_width, 
		  (float)nc.sx/(float)default_width};
   float Y2[4] = {(float)nc.sy/(float)default_height, 
		  (float)nc.sy/(float)default_height, 
		  (float)nc.y/(float)default_height,
		  (float)nc.y/(float)default_height};
      
   ppr->SetFgColour( BLACK );
   ppr->DrawPoly( X2, Y2, 4 );
   ppr->Refresh();
   
}

void ComponentManager::ResizeComponent()
{

   if( active_handle ) {
      active_handle = NULL;

      while( !handles->is_empty() )  
	 delete handles->Detach();

      if( active_component ) {
	 active_component->ResetGeometry();
	  
	 handles->Add( new Handle( TOP_LEFT, active_component ) );
	 handles->Add( new Handle( TOP_MIDDLE, active_component ) );
	 handles->Add( new Handle( TOP_RIGHT, active_component ) );
	 handles->Add( new Handle( MIDDLE_LEFT, active_component ) );
	 handles->Add( new Handle( MIDDLE_RIGHT, active_component ) );
	 handles->Add( new Handle( BOTTOM_LEFT, active_component ) );
	 handles->Add( new Handle( BOTTOM_MIDDLE, active_component ) );
	 handles->Add( new Handle( BOTTOM_RIGHT, active_component ) );

	 active_component->NotifyDependents();
      }
      DrawScreen();
   }

   if( active_component ) {
      if( (active_component->CheckTypeContainment( active_component->ComponentType() ) == 0) && showing_enclosing_warning )
	 fl_show_message( "Warning:", "Invalid component containment.", "" );								   
   }
}

void ComponentManager::CreateNewComponent( new_component & nc )
{
	    
   ComponentReference *current_component, *new_component;
   float min_width = 10000.0, min_height = 10000.0;

   active_component = NULL;

   for( component_references->First(); !component_references->IsDone(); component_references->Next() ) {

      current_component = component_references->CurrentItem();
      
      if( current_component->PointInBox( (float)nc.x/(float)default_width,
					 (float)nc.y/(float)default_height ) ) {
	 if( current_component->GetWidth() < min_width
	     && current_component->GetHeight() < min_height ) {
	    min_width = current_component->GetWidth();
	    min_height = current_component->GetHeight();

	    active_component = current_component;
	 }
      }
   }


   component_references->Add( new_component = new ComponentReference( (float)nc.sx/(float)default_width,
						       (float)nc.sy/(float)default_height,
						       (float)(nc.x-nc.sx)/(float)default_width,
						       (float)(nc.y-nc.sy)/(float)default_height,
						       active_component ) );

   if( (new_component->CheckTypeContainment( new_component->ComponentType() ) == 0) && showing_enclosing_warning )
      fl_show_message( "Warning:", "Invalid component containment.", "" );
	
   DrawScreen();

}

void ComponentManager::CutComponent()
{

   if( active_component == NULL )
      return;
   
   copy_component = active_component->Copy( NULL );

   component_references->Delete( active_component );
   delete active_component;
   active_component = NULL;
   
   while( !handles->is_empty() )  
      delete handles->Detach();

   DrawScreen();
}

void ComponentManager::DeleteSelectedComponents()
{
   ComponentReference *current_component;

   if( selected_components.Size() == 0 ) return;

   if( selected_components.Size() == 1 )
      copy_component = selected_components.GetFirst()->Copy( NULL );

   while( !selected_components.is_empty() ) {
      current_component = selected_components.Detach();
      component_references->Delete( current_component );
      delete current_component;
   }

   active_component = NULL;
   
   while( !handles->is_empty() )  
      delete handles->Detach();
   
   SetPromptUserForSave();
}

void ComponentManager::CopyComponent()
{
   if( active_component )
      copy_component = active_component->Copy( NULL );
}

void ComponentManager::PasteComponent()
{
   float x, y;
   Cltn<ComponentReference *> component_children;

   if( copy_component == NULL )
      return;

   ComponentReference *new_component = copy_component->Copy( NULL );

   new_component->GetAllChildren( component_children );

   component_references->Add( new_component );
   while( !component_children.is_empty() )
      component_references->Add( component_children.RemoveFirstElement() );
   
   new_component->GetPosition( x, y );
   new_component->SetPosition( x+.02, y+.02 );
   copy_component->SetPosition( x+.02, y+.02 );
   if( active_component ) {
      active_component->ResetSelected();
      DeleteHandles();
   }
   new_component->SetSelected();
   active_component = new_component;
   AddHandles();
   DrawScreen();

}

void ComponentManager::LowerLastComponent()
{
   if( component_references->Size() != 0 ) {
      component_references->PutStart( component_references->RemoveLastElement() );
      DrawScreen();
   }
}

void ComponentManager::AlignTop()
{
   float x, y, min_y = 10.0;

   if( selected_components.Size() < 2 ) return;
   
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      if( y < min_y ) min_y = y;
   }

   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      selected_components.CurrentItem()->SetPosition( x, min_y );
   }
}

void ComponentManager::AlignBottom()
{
   float x, y, h, max_y = 0.0;

   if( selected_components.Size() < 2 ) return;
   
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      h = selected_components.CurrentItem()->GetHeight();
      if( (y+h) > max_y ) max_y = y+h;
   }

   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      h = selected_components.CurrentItem()->GetHeight();
      selected_components.CurrentItem()->SetPosition( x, max_y-h );
   }
}

void ComponentManager::CenterVertically()
{
   float x, y, h, mid_y, min_y = 10.0, max_y = 0.0;

   if( selected_components.Size() < 2 ) return;
   
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      h = selected_components.CurrentItem()->GetHeight();
      if( y < min_y ) min_y = y;
      if( (y+h) > max_y ) max_y = y+h;
   }

   mid_y = (min_y + max_y)/2;

   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      h = selected_components.CurrentItem()->GetHeight();
      selected_components.CurrentItem()->SetPosition( x, mid_y-(h/2) );
   }

}

void ComponentManager::AlignLeft()
{
   float x, y, min_x = 10.0;

   if( selected_components.Size() < 2 ) return;
   
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      if( x < min_x ) min_x = x;
   }

   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      selected_components.CurrentItem()->SetPosition( min_x, y );
   }
}

void ComponentManager::AlignRight()
{
   float x, y, w, max_x = 0.0;

   if( selected_components.Size() < 2 ) return;
   
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      w = selected_components.CurrentItem()->GetWidth();
      if( (x+w) > max_x ) max_x = x+w;
   }

   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      w = selected_components.CurrentItem()->GetWidth();
      selected_components.CurrentItem()->SetPosition( max_x-w, y );
   }
}

void ComponentManager::CenterHorizontally()
{
   float x, y, w, mid_x, min_x = 10.0, max_x = 0.0;

   if( selected_components.Size() < 2 ) return;

   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      w = selected_components.CurrentItem()->GetWidth();
      if( (x+w) > max_x ) max_x = x+w;
      if( x < min_x ) min_x = x;
   }

   mid_x = (min_x + max_x)/2;

   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      selected_components.CurrentItem()->GetPosition( x, y );
      w = selected_components.CurrentItem()->GetWidth();
      selected_components.CurrentItem()->SetPosition( mid_x-(w/2), y );
   }
}

void ComponentManager::DistributeHorizontally()
{
   ComponentReference *list[50], *ordered_list[50];
   float x, y, px, py, pb, spacing, total_width = 0, min_x, max_x;
   int i = 0, j, min_index, num_selected;

   if( (num_selected = selected_components.Size()) < 3 ) return;
   
   // populate unordered list and calculate total widths
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      list[i++] = selected_components.CurrentItem();
      total_width += selected_components.CurrentItem()->GetWidth();
   }

   // create ordered list of components in increasing x-coordinate
   for( i = 0; i < num_selected; i++ ) {
      min_x = 10.0;
      for( j = 0; j < num_selected; j++ ) {
	 if( list[j] != NULL ){
	    list[j]->GetPosition( x, y );
	    if( x < min_x ) {
	       min_x = x;
	       min_index = j;
	    }
	 }
      }
      ordered_list[i] = list[min_index];
      list[min_index] = NULL;
   }

   // calculate even horizontal spacing between components
   ordered_list[0]->GetPosition( min_x, y );
   ordered_list[num_selected-1]->GetPosition( max_x, y );
   max_x += ordered_list[num_selected-1]->GetWidth(); // add width of rightmost component
   spacing = (max_x - min_x - total_width)/(num_selected-1);

   // adjust x-coordinates of interior components ignoring leftmost and rightmost components
   for( i = 1; i < num_selected-1; i++ ) {
      ordered_list[i]->GetPosition( x, y );
      ordered_list[i-1]->GetPosition( px, py );
      pb = px + ordered_list[i-1]->GetWidth(); // determine right border of previous component
      ordered_list[i]->SetPosition( pb+spacing, y );
   }   
   
}

void ComponentManager::DistributeVertically()
{
   ComponentReference *list[50], *ordered_list[50];
   float x, y, px, py, pb, spacing, total_height = 0, min_y, max_y;
   int i = 0, j, min_index, num_selected;

   if( (num_selected = selected_components.Size()) < 3 ) return;
   
   // populate unordered list and calculate total heights
   for( selected_components.First(); !selected_components.IsDone(); selected_components.Next() ) {
      list[i++] = selected_components.CurrentItem();
      total_height += selected_components.CurrentItem()->GetHeight();
   }

   // create ordered list of components in increasing y-coordinate
   for( i = 0; i < num_selected; i++ ) {
      min_y = 10.0;
      for( j = 0; j < num_selected; j++ ) {
	 if( list[j] != NULL ){
	    list[j]->GetPosition( x, y );
	    if( y < min_y ) {
	       min_y = y;
	       min_index = j;
	    }
	 }
      }
      ordered_list[i] = list[min_index];
      list[min_index] = NULL;
   }

   // calculate even vertical spacing between components
   ordered_list[0]->GetPosition( x, min_y );
   ordered_list[num_selected-1]->GetPosition( x, max_y );
   max_y += ordered_list[num_selected-1]->GetHeight(); // add height of bottommost component
   spacing = (max_y - min_y - total_height)/(num_selected-1);

   // adjust y-coordinates of interior components ignoring topmost and bottommost components
   for( i = 1; i < num_selected-1; i++ ) {
      ordered_list[i]->GetPosition( x, y );
      ordered_list[i-1]->GetPosition( px, py );
      pb = py + ordered_list[i-1]->GetHeight(); // determine bottom border of previous component
      ordered_list[i]->SetPosition( x, pb+spacing );
   }   
}

void ComponentManager::OutputDXL( std::ofstream &dxl_file ) {
	Component *cp;

	for( component_pool.First(); !component_pool.IsDone(); component_pool.Next() ) {
		cp = component_pool.CurrentItem();	
      	dxl_file << "component( " << "\"c" << cp->GetComponentNumber() << "\", ";
		if( PrintManager::TextNonempty( cp->GetLabel()  ) )
			dxl_file << "\"" << cp->GetLabel() << "\", ";
		else
			dxl_file << "\"\", ";
		dxl_file << "\"" << cp->ComponentType() << "\", ";
		if( PrintManager::TextNonempty( cp->Description() ) )
			dxl_file << "\"" << removeNewlineinString( cp->Description() ) << "\", ";
		else 
			dxl_file << "\"\", ";
		if (cp->ProcessorId() != -1)
			dxl_file << "\"d" << cp->ProcessorId() << "\" )\n";
		else
			dxl_file << "\"\" )\n";
	}
}

