/***********************************************************
 *
 * File:			component_ref.cc
 * Author:			Andrew Miga
 * Created:			December 1999
 *
 * Modification history:
 *
 ***********************************************************/

#include <math.h>
#include "component_ref.h"
#include "hyperedge_figure.h"
#include "resp_figure.h"
#include "hyperedge.h"
#include "display.h"
#include "component_mgr.h"
#include "xml_generator.h"
#include "print_mgr.h"
#include "xml_mgr.h"

extern int map_deletion;
extern ComponentManager *component_manager;
extern float left_border, right_border, top_border, bottom_border;
extern ComponentReference *active_component;
extern void DisplayMapDescription();

int ComponentReference::number_component_refs = 0;

#define SPACING 0.01
#define SCALE 20
#define ONE_PIXEL 0.01 / 6
#define TEXT_HEIGHT 0.04

float ComponentReference::STACK_OFFSET = 0.008;
float ComponentReference::PROTECT_OFFSET = 0.007;
char ComponentReference::reference_name[LABEL_SIZE+DISPLAYED_LABEL+5];

static char pool_types[][12] = {
   
   " (Team)",
   " (Object)",
   " (Process)",
   " (ISR)",
   " (Agent)",
   " (Other)",
   " (Unknown)" };

ComponentReference::ComponentReference( float fNew_x, float fNew_y, float fNew_width, float fNew_height, ComponentReference *new_parent )
{
   anchored = 0;  
   component_parent = new_parent;
   component = NULL; // set the referenced component definition to null

   // assign minimum values to dimensions if necessary
   fNew_width = ( fNew_width > MINIMUM_COMPONENT_DIMENSION ) ? fNew_width : MINIMUM_COMPONENT_DIMENSION;
   fNew_height = ( fNew_height > MINIMUM_COMPONENT_DIMENSION ) ? fNew_height : MINIMUM_COMPONENT_DIMENSION;
  
   SetWidth( fNew_width );
   SetHeight( fNew_height );

   if( component_parent ) {
      SetPosition( fNew_x, fNew_y );
      component_parent->AddChild( this );
   } else {
      fX = fNew_x;
      fY = fNew_y;
   }

   ResetGeometry();
   anchored = FALSE;
   component_fixed = FALSE;
   component_ref_number = number_component_refs++;
   selected = FALSE;
   role[0] = 0;
   role[DISPLAYED_LABEL] = 0;
}

ComponentReference::ComponentReference( int component_id, float x, float y, float w, float h, ComponentReference *parent, bool is_anchored,
					int reference_id, Component *referenced_component, const char *component_role )
{
   load_number = component_id;

   //added by Bo Jiang, April,2005, fixed that ID changes when load a .ucm file
   component_ref_number = component_id;
   if (number_component_refs <= component_ref_number) number_component_refs = component_ref_number+1;    
   //End of the modification by Bo Jiang, April, 2005

   anchored = is_anchored;
   component_parent = parent;
   if( parent ) parent->AddChild( this );
   component = (( reference_id != NA ) ? component_manager->FindComponent(  reference_id ) : referenced_component );
   if( component ) component->IncrementCount();

   fX = x;
   fY = y;
   fWidth = w;
   fHeight = h;

   component_fixed = FALSE;
 //  component_ref_number = number_component_refs++;    //Modified by Bo Jiang. April,2005, fixed that ID changes when load a .ucm file
   ResetGeometry(); 
   selected = FALSE;
   role[0] = 0;
   role[DISPLAYED_LABEL] = 0;
   if( component_role ) strncpy( role, component_role, DISPLAYED_LABEL ); 
}

ComponentReference::~ComponentReference()
{
   if( map_deletion ) return;  // if map is being deleted don't bother

   if( component ) component->DecrementCount();

   if( component_parent )
      component_parent->RemoveChild( this );

   for( children.First(); !children.IsDone(); children.Next() )
      children.CurrentItem()->SetParent( NULL );

   for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() )
      dependent_figures.CurrentItem()->ResetEnclosingComponent();

   if( active_component == this ) {
      active_component = NULL;
      DisplayMapDescription();
   }
}

void ComponentReference::ReferencedComponent( Component *new_ref )
{
   new_ref->IncrementCount();
   if( component ) component->DecrementCount();
   component = new_ref;
}

const char * ComponentReference::ReferenceName()
{
   if( component ) {
      if( PrintManager::TextNonempty( role ) ) {
	 strcpy( reference_name, component->GetLabel() );
	 strcat( reference_name, ":" );
	 strcat( reference_name, role );
      } else
	 return( component->GetLabel() );
   }// else
      return "";
}

void ComponentReference::Fixed( bool isfixed )
{
   bool old_state = component_fixed;
   
   component_fixed = isfixed;
   if( component_fixed == TRUE )
      component_manager->DeleteHandles();
   else {
      if( old_state == TRUE )
	 component_manager->AddHandles();
   }
}

void ComponentReference::AddContainedElement( HyperedgeFigure *dependent )
{
   float x, y;
   
   dependent_figures.Add( dependent );
   dependent->GetPosition( x, y );
   dependent->SetEnclosingComponent( this );
   dependent->SetPosition( x, y, TRUE, TRUE, FALSE );
}

bool ComponentReference::PointInBox( float fCheck_x, float fCheck_y )
{
   float fMy_x, fMy_y;
   etComponent_type component_type = this->ComponentType();
   GetPosition( fMy_x, fMy_y );

   switch ( component_type ) {
   case PROCESS :
      if( fCheck_x >=   -0.1 * (fCheck_y - fMy_y - 10 * fMy_x) - ONE_PIXEL
	  && fCheck_x <= -0.1 * (fCheck_y - fMy_y - 10 * fMy_x) + GetWidth() + ONE_PIXEL
	  && fCheck_y >= fMy_y
	  && fCheck_y <= fMy_y+GetHeight() ) {
	 return( TRUE );
      } else {
	 return( FALSE ); }
      break;
   case ISR :
      if( fCheck_x >=    (GetWidth() / GetHeight() * -0.5) * (fCheck_y - fMy_y) + fMy_x + 0.5 * GetWidth() - ONE_PIXEL
	  && fCheck_x <= (GetWidth() / GetHeight() *  0.5) * (fCheck_y - fMy_y) + fMy_x + 0.5 * GetWidth() + ONE_PIXEL
	  && fCheck_y >= fMy_y
	  && fCheck_y <= fMy_y+GetHeight() ) {
	 return( TRUE );
      } else {
	 return( FALSE ); }
      break;
   case AGENT :
      if( fCheck_x >= fMy_x - (3 * ONE_PIXEL)
	  && fCheck_x <= fMy_x + (3 * ONE_PIXEL) + GetWidth()
	  && fCheck_y >= fMy_y
	  && fCheck_y <= fMy_y+GetHeight() ) {
	 return( TRUE );
      } else {
	 return( FALSE ); }
      break;
   default :
      if( fCheck_x >= fMy_x - ONE_PIXEL
	  && fCheck_x <= fMy_x+GetWidth() + ONE_PIXEL
	  && fCheck_y >= fMy_y
	  && fCheck_y <= fMy_y+GetHeight() ) {
	 return( TRUE );
      } else {
	 return( FALSE ); }
      break;
   }
}

#if 0
bool Component::PointOnBorder( float fCheck_x, float fCheck_y )
{
   float fMy_x, fMy_y;
   GetPosition( fMy_x, fMy_y );

   switch ( component->GetType() ) {
   case PROCESS :
      if( fCheck_x >=   -0.1 * (fCheck_y - fMy_y - 10 * fMy_x) - ONE_PIXEL
	  && fCheck_x <= -0.1 * (fCheck_y - fMy_y - 10 * fMy_x) + GetWidth() + ONE_PIXEL
	  && fCheck_y >= fMy_y
	  && fCheck_y <= fMy_y+GetHeight() ) {
	 return( TRUE );
      } else {
	 return( FALSE ); }
      break;
   case ISR :
      if( fCheck_x >=    (GetWidth() / GetHeight() * -0.5) * (fCheck_y - fMy_y) + fMy_x + 0.5 * GetWidth() - ONE_PIXEL
	  && fCheck_x <= (GetWidth() / GetHeight() *  0.5) * (fCheck_y - fMy_y) + fMy_x + 0.5 * GetWidth() + ONE_PIXEL
	  && fCheck_y >= fMy_y
	  && fCheck_y <= fMy_y+GetHeight() ) {
	 return( TRUE );
      } else {
	 return( FALSE ); }
      break;
   case AGENT :
      if( ( fCheck_x >= fMy_x && fCheck_x <= fMy_x + (3*ONE_PIXEL) && fCheck_y >= fMy_y && fCheck_y <= fMy_y+GetHeight() ) ||  // left
	  ( fCheck_x >= fMy_x+GetWidth() - (3*ONE_PIXEL) && fCheck_x <= fMy_x+GetWidth() && fCheck_y >= fMy_y && fCheck_y <= fMy_y+GetHeight() ) || // right
	  ( fCheck_y >= fMy_y && fCheck_y <= fMy_y + (3*ONE_PIXEL) && fCheck_x >= fMy_x && fCheck_x <= fMy_x+GetWidth() ) || // top
	  ( fCheck_y >= fMy_y+GetHeight() - (3*ONE_PIXEL) && fCheck_y <= fMy_y+GetHeight() && fCheck_x >= fMy_x && fCheck_x <= fMy_x+GetWidth() ) ) { // bottom
	 return( TRUE );
      } else {
	 return( FALSE ); }
      break;
   default :
      if( ( fCheck_x >= fMy_x && fCheck_x <= fMy_x + ONE_PIXEL && fCheck_y >= fMy_y && fCheck_y <= fMy_y+GetHeight() ) ||  // left
	  ( fCheck_x >= fMy_x+GetWidth() - ONE_PIXEL && fCheck_x <= fMy_x+GetWidth() && fCheck_y >= fMy_y && fCheck_y <= fMy_y+GetHeight() ) || // right
	  ( fCheck_y >= fMy_y && fCheck_y <= fMy_y + ONE_PIXEL && fCheck_x >= fMy_x && fCheck_x <= fMy_x+GetWidth() ) || // top
	  ( fCheck_y >= fMy_y+GetHeight() - ONE_PIXEL && fCheck_y <= fMy_y+GetHeight() && fCheck_x >= fMy_x && fCheck_x <= fMy_x+GetWidth() ) ) { // bottom
	 return( TRUE );
      } else {
	 return( FALSE ); }
      break;
   }
}
#endif

void ComponentReference::SetPosition( float fNew_x, float fNew_y )
{
   etComponent_type component_type = this->ComponentType();


   if( component_parent ) {

      float fParent_x, fParent_y;
      component_parent->GetPosition( fParent_x, fParent_y );
      fX = (fNew_x - fParent_x)/component_parent->GetWidth();
      fY = (fNew_y - fParent_y)/component_parent->GetHeight();
   } else {

      if ( component_type == PROCESS ) {
	 if( fNew_x - (fHeight/10) < WINDOW_MIN )
	    fX = WINDOW_MIN + (fHeight/10);
	 else if( fNew_x+fWidth > WINDOW_MAX )
	    fX = WINDOW_MAX-fWidth; 
	 else
	    fX = fNew_x;
      }
      else {
	 if( fNew_x < WINDOW_MIN ) 
	    fX = WINDOW_MIN;
	 else if( fNew_x+fWidth > WINDOW_MAX )
	    fX = WINDOW_MAX-fWidth; 
	 else
	    fX = fNew_x;
      }
      
      if( fNew_y < WINDOW_MIN ) 
	 fY = WINDOW_MIN;
      else if( fNew_y+fHeight > WINDOW_MAX )
	 fY = WINDOW_MAX-fHeight; 
      else
	 fY = fNew_y;
   }
 
   NotifyDependents();
}

void ComponentReference::RegisterDependentFigure( HyperedgeFigure *new_figure )
{

   if( dependent_figures.Includes( new_figure ) )
      std::cerr << "Error: Figure already registered as component dependent.\n";
   else
      dependent_figures.Add( new_figure );
}

void ComponentReference::PurgeDependentFigure( HyperedgeFigure *dependent )
{

   if( dependent_figures.Includes( dependent ) )
      dependent_figures.Remove( dependent );
   else
      std::cerr << "Error: Figure already removed as component dependent.\n";
}

void ComponentReference::BindEnclosedFigures()
{
   HyperedgeFigure *figure;
   ComponentReference *encl_component;
   float x, y, xn, yn, xc, yc;
   Cltn<HyperedgeFigure *> *figures = display_manager->Figures();
   
   for( children.First(); !children.IsDone(); children.Next()) 
      children.CurrentItem()->BindEnclosedFigures();

   for( figures->First(); !figures->IsDone(); figures->Next() ) {
      figure = figures->CurrentItem();
      encl_component = figure->GetEnclosingComponent();
      if( (encl_component == NULL) || (encl_component == component_parent) ) {
	 figure->GetPosition( x, y );
	 if( component_manager->FindEnclosingComponent( x, y ) == this ) {
	    dependent_figures.Add( figure );
	    if( (encl_component == component_parent) && component_parent )
	       encl_component->PurgeDependentFigure( figure );
	    figure->SetEnclosingComponent( this );
	    this->GetPosition( xc, yc );
	    xn = (x - xc)/this->GetWidth();
	    yn = (y -yc)/this->GetHeight();
	    figure->SetActualPosition( xn, yn );   // account for different interpretation of coordinates
	 }
      }
   }
}

void ComponentReference::UnbindEnclosedFigures()
{

   for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() )
      dependent_figures.CurrentItem()->ResetEnclosingComponent();
   
   while( !dependent_figures.is_empty() )
      dependent_figures.Detach();
}

void ComponentReference::UnbindEnclosedComponents()
{
   for( children.First(); !children.IsDone(); children.Next() )
      children.CurrentItem()->SetParent( NULL );

   while( !children.is_empty() )
      children.Detach();
}

void ComponentReference::NotifyDependents()
{
   for( children.First(); !children.IsDone(); children.Next()) 
      children.CurrentItem()->NotifyDependents();
   
   for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() )
      dependent_figures.CurrentItem()->PathChanged();
}

void ComponentReference::GetPosition( float &fReturn_x, float &fReturn_y )
{
   float fParent_x, fParent_y;

   if( component_parent ) {
      component_parent->GetPosition( fParent_x, fParent_y );
      fReturn_x = fX*component_parent->GetWidth() + fParent_x;
      fReturn_y = fY*component_parent->GetHeight() + fParent_y;
   }
   else {
      fReturn_x = fX;
      fReturn_y = fY;
   }
}

void ComponentReference::ValidatePositionShift( float x_offset, float y_offset, float& x_limit, float& y_limit )
{
   float x, y, new_x, new_y, width, height;
   etComponent_type component_type = TEAM;

   if( component != NULL )
      component_type = component->GetType();

   if( component_parent ) {
      if( component_parent->Selected() ) // return immediately to avoid shifting
	 return;                         // multiple times
   }

   GetPosition( x, y );
   new_x = x+x_offset;
   new_y = y+y_offset;
   width = GetWidth();
   height = GetHeight();
   
   if ( component_type == PROCESS ) {
      if( new_x - (height/10) < WINDOW_MIN )
	 x_limit = WINDOW_MIN-x_offset-x+(height/10);
      else if( new_x+width > WINDOW_MAX )
	 x_limit = x_offset-(WINDOW_MAX-(x+width));
      else
	 x_limit = 0;
   }
   else {
      if( new_x < WINDOW_MIN )
	 x_limit = WINDOW_MIN-x_offset-x;
      else if( new_x+width > WINDOW_MAX )
	 x_limit = x_offset-(WINDOW_MAX-(x+width));
      else
	 x_limit = 0;
   }
      
   if( new_y < WINDOW_MIN )
      y_limit = WINDOW_MIN-y_offset-y;
   else if( new_y+height > WINDOW_MAX )
      y_limit = y_offset-(WINDOW_MAX-(y+height));
   else
      y_limit = 0;   
}

void ComponentReference::ShiftPosition( float x_offset, float y_offset )
{
   float x, y;

   if( component_parent ) {
      if( component_parent->Selected() ) // return immediately to avoid shifting
	 return;                         // multiple times
   }
   
   GetPosition( x, y );
   x += x_offset;
   y += y_offset;
   SetPosition( x, y );
}

float ComponentReference::GetHeight()
{
   if( component_parent )
      return( fHeight*component_parent->GetHeight() );
   else 
      return( fHeight );
}

float ComponentReference::GetWidth()
{
   if( component_parent )
      return( fWidth*component_parent->GetWidth() );
   else
      return( fWidth );
}

void ComponentReference::SetHeight( float fNew_height, int iResizing_decoupled )
{

   Cltn<float> lfChild_height, lfChild_x, lfChild_y;

   fNew_height = ( fNew_height > MINIMUM_COMPONENT_DIMENSION ) ? fNew_height : MINIMUM_COMPONENT_DIMENSION;

   if( iResizing_decoupled ) {

      for( children.First(); !children.IsDone(); children.Next()) {
	 float fx, fy;

	 lfChild_height.Add( children.CurrentItem()->GetHeight() );
	 children.CurrentItem()->GetPosition( fx, fy );
	 lfChild_x.Add( fx );
	 lfChild_y.Add( fy );
      }

   }    

   if( component_parent )
      fHeight = fNew_height/component_parent->GetHeight();
   else
      fHeight = fNew_height;

   if( iResizing_decoupled ) {

      for( children.First(); !children.IsDone(); children.Next() )
      {
	 children.CurrentItem()->SetHeight( lfChild_height.RemoveFirstElement() );
	 children.CurrentItem()->SetPosition( lfChild_x.RemoveFirstElement(), lfChild_y.RemoveFirstElement() );
      }
   }
}

void ComponentReference::SetWidth( float fNew_width, int iResizing_decoupled )
{
   Cltn<float> lfChild_width, lfChild_x, lfChild_y;

   fNew_width = ( fNew_width > MINIMUM_COMPONENT_DIMENSION ) ? fNew_width : MINIMUM_COMPONENT_DIMENSION;

   if( iResizing_decoupled ) {

      for( children.First(); !children.IsDone(); children.Next()) {
	 float fx, fy;

	 lfChild_width.Add( children.CurrentItem()->GetWidth() );
	 children.CurrentItem()->GetPosition( fx, fy );
	 lfChild_x.Add( fx );
	 lfChild_y.Add( fy );
      }
   }    

   if( component_parent )
      fWidth = fNew_width/component_parent->GetWidth();
   else
      fWidth = fNew_width;

   if( iResizing_decoupled ) {

      for( children.First(); !children.IsDone(); children.Next()) {
	 children.CurrentItem()->SetWidth( lfChild_width.RemoveFirstElement() );
	 children.CurrentItem()->SetPosition( lfChild_x.RemoveFirstElement(), lfChild_y.RemoveFirstElement() );
      }
   }
}

void ComponentReference::GetBoundaries( float& top, float& bottom, float& left, float& right )
{
   // this method is called only for top level components
    int dyn, stack, prot, s;
   top = fY-TEXT_HEIGHT;
   bottom = fY+fHeight;
   right = fX+fWidth;
   left = fX;

   if( component == NULL ) return; // empty reference will be drawn as team anyway

   if( component->GetType() == PROCESS )
      left -= fHeight*0.1;

   component->GetAttributes( dyn, stack, prot );
   if( stack ) {
      s = component->GetDrawReplNumber();
      top -= s*STACK_OFFSET;
      right += s*STACK_OFFSET;
   }
}

void ComponentReference::Draw( Presentation *ppr )
{
//   int iNum_points = 4
   int i, number_tines;
   int dynamic = 0 , stack = 0, is_protected = 0, colour, draw_replication;
   etComponent_type component_type = this->ComponentType();
   etPool_type pool_type;
   etComponent_type pool_component_type;
   float fMy_x, fMy_y,
      height = GetHeight(),
      width = GetWidth(),
      x_text_offset = 0, y_text_offset = 0;
   char labelstring[90];
   float bottom, bottom2, xoffset = 0;
   labelstring[89] = '\0';

   GetPosition( fMy_x, fMy_y );
   if( component ) component->GetAttributes( dynamic, stack, is_protected ); // get attributes of referenced component
   colour = component ? component->GetColour() : 0;
   draw_replication = (component ? (stack? component->GetDrawReplNumber() : 0) : 0);

   if( component_type == POOL ) {
      pool_type = component->GetPoolType();
      pool_component_type = component->PoolComponentType();
   }

   // determine if component is visible and return if not
   if( ((fMy_x+width) < left_border) || (fMy_x > right_border) || (fMy_y > bottom_border) || ((fMy_y+height) < top_border) )
      return;   

   eLine_styles els;

   if( dynamic )
      els = LINE_DASHED;
   else
      els = LINE_SOLID;

   ppr->SetLineWidth( 2 );
   ppr->SetLinePattern( els );

   if( component_type == TEAM || component_type == PROCESS || component_type == AGENT || component_type == OTHERCOM ) {

      if( component_type == AGENT )
	 ppr->SetLineWidth( 4 );
     
      for( i = draw_replication; i >= 0; i-- ) {
	 
	 if( is_protected && (i == 0) ) {

	    int s = component_type == AGENT ? 2 : 1; // allow extra spacing for agents due to thick borders
	    float rgfX[4] = { fMy_x-s*PROTECT_OFFSET, fMy_x+width+s*PROTECT_OFFSET, fMy_x+width+s*PROTECT_OFFSET, fMy_x-s*PROTECT_OFFSET };
	    float rgfY[4] = { fMy_y-s*PROTECT_OFFSET, fMy_y-s*PROTECT_OFFSET, fMy_y+height+s*PROTECT_OFFSET, fMy_y+height+s*PROTECT_OFFSET };

	    ppr->SetFgColour( WHITE );
	    ppr->DrawPoly( rgfX, rgfY, 4 );
	    ppr->DrawFilledPoly( rgfX, rgfY, 4 );
	    ppr->SetFgColour( (selected && !postscript_output) ? BLUE : colour );
	    ppr->DrawPoly( rgfX, rgfY, 4 );
	 }

	 float rgfX[4] = { fMy_x+i*STACK_OFFSET, fMy_x+i*STACK_OFFSET+width, fMy_x+i*STACK_OFFSET+width, fMy_x+i*STACK_OFFSET };
	 float rgfY[4] = { fMy_y-i*STACK_OFFSET, fMy_y-i*STACK_OFFSET, fMy_y-i*STACK_OFFSET+height, fMy_y-i*STACK_OFFSET+height };

	 if( component_type == PROCESS ) {
	    rgfX[2] -= height*.1;
	    rgfX[3] -= height*.1;
	    xoffset = -1*(height*.1);
	 }

	 ppr->SetFgColour( WHITE );
	 ppr->DrawPoly( rgfX, rgfY, 4 );
	 ppr->DrawFilledPoly( rgfX, rgfY, 4 );
	 ppr->SetFgColour( (selected && !postscript_output) ? BLUE : colour );
	 ppr->DrawPoly( rgfX, rgfY, 4 );
	 
      }

      if( component_type == AGENT )
	 ppr->SetLineWidth( 2 );

      number_tines = (int)(width/SPACING);

   } else if( component_type == OBJECT ) {
      
      int draw_colour = (selected && !postscript_output) ? BLUE : colour;
      
      for( i = draw_replication; i >= 0; i-- ) { 
	 if( is_protected && (i == 0) )
	    ppr->DrawRoundedRectangle( fMy_x-PROTECT_OFFSET, fMy_y-PROTECT_OFFSET, width+2*PROTECT_OFFSET, height+2*PROTECT_OFFSET, draw_colour );
	 ppr->DrawRoundedRectangle( fMy_x+i*STACK_OFFSET, fMy_y-i*STACK_OFFSET, width, height, draw_colour, !((i == 0) && is_protected) );
      }

      float rounding = Min( height, width )*RADIUS_FACTOR;
      xoffset = rounding;
      number_tines = (int)((width-2*rounding)/SPACING);
      
   } else if( component_type == POOL ) {

      float rgfX[4] = { fMy_x+width, fMy_x+width, fMy_x, fMy_x };
      float rgfY[4] = { fMy_y, fMy_y+height, fMy_y+height, fMy_y };
    
      ppr->SetFgColour( WHITE );
      ppr->DrawFilledPoly( rgfX, rgfY, 4 );
      ppr->SetFgColour( (selected && !postscript_output) ? BLUE : colour );

      for( i=0; i < 3; i++ )
	 ppr->DrawLine( rgfX[i], rgfY[i], rgfX[i+1], rgfY[i+1] );
    
      for( i=1; i < 6; i++ ) {
	 float rgfX[2] = {fMy_x, fMy_x+width};
	 float rgfY[2] = {fMy_y+height-i*height/8.0,
			  fMy_y+height-i*height/8.0};
	 ppr->DrawLine( rgfX[0], rgfY[0], rgfX[1], rgfY[1] );
      }
      number_tines = (int)(width/SPACING);
   }
   else if( component_type == ISR ) {
      for( i = draw_replication; i >= 0; i-- ) {

	 float rgfX[3] = { fMy_x+i*STACK_OFFSET+width/2.0, fMy_x+i*STACK_OFFSET+width, fMy_x+i*STACK_OFFSET };
	 float rgfY[3] = { fMy_y-i*STACK_OFFSET, fMy_y-i*STACK_OFFSET+height, fMy_y-i*STACK_OFFSET+height };
	
	 ppr->SetFgColour( WHITE );
	 ppr->DrawFilledPoly( rgfX, rgfY, 3 );
	 ppr->SetFgColour( (selected && !postscript_output) ? BLUE : colour );
	 ppr->DrawPoly( rgfX, rgfY, 3 );
      }
      number_tines = (int)(width/SPACING);
   }
   
   if( anchored ) {
      bottom = fMy_y+height;
      bottom2 = bottom + height/SCALE;
  
      for( i = 2; i < number_tines; i++ )
	 ppr->DrawLine( fMy_x+xoffset+i*SPACING, bottom, fMy_x+xoffset+(i-1)*SPACING, bottom2 );
   }
   
   ppr->SetLinePattern( LINE_SOLID );

   strcpy( labelstring, component ? component->GetLabel() : "" );

   if( PrintManager::TextNonempty( role ) ) {
      strcat( labelstring, ":" );
      strcat( labelstring, role );
   }

   if( (component_type == POOL) && ( pool_type == P_COMPONENT ) )
      strcat( labelstring, pool_types[pool_component_type] );
   
   if( component_type == OBJECT )
      x_text_offset = Min( width, height)*RADIUS_FACTOR;
   
   if( stack )
      y_text_offset = draw_replication*STACK_OFFSET;
   else if( is_protected )
      y_text_offset = PROTECT_OFFSET;

   ppr->SetFgColour( (selected && !postscript_output) ? BLUE : BLACK );
   ppr->DrawText( fMy_x+x_text_offset, fMy_y-.01-y_text_offset, labelstring );
   ppr->SetFgColour( BLACK );

   for( children.First(); !children.IsDone(); children.Next())
      children.CurrentItem()->Draw( ppr );
}

int ComponentReference::SetParent( ComponentReference *new_parent )
{

   if( new_parent == NULL ) {

      float x, y;
      GetPosition( x, y );
      fX = x;
      fY = y;

   }
   
   if( !IsComponentDescendent( new_parent ) ) {
      float fold_width = GetWidth();
      float fold_height = GetHeight();

      component_parent = new_parent;
      if( new_parent )
	 new_parent->AddChild( this );
      
      SetWidth( fold_width );
      SetHeight( fold_height );
      return( 1 );
   } else {
      return( 0 );
   }
}

void ComponentReference::AddChild( ComponentReference *new_child )
{
   for( children.First(); !children.IsDone(); children.Next()) {
      if( children.CurrentItem() == new_child ) {
	 return;
      }
   }

   children.Add( new_child );
}

void ComponentReference::RemoveChild( ComponentReference *pbChild )
{
   for( children.First(); !children.IsDone(); children.Next()) {
      if( children.CurrentItem() == pbChild ) {
	 children.Delete( children.CurrentItem() );
	 break;
      }
   }
}

ComponentReference *ComponentReference::GetChild( int iChild_num )
{
   if( iChild_num >= 0 && iChild_num < GetNumChildren() )
      return( children.Get( iChild_num ) );
   else
      return( NULL );
}

int ComponentReference::IsComponentDescendent( ComponentReference *pbComponent )
{
   if( pbComponent == NULL )
      return( 0 );
   
   for( children.First(); !children.IsDone(); children.Next()) {

      if( children.CurrentItem() == pbComponent )
	 return( 1 );

      if( children.CurrentItem()->IsComponentDescendent( pbComponent ) )
	 return( 1 );
   }

   return( 0 );
}

ComponentReference *ComponentReference::FindRoot()
{
   if( component_parent )
      return( component_parent->FindRoot() );
   else
      return( this );
}

void ComponentReference::CheckGeometry()
{
   if( component_parent ) {
      if( component_parent->GetWidth() < 0.0 ) {
	 fX = 1.0-fX-fWidth;
      } 

      if( component_parent->GetHeight() < 0.0 ) {
	 fY = 1.0-fY-fHeight;
      }
   }
}

void ComponentReference::ResetGeometry()
{
   float fMy_x, fMy_y;

   for( children.First(); !children.IsDone(); children.Next())
      children.CurrentItem()->CheckGeometry();

   GetPosition( fMy_x, fMy_y );

   SetPosition( Min( fMy_x, fMy_x+GetWidth() ),
		Min( fMy_y, fMy_y+GetHeight() ) );
   SetWidth( fabs( GetWidth() ) );
   SetHeight( fabs( GetHeight() ) );

}

int ComponentReference::CheckTypeContainment( etComponent_type ect )
{
   if( component == NULL ) return 1;

   if( CheckTypesDownward( ect ) == 0 )
      return( 0 );

   etComponent_type ectOld_type = this->ComponentType();
   ComponentReference *pcRoot = FindRoot();
   component->SetType( ect );
   int iReturn = 1;

   if( pcRoot != this ) {
      if( pcRoot->CheckTypesDownward( pcRoot->ComponentType() ) == 0 )
	 iReturn = 0;
   }

   component->SetType( ectOld_type );

   if( component->GetType() == OTHERCOM )
      iReturn = 1;
  
   return( iReturn );
}

int ComponentReference::CheckTypesDownward( etComponent_type ect )
{

   switch( ect ) {

   case PROCESS:
      for( children.First(); !children.IsDone(); children.Next()) {
	 if( children.CurrentItem()->IsPassive() == 0 )
	    return( 0 );
      }
      break;

   case POOL:
   case ISR:
   case OBJECT:
      if( children.Size() > 0 )
	 return( 0 );
      break;
    default:	// TEAM, AGENT, OTHERCOM, PLUGIN_MAP, NO_TYPE
	break;
   }

   return( 1 );
}

int ComponentReference::IsPassive()
{
   etComponent_type component_type = this->ComponentType();

   if( component_type == OTHERCOM )
      return( 1 );
   
   if( component_type == PROCESS
       || component_type == ISR 
       || component_type == POOL ) {
      return( 0 );
   }

   for( children.First(); !children.IsDone(); children.Next()) {
      if( children.CurrentItem()->IsPassive() == 0 )
	 return( 0 );
   }
   
   return( 1 );
}

ComponentReference *ComponentReference::Copy( ComponentReference *pcCopy_parent )
{
   float fNew_x, fNew_y;
   GetPosition( fNew_x, fNew_y );

   ComponentReference *cr = new ComponentReference( fNew_x, fNew_y, GetWidth(), GetHeight(), pcCopy_parent );
   if( component ) cr->ReferencedComponent( component ); // set the referenced component of the new reference to that of this reference

   for( children.First(); !children.IsDone(); children.Next())
      children.CurrentItem()->Copy( cr );

   return( cr );
}

void ComponentReference::GetAllChildren( Cltn<ComponentReference *> &components )
{
   for( children.First(); !children.IsDone(); children.Next() ) {
      components.Add( children.CurrentItem() );
      children.CurrentItem()->GetAllChildren( components );
   }
}

void ComponentReference::SaveXML( FILE *fp, bool ignore_children )
{
   char buffer[1000];
   int num_resps, num_non_resps;

   if( ignore_children ) {   // return immediately if this component is a child
      if( component_parent != NULL ) // and the call is not made from its parent
	 return;
   }

   sprintf( buffer, "<component-ref component-ref-id=\"cr%d\" fx=\"%f\" fy=\"%f\" width=\"%f\" height=\"%f\" anchored=\"%s\"",
	    component_ref_number, fX/VIRTUAL_SCALING, fY/VIRTUAL_SCALING, fWidth/VIRTUAL_SCALING, fHeight/VIRTUAL_SCALING,
	    ( anchored == TRUE ? "yes" : "no" ) );
   PrintXMLText( fp, buffer );
   
   if( component )
      fprintf( fp, " referenced-component=\"c%d\"", component->GetComponentNumber() );
   if( component_parent )
      fprintf( fp, " component-parent=\"cr%d\"", component_parent->ComponentReferenceNumber() );
   if( PrintManager::TextNonempty( role ) )
      fprintf( fp, " role=\"%s\"", PrintDescription( role ) );
   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );

   //  go count how many bound responsibilities/non-responsibilities there are
   num_resps = 0;
   num_non_resps = 0;
   for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() ) {
      if( dependent_figures.CurrentItem()->Edge()->EdgeType() == RESPONSIBILITY_REF )
	 num_resps++;
      else
	 num_non_resps++;
   }

   //   save bound Responsibilities
   if( num_resps > 0 ) {
      
      PrintNewXMLBlock( fp, "responsibility-list" );

      for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() )
	 if( dependent_figures.CurrentItem()->Edge()->EdgeType() == RESPONSIBILITY_REF )
	 {
	    sprintf( buffer, "hyperedge-ref hyperedge-id=\"h%d\" /", dependent_figures.CurrentItem()->Edge()->GetNumber() );
	    PrintXML( fp, buffer );
	 }
      
      PrintEndXMLBlock( fp, "responsibility-list" );
   }
   
   //   save bound non-Responsibilities
   if( num_non_resps > 0 ) {
      
      PrintNewXMLBlock( fp, "other-hyperedge-list" );

      for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() ) {
	 if( dependent_figures.CurrentItem()->Edge()->EdgeType() != RESPONSIBILITY_REF ) {
	    sprintf( buffer, "hyperedge-ref hyperedge-id=\"h%d\" /", dependent_figures.CurrentItem()->Edge()->GetNumber() );
	    PrintXML( fp, buffer );
	 }
      }
      
      PrintEndXMLBlock( fp, "other-hyperedge-list" );
   }
   
   PrintEndXMLBlock( fp, "component-ref" );
   LinebreakXML( fp );   

   for( children.First(); !children.IsDone(); children.Next() )
      children.CurrentItem()->SaveXML( fp, FALSE );   
}

void ComponentReference::SaveGraphDXL( FILE *fp )
{
   char buffer[1000];
 
   sprintf( buffer, "compRef( \"cr%d\", %d, %d, %d, %d, \"%s\",",
	    component_ref_number, (int)((fX/VIRTUAL_SCALING)*WIDTH), (int)((fY/VIRTUAL_SCALING)*HEIGHT) ,(int)((fWidth/VIRTUAL_SCALING)*WIDTH), (int)((fHeight/VIRTUAL_SCALING)*HEIGHT) ,
	    ( anchored == TRUE ? "yes" : "no" ) );
   PrintXMLText( fp, buffer );
   
   if( component ){
      if (PrintManager::TextNonempty(component->GetLabel()) )
            fprintf( fp, " \"c%d\", \"%s\"", component->GetComponentNumber(), component->GetLabel() );
      else
            fprintf( fp, " \"c%d\", \"\"", component->GetComponentNumber() );
   }   
   else
      fprintf( fp, " \"\", \"\""); 

   if( PrintManager::TextNonempty( role ) )
      fprintf( fp, ", \"%s\",", PrintDescription( role ) ); 
   else
     fprintf( fp, ", \"\"," ); 
      
   if (component_parent ){
         fprintf( fp, " \"cr%d\" )\n", component_parent->ComponentReferenceNumber() );
    }
    else{
         fprintf( fp, " \"\" )\n");
    }     
}

void ComponentReference::SaveCSMXML( FILE *fp, bool ignore_children )
{
   char buffer[1000];

   if( ignore_children ) {   // return immediately if this component is a child
      if( component_parent != NULL ) // and the call is not made from its parent
	 return;
   }

   sprintf( buffer, "<Component id=\"c%d\" ",component_ref_number);
   PrintXMLText( fp, buffer );
   
   if( component )
      fprintf( fp, " name=\"%s\"", PrintDescription( component->GetLabel()));
   if (component->ProcessorId()!= NA)
         //if( processor_id != NA )
	   fprintf( fp, " host=\"dv%d\"", component->ProcessorId());
   else
	   fprintf(fp, " host=\"\"");
   if( component_parent )
   {
      fprintf(fp, " parent=\"c%d\" ", component_parent->ComponentReferenceNumber() );
	  //PrintXMLText(fp, buffer);
   }
   if (children.Size()>0){
	   fprintf( fp, " sub=\" ");
       for( children.First(); !children.IsDone(); children.Next() )
	   
		   fprintf( fp, "c%d ",children.CurrentItem()->ComponentReferenceNumber() );

   
   //if( PrintManager::TextNonempty( role ) )
     // fprintf( fp, " role=\"%s\"", PrintDescription( role ) );
     fprintf( fp, "\">\n" );
   }
   else
     fprintf(fp, ">\n");
   IndentNewXMLBlock( fp );
/*
   //  go count how many bound responsibilities/non-responsibilities there are
   num_resps = 0;
   num_non_resps = 0;
   for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() ) {
      if( dependent_figures.CurrentItem()->Edge()->EdgeType() == RESPONSIBILITY_REF )
	 num_resps++;
      else
	 num_non_resps++;
   }

   //   save bound Responsibilities
   if( num_resps > 0 ) {
      
      PrintNewXMLBlock( fp, "responsibility-list" );

      for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() )
	 if( dependent_figures.CurrentItem()->Edge()->EdgeType() == RESPONSIBILITY_REF )
	 {
	    sprintf( buffer, "hyperedge-ref hyperedge-id=\"h%d\" /", dependent_figures.CurrentItem()->Edge()->GetNumber() );
	    PrintXML( fp, buffer );
	 }
      
      PrintEndXMLBlock( fp, "responsibility-list" );
   }
   
   //   save bound non-Responsibilities
   if( num_non_resps > 0 ) {
      
      PrintNewXMLBlock( fp, "other-hyperedge-list" );

      for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() ) {
	 if( dependent_figures.CurrentItem()->Edge()->EdgeType() != RESPONSIBILITY_REF ) {
	    sprintf( buffer, "hyperedge-ref hyperedge-id=\"h%d\" /", dependent_figures.CurrentItem()->Edge()->GetNumber() );
	    PrintXML( fp, buffer );
	 }
      }
      
      PrintEndXMLBlock( fp, "other-hyperedge-list" );
   }
  */ 
   PrintEndXMLBlock( fp, "Component" );
   LinebreakXML( fp );   

   for( children.First(); !children.IsDone(); children.Next() )
      children.CurrentItem()->SaveCSMXML( fp, FALSE );   
}

void ComponentReference::GeneratePostScriptDescription( FILE *ps_file )
{
   if( component == NULL ) return; // return immediately if there is no referenced component as for a newly created reference
   fprintf( ps_file, "%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( component->GetLabel() ) );
   if( PrintManager::TextNonempty( component->Description() ) ) {
      fprintf( ps_file, "%d rom 1 IN ID\n", PrintManager::text_font_size );
      PrintManager::PrintDescription( component->Description() );
      fprintf( ps_file, "OD\n" );
   }
   fprintf( ps_file, "L\n" );
}

void ComponentReference::RecheckBindings()
{
   float tX, tY;

   for( dependent_figures.First(); !dependent_figures.IsDone(); dependent_figures.Next() )  {
      dependent_figures.CurrentItem()->GetPosition( tX, tY );
      dependent_figures.CurrentItem()->SetPosition( tX, tY );
   }
}

std::ostream& operator<<( std::ostream& output,  ComponentReference *cr )
{
   output  << "\nComponentReference fX: " << cr->fX << " fY: " << cr->fY << " # dependent figures: " 
	   << cr->dependent_figures.Size() << std::endl;
   return output;
}
