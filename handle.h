/***********************************************************
 *
 * File:			handle.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef HANDLE_H
#define HANDLE_H


#include "component_ref.h"
#include "presentation.h"

typedef enum {TOP_LEFT, TOP_MIDDLE, TOP_RIGHT, 
	      MIDDLE_LEFT, MIDDLE_RIGHT,
	      BOTTOM_LEFT, BOTTOM_MIDDLE, BOTTOM_RIGHT} etHandle_type;

class Handle {

public:

   const float HANDLE_WIDTH;
   const float HANDLE_HEIGHT;

   Handle( etHandle_type htNew_type, ComponentReference *new_cr );
   ~Handle() {};
   void MoveHandle( float fX, float fY, int iResize_decoupled = 0 );
  
   void Draw( Presentation *ppr );

   int PointinHandle( float fCheck_x, float fCheck_y );
   void GetPosition( float &fX, float &fY );

private:

   ComponentReference *component_ref;
   etHandle_type htType;

};


#endif
