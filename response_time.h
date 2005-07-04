/***********************************************************
 *
 * File:			response_time.h
 * Author:			Andrew Miga
 * Created:			October 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef RESPONSE_TIME_H
#define RESPONSE_TIME_H

#include "timestamp.h"
#include <string.h>

#define DEFAULT_TIME 0
#define DEFAULT_PERCENTAGE 90

class ResponseTimeManager;

class ResponseTime { // class which encapsulates response time relationships between timestamp points

   friend class ResponseTimeManager;
   
public:

   ResponseTime( Timestamp *t1, Timestamp *t2, const char *new_name, // constructor, accepts timestamp points and user inputted characteristics as parameters
		 int time = DEFAULT_TIME, int pct = DEFAULT_PERCENTAGE );
   void Time( int time ) { response_time = time; } // access methods for relationship elements
   int Time() { return( response_time ); }
   void Percentage( int pct ) { percentage = pct; }
   int Percentage() { return( percentage ); }
   char * Name() { return( name ); }
   void Name( const char *new_name ) { strncpy( name, new_name, 19 ); }
   Timestamp * Timestamps( int index ); // access method for timestamp points
   void Highlight(); // highlights the pair of timestamps on screen
   void Unhighlight(); // unhighlights the pair
   
private:
   
   Timestamp *ts1, *ts2; // the two timestamp points involved in the relationship
   int response_time, percentage; // the desired response time and percentage
   char name[20]; // user given name for relationship

};

class ResponseTimeManager {

public:
   
   ~ResponseTimeManager() {}
   static ResponseTimeManager * Instance(); // returns pointer to sole instance, creates instance on first call
   
   void CreateResponseTime( Timestamp *t1, Timestamp *t2 ); // creates new response time object, invokes dialog
   void EditResponseTime( ResponseTime *rt ); // invokes dialog for existing requirement
   ResponseTime * ResponseTimeExists( Timestamp *t1, Timestamp *t2 ); // determines if a requirement exists between timestamp points t1, t2
   bool ResponseTimesExist( Timestamp *ts ); // determines if timestamp is involved in any requirement
   bool NewResponseTimeValid( Timestamp *t1, Timestamp *t2 ); // determines if a potential requirement is valid, if they can be reached from each other
   ResponseTime * GetResponseTime( int index ) // returns the response time object with given index
   { return( response_times.Get(index) ); }
   ResponseTime * GetResponseTime( int selection, int index )
   { return( timestamp_responses[selection][index-1] ); }
   void DeleteResponseTime( ResponseTime *rt );
   void AddResponseTime( ResponseTime *rt ); // add methods for relationships
   void AddNewResponseTime( Timestamp *tf, Timestamp *ts, const char *nm, int time, int pct );
   void DeleteTimestamp( Timestamp *ts ); // removes timestamp from all relationships
   void ViewResponseTimes( Timestamp *ts ); // invokes dialog for response time requirements for given timestamp
   void ListTimestampRT( Timestamp *ts ); // lists all current relationships for timestamp in list box
   void ViewAllResponseTimes(); // invokes dialog for all response time requirements
   void ListResponseTimes(); // lists all current relationships in list box
   void SaveXML( FILE *fp ); // saves directory to file
   void GeneratePostscriptDescription( FILE *ps_file );

private:

   ResponseTimeManager() {}
   Cltn<ResponseTime *> response_times; // global list of response time relationships for the current design
   bool OrderCorrect( Hyperedge *first, Hyperedge *second );
   
   static ResponseTime *timestamp_responses[2][10];

};


#endif
