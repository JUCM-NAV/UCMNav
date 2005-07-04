/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "ucmnavui.h"

FD_Main *create_form_Main(void)
{
  FL_OBJECT *obj;
  FD_Main *fdui = (FD_Main *) fl_calloc(1, sizeof(*fdui));

  fdui->Main = fl_bgn_form(FL_NO_BOX, 952, 746);
  obj = fl_add_box(FL_UP_BOX,0,0,952,746,"");
  fdui->DrawingArea = obj = fl_add_canvas(FL_NORMAL_CANVAS,10,165,660,550,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_box(FL_FRAME_BOX,8,44,684,117,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,700,254,234,312,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_Y);
  fdui->Responsibilities = obj = fl_add_browser(FL_MULTI_BROWSER,705,40,230,180,"Responsibilities");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,Responsibilities_cb,0);
  fdui->Title = obj = fl_add_input(FL_NORMAL_INPUT,70,52,300,26,"Map Title");
    fl_set_object_color(obj,FL_WHITE,FL_WHITE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,Title_cb,0);
  fdui->Description = obj = fl_add_input(FL_MULTILINE_INPUT,700,596,234,132,"Description");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,Description_cb,0);

  fdui->Tools = fl_bgn_group();
  fdui->ComponentButton = obj = fl_add_bitmapbutton(FL_RADIO_BUTTON,15,115,59,25,"Component");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,ComponentButton_cb,0);
  fdui->PathButton = obj = fl_add_bitmapbutton(FL_RADIO_BUTTON,79,115,55,25,"Path");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,PathButton_cb,0);
  fdui->SelectButton = obj = fl_add_bitmapbutton(FL_RADIO_BUTTON,144,115,54,25,"Select");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,SelectButton_cb,0);
  fl_end_group();

  fdui->DecompositionLevel = obj = fl_add_browser(FL_HOLD_BROWSER,378,52,308,88,"Decomposition Level");
    fl_set_object_color(obj,FL_WHITE,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DecompositionLevel_cb,0);

  fdui->ConditionControls = fl_bgn_group();
  fdui->Conditions1_Delete = obj = fl_add_button(FL_NORMAL_BUTTON,857,384,66,25,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Conditions_Delete_cb,0);
  fdui->Conditions1_Edit = obj = fl_add_button(FL_NORMAL_BUTTON,788,384,65,25,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Conditions_Edit_cb,0);
  fdui->Conditions2_Delete = obj = fl_add_button(FL_NORMAL_BUTTON,858,534,66,25,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Conditions_Delete_cb,1);
  fdui->Conditions2_Edit = obj = fl_add_button(FL_NORMAL_BUTTON,784,534,65,25,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Conditions_Edit_cb,1);
  fl_end_group();

  fdui->EditResponsibility = obj = fl_add_button(FL_NORMAL_BUTTON,760,226,122,22,"Edit Responsibility");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,EditResponsibility_cb,0);

  fdui->ConditionAddControls = fl_bgn_group();
  fdui->Conditions1 = obj = fl_add_browser(FL_MULTI_BROWSER,706,280,220,95,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,Conditions_cb,0);
  fdui->Conditions1_Add = obj = fl_add_button(FL_NORMAL_BUTTON,714,384,66,25,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Conditions_Add_cb,0);
  fdui->Conditions2 = obj = fl_add_browser(FL_MULTI_BROWSER,706,432,220,95,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,Conditions_cb,1);
  fdui->Conditions2_Add = obj = fl_add_button(FL_NORMAL_BUTTON,706,534,72,25,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Conditions_Add_cb,1);
  fl_end_group();

  fdui->AboutMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,633,17,60,21,"About");
    fl_set_object_shortcut(obj,"#B#b",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,AboutMenu_cb,0);
  fdui->HorizontalScrollbar = obj = fl_add_scrollbar(FL_HOR_SCROLLBAR,8,721,664,16,"");
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,HorizontalScrollbar_cb,0);
    fl_set_scrollbar_value(obj, 0);
    fl_set_scrollbar_size(obj, 0.50);
    fl_set_scrollbar_increment(obj, 0.1, 0.1);
     fl_set_scrollbar_return(obj, FL_RETURN_END_CHANGED);
  fdui->VerticalScrollbar = obj = fl_add_scrollbar(FL_VERT_SCROLLBAR,674,163,16,556,"");
    fl_set_object_gravity(obj, FL_NorthEast, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,VerticalScrollbar_cb,0);
    fl_set_scrollbar_value(obj, 0);
    fl_set_scrollbar_size(obj, 0.50);
    fl_set_scrollbar_increment(obj, 0.1, 0.1);
     fl_set_scrollbar_return(obj, FL_RETURN_END_CHANGED);
  fdui->ScaleChoice = obj = fl_add_choice(FL_NORMAL_CHOICE2,206,115,59,25,"Scale");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,ScaleChoice_cb,0);
  fdui->ModeChoice = obj = fl_add_choice(FL_NORMAL_CHOICE2,270,115,102,25,"Editing Mode");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,ModeChoice_cb,0);
  fdui->NavigationMode = obj = fl_add_browser(FL_NORMAL_BROWSER,116,85,254,25,"Navigation Mode");
    fl_set_object_color(obj,FL_WHITE,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_browser_hscrollbar(obj, FL_OFF);
    fl_set_browser_vscrollbar(obj, FL_OFF);
  fdui->FileMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,10,18,43,20,"File");
    fl_set_object_shortcut(obj,"#f#f",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,FileMenu_cb,0);
  fdui->ComponentMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,50,18,105,20,"Components");
    fl_set_object_shortcut(obj,"#C#c",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,ComponentMenu_cb,0);
  fdui->OptionsMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,159,17,71,21,"Options");
    fl_set_object_shortcut(obj,"#O#o",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->PerformanceMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,234,18,100,20,"Performance");
    fl_set_object_shortcut(obj,"#P#p",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,PerformanceMenu_cb,0);
  fdui->MapsMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,335,17,55,21,"Maps");
    fl_set_object_shortcut(obj,"#M#m",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,MapsMenu_cb,0);
  fdui->AlignMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,394,18,52,20,"Align");
    fl_set_object_shortcut(obj,"#A#a",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,AlignMenu_cb,0);
  fdui->UtilitiesMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,451,18,67,20,"Utilities");
    fl_set_object_shortcut(obj,"#U#u",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,UtilitiesMenu_cb,0);
  fdui->ScenariosMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,524,18,87,20,"Scenarios");
    fl_set_object_shortcut(obj,"#S#s",1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,ScenariosMenu_cb,0);
  fl_end_form();

  fdui->Main->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_CompAttrPopup *create_form_CompAttrPopup(void)
{
  FL_OBJECT *obj;
  FD_CompAttrPopup *fdui = (FD_CompAttrPopup *) fl_calloc(1, sizeof(*fdui));

  fdui->CompAttrPopup = fl_bgn_form(FL_NO_BOX, 407, 673);
  obj = fl_add_box(FL_UP_BOX,0,0,407,673,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,12,228,377,396,"");
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,27,431,346,49,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,181,391,196,33,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->ReplicationFactor = obj = fl_add_input(FL_NORMAL_INPUT,294,561,84,25,"Replication");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->PoolComponentType = obj = fl_add_choice(FL_NORMAL_CHOICE,288,525,90,25,"Type");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->PoolType = obj = fl_add_choice(FL_NORMAL_CHOICE,288,488,90,25,"Pool Type");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PoolType_cb,0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,18,391,156,33,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->CAOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,75,630,90,30,"Accept");
    fl_set_button_shortcut(obj,"^M",1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,CAOKButton_cb,0);
  fdui->CACancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,215,630,90,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,CACancelButton_cb,0);
  fdui->StackButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,115,355,60,30,"Stack");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,StackButton_cb,0);
  fdui->ComponentType = obj = fl_add_choice(FL_NORMAL_CHOICE,21,355,90,30,"Component Type");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ComponentType_cb,0);
  fdui->ProtectedButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,180,355,80,30,"Protected");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DynamicButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,265,355,55,30,"Slot");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);

  fdui->ActualGroup = fl_bgn_group();
  fdui->ActualButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,29,398,64,22,"Actual");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->FormalButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,94,398,64,22,"Formal");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();


  fdui->AnchoredGroup = fl_bgn_group();
  fdui->AnchoredButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,189,398,80,20,"Anchored");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->NonAnchoredButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,273,398,100,20,"Non-Anchored");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();

  fdui->PoolPluginButton = obj = fl_add_button(FL_NORMAL_BUTTON,278,592,100,25,"Choose Plug-ins");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,PoolPluginButton_cb,0);
  fdui->FixedButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,325,355,55,30,"Fixed");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  obj = fl_add_box(FL_DOWN_BOX,13,31,377,33,"Editing Mode");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->InstallExistingButton = obj = fl_add_checkbutton(FL_RADIO_BUTTON,26,33,164,28,"Install Existing Component");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,InstallExistingButton_cb,0);
  fdui->CreateNewButton = obj = fl_add_checkbutton(FL_PUSH_BUTTON,213,33,164,28,"Create New Component");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,CreateNewButton_cb,0);
  fdui->ExistingComponentsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,13,94,375,122,"Existing Components");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,ExistingComponentsBrowser_cb,0);

  fdui->ColourGroup = fl_bgn_group();
  fdui->ColourButton17 = obj = fl_add_button(FL_RADIO_BUTTON,352,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,17);
  fdui->ColourButton0 = obj = fl_add_button(FL_RADIO_BUTTON,28,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,0);
  fdui->ColourButton1 = obj = fl_add_button(FL_RADIO_BUTTON,47,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,1);
  fdui->ColourButton2 = obj = fl_add_button(FL_RADIO_BUTTON,66,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,2);
  fdui->ColourButton3 = obj = fl_add_button(FL_RADIO_BUTTON,85,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,3);
  fdui->ColourButton4 = obj = fl_add_button(FL_RADIO_BUTTON,104,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,4);
  fdui->ColourButton5 = obj = fl_add_button(FL_RADIO_BUTTON,123,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,5);
  fdui->ColourButton6 = obj = fl_add_button(FL_RADIO_BUTTON,142,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,6);
  fdui->ColourButton7 = obj = fl_add_button(FL_RADIO_BUTTON,161,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,7);
  fdui->ColourButton8 = obj = fl_add_button(FL_RADIO_BUTTON,180,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,8);
  fdui->ColourButton9 = obj = fl_add_button(FL_RADIO_BUTTON,199,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,9);
  fdui->ColourButton10 = obj = fl_add_button(FL_RADIO_BUTTON,218,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,10);
  fdui->ColourButton11 = obj = fl_add_button(FL_RADIO_BUTTON,237,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,11);
  fdui->ColourButton12 = obj = fl_add_button(FL_RADIO_BUTTON,257,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,12);
  fdui->ColourButton13 = obj = fl_add_button(FL_RADIO_BUTTON,276,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,13);
  fdui->ColourButton14 = obj = fl_add_button(FL_RADIO_BUTTON,295,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,14);
  fdui->ColourButton15 = obj = fl_add_button(FL_RADIO_BUTTON,314,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,15);
  fdui->ColourButton16 = obj = fl_add_button(FL_RADIO_BUTTON,333,432,19,31,"");
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Colour_cb,16);
  fl_end_group();

  fdui->ComponentRole = obj = fl_add_input(FL_NORMAL_INPUT,16,302,365,30,"Component Role");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ColourDisplay = obj = fl_add_button(FL_NORMAL_BUTTON,27,464,344,15,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Label = obj = fl_add_input(FL_NORMAL_INPUT,16,249,365,30,"Component Label");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ProcessorList = obj = fl_add_browser(FL_HOLD_BROWSER,19,504,201,115,"Processor");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_form();

  fdui->CompAttrPopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_EditRespPopup *create_form_EditRespPopup(void)
{
  FL_OBJECT *obj;
  FD_EditRespPopup *fdui = (FD_EditRespPopup *) fl_calloc(1, sizeof(*fdui));

  fdui->EditRespPopup = fl_bgn_form(FL_NO_BOX, 451, 597);
  obj = fl_add_box(FL_UP_BOX,0,0,451,597,"");
  obj = fl_add_box(FL_DOWN_BOX,15,197,415,347,"Selected Responsibility Characteristics");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,85,499,140,35,"");
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
  obj = fl_add_box(FL_FRAME_BOX,12,10,417,165,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_box(FL_FRAME_BOX,320,220,100,100,"Label Placement");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->EROKButton = obj = fl_add_button(FL_NORMAL_BUTTON,110,550,82,30,"OK");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,EROKButton_cb,0);
  fdui->ERCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,273,550,82,30,"Cancel");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ERCancelButton_cb,0);
  fdui->Name = obj = fl_add_input(FL_NORMAL_INPUT,22,220,289,29,"Name");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Description = obj = fl_add_input(FL_MULTILINE_INPUT,22,271,288,55,"Description");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);

  fdui->DirectionButtons = fl_bgn_group();
  fdui->RERightButton = obj = fl_add_button(FL_RADIO_BUTTON,385,255,30,30,"@->");
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->REUpButton = obj = fl_add_button(FL_RADIO_BUTTON,355,225,30,30,"@8->");
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->RELeftButton = obj = fl_add_button(FL_RADIO_BUTTON,325,255,30,30,"@4->");
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->REDownButton = obj = fl_add_button(FL_RADIO_BUTTON,355,285,30,30,"@2->");
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();

  fdui->ExecutionSequence = obj = fl_add_input(FL_MULTILINE_INPUT,22,438,398,53,"Execution Sequence");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ERAddDPButton = obj = fl_add_button(FL_NORMAL_BUTTON,86,506,39,22,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj,ERAddDPButton_cb,0);
  fdui->EREditDPButton = obj = fl_add_button(FL_NORMAL_BUTTON,127,506,37,22,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj,EREditDPButton_cb,0);
  fdui->ERRemoveDPButton = obj = fl_add_button(FL_NORMAL_BUTTON,168,506,54,22,"Remove");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj,ERRemoveDPButton_cb,0);
  fdui->DataStoresButton = obj = fl_add_button(FL_NORMAL_BUTTON,233,506,80,28,"Data Stores");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ERDataStoresButton_cb,0);
  fdui->ServiceRequestsButton = obj = fl_add_button(FL_NORMAL_BUTTON,318,506,108,27,"Service Requests");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ERServiceRequestButton_cb,0);
  fdui->EditingModeBox = obj = fl_add_box(FL_DOWN_BOX,20,32,398,27,"Editing Mode");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->ExistingRespBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,21,82,398,83,"Existing Responsibilities");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,ExistingRespBrowser_cb,0);

  fdui->ModeButtons = fl_bgn_group();
  fdui->IERButton = obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,33,202,20,"Install Existing Responsibility");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,IERButton_cb,0);
    fl_set_button(obj, 1);
  fdui->CNRButton = obj = fl_add_checkbutton(FL_RADIO_BUTTON,230,33,183,22,"Create New Responsibility");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,CNRButton_cb,0);
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->VariableSettings = obj = fl_add_browser(FL_NORMAL_BROWSER,23,355,397,59,"Variable Operations");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_text(FL_NORMAL_TEXT,20,501,60,18,"Dynamic");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,517,55,16,"Pointer");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->EditSettingsButton = obj = fl_add_button(FL_NORMAL_BUTTON,264,328,158,23,"Edit Variable Operations");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,EditSettingsButton_cb,0);
  fl_end_form();

  fdui->EditRespPopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_StructDynPopup *create_form_StructDynPopup(void)
{
  FL_OBJECT *obj;
  FD_StructDynPopup *fdui = (FD_StructDynPopup *) fl_calloc(1, sizeof(*fdui));

  fdui->StructDynPopup = fl_bgn_form(FL_NO_BOX, 341, 412);
  obj = fl_add_box(FL_UP_BOX,0,0,341,412,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_box(FL_UP_BOX,20,10,300,350,"");
  fdui->SDOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,50,370,90,30,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,SDOKButton_cb,0);
  fdui->SDCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,200,370,90,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,SDCancelButton_cb,0);

  fdui->DirectionButtons = fl_bgn_group();
  fdui->RightButton = obj = fl_add_button(FL_RADIO_BUTTON,240,70,30,30,"@->");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->UpButton = obj = fl_add_button(FL_RADIO_BUTTON,210,40,30,30,"@8->");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->LeftButton = obj = fl_add_button(FL_RADIO_BUTTON,180,70,30,30,"@4->");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DownButton = obj = fl_add_button(FL_RADIO_BUTTON,210,100,30,30,"@2->");
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();


  fdui->SDTypeButtons = fl_bgn_group();
  fdui->MoveButton = obj = fl_add_bitmapbutton(FL_RADIO_BUTTON,50,30,70,30,"");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->MoveStayButton = obj = fl_add_bitmapbutton(FL_RADIO_BUTTON,50,70,70,30,"");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->CreateButton = obj = fl_add_bitmapbutton(FL_RADIO_BUTTON,50,110,70,30,"");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DestroyButton = obj = fl_add_bitmapbutton(FL_RADIO_BUTTON,50,150,70,30,"");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->CopyButton = obj = fl_add_bitmapbutton(FL_RADIO_BUTTON,50,190,70,30,"");
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();


  fdui->PathButtons = fl_bgn_group();
  fdui->IntoPathButton = obj = fl_add_button(FL_RADIO_BUTTON,145,160,75,30,"Into Path");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->OutofPathButton = obj = fl_add_button(FL_RADIO_BUTTON,225,160,85,30,"Out of path");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();

  fdui->LengthSlider = obj = fl_add_valslider(FL_HOR_SLIDER,50,240,250,20,"Length");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->SourcePool = obj = fl_add_input(FL_NORMAL_INPUT,50,310,250,30,"Source Pool Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_form();

  fdui->StructDynPopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_PrintPopup *create_form_PrintPopup(void)
{
  FL_OBJECT *obj;
  FD_PrintPopup *fdui = (FD_PrintPopup *) fl_calloc(1, sizeof(*fdui));

  fdui->PrintPopup = fl_bgn_form(FL_NO_BOX, 395, 549);
  obj = fl_add_box(FL_UP_BOX,0,0,395,549,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_box(FL_UP_BOX,11,10,374,487,"");
  fdui->PrintPrintButton = obj = fl_add_button(FL_NORMAL_BUTTON,20,505,90,30,"Print");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,PrintPrintButton_cb,0);
  fdui->PrintCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,280,505,90,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,PrintCancelButton_cb,0);
  fdui->PrintPrinttoFileButton = obj = fl_add_button(FL_NORMAL_BUTTON,130,505,130,30,"Print to File");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,PrintPrinttoFileButton_cb,0);
  fdui->PrinterInput = obj = fl_add_input(FL_NORMAL_INPUT,80,20,250,30,"Printer:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->FlagBox = obj = fl_add_box(FL_DOWN_BOX,28,180,343,174,"Include in Report");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->MapDescription = obj = fl_add_checkbutton(FL_PUSH_BUTTON,179,215,120,28,"Map Descriptions");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Responsibilities = obj = fl_add_checkbutton(FL_PUSH_BUTTON,41,188,110,28,"Responsibilities");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->PathElements = obj = fl_add_checkbutton(FL_PUSH_BUTTON,41,243,105,28,"Path Elements");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Goals = obj = fl_add_checkbutton(FL_PUSH_BUTTON,179,188,65,28,"Goals");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->StubDescriptions = obj = fl_add_checkbutton(FL_PUSH_BUTTON,41,215,120,28,"Stub Descriptions");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ComponentDescriptions = obj = fl_add_checkbutton(FL_PUSH_BUTTON,179,243,155,28,"Component Descriptions");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->LayoutOptions = obj = fl_add_box(FL_DOWN_BOX,28,382,344,100,"Printout Options");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->NewPage = obj = fl_add_checkbutton(FL_PUSH_BUTTON,31,389,172,26,"Start map on new page");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->CenterHeadings = obj = fl_add_checkbutton(FL_PUSH_BUTTON,31,419,156,26,"Center Section Headings");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Date = obj = fl_add_checkbutton(FL_PUSH_BUTTON,201,389,110,26,"Include Date");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,Date_cb,0);
  fdui->Time = obj = fl_add_checkbutton(FL_PUSH_BUTTON,201,420,122,24,"Include Date/Time");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,Time_cb,0);
  fdui->DesignDescription = obj = fl_add_checkbutton(FL_PUSH_BUTTON,41,271,125,28,"Design Description");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Performance = obj = fl_add_checkbutton(FL_PUSH_BUTTON,179,271,155,28,"Performance Annotations");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ParentListing = obj = fl_add_checkbutton(FL_PUSH_BUTTON,41,297,125,28,"Parent Map Listing");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->UCMSets = obj = fl_add_checkbutton(FL_PUSH_BUTTON,179,297,125,28,"UCM Sets");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  obj = fl_add_box(FL_DOWN_BOX,28,82,342,74,"Include Maps");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);

  fdui->MapList = fl_bgn_group();
  fdui->AllMaps = obj = fl_add_checkbutton(FL_RADIO_BUTTON,47,94,97,26,"All Maps");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->UCMSet = obj = fl_add_checkbutton(FL_RADIO_BUTTON,47,124,101,26,"UCM Set ...");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Current = obj = fl_add_checkbutton(FL_RADIO_BUTTON,167,94,120,26,"Current Map only");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->CurrentSubtree = obj = fl_add_checkbutton(FL_RADIO_BUTTON,167,125,162,24,"Current Map and Submaps");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();


  fdui->FontSize = fl_bgn_group();
  fdui->RegularFonts = obj = fl_add_checkbutton(FL_RADIO_BUTTON,31,448,156,26,"Regular Fonts (12/10 pt )");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->LargeFonts = obj = fl_add_checkbutton(FL_RADIO_BUTTON,201,449,137,24,"Large Fonts (14/12 pt.)");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,Time_cb,0);
  fl_end_group();

  fdui->Scenarios = obj = fl_add_checkbutton(FL_PUSH_BUTTON,41,322,137,28,"Scenario Definitions");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_form();

  fdui->PrintPopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_StringParagraphDialog *create_form_StringParagraphDialog(void)
{
  FL_OBJECT *obj;
  FD_StringParagraphDialog *fdui = (FD_StringParagraphDialog *) fl_calloc(1, sizeof(*fdui));

  fdui->StringParagraphDialog = fl_bgn_form(FL_NO_BOX, 308, 240);
  obj = fl_add_box(FL_UP_BOX,0,0,308,240,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,14,12,277,164,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->String = obj = fl_add_input(FL_NORMAL_INPUT,24,37,255,30,"String");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->Paragraph = obj = fl_add_input(FL_MULTILINE_INPUT,24,87,255,80,"Paragraph");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->AcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,40,190,100,30,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,170,190,100,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fl_end_form();

  fdui->StringParagraphDialog->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DesignData *create_form_DesignData(void)
{
  FL_OBJECT *obj;
  FD_DesignData *fdui = (FD_DesignData *) fl_calloc(1, sizeof(*fdui));

  fdui->DesignData = fl_bgn_form(FL_NO_BOX, 501, 273);
  obj = fl_add_box(FL_UP_BOX,0,0,501,273,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,14,16,472,192,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->DesignName = obj = fl_add_input(FL_NORMAL_INPUT,24,42,450,30,"Design Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DesignDescription = obj = fl_add_input(FL_MULTILINE_INPUT,24,98,448,99,"Description");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DesignData_OKButton = obj = fl_add_button(FL_NORMAL_BUTTON,100,220,100,30,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DesignData_OKButton_cb,0);
  fdui->DesignData_CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,300,220,100,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DesignData_CancelButton_cb,0);
  fl_end_form();

  fdui->DesignData->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_StubCreationPopup *create_form_StubCreationPopup(void)
{
  FL_OBJECT *obj;
  FD_StubCreationPopup *fdui = (FD_StubCreationPopup *) fl_calloc(1, sizeof(*fdui));

  fdui->StubCreationPopup = fl_bgn_form(FL_NO_BOX, 410, 434);
  obj = fl_add_box(FL_UP_BOX,0,0,410,434,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_box(FL_DOWN_BOX,75,68,139,32,"Stub Type");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->StubLabel = obj = fl_add_input(FL_NORMAL_INPUT,20,30,375,30,"Stub Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->AcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,100,390,70,30,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,StubLabelOKButton_cb,0);

  fdui->StubTypeButtons = fl_bgn_group();
  fdui->StaticButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,81,73,55,20,"Static");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DynamicButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,136,73,70,20,"Dynamic");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();

  fdui->CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,239,390,70,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,StubLabelCancelButton_cb,0);
  fdui->SharedButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,215,72,66,22,"Shared");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->SelectionPolicy = obj = fl_add_input(FL_MULTILINE_INPUT,20,280,376,102,"Selection Policy");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,dummy_cb,0);
  obj = fl_add_box(FL_DOWN_BOX,20,130,376,122,"Enforce Bindings");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->ContinuityBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,30,138,98,92,"Continuity Paths");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ContinuityBrowser_cb,0);
  fdui->InputsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,248,140,66,87,"Inputs");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,InputOutputBrowsers_cb,0);
  fdui->OutputsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,325,140,61,87,"Outputs");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,InputOutputBrowsers_cb,0);
  fdui->Unbind = obj = fl_add_button(FL_NORMAL_BUTTON,137,152,98,30,"Unbind Path");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,Unbind_cb,0);
  fdui->Bind = obj = fl_add_button(FL_NORMAL_BUTTON,132,192,113,30,"Bind Input/Output");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,Bind_cb,0);
  fdui->ServiceRequests = obj = fl_add_button(FL_NORMAL_BUTTON,284,68,117,30,"Service Requests");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SDServiceRequestButton_cb,0);
  fl_end_form();

  fdui->StubCreationPopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_PluginChoicePopup *create_form_PluginChoicePopup(void)
{
  FL_OBJECT *obj;
  FD_PluginChoicePopup *fdui = (FD_PluginChoicePopup *) fl_calloc(1, sizeof(*fdui));

  fdui->PluginChoicePopup = fl_bgn_form(FL_NO_BOX, 339, 483);
  obj = fl_add_box(FL_UP_BOX,0,0,339,483,"");
  fdui->PluginSelector = obj = fl_add_browser(FL_HOLD_BROWSER,10,10,320,160,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,PluginSelector_cb,0);
  fdui->PluginChoiceOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,10,180,60,25,"Label");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PluginChoiceOKButton_cb,0);
  fdui->BindPluginButton = obj = fl_add_button(FL_NORMAL_BUTTON,75,180,80,25,"Bind Plugin");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,BindPluginButton_cb,0);
  fdui->PICCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,235,180,95,25,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PICCancelButton_cb,0);
  fdui->RenameButton = obj = fl_add_button(FL_NORMAL_BUTTON,160,180,70,25,"Rename");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PICRenameButton_cb,0);
  fdui->SelectionPolicy = obj = fl_add_browser(FL_NORMAL_BROWSER,12,372,314,82,"Plugin Selection Policy Description of Stub");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
  fdui->LogicalCondition = obj = fl_add_browser(FL_NORMAL_BROWSER,13,221,314,82,"Logical Selection Condition");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->PICEditConditionButton = obj = fl_add_button(FL_NORMAL_BUTTON,248,309,76,24,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PICEditConditionButton_cb,0);
  fdui->Probability = obj = fl_add_input(FL_FLOAT_INPUT,136,336,99,26,"Selection Probability");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,Probability_cb,0);
  fl_end_form();

  fdui->PluginChoicePopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_StubBindingPopup *create_form_StubBindingPopup(void)
{
  FL_OBJECT *obj;
  FD_StubBindingPopup *fdui = (FD_StubBindingPopup *) fl_calloc(1, sizeof(*fdui));

  fdui->StubBindingPopup = fl_bgn_form(FL_NO_BOX, 292, 586);
  obj = fl_add_box(FL_UP_BOX,0,0,292,586,"");
  obj = fl_add_box(FL_FRAME_BOX,10,230,80,230,"Stub");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_West, FL_NoGravity);
  obj = fl_add_box(FL_FRAME_BOX,190,230,80,230,"Plug-In Map");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_NoGravity, FL_East);
  fdui->UnbindButton = obj = fl_add_button(FL_NORMAL_BUTTON,180,100,100,30,"Unbind");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,UnbindButton_cb,0);
  fdui->StubBindingOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,180,150,100,30,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,StubBindingOKButton_cb,0);
  fdui->EntryPointsBindButton = obj = fl_add_button(FL_NORMAL_BUTTON,100,270,80,30,"Bind Entries");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,EntryPointsBindButton_cb,0);
  fdui->ExitPointsBindButton = obj = fl_add_button(FL_NORMAL_BUTTON,100,380,80,30,"Bind Exits");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ExitPointsBindButton_cb,0);
  fdui->StubBindingsList = obj = fl_add_browser(FL_HOLD_BROWSER,10,80,160,120,"Stub Bindings");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
    fl_set_object_callback(obj,StubBindingsList_cb,0);
  fdui->StubEntryPointsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,16,240,68,90,"Entry Points");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_gravity(obj, FL_West, FL_NoGravity);
    fl_set_object_callback(obj,StubEntryPointsBrowser_cb,0);
  fdui->PluginEntryPointsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,196,240,68,90,"Entry Points");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_gravity(obj, FL_NoGravity, FL_East);
    fl_set_object_callback(obj,PluginEntryPointsBrowser_cb,0);
  fdui->PluginExitPointsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,196,350,68,90,"Exit Points");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_gravity(obj, FL_NoGravity, FL_East);
    fl_set_object_callback(obj,PluginExitPointsBrowser_cb,0);
  fdui->StubExitPointsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,16,350,68,90,"Exit Points");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_gravity(obj, FL_West, FL_NoGravity);
    fl_set_object_callback(obj,StubExitPointsBrowser_cb,0);
  fdui->PluginName = obj = fl_add_text(FL_NORMAL_TEXT,20,20,240,30,"Plug-in Name:");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->EnforcedBindings = obj = fl_add_browser(FL_NORMAL_BROWSER,18,495,255,77,"Enforced Bindings");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_West, FL_SouthEast);
  fl_end_form();

  fdui->StubBindingPopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_TimestampCreationPopup *create_form_TimestampCreationPopup(void)
{
  FL_OBJECT *obj;
  FD_TimestampCreationPopup *fdui = (FD_TimestampCreationPopup *) fl_calloc(1, sizeof(*fdui));

  fdui->TimestampCreationPopup = fl_bgn_form(FL_NO_BOX, 304, 288);
  obj = fl_add_box(FL_UP_BOX,0,0,304,288,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_box(FL_DOWN_BOX,30,190,210,50,"Map Element Reference :");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_frame(FL_DOWN_FRAME,30,110,250,40,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->TimestampLabel = obj = fl_add_input(FL_NORMAL_INPUT,30,30,250,40,"Enter Label for Timestamp Point");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);

  fdui->TimestampDirectionButons = fl_bgn_group();
  fdui->UpButton = obj = fl_add_button(FL_RADIO_BUTTON,40,115,30,30,"@8->");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DownButton = obj = fl_add_button(FL_RADIO_BUTTON,100,115,30,30,"@2->");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->LeftButton = obj = fl_add_button(FL_RADIO_BUTTON,170,115,30,30,"@->");
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->RightButton = obj = fl_add_button(FL_RADIO_BUTTON,230,115,30,30,"@4->");
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();

  fdui->TimestampCreationOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,120,246,60,30,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,TimestampCreationOKButton_cb,0);

  fdui->ReferenceButtons = fl_bgn_group();
  fdui->PreviousButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,40,201,195,17,"Previous Element Completes");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->NextButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,40,214,134,20,"Next Element Starts");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT,26,81,70,26,"Placement");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();

  fdui->TimestampCreationPopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ResponseTimePopup *create_form_ResponseTimePopup(void)
{
  FL_OBJECT *obj;
  FD_ResponseTimePopup *fdui = (FD_ResponseTimePopup *) fl_calloc(1, sizeof(*fdui));

  fdui->ResponseTimePopup = fl_bgn_form(FL_NO_BOX, 260, 240);
  obj = fl_add_box(FL_UP_BOX,0,0,260,240,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_box(FL_DOWN_BOX,20,50,220,130,"");
  fdui->ResponseTimeOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,40,190,70,30,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,ResponseTimeResult_cb,1);
  fdui->ResponseTimeCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,140,190,70,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,ResponseTimeResult_cb,0);
  fdui->ResponseTimeValue = obj = fl_add_input(FL_INT_INPUT,150,100,80,30," Response Time (us)");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ResponseTimePrompt = obj = fl_add_text(FL_NORMAL_TEXT,20,10,230,30,"Prompt");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ResponseTimePercentage = obj = fl_add_input(FL_NORMAL_INPUT,150,140,80,30,"Percentage");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Name = obj = fl_add_input(FL_NORMAL_INPUT,70,60,160,30,"Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_form();

  fdui->ResponseTimePopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ViewResponseTimes *create_form_ViewResponseTimes(void)
{
  FL_OBJECT *obj;
  FD_ViewResponseTimes *fdui = (FD_ViewResponseTimes *) fl_calloc(1, sizeof(*fdui));

  fdui->ViewResponseTimes = fl_bgn_form(FL_NO_BOX, 465, 435);
  obj = fl_add_box(FL_UP_BOX,0,0,465,435,"");
  obj = fl_add_box(FL_DOWN_BOX,20,250,425,135,"Selected Response Time Requirement");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->ResponseTimesBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,10,40,440,180,"Name                                Start      End      Response Time     Percentage");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,ResponseTimesBrowser_cb,0);
  fdui->ResponseTimeValue = obj = fl_add_input(FL_INT_INPUT,155,260,80,30," Response Time (us)");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ResponseTimePercentage = obj = fl_add_input(FL_INT_INPUT,350,260,80,30,"Percentage");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->VRTRemoveDialogButton = obj = fl_add_button(FL_NORMAL_BUTTON,180,395,120,30,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,VRTRemoveDialogButton_cb,0);

  fdui->EditRTButtons = fl_bgn_group();
  fdui->VRTAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,70,345,60,30,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,VRTAcceptButton_cb,0);
  fdui->VRTCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,220,345,60,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,VRTCancelButton_cb,0);
  fdui->VRTDeleteButton = obj = fl_add_button(FL_NORMAL_BUTTON,340,345,60,30,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,VRTDeleteButton_cb,0);
  fl_end_group();

  fdui->Name = obj = fl_add_input(FL_NORMAL_INPUT,65,300,365,30,"Name");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_form();

  fdui->ViewResponseTimes->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DeviceDirectory *create_form_DeviceDirectory(void)
{
  FL_OBJECT *obj;
  FD_DeviceDirectory *fdui = (FD_DeviceDirectory *) fl_calloc(1, sizeof(*fdui));

  fdui->DeviceDirectory = fl_bgn_form(FL_NO_BOX, 366, 703);
  obj = fl_add_box(FL_UP_BOX,0,0,366,703,"");
  fdui->DeviceBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,20,100,330,160,"Device Name       Operation Time   Characteristics");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
    fl_set_object_callback(obj,DeviceBrowser_cb,0);
  fdui->DeviceTypeChoice = obj = fl_add_choice(FL_NORMAL_CHOICE,90,30,126,34,"Device Type");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,DeviceTypeChoice_cb,0);
  fdui->AddDeviceButton = obj = fl_add_button(FL_NORMAL_BUTTON,76,272,102,36,"Add Device");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AddDeviceButton_cb,0);
  fdui->DDRemoveDialogButton = obj = fl_add_button(FL_NORMAL_BUTTON,136,658,106,30,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DDRemoveDialogButton_cb,0);
  fdui->DDDeleteButton = obj = fl_add_button(FL_NORMAL_BUTTON,196,272,98,36,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DDDeleteButton_cb,0);

  fdui->EditDevice = fl_bgn_group();
  fdui->EditDeviceBox = obj = fl_add_box(FL_DOWN_BOX,22,340,328,309,"Selected Device");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->DeviceNameInput = obj = fl_add_input(FL_NORMAL_INPUT,32,366,190,34,"Device Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_SouthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DDAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,79,609,88,31,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DDAcceptButton_cb,0);
  fdui->DDCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,194,609,88,31,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DDCancelButton_cb,0);
  fl_end_group();

  fdui->DeviceCharacteristics = obj = fl_add_input(FL_MULTILINE_INPUT,32,483,302,116,"Device Characteristics");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_SouthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->OpTime = obj = fl_add_input(FL_FLOAT_INPUT,33,425,190,34,"Operation Time");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_SouthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_form();

  fdui->DeviceDirectory->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ViewTimestampRT *create_form_ViewTimestampRT(void)
{
  FL_OBJECT *obj;
  FD_ViewTimestampRT *fdui = (FD_ViewTimestampRT *) fl_calloc(1, sizeof(*fdui));

  fdui->ViewTimestampRT = fl_bgn_form(FL_NO_BOX, 594, 380);
  obj = fl_add_box(FL_UP_BOX,0,0,594,380,"");
  obj = fl_add_box(FL_DOWN_BOX,14,50,564,113,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_box(FL_DOWN_BOX,40,190,512,128,"Selected Response Time Requirement");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->ResponseTimeValue = obj = fl_add_input(FL_INT_INPUT,230,200,80,30," Response Time (us)");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ResponseTimePercentage = obj = fl_add_input(FL_INT_INPUT,390,200,80,30,"Percentage");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);

  fdui->EditRTButtons = fl_bgn_group();
  fdui->TSRAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,190,280,60,30,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,TSRAcceptButton_cb,0);
  fdui->TSRCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,290,280,60,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,TSRCancelButton_cb,0);
  fdui->TSRDeleteButton = obj = fl_add_button(FL_NORMAL_BUTTON,390,280,60,30,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,TSRDeleteButton_cb,0);
  fl_end_group();

  fdui->TSRRemoveDialogButton = obj = fl_add_button(FL_NORMAL_BUTTON,275,330,104,30,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,TSRRemoveDialogButton_cb,0);
  fdui->TerminatingTSBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,24,60,267,80,"Name                 Start      Time   Percentage");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_South);
    fl_set_object_callback(obj,TSBrowser_cb,0);
  fdui->StartingTSBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,305,60,267,81,"Name                   End       Time  Percentage");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_SouthEast);
    fl_set_object_callback(obj,TSBrowser_cb,1);
  fdui->Prompt = obj = fl_add_text(FL_NORMAL_TEXT,15,20,560,20,"Prompt");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_North, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Name = obj = fl_add_input(FL_NORMAL_INPUT,230,240,240,30,"Name");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_form();

  fdui->ViewTimestampRT->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DataStores *create_form_DataStores(void)
{
  FL_OBJECT *obj;
  FD_DataStores *fdui = (FD_DataStores *) fl_calloc(1, sizeof(*fdui));

  fdui->DataStores = fl_bgn_form(FL_NO_BOX, 328, 520);
  obj = fl_add_box(FL_UP_BOX,0,0,328,520,"");
  obj = fl_add_box(FL_DOWN_BOX,20,60,290,275,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->DataStoreChoice = obj = fl_add_choice(FL_NORMAL_CHOICE,80,20,140,30,"Display: ");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,DSChoice_cb,0);
  fdui->DSBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,30,90,270,200,"Label");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,DSBrowser_cb,0);
  fdui->DSAddButton = obj = fl_add_button(FL_NORMAL_BUTTON,70,300,84,25,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DSAddButton_cb,0);
  fdui->DSDeleteButton = obj = fl_add_button(FL_NORMAL_BUTTON,176,300,84,25,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DSDeleteButton_cb,0);
  fdui->DSRemoveDialog = obj = fl_add_button(FL_NORMAL_BUTTON,120,480,100,30,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DSRemoveDialog_cb,0);
  obj = fl_add_box(FL_DOWN_BOX,20,360,290,110,"");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);

  fdui->InputFields = fl_bgn_group();
  fdui->DSInput = obj = fl_add_input(FL_NORMAL_INPUT,36,388,260,30,"Input");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DSAcceptButton_cb,0);
  fdui->DSCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,180,430,100,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DSCancelButton_cb,0);
  fdui->DSAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,50,430,100,30,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DSAcceptButton_cb,0);
  fl_end_group();

  fl_end_form();

  fdui->DataStores->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DataStoreUsage *create_form_DataStoreUsage(void)
{
  FL_OBJECT *obj;
  FD_DataStoreUsage *fdui = (FD_DataStoreUsage *) fl_calloc(1, sizeof(*fdui));

  fdui->DataStoreUsage = fl_bgn_form(FL_NO_BOX, 348, 538);
  obj = fl_add_box(FL_UP_BOX,0,0,348,538,"");
  fdui->Selections = obj = fl_add_box(FL_DOWN_BOX,20,295,312,166,"Available Data Stores");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->DUBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,20,50,310,170,"      Data Store                          Access Mode");
    fl_set_object_color(obj,FL_WHITE,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,DUBrowser_cb,0);
  fdui->AvailableData = obj = fl_add_browser(FL_HOLD_BROWSER,30,305,138,130,"Data Stores");
    fl_set_object_color(obj,FL_WHITE,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AvailableData_cb,0);
  fdui->AccessModes = obj = fl_add_browser(FL_HOLD_BROWSER,188,305,128,130,"Access Modes");
    fl_set_object_color(obj,FL_WHITE,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AccessModes_cb,0);
  fdui->Prompt = obj = fl_add_text(FL_NORMAL_TEXT,20,20,280,20,"Prompt");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_North, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->DURemoveDialogButton = obj = fl_add_button(FL_NORMAL_BUTTON,110,495,120,30,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DURemoveDialogButton_cb,0);
  fdui->DUAddButton = obj = fl_add_button(FL_NORMAL_BUTTON,65,250,90,30,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DUAddButton_cb,0);
  fdui->DUDeleteButton = obj = fl_add_button(FL_NORMAL_BUTTON,200,250,90,30,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DUDeleteButton_cb,0);
  fl_end_form();

  fdui->DataStoreUsage->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ArrivalProcess *create_form_ArrivalProcess(void)
{
  FL_OBJECT *obj;
  FD_ArrivalProcess *fdui = (FD_ArrivalProcess *) fl_calloc(1, sizeof(*fdui));

  fdui->ArrivalProcess = fl_bgn_form(FL_NO_BOX, 428, 330);
  obj = fl_add_box(FL_UP_BOX,0,0,428,330,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->ExpertInput = obj = fl_add_input(FL_NORMAL_INPUT,17,225,369,30," Arrival Process Characteristics");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DistributionBox = obj = fl_add_frame(FL_DOWN_FRAME,9,126,404,144,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_DOWN_BOX,12,155,395,43,"Time Distribution");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->Input1 = obj = fl_add_input(FL_FLOAT_INPUT,17,227,96,30,"Input1");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->Input2 = obj = fl_add_input(FL_FLOAT_INPUT,158,227,96,30,"Input2");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->APOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,110,283,76,32,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,APOKButton_cb,0);
  fdui->APCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,220,283,76,32,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,APCancelButton_cb,0);
  fdui->WorkloadType = obj = fl_add_choice(FL_NORMAL_CHOICE2,106,15,277,35,"Workload Type");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,WorkloadType_cb,0);
  fdui->PopulationSize = obj = fl_add_input(FL_INT_INPUT,106,62,96,28,"Population Size");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);

  fdui->DistributionButtons = fl_bgn_group();
  fdui->APExponentialButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,16,167,91,22,"Exponential");
    fl_set_object_color(obj,FL_MCOL,FL_PALEGREEN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,APExponentialButton_cb,0);
  fdui->APErlangButton = obj = fl_add_roundbutton(FL_NORMAL_BUTTON,270,167,60,22,"Erlang");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,APErlangButton_cb,0);
  fdui->APExpertButton = obj = fl_add_roundbutton(FL_NORMAL_BUTTON,333,167,64,22,"Expert");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,APExpertButton_cb,0);
  fdui->APDeterministicButton = obj = fl_add_roundbutton(FL_NORMAL_BUTTON,104,167,87,22,"Deterministic");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,APDeterministicButton_cb,0);
  fdui->APUniformButton = obj = fl_add_roundbutton(FL_NORMAL_BUTTON,200,167,69,22,"Uniform");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,APUniformButton_cb,0);
  fl_end_group();

  fdui->DistributionLabel = obj = fl_add_text(FL_NORMAL_TEXT,10,99,292,25,"Distribution Label");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();

  fdui->ArrivalProcess->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_PoolPluginChoicePopup *create_form_PoolPluginChoicePopup(void)
{
  FL_OBJECT *obj;
  FD_PoolPluginChoicePopup *fdui = (FD_PoolPluginChoicePopup *) fl_calloc(1, sizeof(*fdui));

  fdui->PoolPluginChoicePopup = fl_bgn_form(FL_NO_BOX, 340, 220);
  obj = fl_add_box(FL_UP_BOX,0,0,340,220,"");
  fdui->PoolPluginSelector = obj = fl_add_browser(FL_MULTI_BROWSER,10,10,320,160,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,PoolPluginSelector_cb,0);
  fdui->PoolPluginChoiceCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,260,180,70,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PoolPluginChoiceCancelButton_cb,0);
  fdui->PoolPluginChoiceOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,10,180,70,30,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PoolPluginChoiceOKButton_cb,0);
  fdui->PoolPluginChoiceRenameButton = obj = fl_add_button(FL_NORMAL_BUTTON,180,180,70,30,"Rename");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PoolPluginChoiceRenameButton_cb,0);
  fl_end_form();

  fdui->PoolPluginChoicePopup->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_GoalEditor *create_form_GoalEditor(void)
{
  FL_OBJECT *obj;
  FD_GoalEditor *fdui = (FD_GoalEditor *) fl_calloc(1, sizeof(*fdui));

  fdui->GoalEditor = fl_bgn_form(FL_NO_BOX, 572, 412);
  obj = fl_add_box(FL_UP_BOX,0,0,572,412,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,8,550,336,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->Label = obj = fl_add_text(FL_NORMAL_TEXT,16,14,230,46,"Text");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->GoalName = obj = fl_add_input(FL_NORMAL_INPUT,20,90,224,30,"Goal Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->GoalDescription = obj = fl_add_input(FL_MULTILINE_INPUT,22,148,222,176,"Goal Description");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_South);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->GoalPreconditions = obj = fl_add_browser(FL_MULTI_BROWSER,278,40,262,100,"Preconditions");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_East);
    fl_set_object_callback(obj,GoalConditions_cb,0);
  fdui->GoalPostconditions = obj = fl_add_browser(FL_MULTI_BROWSER,276,200,264,110,"Postconditions");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NoGravity, FL_SouthEast);
    fl_set_object_callback(obj,GoalConditions_cb,1);
  fdui->GoalAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,146,357,98,38,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GoalAcceptButton_cb,0);
  fdui->GoalCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,321,357,98,38,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GoalCancelButton_cb,0);
  fdui->GoalPostconditionsAdd = obj = fl_add_button(FL_NORMAL_BUTTON,279,314,82,22,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GoalConditionsAdd_cb,1);
  fdui->GoalPostconditionsEdit = obj = fl_add_button(FL_NORMAL_BUTTON,369,314,82,22,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GoalConditionsEdit_cb,1);
  fdui->GoalPostconditionsDelete = obj = fl_add_button(FL_NORMAL_BUTTON,457,314,82,22,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GoalConditionsDelete_cb,1);
  fdui->GoalPreconditionsAdd = obj = fl_add_button(FL_NORMAL_BUTTON,280,147,82,22,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GoalConditionsAdd_cb,0);
  fdui->GoalPreconditionsEdit = obj = fl_add_button(FL_NORMAL_BUTTON,368,147,82,22,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GoalConditionsEdit_cb,0);
  fdui->GoalPreconditionsDelete = obj = fl_add_button(FL_NORMAL_BUTTON,458,147,82,22,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GoalConditionsDelete_cb,0);
  fl_end_form();

  fdui->GoalEditor->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ServiceRequests *create_form_ServiceRequests(void)
{
  FL_OBJECT *obj;
  FD_ServiceRequests *fdui = (FD_ServiceRequests *) fl_calloc(1, sizeof(*fdui));

  fdui->ServiceRequests = fl_bgn_form(FL_NO_BOX, 336, 492);
  obj = fl_add_box(FL_UP_BOX,0,0,336,492,"");
  obj = fl_add_box(FL_DOWN_BOX,16,197,302,246,"Selected Service Request");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->SRAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,37,403,71,30,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SRAcceptButton_cb,0);
  fdui->SRDeleteButton = obj = fl_add_button(FL_NORMAL_BUTTON,224,403,73,30,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SRDeleteButton_cb,0);
  fdui->SRRemoveDialogButton = obj = fl_add_button(FL_NORMAL_BUTTON,120,452,110,30,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SRRemoveDialogButton_cb,0);
  fdui->SRCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,131,403,72,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SRCancelButton_cb,0);
  fdui->ServiceCategoryChoice = obj = fl_add_choice(FL_NORMAL_CHOICE2,130,206,86,32,"Service Category");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ServiceCategoryChoice_cb,0);
  fdui->RequestNumberInput = obj = fl_add_input(FL_NORMAL_INPUT,130,359,176,32,"Request Quantity");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->SRBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,17,38,296,130,"Service Type                               Quantity");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,SRBrowser_cb,0);
  fdui->ServiceType = obj = fl_add_browser(FL_HOLD_BROWSER,125,251,181,96,"Service Name");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_form();

  fdui->ServiceRequests->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_SelectMap *create_form_SelectMap(void)
{
  FL_OBJECT *obj;
  FD_SelectMap *fdui = (FD_SelectMap *) fl_calloc(1, sizeof(*fdui));

  fdui->SelectMap = fl_bgn_form(FL_NO_BOX, 397, 241);
  obj = fl_add_box(FL_UP_BOX,0,0,397,241,"");
  fdui->SelectMapBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,12,77,373,114,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,SelectMapBrowser_cb,0);
  fdui->InstallButton = obj = fl_add_button(FL_NORMAL_BUTTON,12,201,49,26,"Install");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SMInstallButton_cb,0);
  fdui->DeleteButton = obj = fl_add_button(FL_NORMAL_BUTTON,131,201,70,26,"Delete Map");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SMDeleteButton_cb,0);
  fdui->CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,328,201,58,26,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SMCancelButton_cb,0);
  fdui->RenameButton = obj = fl_add_button(FL_NORMAL_BUTTON,66,201,57,26,"Rename");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SMRenameButton_cb,0);
  fdui->MapTypeChoice = obj = fl_add_choice(FL_NORMAL_CHOICE2,151,9,102,28,"Map Type");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,SMMapTypeChoice_cb,0);
  fdui->MapName = obj = fl_add_input(FL_NORMAL_INPUT,83,44,301,25,"Map Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,MapName_cb,0);
  fdui->ConvertTypeButton = obj = fl_add_button(FL_NORMAL_BUTTON,207,201,120,26,"Convert To Plugin");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ConvertTypeButton_cb,0);
  fl_end_form();

  fdui->SelectMap->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_HelpWindow *create_form_HelpWindow(void)
{
  FL_OBJECT *obj;
  FD_HelpWindow *fdui = (FD_HelpWindow *) fl_calloc(1, sizeof(*fdui));

  fdui->HelpWindow = fl_bgn_form(FL_NO_BOX, 474, 484);
  obj = fl_add_box(FL_UP_BOX,0,0,474,484,"");
  fdui->HelpDisplay = obj = fl_add_browser(FL_NORMAL_BROWSER,19,17,437,420,"");
    fl_set_object_color(obj,FL_WHITE,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->HelpCloseButton = obj = fl_add_button(FL_NORMAL_BUTTON,200,443,84,28,"Close");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,HelpCloseButton_cb,0);
  fl_end_form();

  fdui->HelpWindow->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ParentMapList *create_form_ParentMapList(void)
{
  FL_OBJECT *obj;
  FD_ParentMapList *fdui = (FD_ParentMapList *) fl_calloc(1, sizeof(*fdui));

  fdui->ParentMapList = fl_bgn_form(FL_NO_BOX, 300, 194);
  obj = fl_add_box(FL_UP_BOX,0,0,300,194,"");
  fdui->ParentListBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,13,30,272,116,"Parent Map                                       Stub");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,ParentListBrowser_cb,0);
  fdui->DismissButton = obj = fl_add_button(FL_NORMAL_BUTTON,185,155,78,28,"Dismiss");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PML_Dismiss_cb,0);
  fdui->InstallButton = obj = fl_add_button(FL_NORMAL_BUTTON,36,155,78,28,"Install");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,PML_Install_cb,0);
  fl_end_form();

  fdui->ParentMapList->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_GraphicCreationDialog *create_form_GraphicCreationDialog(void)
{
  FL_OBJECT *obj;
  FD_GraphicCreationDialog *fdui = (FD_GraphicCreationDialog *) fl_calloc(1, sizeof(*fdui));

  fdui->GraphicCreationDialog = fl_bgn_form(FL_NO_BOX, 240, 188);
  obj = fl_add_box(FL_UP_BOX,0,0,240,188,"");
  fdui->ChoicesBox = obj = fl_add_box(FL_FRAME_BOX,26,30,196,104,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
  fdui->GenerateButton = obj = fl_add_button(FL_NORMAL_BUTTON,46,145,67,25,"Generate");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,GenerateButton_cb,0);
  fdui->CancelGenerate = obj = fl_add_button(FL_NORMAL_BUTTON,142,145,67,25,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,CancelGenerate_cb,0);

  fdui->MapGroup = fl_bgn_group();
  fdui->CurrentMap = obj = fl_add_roundbutton(FL_RADIO_BUTTON,31,33,125,22,"Current Map Only");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->CurrentSubtree = obj = fl_add_roundbutton(FL_RADIO_BUTTON,31,58,165,22,"Current Map and Submaps");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->AllMaps = obj = fl_add_roundbutton(FL_RADIO_BUTTON,31,101,75,22,"All Maps");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->UCMSet = obj = fl_add_roundbutton(FL_RADIO_BUTTON,31,79,85,22,"UCM Set ...");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();

  fl_end_form();

  fdui->GraphicCreationDialog->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_UcmSetSpecification *create_form_UcmSetSpecification(void)
{
  FL_OBJECT *obj;
  FD_UcmSetSpecification *fdui = (FD_UcmSetSpecification *) fl_calloc(1, sizeof(*fdui));

  fdui->UcmSetSpecification = fl_bgn_form(FL_NO_BOX, 508, 570);
  obj = fl_add_box(FL_UP_BOX,0,0,508,570,"");
  fdui->RemainingMapsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,298,125,190,192,"Don't Include");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_SouthEast);
    fl_set_object_callback(obj,RemainingMapsBrowser_cb,0);
  fdui->SetMapsBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,14,127,190,192,"Include Maps");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_South);
    fl_set_object_callback(obj,SetMapsBrowser_cb,0);
  fdui->SetNameInput = obj = fl_add_input(FL_NORMAL_INPUT,104,20,389,28,"UCM Set Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->PrimaryMapDisplay = obj = fl_add_browser(FL_NORMAL_BROWSER,89,62,291,30,"Primary Map");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_browser_hscrollbar(obj, FL_OFF);
    fl_set_browser_vscrollbar(obj, FL_OFF);
  fdui->SetPrimaryMapButton = obj = fl_add_button(FL_NORMAL_BUTTON,384,61,112,30,"Set Primary Map");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,SetPrimaryMapButton_cb,0);
  fdui->IncludeMapButton = obj = fl_add_button(FL_NORMAL_BUTTON,214,184,74,22,"@4->");
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,IncludeMapButton_cb,0);
  fdui->RemoveMapButton = obj = fl_add_button(FL_NORMAL_BUTTON,214,239,74,22,"@->");
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,RemoveMapButton_cb,0);
  fdui->MapCommentInput = obj = fl_add_input(FL_MULTILINE_INPUT,14,348,475,76,"Map Comment");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->SetDescriptionInput = obj = fl_add_input(FL_MULTILINE_INPUT,14,444,475,76,"UCM Set Description");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->USSRemoveButton = obj = fl_add_button(FL_NORMAL_BUTTON,262,529,114,28,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,USSRemoveButton_cb,0);
  fdui->MapTypeChoice = obj = fl_add_choice(FL_NORMAL_CHOICE2,213,127,75,32,"Map Type");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,MapTypeChoice_cb,0);
  fdui->USSAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,110,529,114,28,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,USSAcceptButton_cb,0);
  fl_end_form();

  fdui->UcmSetSpecification->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_UcmSetChoice *create_form_UcmSetChoice(void)
{
  FL_OBJECT *obj;
  FD_UcmSetChoice *fdui = (FD_UcmSetChoice *) fl_calloc(1, sizeof(*fdui));

  fdui->UcmSetChoice = fl_bgn_form(FL_NO_BOX, 386, 210);
  obj = fl_add_box(FL_UP_BOX,0,0,386,210,"");
  fdui->UcmSetBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,14,12,361,148,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,UcmSetBrowser_cb,0);
  fdui->USCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,310,169,65,29,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,USCancelButton_cb,0);

  fdui->ModificationButtons = fl_bgn_group();
  fdui->RestrictNavigation = obj = fl_add_button(FL_NORMAL_BUTTON,92,169,130,29,"Restrict Navigation");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,RestrictNavigation_cb,0);
  fdui->EditSetButton = obj = fl_add_button(FL_NORMAL_BUTTON,14,169,75,29,"Edit Set");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,EditSetButton_cb,0);
  fdui->DeleteSetButton = obj = fl_add_button(FL_NORMAL_BUTTON,228,169,75,29,"Delete Set");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DeleteSetButton_cb,0);
  fl_end_group();

  fl_end_form();

  fdui->UcmSetChoice->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ReferenceReplacement *create_form_ReferenceReplacement(void)
{
  FL_OBJECT *obj;
  FD_ReferenceReplacement *fdui = (FD_ReferenceReplacement *) fl_calloc(1, sizeof(*fdui));

  fdui->ReferenceReplacement = fl_bgn_form(FL_NO_BOX, 394, 358);
  obj = fl_add_box(FL_UP_BOX,0,0,394,358,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,9,375,296,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->ReplacingReference = obj = fl_add_browser(FL_HOLD_BROWSER,200,29,176,235,"With References To");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_SouthEast);
    fl_set_object_callback(obj,ReplacingReference_cb,0);
  fdui->ReplaceList = obj = fl_add_browser(FL_MULTI_BROWSER,15,29,176,235,"Replace References To");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_South);
    fl_set_object_callback(obj,ReplaceList_cb,0);
  fdui->Deselect = obj = fl_add_button(FL_NORMAL_BUTTON,35,272,143,26,"Deselect References");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,Deselect_cb,0);
  fdui->Replace = obj = fl_add_button(FL_NORMAL_BUTTON,215,272,143,26,"Replace");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,Replace_cb,0);
  fdui->Dismiss = obj = fl_add_button(FL_NORMAL_BUTTON,153,317,98,31,"Dismiss");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,Dismiss_cb,0);
  fl_end_form();

  fdui->ReferenceReplacement->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_AddCurrentMapUCMSet *create_form_AddCurrentMapUCMSet(void)
{
  FL_OBJECT *obj;
  FD_AddCurrentMapUCMSet *fdui = (FD_AddCurrentMapUCMSet *) fl_calloc(1, sizeof(*fdui));

  fdui->AddCurrentMapUCMSet = fl_bgn_form(FL_NO_BOX, 386, 245);
  obj = fl_add_box(FL_UP_BOX,0,0,386,245,"");
  fdui->CurrentMapDisplay = obj = fl_add_browser(FL_NORMAL_BROWSER,15,25,358,28,"Current Map");
    fl_set_object_color(obj,FL_WHITE,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_browser_vscrollbar(obj, FL_OFF);
  fdui->UCMSetsList = obj = fl_add_browser(FL_HOLD_BROWSER,15,77,357,114,"UCM Sets");
    fl_set_object_color(obj,FL_WHITE,FL_CYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,UCMSetsList_cb,0);
  fdui->AddCurrentMapButton = obj = fl_add_button(FL_NORMAL_BUTTON,12,201,113,29,"Add Current Map");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AddCurrentMapButton_cb,0);
  fdui->RemoveCurrentMapButton = obj = fl_add_button(FL_NORMAL_BUTTON,130,201,138,29,"Remove Current Map");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,RemoveCurrentMapButton_cb,0);
  fdui->RemoveAMSDialog = obj = fl_add_button(FL_NORMAL_BUTTON,270,201,103,29,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,RemoveAMSDialog_cb,0);
  fl_end_form();

  fdui->AddCurrentMapUCMSet->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_TwoStringDialog *create_form_TwoStringDialog(void)
{
  FL_OBJECT *obj;
  FD_TwoStringDialog *fdui = (FD_TwoStringDialog *) fl_calloc(1, sizeof(*fdui));

  fdui->TwoStringDialog = fl_bgn_form(FL_NO_BOX, 392, 167);
  obj = fl_add_box(FL_UP_BOX,0,0,392,167,"");
  fdui->String1 = obj = fl_add_input(FL_NORMAL_INPUT,17,29,355,27,"Input1");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->String2 = obj = fl_add_input(FL_NORMAL_INPUT,17,77,355,27,"Input2");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->AcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,64,119,102,31,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,220,119,102,31,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fl_end_form();

  fdui->TwoStringDialog->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_QuestionDialog *create_form_QuestionDialog(void)
{
  FL_OBJECT *obj;
  FD_QuestionDialog *fdui = (FD_QuestionDialog *) fl_calloc(1, sizeof(*fdui));

  fdui->QuestionDialog = fl_bgn_form(FL_NO_BOX, 480, 130);
  obj = fl_add_box(FL_UP_BOX,0,0,480,130,"");
  fdui->YesButton = obj = fl_add_button(FL_NORMAL_BUTTON,10,81,224,37,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->NoButton = obj = fl_add_button(FL_NORMAL_BUTTON,243,81,224,37,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->String1 = obj = fl_add_box(FL_NO_BOX,13,12,452,28,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->String2 = obj = fl_add_box(FL_NO_BOX,13,43,452,28,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();

  fdui->QuestionDialog->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_BranchSpecification *create_form_BranchSpecification(void)
{
  FL_OBJECT *obj;
  FD_BranchSpecification *fdui = (FD_BranchSpecification *) fl_calloc(1, sizeof(*fdui));

  fdui->BranchSpecification = fl_bgn_form(FL_NO_BOX, 334, 426);
  obj = fl_add_box(FL_UP_BOX,0,0,334,426,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,17,109,301,258,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->BranchChoice = obj = fl_add_choice(FL_NORMAL_CHOICE2,151,65,79,35,"Fork Output Branch");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,BranchChoice_cb,0);
  fdui->BranchSelection = obj = fl_add_input(FL_MULTILINE_INPUT,26,240,284,66,"Branch Selection");
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_callback(obj,BranchSelection_cb,0);
  fdui->Probability = obj = fl_add_input(FL_FLOAT_INPUT,26,328,129,32,"Probability");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->ForkName = obj = fl_add_input(FL_NORMAL_INPUT,16,26,306,28,"Fork Name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->FSAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,47,377,105,34,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,FSAcceptButton_cb,0);
  fdui->FSCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,184,377,105,34,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,FSCancelButton_cb,0);
  fdui->LogicalCondition = obj = fl_add_browser(FL_NORMAL_BROWSER,28,146,282,71,"Logical Selection Condition");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->FSEditConditionButton = obj = fl_add_button(FL_NORMAL_BUTTON,207,116,105,24,"Edit Condition");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,FSEditConditionButton_cb,0);
  fl_end_form();

  fdui->BranchSpecification->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_BooleanVariables *create_form_BooleanVariables(void)
{
  FL_OBJECT *obj;
  FD_BooleanVariables *fdui = (FD_BooleanVariables *) fl_calloc(1, sizeof(*fdui));

  fdui->BooleanVariables = fl_bgn_form(FL_NO_BOX, 234, 364);
  obj = fl_add_box(FL_UP_BOX,0,0,234,364,"");
  obj = fl_add_box(FL_FRAME_BOX,17,215,197,99,"");
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->BooleanList = obj = fl_add_browser(FL_HOLD_BROWSER,15,14,200,152,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,BooleanList_cb,0);
  fdui->CurrentBoolean = obj = fl_add_input(FL_NORMAL_INPUT,30,243,172,28,"Label");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->BVAcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,29,279,68,27,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,BVAcceptButton_cb,0);
  fdui->BVAddButton = obj = fl_add_button(FL_NORMAL_BUTTON,15,173,88,27,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,BVAddButton_cb,0);
  fdui->BVDeleteButton = obj = fl_add_button(FL_NORMAL_BUTTON,129,173,88,27,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,BVDeleteButton_cb,0);
  fdui->BVCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,131,279,68,27,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,BVCancelButton_cb,0);
  fdui->BVRemoveDialogButton = obj = fl_add_button(FL_NORMAL_BUTTON,64,322,113,27,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,BVRemoveDialogButton_cb,0);
  fl_end_form();

  fdui->BooleanVariables->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ScenarioSpecification *create_form_ScenarioSpecification(void)
{
  FL_OBJECT *obj;
  FD_ScenarioSpecification *fdui = (FD_ScenarioSpecification *) fl_calloc(1, sizeof(*fdui));

  fdui->ScenarioSpecification = fl_bgn_form(FL_NO_BOX, 609, 632);
  obj = fl_add_box(FL_UP_BOX,0,0,609,632,"");
  obj = fl_add_box(FL_FRAME_BOX,11,307,585,278,"Selected Scenario");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_box(FL_DOWN_BOX,22,313,351,31,"");
  obj = fl_add_browser(FL_HOLD_BROWSER,321,46,266,100,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GroupsList_cb,0);
  obj = fl_add_frame(FL_BORDER_FRAME,392,314,189,165,"Scenario  Starting Points");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_SouthEast);
  fdui->GroupsBox = obj = fl_add_box(FL_FRAME_BOX,11,35,285,251,"Scenario Groups");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->DeleteGroupButton = obj = fl_add_button(FL_NORMAL_BUTTON,204,153,83,22,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DeleteGroupButton_cb,0);
  fdui->ScenariosBox = obj = fl_add_box(FL_FRAME_BOX,306,35,290,251,"Scenarios");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->ScenariosList = obj = fl_add_browser(FL_HOLD_BROWSER,315,46,271,100,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ScenariosList_cb,0);
  fdui->EditScenarioButton = obj = fl_add_button(FL_NORMAL_BUTTON,375,153,60,23,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,EditScenarioButton_cb,0);
  fdui->DeleteScenarioButton = obj = fl_add_button(FL_NORMAL_BUTTON,438,153,59,23,"Delete");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DeleteScenarioButton_cb,0);
  fdui->RemoveDialogButton = obj = fl_add_button(FL_NORMAL_BUTTON,252,589,99,28,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,VSRemoveDialogButton_cb,0);
  fdui->AddScenarioButton = obj = fl_add_button(FL_NORMAL_BUTTON,314,153,59,23,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AddScenarioButton_cb,0);
  fdui->GroupMSCButton = obj = fl_add_button(FL_NORMAL_BUTTON,176,177,55,27,"To MSC");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GroupMSCButton_cb,0);
  fdui->GroupsList = obj = fl_add_browser(FL_HOLD_BROWSER,21,46,266,100,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GroupsList_cb,0);
  fdui->AddGroupButton = obj = fl_add_button(FL_NORMAL_BUTTON,19,153,84,22,"Add");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AddGroupButton_cb,0);
  fdui->EditGroupButton = obj = fl_add_button(FL_NORMAL_BUTTON,110,153,87,22,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,EditGroupButton_cb,0);
  fdui->VariableInitializations = obj = fl_add_browser(FL_HOLD_BROWSER,194,366,186,170,"Variable  Initializations");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_South);
    fl_set_object_callback(obj,VariableInitializations_cb,0);
  fdui->BooleanVariables = obj = fl_add_browser(FL_HOLD_BROWSER,19,366,166,89," Boolean Variables (unused)");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_South);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->DeleteInitButton = obj = fl_add_button(FL_NORMAL_BUTTON,191,547,116,25,"Delete Initialization");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DeleteInitButton_cb,0);
  fdui->ToggleInitButton = obj = fl_add_button(FL_NORMAL_BUTTON,310,547,74,25,"Toggle Value");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ToggleInitButton_cb,0);
  obj = fl_add_box(FL_DOWN_BOX,19,509,162,29,"Initial Value");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->AddInitButton = obj = fl_add_button(FL_NORMAL_BUTTON,19,547,161,25,"Add Variable Initialization");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AddInitButton_cb,0);

  fdui->ValueGroup = fl_bgn_group();
  fdui->TrueButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,26,513,75,23,"True");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fdui->FalseButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,107,513,70,23,"False");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,dummy_cb,0);
  fl_end_group();

  fdui->StartingPoints = obj = fl_add_browser(FL_HOLD_BROWSER,396,336,180,106,"");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_SouthEast);
    fl_set_object_callback(obj,StartingPoints_cb,0);
  fdui->ChangeGroupButton = obj = fl_add_button(FL_NORMAL_BUTTON,499,153,87,23,"Change Group");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ChangeGroupButton_cb,0);
  fdui->ScenarioDescription = obj = fl_add_browser(FL_NORMAL_BROWSER,311,182,273,84,"Description of Scenario");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_North, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->ScenarioGroupDescription = obj = fl_add_browser(FL_NORMAL_BROWSER,21,209,265,57,"Description of Scenario Group");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->EditStartList = obj = fl_add_button(FL_NORMAL_BUTTON,402,449,172,26,"Edit  List");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,EditStartList_cb,0);
  fdui->AddSelectedGroupButton = obj = fl_add_button(FL_NORMAL_BUTTON,20,177,155,27,"Add Selected Path Start");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AddSelectedGroupButton_cb,0);
  fdui->ViewBVButton = obj = fl_add_button(FL_NORMAL_BUTTON,21,461,161,25,"View Boolean Variables");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ViewBVButton_cb,0);

  fdui->TypeGroup = fl_bgn_group();
  fdui->InitializationType = obj = fl_add_roundbutton(FL_RADIO_BUTTON,25,319,167,21,"Variable Initializations");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,InitializationType_cb,0);
  fdui->PostconditionType = obj = fl_add_roundbutton(FL_RADIO_BUTTON,203,319,162,21,"Scenario Postconditions");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,PostconditionType_cb,0);
  fl_end_group();

  fdui->GroupXMLButton = obj = fl_add_button(FL_NORMAL_BUTTON,232,177,56,27,"To XML");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GroupXMLButton_cb,0);

  fdui->ScenarioOperationGroup = fl_bgn_group();
  fdui->ScenarioXMLButton = obj = fl_add_button(FL_NORMAL_BUTTON,503,486,79,24,"To XML");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ScenarioXMLButton_cb,0);
  fdui->ScenarioMSCButton = obj = fl_add_button(FL_NORMAL_BUTTON,394,486,84,25,"To MSC");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ScenarioMSCButton_cb,0);
  fdui->TraceButton = obj = fl_add_button(FL_NORMAL_BUTTON,393,516,192,25,"Highlight Path");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,TraceButton_cb,0);
  fdui->Duplicate = obj = fl_add_button(FL_NORMAL_BUTTON,393,546,192,25,"Duplicate Scenario");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,Duplicate_cb,0);
  fl_end_group();

  fl_end_form();

  fdui->ScenarioSpecification->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ConditionEditor *create_form_ConditionEditor(void)
{
  FL_OBJECT *obj;
  FD_ConditionEditor *fdui = (FD_ConditionEditor *) fl_calloc(1, sizeof(*fdui));

  fdui->ConditionEditor = fl_bgn_form(FL_NO_BOX, 379, 301);
  obj = fl_add_box(FL_UP_BOX,0,0,379,301,"");
  fdui->BooleanVariables = obj = fl_add_browser(FL_SELECT_BROWSER,14,141,345,100,"Global Boolean Variables (select to insert)");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_West, FL_SouthEast);
  fdui->Condition = obj = fl_add_input(FL_MULTILINE_INPUT,15,25,349,92,"Edit Selection Condition ( use variables, &,+,! ,= ,!=, ~, ( ) )");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
  fdui->AcceptButton = obj = fl_add_button(FL_NORMAL_BUTTON,55,253,96,33,"Accept");
    fl_set_object_color(obj,FL_COL1,FL_MCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,224,253,96,33,"Cancel");
    fl_set_object_color(obj,FL_COL1,FL_MCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fl_end_form();

  fdui->ConditionEditor->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_StartScenarios *create_form_StartScenarios(void)
{
  FL_OBJECT *obj;
  FD_StartScenarios *fdui = (FD_StartScenarios *) fl_calloc(1, sizeof(*fdui));

  fdui->StartScenarios = fl_bgn_form(FL_NO_BOX, 424, 201);
  obj = fl_add_box(FL_UP_BOX,0,0,424,201,"");
  fdui->ScenarioList = obj = fl_add_browser(FL_HOLD_BROWSER,12,28,401,130,"Scenario ( Group )");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,ScenarioList_cb,0);
  fdui->HighlightPath = obj = fl_add_button(FL_NORMAL_BUTTON,10,166,94,27,"Highlight Path");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,HighlightPath_cb,0);
  fdui->Generate = obj = fl_add_button(FL_NORMAL_BUTTON,111,166,99,27,"Generate MSC");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,Generate_cb,0);
  fdui->GenerateXML = obj = fl_add_button(FL_NORMAL_BUTTON,221,166,103,27,"Generate XML");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,GenerateXML_cb,0);
  fdui->LSSClose = obj = fl_add_button(FL_NORMAL_BUTTON,333,167,77,26,"Close");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,LSSClose_cb,0);
  fl_end_form();

  fdui->StartScenarios->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_MSCGenerationStatus *create_form_MSCGenerationStatus(void)
{
  FL_OBJECT *obj;
  FD_MSCGenerationStatus *fdui = (FD_MSCGenerationStatus *) fl_calloc(1, sizeof(*fdui));

  fdui->MSCGenerationStatus = fl_bgn_form(FL_NO_BOX, 452, 229);
  obj = fl_add_box(FL_UP_BOX,0,0,452,229,"");
  fdui->GenerateButton = obj = fl_add_button(FL_NORMAL_BUTTON,53,132,143,30,"Generate MSC's");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->CancelButton = obj = fl_add_button(FL_TOUCH_BUTTON,220,132,168,30,"Cancel MSC Generation");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,17,20,99,25,"Scenario Count :");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->Count = obj = fl_add_text(FL_NORMAL_TEXT,123,21,79,25,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_WHITE,FL_MCOL);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,212,21,89,25,"Elapsed Time: ");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->Time = obj = fl_add_text(FL_NORMAL_TEXT,304,21,104,25,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_WHITE,FL_MCOL);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->CurrentScenario = obj = fl_add_text(FL_NORMAL_TEXT,133,55,69,25,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_WHITE,FL_MCOL);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->CurrentNumberLabel = obj = fl_add_text(FL_NORMAL_TEXT,4,55,129,25,"Generating Scenario :");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->ScenarioBasename = obj = fl_add_input(FL_NORMAL_INPUT,131,89,276,25,"");
    fl_set_object_color(obj,FL_RIGHT_BCOL,FL_WHITE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,5,89,130,21,"Scenario Basename");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->BytesWritten = obj = fl_add_text(FL_NORMAL_TEXT,304,55,104,25,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_WHITE,FL_MCOL);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,202,54,104,25,"Data Generated:: ");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();

  fdui->MSCGenerationStatus->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_ErrorDialog *create_form_ErrorDialog(void)
{
  FL_OBJECT *obj;
  FD_ErrorDialog *fdui = (FD_ErrorDialog *) fl_calloc(1, sizeof(*fdui));

  fdui->ErrorDialog = fl_bgn_form(FL_NO_BOX, 428, 203);
  obj = fl_add_box(FL_UP_BOX,0,0,428,203,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->ErrorLog = obj = fl_add_browser(FL_NORMAL_BROWSER,14,12,400,130,"");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->ClearLogButton = obj = fl_add_button(FL_NORMAL_BUTTON,64,158,120,28,"Clear Log");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ClearLogButton_cb,0);
  fdui->DismissButton = obj = fl_add_button(FL_NORMAL_BUTTON,229,157,120,28,"Dismiss");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DismissButton_cb,0);
  fl_end_form();

  fdui->ErrorDialog->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_VariableOperations *create_form_VariableOperations(void)
{
  FL_OBJECT *obj;
  FD_VariableOperations *fdui = (FD_VariableOperations *) fl_calloc(1, sizeof(*fdui));

  fdui->VariableOperations = fl_bgn_form(FL_NO_BOX, 554, 389);
  obj = fl_add_box(FL_UP_BOX,0,0,554,389,"");
  fdui->OperationsList = obj = fl_add_browser(FL_HOLD_BROWSER,15,25,512,115,"Variable Operations");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,OperationsList_cb,0);
  fdui->UBVList = obj = fl_add_browser(FL_HOLD_BROWSER,15,194,207,146,"Unreferenced Boolean Variables");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj,UBVList_cb,0);
  obj = fl_add_box(FL_DOWN_BOX,237,197,291,41,"New Variable Value");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);

  fdui->ValueButtons = fl_bgn_group();
  fdui->TrueButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,243,203,54,24,"True");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,TrueButton_cb,0);
  fdui->FalseButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,302,203,59,24,"False");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,FalseButton_cb,0);
  fdui->ExpressionButton = obj = fl_add_roundbutton(FL_RADIO_BUTTON,363,203,159,24,"Expression Evaluation");
    fl_set_object_color(obj,FL_MCOL,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ExpressionButton_cb,0);
  fl_end_group();

  fdui->ExpressionDisplay = obj = fl_add_browser(FL_NORMAL_BROWSER,237,266,290,73,"Logical Expression");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->EditExpression = obj = fl_add_button(FL_NORMAL_BUTTON,388,243,138,20,"Edit Expression");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,EditExpression_cb,0);
  fdui->VariableOperationsClose = obj = fl_add_button(FL_NORMAL_BUTTON,222,348,112,28,"Remove Dialog");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,VariableOperationsClose_cb,0);
  fdui->AddOperation = obj = fl_add_button(FL_NORMAL_BUTTON,92,147,182,22,"Add/Edit Variable Operation");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,AddOperation_cb,0);
  fdui->DeleteOperation = obj = fl_add_button(FL_NORMAL_BUTTON,314,147,182,22,"Delete Selected Operation");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,DeleteOperation_cb,0);
  fl_end_form();

  fdui->VariableOperations->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_EditStartList *create_form_EditStartList(void)
{
  FL_OBJECT *obj;
  FD_EditStartList *fdui = (FD_EditStartList *) fl_calloc(1, sizeof(*fdui));

  fdui->EditStartList = fl_bgn_form(FL_NO_BOX, 538, 394);
  obj = fl_add_box(FL_UP_BOX,0,0,538,394,"");
  obj = fl_add_box(FL_FRAME_BOX,6,201,505,173,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->ScenarioStartList = obj = fl_add_browser(FL_HOLD_BROWSER,12,28,370,165,"Scenario Starting Points       Path Start  (Map)");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,ScenarioStartList_cb,0);
  fdui->MapsList = obj = fl_add_browser(FL_HOLD_BROWSER,12,224,241,139,"Maps");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_South);
    fl_set_object_callback(obj,MapsList_cb,0);
  fdui->PathStartList = obj = fl_add_browser(FL_HOLD_BROWSER,263,224,241,139,"Path Start Points");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_North, FL_SouthEast);
    fl_set_object_callback(obj,PathStartList_cb,0);
  fdui->SPClose = obj = fl_add_button(FL_NORMAL_BUTTON,390,165,125,29,"Close");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,SPClose_cb,0);
  fdui->RemoveStart = obj = fl_add_button(FL_NORMAL_BUTTON,390,132,125,29,"Remove Path Start");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,RemoveStart_cb,0);
  fdui->AddStart = obj = fl_add_button(FL_NORMAL_BUTTON,390,95,125,29,"Add Path Start");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,AddStart_cb,0);
  fdui->Lower = obj = fl_add_button(FL_NORMAL_BUTTON,390,61,125,29,"@2->");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,Lower_cb,0);
  fdui->Raise = obj = fl_add_button(FL_NORMAL_BUTTON,390,27,125,29,"@8->");
    fl_set_object_color(obj,FL_COL1,FL_DARKCYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,Raise_cb,0);
  fl_end_form();

  fdui->EditStartList->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

