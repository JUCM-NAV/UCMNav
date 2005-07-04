/***********************************************************
 *
 * File:			component_mgr.h
 * Author:			Andrew Miga
 * Created:			May 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include "component_ref.h"
#include "collection.h"
#include "presentation.h"
#include "handle.h"
#include <stdio.h>

extern ComponentReference *active_component;

typedef struct {
   int sx;
   int sy;
   int cx;
   int cy;
   int x;
   int y;
} new_component;

class ComponentManager {

public:

   ComponentManager();
   ~ComponentManager();

   void ComponentReferences( Cltn<ComponentReference *> *components ) { component_references = components; }
   Cltn<ComponentReference *> * ComponentReferences() { return( component_references ); }
   ComponentReference * FindComponent( float XCoord, float YCoord );
   bool FindHandle( float XCoord, float YCoord );

   void AddComponent( Component *new_component );
   void RemoveComponent( Component *component ) { component_pool.Delete( component ); }
   void RemoveComponentReference( ComponentReference *ref ) { selected_components.Delete( ref ); }
   void RenameComponent( Component *component ) { RemoveComponent( component ); AddComponent( component ); }
   Cltn<Component *> * ComponentList() { return( &component_pool ); }
   void SaveComponentList( FILE *fp, Cltn<Map *> *map_list );
   void SaveCSMComponentList( FILE *fp, Cltn<Map *> *map_list );
   void ResetGenerate();
   Component * FindComponent( int parent_id );
   bool UniqueComponentName( const char *new_name, const char *old_name, char *name );
   const char * VerifyUniqueComponentName( const char *name );
   bool DuplicateName(const char *name, int &index );
   void CreateMapping( int replaced_index, int existing_index ) { import_mappings.Add( Mapping( replaced_index, existing_index ) ); }
   void DeleteMappings();
   int MaximumLoadNumber();
   void ClearComponentList();
   void CleanList();

   void PurgeComponentReference( ComponentReference *cr ) { component_references->Delete( cr ); }
   void MoveComponent( float XCoord, float YCoord );
   void AddHandles();
   void DeleteHandles();
   ComponentReference *FindEnclosingComponent( float fX, float fY, Map *map = NULL );
   void Draw( Presentation *ppr );
   void DrawNewComponent( Presentation *ppr, new_component & nc );
   void ResizeComponent();
   void CreateNewComponent( new_component & nc );
   void CutComponent();
   void DeleteSelectedComponents();
   void CopyComponent();
   void PasteComponent();
   void Reset();
   void LowerLastComponent();
   void BindEnclosedFigures() { active_component->BindEnclosedFigures(); }
   void UnbindEnclosedFigures() { active_component->UnbindEnclosedFigures(); }
   void BindEnclosedComponents();
   void UnbindEnclosedComponents() { active_component->UnbindEnclosedComponents(); }
   void ChangeHandleMode();
   void DetermineSelectedComponents( float lb, float rb, float tb, float bb );
   void AddSelectedComponent( ComponentReference *new_component );
   bool ComponentsSelected() { return( selected_components.Size() != 0 ? TRUE : FALSE ); }
   bool SingleSelection();
   void ResetSelectedComponents();
   void SelectAllComponents();
   void ShiftSelectedComponents( float x_offset, float y_offset );
   void ValidatePositionShifts( float x_offset, float y_offset, float& x_limit, float& y_limit );

   void AlignTop();
   void AlignBottom();
   void CenterVertically();
   void AlignLeft();
   void AlignRight();
   void CenterHorizontally();
   void DistributeHorizontally();
   void DistributeVertically();
   
   void OutputDXL( std::ofstream &dxl_file ); // for DXL export
   
private:

   void DetermineParent( ComponentReference *component );
   
   Cltn<ComponentReference *> *component_references, selected_components;
   Cltn<Component *> component_pool; // list of component definitions global to design
   Cltn<Handle *> *handles;
   Cltn<Mapping> import_mappings;
   ComponentReference *copy_component;
   Handle *active_handle;
   float xoffset, yoffset;
   
};


#endif
