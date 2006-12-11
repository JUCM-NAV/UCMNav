/***********************************************************
 *
 * File:			component.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:        redone May 1997
 *                              Andrew Miga
 *
 ***********************************************************/

#include "component.h"
#include "display.h"
#include "component_mgr.h"
#include "xml_generator.h"
#include "print_mgr.h"
#include "xml_mgr.h"
#include "devices.h"
#include <stdlib.h>

extern ComponentManager *component_manager;

int Component::number_components = 0;

char Component::type_names[][8] = {

   "team",
   "object",
   "process",
   "isr",
   "pool",
   "agent",
   "other",
   "plugin",
   "none"
};

static char component_types[][8] = {
   
   "Team",
   "Object",
   "Process",
   "ISR",
   "Pool",
   "Agent",
   "Other" };

Component::Component()
{
   dynamic = 0;
   stack = 0;
   is_protected = 0;
   pool_component_type = (etComponent_type)0;
  
   component_type = TEAM;
   actual = TRUE;
   SetColour( 0 );
   component_label[0] = 0;
   component_label[LABEL_SIZE] = 0;
   processor_id = NA;
   poolnum[4] = 0;
   component_number = number_components++;
   SetReplNumber( "2" );
   component_description = NULL;
   reference_count = 0;
}

Component::Component( int component_id, const char *name, bool is_formal, int col, const char *description )
{
   load_number = component_id;
   
   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file
   component_number = component_id;    
   if (number_components <= component_number) number_components = component_number+1;    
   //End of the modification by Bo Jiang, April, 2005
   
   component_label[LABEL_SIZE] = 0;
   strncpy( component_label, component_manager->VerifyUniqueComponentName( name ), LABEL_SIZE );
   component_manager->AddComponent( this );
   actual = !is_formal;
   colour = col;
   component_description = (( description != NULL ) ? strdup( description ) : NULL );

   dynamic = 0;
   stack = 0;
   is_protected = 0;
   pool_component_type = (etComponent_type)0;
   processor_id = NA;
   poolnum[4] = 0;
   //component_number = number_components++;     //Modified by Bo Jiang. April,2005, fixed that ID changes when load a .ucm file
   SetReplNumber( "2" );
   reference_count = 0;
}

Component::~Component()
{
   component_manager->RemoveComponent( this );
}

void Component::DecrementCount()
{
   if( --reference_count == 0 )
      delete this;
}

void Component::SetLabel( const char *new_name )
{
   strncpy( component_label, new_name, LABEL_SIZE );
   component_manager->RenameComponent( this );
}

void Component::SetCharacteristics( const char *type, bool prot, bool replicated, const char *replication_factor,
				    bool slot, int proc_id )
{
   int i = 0;

   while( !strequal( type_names[i++], type ) );
   component_type = (etComponent_type)(i-1);

   is_protected = prot;
   stack = replicated;
   strncpy( poolnum, replication_factor, 4 );
   dynamic = slot;
   processor_id = proc_id;
}

void Component::SetAttributes( int new_dynamic, int new_stack, int new_protected )
{
   dynamic = new_dynamic;
   stack = new_stack;
   is_protected = new_protected;
}

void Component::GetAttributes( int &new_dynamic, int &new_stack, int &new_protected )
{
   new_dynamic = dynamic;
   new_stack = stack;
   new_protected = is_protected;
}

void Component::SaveXML( FILE *fp )
{
   char buffer[1000];
   
   sprintf( buffer, "<component component-id=\"c%d\" component-name=\"%s\" formal=\"%s\" colour=\"%d\"", 
	    component_number, PrintDescription( component_label ), ((actual == FALSE) ? "yes" : "no" ), colour );
   PrintXMLText( fp, buffer );   
   if( PrintManager::TextNonempty( component_description ) )
      fprintf( fp, " description=\"%s\"", PrintDescription( component_description ) );
   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );

   if( component_type != POOL ) {
      sprintf( buffer, "<regular type=\"%s\" protected=\"%s\" replicated=\"%s\" replication-factor=\"%s\" slot=\"%s\"",
	       ComponentStringType( component_type ), ((is_protected == 1) ? "yes" : "no"), ((stack == 1) ? "yes" : "no"), poolnum,
	       ((dynamic == 1) ? "yes" : "no") );
      PrintXMLText( fp, buffer );
      if( processor_id != NA )
	 fprintf( fp, " processor-id=\"dv%d\"", processor_id );
      fprintf( fp, " />\n" );
   }
   else {    // save Pool information
      sprintf( buffer, "pool type=\"%s\" /", ComponentStringType( pool_component_type ) );
      PrintXML( fp, buffer );      
   }
   
   PrintEndXMLBlock( fp, "component" );
   LinebreakXML( fp );   
}

void Component::SaveCSMXML( FILE *fp )
{
    char buffer[1000];
    
   sprintf( buffer, "<Component id=\"c%d\" name=\"%s\"", component_number, PrintDescription( component_label ));
   PrintXMLText( fp, buffer );   
   //if( PrintManager::TextNonempty( component_description ) )
     // fprintf( fp, " description=\"%s\"", PrintDescription( component_description ) );
   if( processor_id != NA )
	 fprintf( fp, " host=\"dv%d\"", processor_id );
   fprintf( fp, " />\n" );
   LinebreakXML( fp );   
}

void Component::GeneratePostScriptDescription( FILE *ps_file )
{
   fprintf( ps_file, "%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( component_label ) );
   if( PrintManager::TextNonempty( component_description ) ) {
      fprintf( ps_file, "%d rom 1 IN ID\n", PrintManager::text_font_size );
      PrintManager::PrintDescription( component_description );
      fprintf( ps_file, "OD\n" );
   }
   fprintf( ps_file, "L\n" );
}

void Component::SavePoolPlugins( FILE *fp )
{
   char buffer[300];
   
   if( component_type == POOL ) {
      if( pool_type == P_PLUGIN ) {
	 for( pool_plugins.First(); !pool_plugins.IsDone(); pool_plugins.Next() ) {
	    if( !XmlManager::MapExportable( pool_plugins.CurrentItem() ) )
	       continue; // skip over maps not contained in an export list
	    sprintf( buffer, "plugin-pool pool-id=\"c%d\" model-id=\"m%d\" /",
		     component_number, pool_plugins.CurrentItem()->GetNumber() );
	    PrintXML( fp, buffer );
	 }
      }
   }
}

const char * Component::ComponentType()
{
   return( component_types[component_type] );
}

int Component::GetDrawReplNumber()
{
   int azesd = 0;

   azesd = atoi( poolnum );
   if( azesd < 1 )
      return( 3 );
   else if( azesd > 6 )
      return( 6 );
   else
      return ( azesd );
}

void Component::Description( const char *new_desc )
{
   if( new_desc != NULL ) {
      if( strlen( new_desc ) > 0 ) {
	 if( component_description != NULL ) {
	    free( component_description );
	    component_description = NULL;
	 }
	 component_description = strdup( new_desc );
      }
   }
}  

const char * Component::ProcessorName()
{
   return( (processor_id != NA) ? DeviceDirectory::Instance()->DeviceName( processor_id ) : NULL );
}

void Component::AddPoolPlugin( Map *pluginmap )
{

   if( pool_plugins.Includes( pluginmap ) )
      std::cerr << "Error: Map already present in pool's plugin list.\n";
   else
      pool_plugins.Add( pluginmap );
}

void Component::RemovePoolPlugin( Map *pluginmap )
{

   if( pool_plugins.Includes( pluginmap ) )
      pool_plugins.Remove( pluginmap );
   else
      std::cerr << "Error: Map not a member of pool's plugin list.\n";
}

void Component::ClearPoolPlugins()
{
   while( !pool_plugins.is_empty() )
      pool_plugins.Detach();

}

bool Component::PoolPluginsInclude( Map *pluginmap )
{
   return( pool_plugins.Includes( pluginmap ) );
}

const char * Component::ComponentStringType( etComponent_type ctype )
{
   return( type_names[ctype] );
}

void Component::PoolComponentType( const char *type )
{
   int i = 0;
   
   while( !strequal( type_names[i++], type ) );
   pool_component_type = (etComponent_type)(i-1);

   if( pool_component_type == PLUGIN_MAP )
      pool_type = P_PLUGIN;
   else if( pool_component_type == NO_TYPE )
      pool_type = P_NONE;
   else
      pool_type = P_COMPONENT;
}
