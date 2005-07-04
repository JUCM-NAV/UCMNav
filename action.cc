/***********************************************************
 * File:			action.cc
 * Author:			XiangYang He
 *                              July 2002    
 *
 * Modification history:        modified Feb 2003
                                Shuhua Cui
 ***********************************************************/

#include "action.h"
#include "defines.h"
#include "print_mgr.h"
#include <stdlib.h>
#include "scenario_generator.h"
#include <unistd.h>
#include "xml_generator.h"
#include <iostream>
extern int  parID;  // global variable for par ID generation in DXL exporting to DOORS , added by Bo Jiang August 2004
extern int  seqID;  // global variable for seq ID generation in DXL exporting to DOORS , added by Bo Jiang August 2004
 
char * TypeName[] = {"Start", "End_Point", "WP_Enter", "WP_Leave", "Timer_Set", 
                     "Timer_Reset","Time_Out", "And_join", "Resp", "Waiting_place", 
		     "Fork", "Connect_Start", "Connect_End", "Trigger_End" };

class Path_data;

Action::Action() : Path_data() { }

Action::Action(int new_id, const char * new_name, action_type new_type): Path_data() 
{
    name[LABEL_SIZE] = 0;
    strncpy( name, new_name, LABEL_SIZE );
    hyperedge_id = new_id;
    type = new_type;
    description = NULL;
    component_name = NULL;
    component_role = NULL;
    std::cout << "Action name = " << name << std::endl;
}

void Action::Description(const char * new_description) 
{
   if( description != NULL ) free( description );
   description = ( PrintManager::TextNonempty( new_description)
                              ? strdup( new_description ) : NULL );
}

void Action::SetComponentName( const char * new_name)
{
   if( component_name != NULL ) free( component_name );
       component_name = ( PrintManager::TextNonempty( new_name)
                              ? strdup( new_name ) : NULL );
}


void Action::SetComponentRole( const char * new_role)
{

   // strcpy( component_role, new_role);

   if( component_role != NULL ) free( component_role );
       component_role = ( PrintManager::TextNonempty( new_role)
                              ? strdup( new_role ) : NULL );
			      
}

void Action::SetComponentId( int new_id )
{
   component_id = new_id;
}

 // Added by Bo Jiang,.  Sept, 2004
void  Action::SetComponentReferenceId(int   componentRefId)   
{
   component_reference_id=componentRefId;
}

Action::~Action()
{
  if( description != NULL ) {
	  std::cout << description;
	  free( description );
  }
  // Should free component_name and component_role as well???
  if( component_name != NULL ) {
	  std::cout << component_name;
	  free( component_name );
  }
  if( component_role != NULL ) {
	  std::cout << component_role;
	  free( component_role );
  }
}

bool Action::HasName()
{
  return name[0] == 0? false : true;
}

// This method first save the scenario element in XML format. Then it checks its pointer to next,
// if there are multiple elements, it must be a parallel block, so it calls method SaveParallelXML 
// to print the whole block, otherwise it returns the next element.
Path_data * Action::SaveScenarioXML( FILE * outfile)
{
   Path_data * current_data = NULL;
 //  char buffer[100]; 
   if( type != Waiting_place && type != Fork ) {
      PrintXMLData( outfile ); 
   }   
   if( this->NextPath()->Size() > 1 ) {	 
      current_data = this->NextPath()->GetFirst()->GetDestination();
   //   sprintf( buffer, "par" );
/****
// Requires an update to component_name etc. in Synchronization::TraverseScenario( )
      if(component_role == NULL)
      {
      sprintf( buffer, "par component-name = \"%s\" component-id = \"%d\" component-role = \"%s\" " ,
                        component_name, component_id, "");
      }else{
         sprintf( buffer, "par component-name = \"%s\" component-id = \"%d\" component-role = \"%s\" " ,
                        component_name, component_id, component_role);
      }
****/
      PrintNewXMLBlock( outfile, "par");
      SaveParallelXML(outfile, this);
      PrintEndXMLBlock( outfile, "par");
   }
   else {
      if( this->NextPath()->Size() == 1 ) {
         current_data = this->NextPath()->GetFirst();
      }
   }   
   return current_data;    
}


// The actual method to save the Action element in XML format 
void Action::PrintXMLData( FILE * fp )
{
   char buffer[100];
  
   sprintf( buffer, "<do hyperedge-id=\"%d\" ", hyperedge_id );
   PrintXMLText( fp, buffer );
   if( this->HasName() )
      fprintf( fp, "name=\"%s\" ", name );
   fprintf( fp, "type=\"%s\" ", TypeName[type] );
   if( component_name != NULL ) {
      fprintf(fp, "component-name = \"%s\" ", component_name );
      fprintf(fp, "component-id= \"%d\" ", component_id );
      if (component_role != NULL){
         fprintf(fp, "component-role= \"%s\" ",  component_role );
      }
      else{
         fprintf(fp, "component-role= \"%s\" ", "");
      }
   }
   fprintf( fp, "/>\n" );
}



// This method first save the scenario element in DXL format. Then it checks its pointer to next,
// if there are multiple elements, it must be a parallel block, so it calls method SaveParallelDXL 
// to print the whole block, otherwise it returns the next element.
Path_data * Action::SaveScenarioDXL( FILE * outfile, const char * parentID )
{
   Path_data * current_data = NULL;
   char buffer[100]; 
   if( type != Waiting_place && type != Fork ) {
      PrintDXLData( outfile, parentID ); 
   }   
   if( this->NextPath()->Size() > 1 ) {	 
      current_data = this->NextPath()->GetFirst()->GetDestination();
      sprintf(buffer,"par( \"par%d\", %s )\n",parID,parentID);
      PrintXMLText(outfile,buffer);
      sprintf( buffer, "\"par%d\"",parID );
      parID++;
      IndentNewXMLBlock(outfile);
      SaveParallelDXL(outfile, this,buffer);
      IndentEndXMLBlock(outfile);
   }
   else {
      if( this->NextPath()->Size() == 1 ) {
         current_data = this->NextPath()->GetFirst();
      }
   }   
   return current_data;    
}


// The actual method to save the Action element in DXL format 
void Action::PrintDXLData( FILE * fp, const char * parentID )
{
   char buffer[100];
   sprintf( buffer, "doElement( \"h%d\", ", hyperedge_id  );
   PrintXMLText( fp, buffer );  
   if( this->HasName() ){
      fprintf( fp, "\"%s\"", name );
   }else{
      fprintf( fp, "\"\"" );
   }
   fprintf( fp, ", \"%s\"", TypeName[type] );
   if( PrintManager::TextNonempty(description ) ){   
       fprintf( fp, ", \"%s\"", PrintDescription(description ));   
   }
   else {
       fprintf( fp, ", \"\"" );   
   }
   if( component_name != NULL ) {
      fprintf(fp, ", \"%s\"", component_name );
      fprintf(fp, ", \"cr%d\"", component_reference_id );
      if (component_role != NULL){
         fprintf(fp, ", \"%s\"",  component_role );
      }
      else{
         fprintf(fp, ", \"\"" );
      }
   }
   else {
         fprintf(fp, ", \"\"" );
         fprintf(fp, ", \"\"" );
         if (component_role != NULL) {
            fprintf(fp, ", \"%s\"", component_role );
         }
         else{
            fprintf(fp, ", \"\"" );
         }   
   }
   fprintf( fp, ", %s )\n", parentID);
}

int  Action::GetHyperedge(){
  return hyperedge_id;
}
