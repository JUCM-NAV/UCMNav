/***********************************************************
 *
 * File:			DUMMY.h
 * Author:			yong xiang zeng
 * Created:			May 2005
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef DUMMY_H
#define DUMMY_H

#include "hyperedge.h"
#include "or_join.h"
#include "synchronization.h"
#include "condition.h"
#include <fstream>

#define NONE 1
#define FAILURE_POINT 2
#define SHARED_RESPONSIBILITY 3
#define DIRECTION_ARROW 4

class PointFigure;
class Presentation;

class Dummy : public Empty {

   friend class PointFigure;
   friend class DisplayManager;

public:

	Dummy() :Empty() {}
	~Dummy() { }

   virtual edge_type EdgeType() { return( DUMMY ); }  // returns identifier for subclass type
   
   virtual void SaveXMLDetails( FILE *fp ) {}  // implements Save method for this empty hyperedge, in XML format
   virtual void SaveCSMXMLDetails( FILE *fp ) {}  // implements Save method for this Dummy hyperedge, in XML format
   //virtual void SaveCSMResourceAcquire( FILE *fp ){} //Implements Save method for resource acquire element as an independent step
   //virtual void SaveCSMResourceRelease( FILE *fp ); //Implements Save method for resource release element as an independent step
   //virtual void AddCSMResourceAcquire();
   //virtual void AddCSMResourceRelease();
   //void AddResourceAcquire(Component* comp);
   //void AddResourceRelease(Component* comp);
   

private:
 

   
};

#endif
