
// callbacks for or fork specification dialog

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "interface.h"
#include "or_fork.h"
#include "empty.h"
#include "print_mgr.h"

extern bool condition_modified;

extern void SetPromptUserForSave();
extern void DrawScreen();
extern const char * EditLogicalCondition( const char *condition );

FD_BranchSpecification *pfdfs = NULL;

static int ForkDialogClose( FL_FORM *, void * );
static void DisplayBranch( int new_branch, int old_branch );

class ForkData {
public:

   ForkData() { temp_selection = NULL; temp_condition = NULL; temp_probability = 0; selection_changed = FALSE; condition_changed = FALSE; }

   char *temp_selection, *temp_condition;
   float temp_probability;
   int old_number;
   bool selection_changed, condition_changed;
};

static ForkData *branch_data;
static OrFork *or_fork;
static int current_branch;

void EditOrForkSpecification( OrFork *fork )
{
   char title[150], bname[10];
   Cltn<Node *> *branches;
   Empty *empty;
   ExitPoint *exit_point;
   int i = 0;

   FREEZE_TOOL(); // temporarily deactivate the main form
   or_fork = fork;
   current_branch = 1;

   if( pfdfs == NULL ) {
      pfdfs = create_form_BranchSpecification();
      fl_set_form_atclose( pfdfs->BranchSpecification, ForkDialogClose, NULL );
      fl_set_choice_fontsize( pfdfs->BranchChoice, FL_NORMAL_SIZE );
      fl_set_choice_fontstyle( pfdfs->BranchChoice, FL_BOLD_STYLE );
      fl_set_browser_fontsize( pfdfs->LogicalCondition, FL_NORMAL_SIZE );
      fl_set_browser_fontstyle( pfdfs->LogicalCondition, FL_BOLD_STYLE );
      fl_set_input_return( pfdfs->BranchSelection, FL_RETURN_CHANGED );
   }

   fl_set_input( pfdfs->ForkName, fork->HyperedgeName() );
   or_fork->PositionOrderOutputs();
   fl_clear_choice( pfdfs->BranchChoice );
   branches = or_fork->TargetSet();
   branch_data = new ForkData[branches->Size()];

   for( branches->First(); !branches->IsDone(); branches->Next() ) {
      sprintf( bname, "BR %d", i+1 );
      fl_addto_choice( pfdfs->BranchChoice, bname );
      empty = (Empty *)branches->CurrentItem()->NextEdge();
      branch_data[i].old_number = empty->PathNumber();
      empty->PathNumber(FORK_OFFSET+i+1);
      exit_point = or_fork->BranchSpecification( branches->CurrentItem() );
      branch_data[i].temp_probability = exit_point->Probability();
      if( exit_point->BranchChoice() != NULL )
	 branch_data[i].temp_selection = strdup( exit_point->BranchChoice() );
      branch_data[i].temp_condition = exit_point->LogicalCondition();
      i++;
   }

   DrawScreen();
   DisplayBranch( 1, 0 );
   sprintf( title, "Specification of Or Fork %s", fork->HyperedgeName() );
   fl_show_form( pfdfs->BranchSpecification, FL_PLACE_CENTER | FL_FREE_SIZE,
		 FL_TRANSIENT, title );
}

extern "C"
void BranchSelection_cb(FL_OBJECT *, long)
{
   branch_data[current_branch-1].selection_changed = TRUE;
}

extern "C"
void BranchChoice_cb(FL_OBJECT *choice, long)
{
   int old_branch = current_branch;

   current_branch = fl_get_choice( choice );
   if( current_branch != old_branch )
      DisplayBranch( current_branch, old_branch );
}

void DisplayBranch( int new_branch, int old_branch )
{
   char prob[20];

   if( old_branch != 0 ) {
      branch_data[old_branch-1].temp_probability = atof( fl_get_input( pfdfs->Probability ) );
      if( branch_data[old_branch-1].temp_selection != NULL )
	 free( branch_data[old_branch-1].temp_selection );
      branch_data[old_branch-1].temp_selection = strdup( fl_get_input( pfdfs->BranchSelection ) );
   }

   sprintf( prob, "%f", branch_data[new_branch-1].temp_probability );
   fl_set_input( pfdfs->Probability, prob );
   fl_set_input( pfdfs->BranchSelection, ((branch_data[new_branch-1].temp_selection != NULL) ? branch_data[new_branch-1].temp_selection : "") );
   fl_clear_browser( pfdfs->LogicalCondition );
   if( branch_data[new_branch-1].temp_condition != NULL )
      fl_add_browser_line( pfdfs->LogicalCondition, branch_data[new_branch-1].temp_condition );
}

extern "C"
void FSAcceptButton_cb(FL_OBJECT *, long)
{
   Cltn<Node *> *branches;
   ExitPoint *exit_point;
   int i = 0;

   // update data for current branch
   branch_data[current_branch-1].temp_probability = atof( fl_get_input( pfdfs->Probability ) );
   if( branch_data[current_branch-1].temp_selection != NULL )
      free( branch_data[current_branch-1].temp_selection );
   branch_data[current_branch-1].temp_selection = strdup( fl_get_input( pfdfs->BranchSelection ) );

   or_fork->ForkName( fl_get_input( pfdfs->ForkName ) );
   branches = or_fork->TargetSet();

   for( branches->First(); !branches->IsDone(); branches->Next() ) {
      exit_point = or_fork->BranchSpecification( branches->CurrentItem() );
      exit_point->Probability( branch_data[i].temp_probability );
      if( branch_data[i].selection_changed ) {
	 exit_point->BranchChoice( branch_data[i].temp_selection );
	 branch_data[i].temp_selection = NULL; // set to null so cancel function will not redelete pointer
      }
      if( branch_data[i].condition_changed ) {
	 exit_point->LogicalCondition( branch_data[i].temp_condition );
	 free( branch_data[i].temp_condition );
	 branch_data[i].temp_condition = NULL; // set to null so cancel function will not redelete pointer
      }
      i++;
   }

   SetPromptUserForSave();
   FSCancelButton_cb( 0, 0 );
}

extern "C"
void FSCancelButton_cb(FL_OBJECT *, long)
{
   Empty *empty;
   Cltn<Node *> *branches;
   int i = 0;

   UNFREEZE_TOOL();

   branches = or_fork->TargetSet();

   for( branches->First(); !branches->IsDone(); branches->Next() ) {
      empty = (Empty *)branches->CurrentItem()->NextEdge();
      empty->PathNumber( branch_data[i].old_number );
      if( branch_data[i].temp_selection != NULL )
	 free( branch_data[i].temp_selection );
      if( branch_data[i].temp_condition != NULL )
	 free( branch_data[i].temp_condition );
      i++;
   }

   delete [] branch_data;

   DrawScreen();
   fl_hide_form( pfdfs->BranchSpecification );
}

extern "C"
void FSEditConditionButton_cb(FL_OBJECT *, long)
{
   const char *edited_condition;

   if( (edited_condition = EditLogicalCondition( branch_data[current_branch-1].temp_condition )) != NULL ) {
      if( branch_data[current_branch-1].temp_condition != NULL )
	 free( branch_data[current_branch-1].temp_condition );
      branch_data[current_branch-1].temp_condition = strdup( edited_condition );
      branch_data[current_branch-1].condition_changed = TRUE;
      fl_clear_browser( pfdfs->LogicalCondition );
      fl_add_browser_line( pfdfs->LogicalCondition, edited_condition );
      SetPromptUserForSave();
   }
}

int ForkDialogClose( FL_FORM *, void * )
{
   FSCancelButton_cb( 0, 0 );
   return( FL_IGNORE );
}
