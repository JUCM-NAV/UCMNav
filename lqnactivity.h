#ifndef LQNACTIVITY_H
#define LQNACTIVITY_H

#include "collection.h"
#include "defines.h"
#include "hyperedge.h"
#include "lqndefs.h"

class LqnEntry;
class LqnTask;

class LqnActivity {
public:
  // constructors
  LqnActivity( const char* name, LqnTask* task, bool print );
  LqnActivity( const char* name, LqnTask* task, Hyperedge* edge, bool print );
  LqnActivity( const char* name, LqnTask* task, double service_time,\
	       bool print );
  LqnActivity( const char* name, LqnTask* task, Hyperedge* edge,
	       double service_time, bool print );
  ~LqnActivity();  // destructor

  void Init();

  const char* GetName() { return my_name; }
  void AddToName( const char* add );

  LqnTask* GetTask() { return my_task; }

  void SetEdge( Hyperedge* edge ) { my_edge = edge; }
  Hyperedge* GetEdge() { return my_edge; }

  void SetEdgeVisits( int visits ) { my_num_edge_visits = visits; }
  void IncrementEdgeVisits() { my_num_edge_visits++; }
  int GetEdgeVisits() { return my_num_edge_visits; }

  void SetServiceTime( double serv_time ) { my_service_time = serv_time; }
  double GetServiceTime() { return my_service_time; }

  void SetEntry( LqnEntry* entry ) { my_entry = entry; }
  LqnEntry* GetEntry() { return my_entry; }

  void SetConnection( connection_type connection ) { my_connection = connection; }
  connection_type GetConnection() { return my_connection; }

  bool FindActivity( LqnActivity* activity, Cltn<LqnActivity*>* activities );
  // add a predecessor to the activity
  void AddPredecessor( LqnActivity* predecessor );
  Cltn<LqnActivity*>* GetPredecessors() { return my_predecessors; }
  int PredecessorCount() { return my_predecessors->Size(); }
  // add a successor to the activity
  void AddSuccessor( LqnActivity* successor );
  Cltn<LqnActivity*>* GetSuccessors() { return my_successors; }
  int SuccessorCount() { return my_successors->Size(); }

  bool IsFirstActivity() { return am_first_activity; }
  void SetFirstActivity() { am_first_activity = true; }
  void UnsetFirstActivity() { am_first_activity = false; }

  bool IsStochastic() { return am_stochastic; }
  void SetStochastic() { am_stochastic = true; }
  void UnsetStochastic() { am_stochastic = false; }

  void SetReply();
  bool IsReply() { return my_connection == REPLY; }
  void SetReplyTarget( LqnEntry* entry );
  LqnEntry* GetReplyTarget() { return my_reply_target; }

  bool IsCall() { return my_call_type != NOCALL; }
  void SetCall() { SetCallType( UNRESOLVED ); }
  bool SetCallType( call_type call );
  call_type GetCallType() { return my_call_type; }
  void SetNumCalls( double num_calls );
  double GetNumCalls() { return my_num_calls; }
  void SetCallTarget( LqnEntry* entry );
  void SetCallTarget( LqnEntry* entry, call_type call );
  void SetLoopCallTarget( LqnEntry* entry );
  void SetLoopCallTarget( LqnEntry* entry, double num_calls );
  LqnEntry* GetCallTarget() { return my_call_target; }

  void SetLoopCall( double num_calls );
  void SetReps( double repetitions ) { my_num_reps = repetitions; }
  double GetReps() { return my_num_reps; }

  void SetBranchProb( double probability ) { my_branch_prob = probability; }
  double GetBranchProb() { return my_branch_prob; }

  // updates the activity name before printing
  void UpdateName();
  // prints activity definition information to the specified file
  void FilePrint( FILE* outfile );
  // prints activity connection information to the specified file
  bool FilePrintConnections( FILE* outfile );
  // determines if there is activity connection information to print
  bool CanFilePrintConnections();

protected:
  char my_name[ACTIVITY_NAME_LENGTH];  // activity name
  double my_service_time;
  bool print_me;

  LqnTask* my_task;  // pointer to the parent task of the activity
  LqnEntry* my_entry;  // entry that the activity belongs to
  Hyperedge* my_edge;  // hyperedge the activity corresponds to

  Cltn<LqnActivity*>* my_predecessors;  // activities preceding this activity
  Cltn<LqnActivity*>* my_successors;  // activities succeeding this activity
  connection_type my_connection;  // type of connection this activity has with other activities

  bool am_first_activity;
  bool am_stochastic;
  bool am_reply;
  int my_num_edge_visits;  // number of times the corresponding edge has been visited
  double my_branch_prob;  // probability of being chosen after an OR fork
  double my_reply_prob;  // reply probability to call (???)
  double my_num_calls;  // number of calls made
  double my_num_reps;  // number of times the activity is repeated

  call_type my_call_type;  // type of call made
  LqnEntry* my_call_target;  // entry called by this activity
  LqnEntry* my_reply_target;  // entry replied to by this activity
};

#endif
