#ifndef LQNDEFS_H
#define LQNDEFS_H

// maximum number of characters for a number
#define NUMBER_LENGTH 8
// maximum number of characters for a task name
#define TASK_NAME_LENGTH 96
// maximum number of characters for an entry name
#define ENTRY_NAME_LENGTH TASK_NAME_LENGTH + NUMBER_LENGTH
// maximum number of characters for an activity name
#define ACTIVITY_NAME_LENGTH ENTRY_NAME_LENGTH + NUMBER_LENGTH
// maximum number of characters for a device name
#define DEVICE_NAME_LENGTH 48
// maximum number of characters in the complete path of a filename
#define FILE_NAME_LENGTH 128

// call types for LQN activities
// UNRESOLVED: call type is not yet determined
// NOCALL: does not make a call
// ASYNC: asynchronous call
// SYNC: synchronous call
typedef enum { UNRESOLVED, NOCALL, ASYNC, SYNC } call_type;  

// connection types for  LQN activities
// NOCONNECTION: activity does not connect to any subsequent activity
// NORMAL: activity connects to a single subsequent activity
// REPLY: activity makes a synchronous reply
// ORFORK: activity is an OR fork with 2+ outgoing branches
// ORJOIN: activity is an OR join with 2+ incoming branches
// ANDFOR: activity is an AND fork with 2+ outgoing branches
// ANDJOIN: activity is an AND join with 2+ incoming branches
// ANDSYNC: activity is an AND synchronization
//           with 2+ incoming branches and 2+ outgoing branches
typedef enum { NOCONNECTION, NORMAL, REPLY, ORFORK, ORJOIN,\
	       ANDFORK, ANDJOIN, ANDSYNC, JOINBRANCH } connection_type;

// component boundary crossing types
// NO_COMP: not in a component
// STAY: stay in the same component
// ENTER: enter a component (from no component)
// LEAVE: leave a component (to no component)
// CHANGE: change from one component to another component
typedef enum { NO_COMP, STAY, ENTER, LEAVE, CHANGE } xing_type;  

// call types for LQN messages
// UNRESOLVED_MSG: message type is not yet determined
// CALL_MSG: message is a call, the type of call is not yet determined
// ASYNC_MSG: message is an asynchronous call
// SYNC_MSG: message is a synchronous call
// FWD_MSG: message is part of a forwarding chain
// REPLY_MSG: message is a reply
typedef enum { UNRESOLVED_MSG, CALL_MSG, ASYNC_MSG, SYNC_MSG, REPLY_MSG } msg_type;  

#endif
