/***********************************************************
 *
 * File:			resourceacquire.h
 * Author:			Yong Xiang Zeng
 * Created:			March 2005
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef RESOURCERELEASE_H
#define RESOURCERELEASE_H

#include "hyperedge.h"
//#include "condition.h"
//#include <fstream>


class Resourcerelease : public Hyperedge {

   
public:

	Resourcerelease() : Hyperedge() {}
	~Resourcerelease() {}

   virtual edge_type EdgeType() { return( RESOURCERELEASE ); }  // returns identifier for subclass type
   //virtual bool Perform( transformation trans, execution_flag execute ); // executes or validates the transformation with code trans

   //virtual Label *PathLabel();  // returns the hyperedge's path label object
   //virtual void SetLabel( Label *new_label );  // allows label object to be changed

   virtual void SaveXMLDetails( FILE *fp ){}  // implements Save method for this empty hyperedge, in XML format
   virtual void SaveDXLDetails( FILE *fp ) { PrintXMLText( fp, "<abort></abort>\n" ); }  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){}  // implements Save method for this empty hyperedge, in XML format
   //virtual void SaveCSMResourceAcquire( FILE *fp ){} //Implements Save method for resource acquire element as an independent step
   //virtual void SaveCSMResourceRelease( FILE *fp ) {} //Implements Save method for resource release element as an independent step
   //virtual bool DeleteHyperedge();
   
   //void PathNumber( int new_number ) { path_number = new_number; }  // sets the indentifier of the semi path
   //int PathNumber() { return( path_number ); }  // returns the identifier of the semi path
   //void InstallSimpleElement( Hyperedge *new_ref ) { AddSimple( new_ref ); }
   void SetReleasedComponent ( Component * ref) { release=ref;}
   void SetReleasedComponentRef (ComponentReference* ref) { release_ref = ref;}
   Component* GetReleasedComponent() { return release;}
   ComponentReference* GetReleasedComponentRef() { return release_ref;}
   
private:

  
   //Label *label;  // path label object
   Component * release; // acquired component
   ComponentReference * release_ref;
   //int path_number; // flag representing annotations to this empty point ( failure point / shared responsibility ) and stub binding label
};

#endif
