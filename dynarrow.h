/***********************************************************
 *
 * File:			dynarrow.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:	Modified for new design
 *				to use figures
 *				August 1996
 *				Modified to be characteristic
 *				of responsibility
 *
 ***********************************************************/

#ifndef DYNAMIC_ARROW_H
#define DYNAMIC_ARROW_H

#include <stdio.h>
#include "presentation.h"
#include "defines.h"
#include <fstream>
#include <string.h>

typedef enum {MOVE, MOVE_STAY, CREATE, DESTROY, COPY} arrow_types;
typedef enum {ARROW_UP, ARROW_DOWN, ARROW_RIGHT, ARROW_LEFT} arrow_positions;
typedef enum {INTO, OUTOF} arrow_directions;

class DisplayManager;
class HyperedgeFigure;

class DynamicArrow {

public:

   DynamicArrow( arrow_types new_type, arrow_directions new_direction, const char *pool = NULL );
   DynamicArrow( const char *type, const char *direction, const char *sourcepool, float length );
   
   ~DynamicArrow();

   void Draw( Presentation *ppr, HyperedgeFigure *resp_figure ); // implements Draw method

   void SetupArrow( float sX, float sY, arrow_positions arrow_position );  // sets up the arrows coordinates according to user selections of characteristics
   float GetLength() { return( fLength ); }
   void SetLength( float fNew_length ) { fLength = fNew_length; }

   // access methods for pointer attributes
   void SetAttributes( arrow_types new_type, arrow_directions new_direction );
   void GetAttributes( arrow_types &type, arrow_directions &direction );

   const char * SourcePool() { return( source_pool ); } // access methods for source pool name
   void SourcePool( const char *new_pool ) { strncpy( source_pool, new_pool, 19 ); }
   
   char * ActionDescription(); // returns textual description of arrow action
   const char * Direction() { return( (arrow_direction == INTO) ? "in" : "out" ); }  // returns textual description of arrow direction

   static char * Position( arrow_positions position );  // returns textual description of arrow position
   static arrow_positions ArrowPosition( const char *position );
   
private:

   arrow_types ArrowType( const char *type );

   float sX, sY, eX, eY; // the starting and ending coordinates of the arrow
   arrow_types arrow_type; // type of arrow
   arrow_directions arrow_direction; // direction of arrow
   char source_pool[20];  // the name of the pool from which the component came 
   float fLength; // length of arrow

};

#endif
