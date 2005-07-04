#ifndef LQNGENERATOR_H
#define LQNGENERATOR_H

//UCMNav includes
#include "defines.h"
#include "collection.h"
#include "component.h"
#include "component_ref.h"
#include "devices.h"
#include "hyperedge.h"
#include "hypergraph.h"
#include "loop.h"
#include "map.h"
#include "node.h"
#include "or_fork.h"
#include "or_join.h"
#include "resp_ref.h"
#include "responsibility.h"
#include "result.h"
#include "start.h"
#include "stub.h"
#include "synchronization.h"
#include "transformation.h"

//LqnGenerator includes
#include "lqn.h"


class LqnGenerator {
public:
  // return pointer to sole instance, create instance on first call
  static LqnGenerator* Instance();
  ~LqnGenerator();  // destructor
  void Init();  // initialize internal variables
  void Clear();  // clear internal variables
  void Reset();  // reset internal variables

  // generate LQN from UCM
  void Transmorgrify( Cltn<Map*>* maps, Cltn<Device*>* devices );

  // methods to create LQN elements
  void CreateDevices( Cltn<Device*>* devices );
  void CreateTasks( Cltn<ComponentReference*>* component_refs );

  void Start2Lqn( Start* start_pt );
  void Edge2Lqn( Hyperedge* edge, Hyperedge* in_edge );
  void Fork2Lqn( Hyperedge* fork_edge, Cltn<Hyperedge*>* next_edges,\
		 LqnTask* fork_task, connection_type edge_connection );
  void Join2Lqn( Hyperedge* join_edge, Cltn<Hyperedge*>* next_edges,\
		 Cltn<Hyperedge*>* prev_edges, LqnTask* join_task,\
		 LqnActivity* join_activity, connection_type edge_connection );
  void TraverseEdge( Hyperedge* edge, Hyperedge* next_edge );

  void ComponentBoundaryXing2Lqn( Hyperedge* edge, Hyperedge* next_edge );
  void LeaveComponent2Lqn( Hyperedge* edge );
  void EnterComponent2Lqn( Hyperedge* next_edge );
  xing_type FindXing( Hyperedge* edge1, Hyperedge* edge2 );

  // return the loop body task if needed
  LqnTask* CheckForLoopBodyTask( LqnTask* task );
  // return the type of synchronization encountered
  connection_type SyncType( Hyperedge* edge );

  void NextEdges( Hyperedge* edge, Cltn<Hyperedge*>* next );
  void PrevEdges( Hyperedge* edge, Cltn<Hyperedge*>* previous );
  
  LqnMstack* Mstack() { return mstack; }
  void TestMstack();

protected:
  LqnGenerator();  // private constructor to enforce singleton pattern

  Lqn* lqn;  // LQN being created
  Cltn<LqnMstack*>* mstacks;  // set of Mstacks used
  LqnMstack* mstack;  // pointer to the current Mstack
  int mstack_count;
  bool lbody_flag;
  Map* current_map;
  LqnTask* lbody_task;
  Cltn<Hyperedge*>* starts;  // start points for a map
  Cltn<Hyperedge*>* root_starts;  // start points on the top level root map
  bool dynamic_stub;
  Stub* parent_stub;
  LqnActivity* stub_join_activity;
  bool plugin_exit;

  bool print;
};

#ifndef MAINWINDOW 
// pointer to sole instance of the LqnGenerator
static LqnGenerator* SoleLqnGenerator;
#endif

#endif
