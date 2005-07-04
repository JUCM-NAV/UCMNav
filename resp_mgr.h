/***********************************************************
 *
 * File:			resp_mgr.h
 * Author:			Andrew Miga
 * Created:			April 1999
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef RESPONSIBILITY_MANAGER_H
#define RESPONSIBILITY_MANAGER_H

#include "responsibility.h"

class ResponsibilityManager;

class ResponsibilityManager {

public:

   static ResponsibilityManager * Instance(); // returns pointer to sole instance, creates instance on first call
   void AddResponsibility( Responsibility *new_resp ); // add new responsibility to list in alphabetical order
   void RemoveResponsibility( Responsibility *resp ) { responsibilities.Delete( resp ); }
   Cltn<Responsibility *> * ResponsibilityList() { return( &responsibilities ); }
   Responsibility * FindResponsibility( int parent_id );
   bool UniqueResponsibilityName( const char *new_name, const char *old_name, char *name );
   const char * VerifyUniqueResponsibilityName( const char *name );
   void SaveResponsibilityList( FILE *fp, Cltn<Map *> *map_list );
   void SaveCSMResponsibilityList(FILE * fp);
   void ResetGenerate();
   void DeactivateHighlighting();
   bool DuplicateName(const char *name, int &index );
   void CreateMapping( int replaced_index, int existing_index ) { import_mappings.Add( Mapping( replaced_index, existing_index ) ); }
   void DeleteMappings();
   int MaximumLoadNumber();
   void ClearResponsibilityList();
   void CleanList();
   
   void OutputDXL( std::ofstream &dxl_file ); // for DXL export
   
private:
   
  ResponsibilityManager() {} // constructor, private to enforce singleton pattern

   static ResponsibilityManager * SoleResponsibilityManager; // class variable for sole instance of directory object
   Cltn<Responsibility *> responsibilities;   
   Cltn<Mapping> import_mappings;

};


#endif
