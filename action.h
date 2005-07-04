/***********************************************************
 * File:			action.h
 * Author:			XiangYang He
 *                              July 2002
 *
 * Modification history:        Modified Feb 2003
                                Shuhua Cui
  ***********************************************************/

#ifndef ACTION_H
#define ACTION_H

#include "path_data.h"
#include "component_ref.h"
#include "condition.h"
#include <fstream>


class Action : public Path_data {

public:

      Action();
      Action(int, const char *, action_type);
      ~Action();
      void Description(const char *);
      Path_data * SaveScenarioXML( FILE * file);
      Path_data * SaveScenarioDXL( FILE * file, const char * parentID );   // Added by Bo Jiang, for DXL exporting.  July, 2004
      char * GetName() { return name; }
      bool HasName();
      void PrintXMLData( FILE * fp );
      void PrintDXLData( FILE * fp ,const char * parentID );   // Added by Bo Jiang, for DXL exporting.  July, 2004
      void SetComponentName( const char * );
      void SetComponentId( int );
      void SetComponentRole( const char * );
      void SetComponentReferenceId( int);    // Added by Bo Jiang,.  Sept, 2004
      int GetHyperedge() ;
 private:
      int hyperedge_id;
      char name[LABEL_SIZE+1];
      action_type type;
      char * description;
      char * component_name;
      int component_id;
      int component_reference_id;
      char * component_role;
      ComponentReference * containingComponent;

};

#endif
