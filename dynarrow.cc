/***********************************************************
 *
 * File:			dynarrow.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:	Modified to be characteristic
 *				of responsibilities
 *				January 1997
 *
 ***********************************************************/

#include "dynarrow.h"
#include "resp_figure.h"
#include "resp_ref.h"
#include "display.h"
#include <math.h>

#define PI 3.14159265358979

static char type_names[][10] = {
   
   "move",
   "move-stay",
   "create",
   "destroy",
   "copy" };

static char position_names[][6] = {

   "up",
   "down",
   "left",
   "right" };

DynamicArrow::DynamicArrow( arrow_types new_type, arrow_directions new_direction, const char *pool )
{
   arrow_type = new_type;
   arrow_direction = new_direction;  
   fLength = .05;
   source_pool[19] = 0;

   if( pool )
      strncpy( source_pool, pool, 19 );
   else
      source_pool[0] = 0;
}

DynamicArrow::DynamicArrow( const char *type, const char *direction, const char *sourcepool, float length )
{
   arrow_type = ArrowType( type );;
   arrow_direction = (( strcmp( "in", direction ) == 0 ) ? INTO : OUTOF );
   fLength = length;
   source_pool[19] = 0;
   strncpy( source_pool, sourcepool, 19 );
}

DynamicArrow::~DynamicArrow() {}

void DynamicArrow::SetupArrow( float X, float Y, arrow_positions arrow_position )
{

   float fStart_x, fStart_y, fEnd_x, fEnd_y;
   float fOffset;
   
   fStart_x = X;
   fStart_y = Y;

   if( arrow_type == MOVE_STAY || arrow_type == COPY )
      fOffset = .012;
   else
      fOffset = 0.0;

   switch( arrow_position ) {
     
   case ARROW_UP:

      fEnd_x = fStart_x;
      fEnd_y = fStart_y - fLength;

      if( arrow_direction == INTO )
	 fEnd_y += fOffset;
      else
	 fStart_y -= fOffset;
      break;

   case ARROW_DOWN:

      fEnd_x = fStart_x;
      fEnd_y = fStart_y + fLength;

      if( arrow_direction == INTO )
	 fEnd_y -= fOffset;
      else
	 fStart_y += fOffset;
      break;

   case ARROW_RIGHT:

      fEnd_x = fStart_x + fLength;
      fEnd_y = fStart_y;

      if( arrow_direction == INTO )
	 fEnd_x -= fOffset;
      else
	 fStart_x += fOffset;
      break;

   case ARROW_LEFT:

      fEnd_x = fStart_x - fLength;
      fEnd_y = fStart_y;

      if( arrow_direction == INTO )
	 fEnd_x += fOffset;
      else
	 fStart_x -= fOffset;
      break;
   }

   if( arrow_direction == INTO ) {

      float fT = fStart_x;
      fStart_x = fEnd_x;
      fEnd_x = fT;
    
      fT = fStart_y;
      fStart_y = fEnd_y;
      fEnd_y = fT;
   }
  
   if( arrow_position == ARROW_UP ) {
      if( arrow_direction == INTO )
	 fEnd_y -= .01;
      else
	 fEnd_y += .01;
   } else if( arrow_position == ARROW_DOWN ) {
      if( arrow_direction == INTO )
	 fEnd_y += .01;
      else
	 fEnd_y -= .01;
   } else if( arrow_position == ARROW_RIGHT ) {
      if( arrow_direction == INTO )
	 fEnd_x += .01;
      else
	 fEnd_x -= .01;
   } else if( arrow_position == ARROW_LEFT ) {
      if( arrow_direction == INTO )
	 fEnd_x -= .01;
      else
	 fEnd_x += .01;
   }

   if( arrow_direction == INTO ) { 
      eX = fStart_x;
      eY = fStart_y;
   }
   else {
      eX = fEnd_x;
      eY = fEnd_y;
   }
}

void DynamicArrow::Draw( Presentation *ppr, HyperedgeFigure *resp_figure )
{

   float fStart_x, fStart_y, fEnd_x, fEnd_y, fAnnotation_x, fAnnotation_y;
   arrow_positions arrow_position;
   
   resp_figure->GetPosition( sX, sY );
   arrow_position = ((ResponsibilityReference *)resp_figure->Edge())->ArrowPosition();
   
   this->SetupArrow( sX, sY, arrow_position );

   if( arrow_direction == OUTOF ) {
      fStart_x = sX;	fStart_y = sY;
      fEnd_x = eX;	fEnd_y = eY;
   } 
   else {
      fStart_x = eX;	fStart_y = eY;    
      fEnd_x = sX;	fEnd_y = sY;
   }

   ppr->DrawLine( fStart_x, fStart_y, fEnd_x, fEnd_y );

   if( arrow_type == MOVE_STAY || arrow_type == COPY ) {
      if( arrow_position == ARROW_UP || arrow_position == ARROW_DOWN ) {
	 ppr->DrawLine( fStart_x-.008, fStart_y, fStart_x+.008, fStart_y );
      } else {
	 ppr->DrawLine( fStart_x, fStart_y-.008, fStart_x, fStart_y+.008 );
      }
   }

   float fTheta = atan2( fStart_y-fEnd_y, fEnd_x-fStart_x ) + 5.0*PI/4.0;

   float fArrow_end_x = fEnd_x;
   float fArrow_end_y = fEnd_y;

   float rgfX[4] = {
      fArrow_end_x, 
      fArrow_end_x + .015*sin( fTheta ), 
      fArrow_end_x + .015*sin( fTheta+PI/2.0 ), 
      fArrow_end_x 
   };

   float rgfY[4] = {
      fArrow_end_y,
      fArrow_end_y + .015*cos( fTheta ) ,
      fArrow_end_y + .015*cos( fTheta+PI/2.0 ) ,
      fArrow_end_y
   };

   ppr->DrawFilledPoly( rgfX, rgfY, 4 );

   if( arrow_type == COPY || arrow_type == CREATE  || arrow_type == DESTROY ) {
     
      fAnnotation_x = fStart_x, fAnnotation_y = fStart_y;

      switch( arrow_position ) {
      case ARROW_UP:
	 fAnnotation_x += .012;
	 if( arrow_direction == INTO )
	    fAnnotation_y += .015;
	 else
	    fAnnotation_y -= .015;
	 break;
      case ARROW_DOWN:
	 fAnnotation_x += .012;
	 if( arrow_direction == INTO )
	    fAnnotation_y -= .015;
	 else
	    fAnnotation_y += .015;
	 break;
      case ARROW_RIGHT:
	 fAnnotation_y -= .008;
	 if( arrow_direction == INTO )
	    fAnnotation_x -= .015;
	 else
	    fAnnotation_x += .015;
	 break;
      case ARROW_LEFT:
	 fAnnotation_y -= .008;
	 if( arrow_direction == INTO )
	    fAnnotation_x += .015;
	 else
	    fAnnotation_x -= .015;
	 break;
      }

      if( arrow_type == COPY || arrow_type == CREATE )
	 ppr->DrawText( fAnnotation_x, fAnnotation_y, "+" );
      else
	 ppr->DrawText( fAnnotation_x, fAnnotation_y, "-" );
   }
}

void DynamicArrow::SetAttributes( arrow_types new_type, arrow_directions new_direction )
{
   arrow_type = new_type;
   arrow_direction = new_direction;
}

void DynamicArrow::GetAttributes( arrow_types &type, arrow_directions &direction )
{
   type = arrow_type;
   direction = arrow_direction;
}

char * DynamicArrow::ActionDescription()
{
   return( type_names[arrow_type] );
}

char * DynamicArrow::Position( arrow_positions position )
{
   return( position_names[position] );
}

arrow_types DynamicArrow::ArrowType( const char *type )
{
   int i = 0;

   while( strcmp( type_names[i++], type ) != 0 );
   return( (arrow_types)(i-1) );
}

arrow_positions DynamicArrow::ArrowPosition( const char *position )
{
   int i = 0;

   while( strcmp( position_names[i++], position ) != 0 );
   return( (arrow_positions)(i-1) );
}
