/***********************************************************
 *
 * File:			ucm_set.h
 * Author:			Andrew Miga
 * Created:			October 1999
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef UCM_SET_H
#define UCM_SET_H

#include "defines.h"
#include "collection.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SetUCMSetIncluded SetGenerated
#define ResetUCMSetIncluded ResetGenerated
#define UCMSetIncluded IsGenerated
#define type_index (mtype == ROOT_MAP ? 0 : 1)

typedef enum { SET_GRAPHICS, SET_SAVE, SET_REPORT, NO_EXPORT } export_type;

class Map;

class UcmSetElement {
public:

   UcmSetElement( Map *m, const char *md = NULL );
   ~UcmSetElement() { if( comment ) free( comment ); }
   void Comment( const char *new_comment );
   const char * Comment() { return( comment != NULL ? comment : "" ); } 
   Map *SetMap() { return( set_map ); }
   
   Map *set_map;
   char *comment;
};

class UcmSet {

public:

   UcmSet( const char *name, Map *primary, const char *description );
   UcmSet();
   ~UcmSet();

   const char * SetName() { return( set_name ); }
   void SetName( const char *new_name );
   const char * SetDescription() { return( set_description != NULL ? set_description : "" ); }
   void SetDescription( const char *new_description );

   Map * PrimaryMap() { return( primary_map ); }
   void PrimaryMap( Map *map ) { primary_map = map; }
   
   void AddMap( Map *map, const char *desc = NULL );
   void AddNewSubmap( Map *map );
   void RemoveSetMap( Map *map );
   Cltn<UcmSetElement *> * MapList( map_type mtype ) { return( &set_maps[type_index] ); }
   Cltn<Map *> * ExportList();
   void ReorderInMapList( Map *map );
   bool IncludesMap( Map *map );
   bool IsContainedIn( Cltn<Map *> *map_list );
   void MarkIncludedMaps();
   void Save( FILE *fp );
   void GeneratePostScriptSet( FILE *ps_file );

   static void SaveUcmSets( FILE *fp, Cltn<Map *> *map_list );
   static void GeneratePostScript( FILE *ps_file );
   static void AddUcmSet( UcmSet *ucm_set );
   static void DeleteUcmSet( UcmSet *ucm_set );
   static void ReorderUcmSet( UcmSet *ucm_set );
   static bool UniqueSetName( const char *new_name, UcmSet *current_set );
   static bool UcmSetsDefined() { return( ucm_sets.Size() != 0 ); }
   static void CreateUCMSet();
   static void ViewUCMSetList();
   static void AddCurrentMap();
   static void SelectExportSet( export_type etype );
   static void ClearDirectory();
   
private:

   static Cltn<UcmSet *> ucm_sets;

   Map *primary_map;
   Cltn<UcmSetElement *> set_maps[2];
   char set_name[LABEL_SIZE+1], *set_description;
};

#endif
