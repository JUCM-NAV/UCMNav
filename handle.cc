/***********************************************************
 *
 * File:			handle.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "handle.h"

Handle::Handle( etHandle_type htNew_type, ComponentReference *new_reference ) : HANDLE_WIDTH(0.010), HANDLE_HEIGHT(0.010) {
   htType = htNew_type;
   component_ref = new_reference;
}

void Handle::MoveHandle( float fX, float fY, int iResize_decoupled )
{
   float fOld_x, fOld_y, fNew_x, fNew_y, fOld_width, fOld_height;
   float fNew_width, fNew_height;
   // float min_width = 10000.0, min_height = 10000.0;
   etComponent_type component_type = TEAM;

   if( component_ref->ReferencedComponent() ) component_type = component_ref->ReferencedComponent()->GetType();

   if( !component_ref ) return;

   component_ref->GetPosition( fOld_x, fOld_y );
   fOld_width = component_ref->GetWidth();
   fOld_height = component_ref->GetHeight();

   switch( htType ) {
   case TOP_LEFT:

      if ( component_type == PROCESS ) {
	 if( (fX - (fOld_height / 10.0)) < WINDOW_MIN ) {
	    fNew_x = WINDOW_MIN + (fOld_height / 10.0);
	    fNew_width = fOld_width + fOld_x-fNew_x;
	 } else {
	    fNew_x = fX + ( fOld_height / 10.0 );
	    fNew_width = fOld_width + fOld_x-fNew_x;
	 }
      }
      else {
	 if( fX < WINDOW_MIN ) {
	    fNew_x = WINDOW_MIN;
	    fNew_width = fOld_width - (WINDOW_MIN-fOld_x);
	 } else {	 
	    fNew_x = fX;
	    fNew_width = fOld_width - (fX-fOld_x);
	 }
      }
      
      if( fY < WINDOW_MIN ) {
	 fNew_y = WINDOW_MIN;
	 fNew_height = fOld_height - (WINDOW_MIN-fOld_y);
      } else {	 
	 fNew_y = fY;
	 fNew_height = fOld_height - (fY-fOld_y);
      }

      break;
      
   case TOP_MIDDLE:
      
      if( fY < WINDOW_MIN ) {
	 fNew_y = WINDOW_MIN;
	 fNew_height = fOld_height - (WINDOW_MIN-fOld_y);
      } else {	 
	 fNew_y = fY;
	 fNew_height = fOld_height - (fY-fOld_y);
      }

      fNew_x = fOld_x;
      fNew_width = fOld_width;

      break;

   case TOP_RIGHT:

      if( fX > WINDOW_MAX )
	 fNew_width = WINDOW_MAX-fOld_x;
      else
	 fNew_width = fX-fOld_x;

      if( fY < WINDOW_MIN )
	 fNew_height = fOld_height + fOld_y-WINDOW_MIN;
      else
	 fNew_height = fOld_height + fOld_y-fY;

      fNew_x = fOld_x;
      fNew_y = fY;

      break;

   case MIDDLE_LEFT:
      
      if ( component_type == PROCESS ) {
	 if( (fX - (fOld_height / 20.0)) < WINDOW_MIN ) {
	    fNew_x = WINDOW_MIN + (fOld_height / 10.0);
	    fNew_width = fOld_width + fOld_x-fNew_x;
	 } else {
	    fNew_x = fX + ( fOld_height / 20.0 );
	    fNew_width = fOld_width + fOld_x-fNew_x;
	 }
      }
      else {
	 if( fX < WINDOW_MIN ) {
	    fNew_x = WINDOW_MIN;
	    fNew_width = fOld_width - (WINDOW_MIN-fOld_x);
	 } else {	 
	    fNew_x = fX;
	    fNew_width = fOld_width - (fX-fOld_x);
	 }
      }

      fNew_y = fOld_y;
      fNew_height = fOld_height;

      break;

   case MIDDLE_RIGHT:

      if ( component_type == PROCESS ) {
	 if( (fX + (fOld_height / 20.0)) > WINDOW_MAX )
	    fNew_width = WINDOW_MAX - fOld_x;
	 else
	    fNew_width = fX + ( fOld_height / 20.0 ) - fOld_x;
      }
      else {
	 if( fX > WINDOW_MAX )
	    fNew_width = WINDOW_MAX-fOld_x;
	 else
	    fNew_width = fX-fOld_x;
      }

      fNew_x = fOld_x;
      fNew_y = fOld_y;
      fNew_height = fOld_height;

      break;

   case BOTTOM_LEFT:

      if ( component_type == PROCESS ) {
	 if( (fX - (fOld_height / 10.0)) < WINDOW_MIN ) {
	    fNew_x = WINDOW_MIN + (fOld_height / 10.0);
	    fNew_width = fOld_width + fOld_x-fNew_x;
	 } else {
	    fNew_x = fX + ( fOld_height / 10.0 );
	    fNew_width = fOld_width + fOld_x-fNew_x;
	 }
      }
      else {
	 if( fX < WINDOW_MIN ) {
	    fNew_x = WINDOW_MIN;
	    fNew_width = fOld_width - (WINDOW_MIN-fOld_x);
	 } else {	 
	    fNew_x = fX;
	    fNew_width = fOld_width - (fX-fOld_x);
	 }
      }

      fNew_y = fOld_y;
      if( fY > WINDOW_MAX )
	 fNew_height = WINDOW_MAX - fOld_y;
      else
	 fNew_height = fY - fOld_y;

      break;

   case BOTTOM_MIDDLE:
      
      fNew_y = fOld_y;
      if( fY > WINDOW_MAX )
	 fNew_height = WINDOW_MAX - fOld_y;
      else
	 fNew_height = fY - fOld_y;

      fNew_x = fOld_x;
      fNew_width = fOld_width;

      break;

   case BOTTOM_RIGHT:
      
      if ( component_type == PROCESS ) {
	 if( (fX + (fOld_height / 10.0)) > WINDOW_MAX )
	    fNew_width = WINDOW_MAX - fOld_x;
	 else
	    fNew_width = fX - fOld_x + (fOld_height / 10.0);
      }
      else {
	 if( fX > WINDOW_MAX )
	    fNew_width = WINDOW_MAX-fOld_x;
	 else
	    fNew_width = fX-fOld_x;
      }

      fNew_x = fOld_x;
      fNew_y = fOld_y;
      if( fY > WINDOW_MAX )
	 fNew_height = WINDOW_MAX - fOld_y;
      else
	 fNew_height = fY - fOld_y;
      break;

   default:
      break;
   }

   if( fNew_width >= MINIMUM_COMPONENT_DIMENSION ) {                // width is ok
      if( fNew_height >= MINIMUM_COMPONENT_DIMENSION ) {            // both are ok
	 component_ref->SetWidth( fNew_width, iResize_decoupled );
	 component_ref->SetHeight( fNew_height, iResize_decoupled );
	 component_ref->SetPosition( fNew_x, fNew_y );
      } else {                                                      // only width is ok
	 component_ref->SetWidth( fNew_width, iResize_decoupled );
	 component_ref->SetPosition( fNew_x, fOld_y );
      }     
   } else {                                                         // either only height is ok or none are ok
      if( fNew_height >= MINIMUM_COMPONENT_DIMENSION ) {            
	 component_ref->SetHeight( fNew_height, iResize_decoupled );
	 component_ref->SetPosition( fOld_x, fNew_y );
      }
   }
   // unmentioned but implied possibility:  if neither are ok, do nothing.
  
}

void Handle::Draw( Presentation *ppr )
{
   float fX, fY;

   GetPosition( fX, fY );
   ppr->FillRegion( fX, fY, HANDLE_WIDTH, HANDLE_HEIGHT );
}

void Handle::GetPosition( float &fX, float &fY )
{
   float fComponent_x, fComponent_y, fWidth, fHeight;
   etComponent_type component_type = TEAM;

   if( component_ref->ReferencedComponent() ) component_type = component_ref->ReferencedComponent()->GetType();

   if( !component_ref )
      return;

   component_ref->GetPosition( fComponent_x, fComponent_y );
   fWidth = component_ref->GetWidth();
   fHeight = component_ref->GetHeight();
  
   switch( htType ) {
   case TOP_LEFT:
      if ( component_type == ISR ) {
	 fX = 1000;
	 fY = 1000;} else {
	    fX = fComponent_x - HANDLE_WIDTH/2.0;
	    fY = fComponent_y - HANDLE_HEIGHT/2.0; }
      break;
   case TOP_MIDDLE:
      if ( component_type == POOL ) {
	 fX = 1000;
	 fY = 1000;} else {
	    fX = fComponent_x + (fWidth/2);
	    fY = fComponent_y - HANDLE_HEIGHT/2.0;}
      break;
   case TOP_RIGHT:
      if ( component_type == ISR ) {
	 fX = 1000;
	 fY = 1000;} else {
	    fX = fComponent_x + fWidth - HANDLE_WIDTH/2.0;
	    fY = fComponent_y - HANDLE_WIDTH/2.0;}
      break;
   case MIDDLE_LEFT:
      if ( component_type == ISR ) {
	 fX = 1000;
	 fY = 1000;} else {
	    fX = fComponent_x - HANDLE_WIDTH/2.0;
	    fY = fComponent_y + (fHeight/2.0);}
      if ( component_type == PROCESS ) {
	 fX = fX - (fHeight/20.0);}
      break;
   case MIDDLE_RIGHT:
      if ( component_type == ISR ) {
	 fX = 1000;
	 fY = 1000;} else {
	    fX = fComponent_x + fWidth - HANDLE_WIDTH/2.0;
	    fY = fComponent_y + (fHeight/2.0);}
      if ( component_type == PROCESS ) {
	 fX = fX - (fHeight/20.0);}
      break;
   case BOTTOM_LEFT:
      fX = fComponent_x - HANDLE_WIDTH/2.0;
      fY = fComponent_y + fHeight - HANDLE_HEIGHT/2.0;
      if ( component_type == PROCESS ) {
	 fX = fX - (fHeight/10.0);}
      break;
   case BOTTOM_MIDDLE:
      fX = fComponent_x + (fWidth/2.0);
      fY = fComponent_y + fHeight - HANDLE_HEIGHT/2.0;
      if ( component_type == PROCESS ) {
	 fX = fX - (fHeight/10.0);}
      break;
   case BOTTOM_RIGHT:
      fX = fComponent_x + fWidth - HANDLE_WIDTH/2.0;
      fY = fComponent_y + fHeight - HANDLE_HEIGHT/2.0;
      if ( component_type == PROCESS ) {
	 fX = fX - (fHeight/10.0);}
      break;
   }
}

int Handle::PointinHandle( float fCheck_x, float fCheck_y ) {
   float fX, fY;

   GetPosition( fX, fY );

   if( fCheck_x >= fX
       && fCheck_x <= fX+HANDLE_WIDTH
       && fCheck_y >= fY
       && fCheck_y <= fY+HANDLE_HEIGHT ) {
      return( 1 );
   } else {
      return( 0 );
   }
}
