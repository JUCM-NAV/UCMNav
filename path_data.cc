/***********************************************************
 * File:			path_data.cc
 * Author:			XiangYang He
 *                              July 2002    
 *
 * Modification history:
 *
 ***********************************************************/

#include "path_data.h"
 extern int  parID;  // global variable for par ID generation in DXL exporting to DOORS , added by Bo Jiang August 2004
 extern int  seqID;  // global variable for seq ID generation in DXL exporting to DOORS , added by Bo Jiang August 2004
Path_data::Path_data() 
{ 
   destination = NULL;
   root = NULL;
   scanCount = 0;
}

Path_data::~Path_data() { }
   
Cltn<Path_data *> * Path_data::NextPath() 
{ 
   return  &next_path; 
}

Cltn<Path_data *>* Path_data::PreviousPath()
{
   return &previous;
}

// This method adds the parameter element to the list of next_path, at the same time,
// it also adds the address of current element to previous list of parameter element
void Path_data::AddNext( Path_data * next ) 
{
   next_path.Add(next);
   next->AddPrevious(this);
}

void Path_data::RemoveFromNext(Path_data * p_data) 
{
   if(next_path.Includes(p_data)) {
      next_path.Remove(p_data);
      p_data->RemoveFromPrevious(this);
   }
}

void Path_data::RemoveFromPrevious(Path_data * p_data) 
{
    for( previous.First(); !previous.IsDone(); previous.Next() ) {
       if(previous.CurrentItem() == p_data ) {
           previous.RemoveCurrentItem();
	   break;
       }
    }   
}

// This method returns the first elements of next path
Path_data *  Path_data::Next()
{
   if( !next_path.is_empty() ) {
       return next_path.GetFirst();
   }
   return NULL;
}

// This method returns the first elements of previous path
Path_data *  Path_data::Previous()
{
   if(previous.Size() >= 1 ) {
       return previous.GetFirst();
   }
   return NULL;
}

// This method prints scenario data of a parallel block. First it gets the destination of the
// block, then for each branch, it iteratively calls method SaveScenarioXML to print the data 
// of each item until it reaches destination or to NULL pointer.
void  Path_data::SaveParallelXML(FILE * outfile, Path_data * root)
{
   Path_data * current_data = NULL;
   Path_data * des = root->NextPath()->GetFirst()->GetDestination();
   for( root->NextPath()->First(); !root->NextPath()->IsDone(); root->NextPath()->Next() ) {
       current_data = root->NextPath()->CurrentItem();
       PrintNewXMLBlock( outfile, "seq");
       while( current_data != des && current_data != NULL ) {
          current_data = current_data->SaveScenarioXML(outfile);
       }
       PrintEndXMLBlock( outfile, "seq");
   }
}

// This method prints scenario data of a parallel block. First it gets the destination of the
// block, then for each branch, it iteratively calls method SaveScenarioXML to print the data 
// of each item until it reaches destination or to NULL pointer.
void  Path_data::SaveParallelDXL(FILE * outfile, Path_data * root , char * parentID)
{
   char buffer[200];
   Path_data * current_data = NULL;
   Path_data * des = root->NextPath()->GetFirst()->GetDestination();
   for( root->NextPath()->First(); !root->NextPath()->IsDone(); root->NextPath()->Next() ) {
       current_data = root->NextPath()->CurrentItem();
       sprintf(buffer,"seq( \"seq%d\", %s )\n",seqID,parentID);
       PrintXMLText(outfile,buffer);
       sprintf( buffer, "\"seq%d\"",seqID );
       seqID++;
       IndentNewXMLBlock(outfile);
       while( current_data != des && current_data != NULL ) {
             current_data = current_data->SaveScenarioDXL( outfile, buffer );       
       }
       IndentEndXMLBlock(outfile);
   }
}
