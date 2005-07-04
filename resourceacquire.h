/***********************************************************
 *
 * File:			resourceacquire.h
 * Author:			Yong Xiang Zeng
 * Created:			March 2005
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef RESOURCEACQUIRE_H
#define RESOURCEACQUIRE_H

#include "hyperedge.h"
//#include "condition.h"
//#include <fstream>


class Resourceacquire : public Hyperedge {

   
public:

	Resourceacquire() : Hyperedge() {}
	~Resourceacquire() {}

   virtual edge_type EdgeType() { return( RESOURCEACQUIRE ); }  // returns identifier for subclass type
   //virtual bool Perform( transformation trans, execution_flag execute ); // executes or validates the transformation with code trans

   //virtual Label *PathLabel();  // returns the hyperedge's path label object
   //virtual void SetLabel( Label *new_label );  // allows label object to be changed

   virtual void SaveXMLDetails( FILE *fp ){}  // implements Save method for this empty hyperedge, in XML format
   virtual void SaveDXLDetails( FILE *fp ) { PrintXMLText( fp, "<abort></abort>\n" ); }  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ){}  // implements Save method for this empty hyperedge, in XML format
  // virtual void SaveCSMResourceAcquire( FILE *fp ){} //Implements Save method for resource acquire element as an independent step
   //virtual void SaveCSMResourceRelease( FILE *fp ) {} //Implements Save method for resource release element as an independent step
   //virtual bool DeleteHyperedge();
   
   //void PathNumber( int new_number ) { path_number = new_number; }  // sets the indentifier of the semi path
   //int PathNumber() { return( path_number ); }  // returns the identifier of the semi path
   //void InstallSimpleElement( Hyperedge *new_ref ) { AddSimple( new_ref ); }
   void SetAcquiredComponent(Component* comp) { acquire=comp;}  
   Component* GetAcquiredComponent() { return acquire;}
   
   
private:

  
   //Label *label;  // path label object
   Component * acquire; // acquired component
   //int path_number; // flag representing annotations to this empty point ( failure point / shared responsibility ) and stub binding label
};

#endif
