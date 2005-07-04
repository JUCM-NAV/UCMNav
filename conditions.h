/***********************************************************
 * File:			conditions.h
 * Author:			XiangYang He
 *                              August 2002
 *
 * Modification history:        Feb 2003
 *                              Shuhua Cui
 *
 ***********************************************************/

#ifndef CONDITIONS_H
#define  CONDITIONS_H

#include "path_data.h"
#include "component_ref.h"
#include "condition.h"
#include <fstream>


class Conditions : public Path_data {

public:

      Conditions();
      Conditions( int, const char *,  const char * );
      ~Conditions();
      Path_data * SaveScenarioXML( FILE * file);
      Path_data * SaveScenarioDXL( FILE * file, const char * parentID );     // Added by Bo Jiang, for DXL exporting.  July, 2004
      void PrintXMLData( FILE * fp );
      void PrintDXLData( FILE * fp ,const char * parentID );    // Added by Bo Jiang, for DXL exporting.  July, 2004
      char * GetLabel() { return label; }   //for bebuging purpose
      char * GetName() { return label; }    //the same as above method
      bool HasName();
      virtual void SetComponentName( const char * ) {}
      virtual void SetComponentId( int ) {}
      virtual void SetComponentRole( const char * ) {}
      virtual void SetComponentReferenceId( int){}    // Added by Bo Jiang,.  Sept, 2004
      int GetHyperedge();
/*
      void SetComponentName( const char * )
      void SetComponentId( int )
      void SetComponentRole( const char * )
*/
private:
       int hyperedge_id;
      char label[LABEL_SIZE+1];
      char expression[LABEL_SIZE*2+1];
      /*
      char * component_name;
      int component_id;
      char * component_role;
      */

};

#endif

