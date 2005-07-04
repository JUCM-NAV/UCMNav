/***********************************************************
 * File:			conditions.cc
 * Author:			XiangYang He
 *                              August 2002    
 *
 * Modification history:
 *
 ***********************************************************/
 
#include "conditions.h"
#include "component_ref.h"
#include "condition.h"
#include <fstream>
#include "xml_generator.h"
#include "variable.h"
 extern int  parID;  // global variable for par ID generation in DXL exporting to DOORS , added by Bo Jiang August 2004
 extern int  seqID;  // global variable for seq ID generation in DXL exporting to DOORS , added by Bo Jiang August 2004

Conditions::Conditions():Path_data(){ }

Conditions::Conditions ( int new_id, const char * new_label,  const char * new_exp ): Path_data()
{
    label[LABEL_SIZE] = 0;
    expression[LABEL_SIZE*2] = 0;
    strncpy( label, new_label, LABEL_SIZE );
    strncpy( expression, new_exp, LABEL_SIZE*2 );
    hyperedge_id = new_id;
    /*
    component_name = NULL;
    component_role = NULL;
*/
}


Conditions::~Conditions(){ }
/*

void Conditions::SetComponentName( const char * new_name)
{
   if( component_name != NULL ) free( component_name );
       component_name = ( PrintManager::TextNonempty( new_name)
                              ? strdup( new_name ) : NULL );
}


void Conditions::SetComponentRole( const char * new_role)
{

   // strcpy( component_role, new_role);

   if( component_role != NULL ) free( component_role );
       component_role = ( PrintManager::TextNonempty( new_role)
                              ? strdup( new_role ) : NULL );
			      
}

void Condtions::SetComponentId( int new_id )
{
   component_id = new_id;
}

*/  
bool Conditions::HasName()
{
   return label[0] == 0? false : true;
}  
 
// This method first save the condition element in XML format. Then it checks its pointer to next,
// if there are multiple elements, it must be a parallel block, so it calls method SaveParallelXML 
// to print the whole block, and return the destination of the parallel block; otherwise it returns
// the next element.	           
Path_data * Conditions::SaveScenarioXML( FILE * outfile)
{
   Path_data * current_data = NULL;
   PrintXMLData( outfile );
   if( this->NextPath()->Size() > 1 ) {	 
      current_data = this->NextPath()->GetFirst()->GetDestination();
 /*     
      sprintf( buffer, "par component name = \"%s\" component role = \"%s\" component-id = \"%d\" ",
                        component_name, component_role, component_id);
      PrintNewXMLBlock( outfile, buffer);
*/      
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

// The actual method to print condition data in XML format
void Conditions::PrintXMLData( FILE * fp )
{
   char buffer[100];
  
   sprintf( buffer, "<condition hyperedge-id=\"%d\" ", hyperedge_id );
   PrintXMLText( fp, buffer );
   fprintf( fp, "label=\"%s\" ", label );
   // PrintDescriptions substitutes & with &amp;
   // ReferenceToName transforms expression to include real variable names.
   fprintf( fp, "expression =\"%s\" ", 
            PrintDescription( BooleanVariable::ReferenceToName(expression) ) );
  
   fprintf( fp, "/>\n" );
} 
      

// This method first save the condition element in DXL format. Then it checks its pointer to next,
// if there are multiple elements, it must be a parallel block, so it calls method SaveParallelDXL 
// to print the whole block, and return the destination of the parallel block; otherwise it returns
// the next element.	           
Path_data * Conditions::SaveScenarioDXL( FILE * outfile, const char * parentID )
{
   Path_data * current_data = NULL;
   char buffer[100]; 
   PrintDXLData( outfile, parentID );
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

// The actual method to print condition data in DXL format
void Conditions::PrintDXLData( FILE * fp,const char * parentID )
{
   char buffer[100];
   sprintf( buffer, "condition( \"h%d\", ", hyperedge_id );
   PrintXMLText( fp, buffer );
   fprintf( fp, "\"%s\", ", label );
   fprintf( fp, "\"%s\", ", expression );
   fprintf( fp, "%s )\n", parentID );
} 
int Conditions::GetHyperedge(){
  return hyperedge_id;
}

