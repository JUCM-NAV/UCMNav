
// structural dynamics callbacks previously located in callbacks.cc

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "dynarrow.h"
#include "resp_ref.h"
#include "bitmap.h"
#include <stdlib.h>
#include <string.h>

FD_StructDynPopup *pfdsdp = NULL;
extern FD_EditRespPopup *pfderp;
extern void DrawScreen();
extern int SDPvisible;
extern void SetPromptUserForSave();

static DynamicArrow *dynarrow;
static ResponsibilityReference *rref;

static int SDDialogClose( FL_FORM *, void * );

extern "C"
void SDOKButton_cb(FL_OBJECT * pflo, long lData)
{

  fl_hide_form(pfdsdp->StructDynPopup);
  fl_activate_form( pfderp->EditRespPopup ); // reactivate main responsibility popup
  SDPvisible = 0;
  arrow_types at;
  arrow_directions ad;
  arrow_positions ap;

  dynarrow->SetLength(fl_get_slider_value(pfdsdp->LengthSlider));

  if (fl_get_button(pfdsdp->MoveButton))
    at = MOVE;
  else if (fl_get_button(pfdsdp->MoveStayButton))
    at = MOVE_STAY;
  else if (fl_get_button(pfdsdp->CreateButton))
    at = CREATE;
  else if (fl_get_button(pfdsdp->DestroyButton))
    at = DESTROY;
  else if (fl_get_button(pfdsdp->CopyButton))
    at = COPY;

  if (fl_get_button(pfdsdp->UpButton))
    ap = ARROW_UP;
  else if (fl_get_button(pfdsdp->DownButton))
    ap = ARROW_DOWN;
  else if (fl_get_button(pfdsdp->RightButton))
    ap = ARROW_RIGHT;
  else if (fl_get_button(pfdsdp->LeftButton))
    ap = ARROW_LEFT;

  if (fl_get_button(pfdsdp->IntoPathButton))
    ad = INTO;
  else if (fl_get_button(pfdsdp->OutofPathButton))
    ad = OUTOF;

  dynarrow->SetAttributes(at, ad);
  rref->ArrowPosition( ap );
  dynarrow->SourcePool( fl_get_input( pfdsdp->SourcePool ) );
  
  SetPromptUserForSave();

  DrawScreen();

}

void StructDynPopup( DynamicArrow *arrow, ResponsibilityReference *ref )
{
  arrow_types at;
  arrow_directions ad;

  dynarrow = arrow;
  rref = ref;
  
  fl_deactivate_form( pfderp->EditRespPopup );  // freeze main responsibility popup
  
  if (!pfdsdp) {
    pfdsdp = create_form_StructDynPopup();
    fl_set_form_atclose( pfdsdp->StructDynPopup, SDDialogClose, NULL );
    fl_set_slider_bounds(pfdsdp->LengthSlider, .01, .1);
    fl_set_bitmapbutton_data(pfdsdp->MoveButton, move_width, move_height, move_bits );
    fl_set_bitmapbutton_data(pfdsdp->MoveStayButton, move_stay_width, move_stay_height, move_stay_bits );
    fl_set_bitmapbutton_data(pfdsdp->CreateButton, create_width, create_height, create_bits );
    fl_set_bitmapbutton_data(pfdsdp->DestroyButton, destroy_width, destroy_height, destroy_bits );
    fl_set_bitmapbutton_data(pfdsdp->CopyButton, copy_width, copy_height, copy_bits );
  }
  
  dynarrow->GetAttributes( at, ad );
  fl_set_slider_value(pfdsdp->LengthSlider, dynarrow->GetLength());
  fl_set_button(pfdsdp->MoveButton, 0);
  fl_set_button(pfdsdp->MoveStayButton, 0);
  fl_set_button(pfdsdp->CreateButton, 0);
  fl_set_button(pfdsdp->DestroyButton, 0);
  fl_set_button(pfdsdp->CopyButton, 0);

  switch (at) {
  case MOVE:
    fl_set_button(pfdsdp->MoveButton, 1);
    break;
  case MOVE_STAY:
    fl_set_button(pfdsdp->MoveStayButton, 1);
    break;
  case CREATE:
    fl_set_button(pfdsdp->CreateButton, 1);
    break;
  case DESTROY:
    fl_set_button(pfdsdp->DestroyButton, 1);
    break;
  case COPY:
    fl_set_button(pfdsdp->CopyButton, 1);
    break;
  }

  fl_set_button(pfdsdp->UpButton, 0);
  fl_set_button(pfdsdp->DownButton, 0);
  fl_set_button(pfdsdp->RightButton, 0);
  fl_set_button(pfdsdp->LeftButton, 0);

  switch ( rref->ArrowPosition() ) {
  case ARROW_UP:
    fl_set_button(pfdsdp->UpButton, 1);
    break;
  case ARROW_DOWN:
    fl_set_button(pfdsdp->DownButton, 1);
    break;
  case ARROW_RIGHT:
    fl_set_button(pfdsdp->RightButton, 1);
    break;
  case ARROW_LEFT:
    fl_set_button(pfdsdp->LeftButton, 1);
    break;
  }

  fl_set_button(pfdsdp->IntoPathButton, 0);
  fl_set_button(pfdsdp->OutofPathButton, 0);

  switch (ad) {
  case INTO:
    fl_set_button(pfdsdp->IntoPathButton, 1);
    break;
  case OUTOF:
    fl_set_button(pfdsdp->OutofPathButton, 1);
    break;
  }

  fl_set_input( pfdsdp->SourcePool, dynarrow->SourcePool() );

  fl_show_form(pfdsdp->StructDynPopup, FL_PLACE_CENTER,
	       FL_TRANSIENT, "Edit Dynamic Arrow");
  SDPvisible = 1;

}

extern "C"
void SDCancelButton_cb(FL_OBJECT * pflo, long lData)
{
  fl_hide_form(pfdsdp->StructDynPopup);
  fl_activate_form( pfderp->EditRespPopup ); // reactivate main responsibility popup
  SDPvisible = 0;
}

int SDDialogClose( FL_FORM *, void * )
{
   SDCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}
