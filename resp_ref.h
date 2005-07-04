/***********************************************************
 *
 * File:			resp-ref.h
 * Author:			Andrew Miga
 * Created:			April 1999
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef RESPONSIBILITY_REF_H
#define RESPONSIBILITY_REF_H

#include "responsibility.h"
#include "hyperedge.h"
#include "node.h"
#include "transformation.h"
#include "resp_figure.h"
#include "dynarrow.h"
#include <fstream>

class ComponentManager;

class ResponsibilityReference : public Hyperedge {

public:

   ResponsibilityReference();
   ResponsibilityReference( int resp_id, float x, float y, const char *direction, int parent_id, const char *position ); // constructor for file loading
   ~ResponsibilityReference();

   virtual edge_type EdgeType() { return( RESPONSIBILITY_REF ); }  // returns identifier for subclass type
   virtual void EdgeSelected() { responsibility->EditConditions(); }
   virtual const char * HyperedgeName();  // returns name of responsibility which is used as an identifier
   virtual bool HasConditions() { return( responsibility->HasConditions() ); }
   virtual void SaveXMLDetails( FILE *fp ); // implements Save method for responsibility references, in XML format
   virtual void SaveDXLDetails( FILE *fp ); //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp ); // implements Save method for responsibility references, in XML format

   virtual void OutputPerformanceData( std::ofstream &pf );
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual void VerifyAnnotations();
   virtual bool DeleteHyperedge() { return( this->DeleteElement() ); }
   virtual int DoubleClickAction() { return( EditResponsibility( TRUE ) ); }

   virtual char * Description() { return( responsibility->Description() ); }
   virtual void Description( const char* newdesc ) { responsibility->Description( newdesc ); }

   bool DeleteResponsibility( execution_flag execute ); // transformation which deletes this responsibility and replaces it with an empty point
   bool EditResponsibility( execution_flag execute ); // allows user to edit responsibility characteristics through dialog
   bool Perform( transformation trans, execution_flag execute ); // executes or validates transformations with code trans

   void ParentResponsibility( Responsibility *new_parent );
   Responsibility * ParentResponsibility() { return( responsibility ); }

   static const char *Direction( etResponsibility_direction direction );
   arrow_positions ArrowPosition() { return( arrow_position ); }
   void ArrowPosition( arrow_positions new_position ) { arrow_position = new_position; }

   void SetInstall( Empty *e ) { empty = e; }
   void Install();
   virtual Hyperedge * TraverseScenario( );
   Hyperedge * HighlightScenario( );

private:

   etResponsibility_direction Direction( const char *direction );

   Responsibility *responsibility;
   Empty *empty;
   arrow_positions arrow_position; // position of optional dynamic arrow
};

#endif
