 /***********************************************************
 *
 * File:			component.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:        redone May 1997
 *				separated from component
 *				references December 1999
 ***********************************************************/

#ifndef COMPONENT_H
#define COMPONENT_H

#include "collection.h"
#include <fstream>
#include "map.h"
#include <stdio.h>
#include <string.h>

class HyperedgeFigure;

typedef enum {TEAM, OBJECT, PROCESS, ISR, POOL, AGENT, OTHERCOM, PLUGIN_MAP, NO_TYPE} etComponent_type;
typedef enum {P_NONE, P_COMPONENT, P_PLUGIN} etPool_type;

class Component {
   
public:
   
   Component();
   Component( int component_id, const char *name, bool is_formal, int col, const char *description );
   ~Component();

   void SetCharacteristics( const char *type, bool prot, bool replicated, 
			    const char *replication_factor, bool slot, int proc_id );
   void SetType( etComponent_type ectNew_type ) {  component_type = ectNew_type; } // access methods for component type
   etComponent_type GetType() { return( component_type ); }
   void SetAttributes( int new_dynamic, int new_stack, int new_protected ); // access methods for component attributes
   void GetAttributes( int &new_dynamic, int &new_stack, int &new_protected );
   void SetColour( int new_colour ) { colour = new_colour; } // access methods for component colour
   int GetColour() { return( colour ); }

   const char * GetLabel() { return( component_label ); }
   void SetLabel( const char *new_name );
   int ProcessorId() { return( processor_id ); }
   void ProcessorId( int new_id ) { processor_id = new_id; }
   const char * ProcessorName();
   const int GetComponentNumber() { return( component_number ); } // returns integer identifier

   void IncrementCount() { reference_count++; } // increments reference count
   void DecrementCount(); // decrements count and deletes itself if count is zero
   bool IsNotReferenced() { return( reference_count == 0 ); }
   void SaveXML( FILE *fp );
   void SaveCSMXML( FILE *fp );
   void GeneratePostScriptDescription( FILE *ps_file );
   void SavePoolPlugins( FILE *fp ); // saves the bindings of all plugins that are part of pools

   bool Actual() { return( actual ); } // access methods for actual, anchored flags
   void Actual( int act ) { actual = act; }

   char* GetReplNumber () { return ( poolnum ); }
   void SetReplNumber ( const char* rnum ) { strcpy(poolnum, rnum); }
   int GetDrawReplNumber ();

   etPool_type GetPoolType() { return ( pool_type ); }
   void SetPoolType( const etPool_type ptype ) { pool_type = ptype; }
   int GetNPoolType() { return( (int)pool_type ); }
   void PoolComponentType( const etComponent_type pctype ) { pool_component_type = pctype; }
   void PoolComponentType( const char *type );
   etComponent_type PoolComponentType() { return ( pool_component_type ); }

   char* Description() { return( component_description ); }
   void Description( const char* newdesc );

   void AddPoolPlugin( Map *pluginmap );
   void RemovePoolPlugin( Map *pluginmap );
   void ClearPoolPlugins();
   Map *CurrentPoolPlugin() { return( pool_plugins.CurrentItem() ); }
   void FirstPoolPlugin() { pool_plugins.First(); }
   void NextPoolPlugin() { pool_plugins.Next(); }
   bool PoolPluginsInclude( Map *pluginmap );

   static void ResetComponentCount() { number_components = 0; } // resets global count of components
   int LoadIdentifier() { return( load_number ); }

   void ResetVisited() { visited = FALSE; }
   void SetVisited() { visited = TRUE; }
   bool Visited() { return( visited ); }

   const char * ComponentType(); // returns string describing component type
   
private:

   const char * ComponentStringType( etComponent_type ctype );     // returns text with component type in it

   static char type_names[][8];
   
   int colour; // the component's colour
   int dynamic, stack, is_protected; // flags specifying component characteristics
   etComponent_type component_type; // enumerated type for component type
   char poolnum[5];
   etPool_type pool_type;
   etComponent_type pool_component_type;

   int reference_count, component_number, load_number; // reference count, current and past integer identifiers
   static int number_components; // global count of number of components
   char component_label[LABEL_SIZE+1];  // user given component label
   int processor_id; // user selected hardware processor
   char *component_description;
   Cltn<Map *> pool_plugins;
   bool actual, visited;  // flag specifying if component is actual or formal and utility flag
   
};

#endif

