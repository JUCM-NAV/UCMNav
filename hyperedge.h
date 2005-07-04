/***********************************************************
 *
 * File:			hyperedge.h
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef HYPEREDGE_H
#define HYPEREDGE_H

#include "transformation.h"
#include "collection.h"
#include "node.h"
#include "label.h"
#include "xml_generator.h"
#include <fstream>
#include "path_data.h"

extern DisplayManager *display_manager;
extern Label *current_label;

class Hyperedge;
class HyperedgeFigure;

class Hyperedge {

public:

   Hyperedge();
   virtual ~Hyperedge();

   virtual void AttachSource( Node *new_node );  // polymorphic method for attaching source nodes to this hyperedge
   virtual void AttachTarget( Node *new_node );  // polymorphic method for attaching target nodes to this hyperedge
   virtual void DetachSource( Node *node );      // polymorphic method for detaching source nodes
   virtual void DetachTarget( Node *node );      // polymorphic method for detaching target nodes

   void ReplaceSource( Node *old_node, Node *new_node ) { source->Replace( old_node, new_node ); new_node->AttachNext( this ); }
   void ReplaceTarget( Node *old_node, Node *new_node ) { target->Replace( old_node, new_node ); new_node->AttachPrevious( this ); }

   Cltn<Node *> * SourceSet() { return ( source ); }  // access method for source node list
   Cltn<Node *> * TargetSet() { return ( target ); }  // access method for target node list
   Node * PrimaryTarget();  // returns the main target nodes of the path (A,B), ignoring D connection nodes
   Node * PrimarySource();  // returns the main source nodes of the path (A,B), ignoring C connection nodes
   Node * ExceptionPath();  // returns the first D connection node which starts a timeout or exception path
   Hyperedge * FirstOutput() { return( target->GetFirst()->NextEdge() ); }
   Hyperedge * FirstInput() { return( source->GetFirst()->PreviousEdge() ); }
   virtual HyperedgeFigure * GetFigure() { return ( figure ); } // default access method for figure object

   bool HasSourceColour( nodeColour colour );  // returns flag as to whether a source node of a certain colour exists
   bool HasTargetColour( nodeColour colour );  // returns flag as to whether a target node of a certain colour exists
   Node * SourceOfColour( nodeColour colour ); // returns source node matching colour
   Node * TargetOfColour( nodeColour colour ); // returns target node matching colour

   bool DeletePath();  // prompts user for confirmation, deletes entire path
   void DeleteSimpleEdge();
   int DeleteElement();

   void Transform( transformation trans );  // actually performs the transformation trans
   void TransformDouble( transformation trans, Hyperedge *edge );  // actually performs the double selection transformation trans with edge
   bool Validate( transformation trans );  // validates whether transformation trans is valid
   bool ValidateDouble( transformation trans, Hyperedge *edge );  // validates whether double selection transformation transformation trans is valid
   virtual bool Perform( transformation trans, execution_flag execute );  // polymorphic method, validates or performs transformation trans
   // polymorphic method, validates or performs double selection transformation trans
   virtual bool PerformDouble( transformation trans, Hyperedge *edge, execution_flag execute );

   virtual edge_type EdgeType()=0; // polymorphic method which returns integer flag denoting subclass type
   virtual const char * EdgeName();

   virtual void EdgeSelected();  // polymorphic method which is called when the map element corresponding to this hyperedge is selected
   virtual void SaveXMLDetails( FILE *fp )=0;  // polymorphic method which saves hyperedge's specific data to file, in XML format
   virtual void SaveDXLDetails( FILE *fp )=0;  //   Added by Bo Jiang, for DXL exporting.  August, 2004
   virtual void SaveCSMXMLDetails( FILE *fp )=0;  // polymorphic method which saves hyperedge's specific data to file, in XML format
   virtual void AddCSMResourceAcquire (); 
   virtual void AddCSMResourceRelease ();
   virtual void AddCSMEndResourceRelease () {}
   void AddResourceAcquire(Component* comp);
   void AddResourceRelease(Component* comp, ComponentReference* comp_ref);
   bool Normalize( );
   virtual Component* GetAcquiredComponent() { return NULL; }
   virtual Component* GetReleasedComponent() { return NULL; }
   virtual ComponentReference* GetReleasedComponentRef() {return NULL; }
   virtual Hyperedge * GenerateMSC( bool first_pass );
   virtual Hyperedge * ScanScenarios( scan_type type );
   virtual void ResetGenerationState() {}
   virtual void DeleteGenerationData() {}
   virtual void SavePathBranchingSpec( FILE *fp ) {} // polymorphic method which saves branch selection characterisitics of forks
   void SaveXML( FILE *fp ); // method which saves hyperedge's general data to file, in XML format
   void SaveCSMXML( FILE *fp ); // method which saves hyperedge's general data to file, in XML format
   void SaveXMLConnections( FILE *fp ); // method which saves hyperedge's next hyperedges, in XML format
   void SaveCSMXMLConnections( FILE *fp ); // method which saves hyperedge's next hyperedges, in XML format
   void SaveCSMXMLMulPathConnections( FILE *fp );//method which saves Fork and Join 's next hyperedes , in XML format
   void SaveDXL( FILE *fp );    //added by Bo Jiang for graph info exporting in DXL for DOORS, August, 2004
   virtual void OutputPerformanceData( std::ofstream &performance_file ) {}
   virtual const char * HyperedgeName();  // polymorphic method which returns identifier for hyperedge
   virtual int Search( search_direction dir ); // polymorphic method used to search for path joinings by searching for Start hyperedges
   virtual bool HasName(); // polymorphic method returning flag if hyperedge has name defined
   virtual bool HasConditions() { return FALSE; }
   virtual bool LabelCentered() { return TRUE; }
   virtual bool IsAndJoin() { return FALSE; }
   virtual void GeneratePostScriptDescription( FILE *ps_file );
   virtual bool HasTextualAnnotations() { return( HasConditions() || HasDescription() ); }
   virtual bool ReplacePath( Path *new_path, Path *old_path, Label *new_label, search_direction sdir );
   // replace path objects along path, return flag if end reached
   virtual Path * InputPath();
   virtual bool DeleteHyperedge() { return FALSE; }
   virtual int DoubleClickAction() { return UNMODIFIED; }

   void GeneratePSElementDescription( FILE *ps_file );
   bool HasDescription();
   void DeleteEdge();

   virtual void VerifyAnnotations() {}
   virtual void ResetHighlight() {}

   int GetNumber() { return( hyperedge_number ); } // returns the internal integer identifier for this hyperedge
   int LoadIdentifier() { return( load_number ); } // returns the file loading integer identifier

   virtual Label * PathLabel();  // returns the path's path label object
   virtual void SetLabel( Label *new_label );  // allows label object to be changed

   virtual char * Description() { return( unique_desc ); }
   virtual void Description( const char* newdesc );

   static void ResetHyperedgeCount() { number_hyperedges = 0; }  // resets global count of hyperedges, used when editing new files
   static int NumberHyperedges() { return( number_hyperedges ); } // Apr2005 gM: required for map import to avoid conflict with constant IDs

   bool Visited() { return( visited ); } // returns the visited flag
   void ResetVisited() { visited = FALSE; visited_by = 0; }  // resets the visited and visited_by flag
   void SetVisited() { visited = TRUE; } // sets the visited flag
   int VisitedBy() { return( visited_by ); }  // returns the visited_by flag
   void ResetVisitedBy() { visited_by = 0; }  // resets the visited_by flag
   void SetVisitedBy( int visiting_id ) { visited_by = visiting_id; }  // sets the visited_by flag to the visiting_id

   Map * ParentMap() { return( parent_map ); }

   static Node * SearchNode() { return( search_node ); }
   virtual Hyperedge * TraverseScenario();
  // virtual void RegisterTriggerPoint(Path_data * ) = 0;
   virtual  Hyperedge * HighlightScenario( );

#ifdef DEBUG
   void Output();  // debugging method which outputs data for hyperedge
#endif

   virtual void RegisterWaitingElement( Path_data * p_data ) { }
   virtual bool SearchRoot( Cltn<Path_data *> & waiting_element, Path_data * waiting_place );
   virtual void SetDestination( Path_data * des );
   virtual void Collapse( Path_data * des, bool incompleteSearch, Path_data * waiting_place );
   virtual void AddForkAfter( Path_data * root, Path_data * des );
   virtual void MoveToWaitingPlace( Path_data * root, Path_data * des, Path_data * waiting_place );
   virtual Path_data * FindDataWithCount( int count, Cltn<Path_data *> & waiting_element );
   virtual Path_data *  AddSynchronizationPoint( Path_data * root,  Cltn<Path_data *> & waiting_element, Path_data * waiting_place );
   virtual void ResetReferenceCount(  Cltn<Path_data *> & waiting_element );
   virtual Path_data * Advance(Path_data * current_data );

protected:

   void DeletePathSection();  // deletes the path containing this hyperedge

   Cltn<Node *> *source, *target; // lists of source and target nodes
   HyperedgeFigure *figure;  // figure object which manages graphical operation of map element
   Map *parent_map;
   char *unique_desc;

   int hyperedge_number, load_number;  // various integer counters
   bool visited;
   int visited_by;

   static int number_hyperedges;
   static Node *search_node;
   static bool joined_paths;
};

#endif
