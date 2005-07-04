/***********************************************************
 *
 * File:			ucm_set.cc
 * Author:			Andrew Miga
 * Created:			October 1999
 *
 * Modification history:
 *
 ***********************************************************/

#include "ucm_set.h"
#include "print_mgr.h"
#include "xml_generator.h"
#include "map.h"

Cltn<UcmSet *> UcmSet::ucm_sets;

extern void ViewUCMSets( UcmSet *set, bool ns = FALSE );
extern void AllowViewingUCMSets( bool allow );
extern void DisplayUcmSets( Cltn<UcmSet *> *sets, export_type etype = NO_EXPORT );
extern void AddCurrentMapToSets( Cltn<UcmSet *> *sets );

UcmSetElement::UcmSetElement( Map *m, const char *md )
{
 set_map = m; 
 comment = ( (md != NULL) ? strdup( md ) : NULL );
}

void UcmSetElement::Comment( const char *new_comment )
{
   if( comment )
      free( comment );
   if( PrintManager::TextNonempty( new_comment ) )
      comment = strdup( new_comment );
}

UcmSet::UcmSet( const char *name, Map *primary, const char *description )
{
   strncpy( set_name, name, LABEL_SIZE );
   primary_map = primary;
   set_description = (( description != NULL ) ? strdup( description ) : NULL);
}

UcmSet::UcmSet()
{
   set_name[0] = 0;
   set_name[LABEL_SIZE] = 0;
   set_description = NULL;
   primary_map = NULL;
}

UcmSet::~UcmSet()
{
   for( int i = 0; i <= 1; i++ ) {
      while( !set_maps[i].is_empty() )
	 delete set_maps[i].Detach();
   }
   
   if( set_description ) free( set_description );
}

void UcmSet::AddMap( Map *map, const char *desc )
{
   UcmSetElement *current_element, *new_element;
   int i = ((map->IsRootMap()) ? 0 : 1);
   
   map->RegisterUCMSet( this );
   new_element = new UcmSetElement( map, desc );

   for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
      current_element = set_maps[i].CurrentItem();
      if( strcmp( map->MapLabel(), current_element->SetMap()->MapLabel() ) < 0 ) {
	 set_maps[i].AddBefore( new_element, current_element );
	 return;
      }
   }      
   
   set_maps[i].Add( new_element ); // add set at end of list if greater than all existing names
}

void UcmSet::AddNewSubmap( Map *map )
{
   for( int i = 0; i <= 1; i++ ) { // determine if map is already included in set, if so return immediately
      for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
	 if( set_maps[i].CurrentItem()->SetMap() == map )
	    return;
      }
   }

   this->AddMap( map ); // add to set if it doesn't already exist
}

void UcmSet::RemoveSetMap( Map *map )
{
   UcmSetElement *matching_element;
   int i = ((map->IsRootMap()) ? 0 : 1);   
   if( map == primary_map ) primary_map = NULL;

   for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
      if( set_maps[i].CurrentItem()->set_map == map ) {
	 matching_element = set_maps[i].CurrentItem();
	 set_maps[i].RemoveCurrentItem();
	 delete matching_element;
	 return;
      }
   }
}

void UcmSet::ReorderInMapList( Map *map )
{
   UcmSetElement *matching_element, *current_element;
   int i = ((map->IsRootMap()) ? 0 : 1);   

   // find pointer to set element and remove from list
   for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
      if( set_maps[i].CurrentItem()->set_map == map ) {
	 matching_element = set_maps[i].CurrentItem();
	 set_maps[i].RemoveCurrentItem();
	 break;
      }
   }
   // re-add found set element in alphabetical order
   for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
      current_element = set_maps[i].CurrentItem();
      if( strcmp( map->MapLabel(), current_element->SetMap()->MapLabel() ) < 0 ) {
	 set_maps[i].AddBefore( matching_element, current_element );
	 return;
      }
   }      
   
   set_maps[i].Add( matching_element ); // add set at end of list if greater than all existing names
}

void UcmSet::SetName( const char *new_name )
{
   bool reorder_list = FALSE;

   if( !strequal( set_name, new_name ) )
      reorder_list = TRUE;
   
   strncpy( set_name, new_name, LABEL_SIZE );

   if( reorder_list )
      UcmSet::ReorderUcmSet( this );   
}

void UcmSet::SetDescription( const char *new_description )
{
   if( set_description )
      free( set_description );
   if( PrintManager::TextNonempty( new_description ) )
      set_description = strdup( new_description );
}

void UcmSet::Save( FILE *fp )
{
   char buffer[200];
   UcmSetElement *cs;
  
   sprintf( buffer, "<ucm-set name=\"%s\" ", set_name );
   PrintXMLText( fp, buffer );
   if( primary_map != NULL )
      fprintf( fp, "primary-map-id=\"m%d\" ", primary_map->GetNumber() );
   if( PrintManager::TextNonempty( set_description ) )
      fprintf( fp, "description=\"%s\" ", PrintDescription( set_description ) );
   fprintf( fp, ">\n" );
   IndentNewXMLBlock( fp );

   for( int i = 0; i <= 1; i++ ) {
      for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
	 cs = set_maps[i].CurrentItem();
	 sprintf( buffer, "<ucm-set-elem map-id=\"m%d\" ", cs->set_map->GetNumber() );
	 PrintXMLText( fp, buffer );
	 if( PrintManager::TextNonempty( cs->comment ) )
	    fprintf( fp, "comment=\"%s\" ", PrintDescription( cs->comment ) );
	 fprintf( fp, "/>\n" );
      }
   }

   PrintEndXMLBlock( fp, "ucm-set" );
}

Cltn<Map *> * UcmSet::ExportList()
{
   Cltn<Map *> *map_list = new Cltn<Map *>;

   for( int i = 0; i <= 1; i++ ) {
      for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() )
	 map_list->Add( set_maps[i].CurrentItem()->SetMap() );
   }

   return( map_list );
}


void UcmSet::GeneratePostScriptSet( FILE *ps_file )
{
   UcmSetElement *cse;

   fprintf( ps_file, "%d bol (%s ) P\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( set_name ) ); 
   fprintf( ps_file, "[/Title(%s) /OUT pdfmark\n", PrintManager::PrintPostScriptText( set_name, FALSE ) );

   if( PrintManager::TextNonempty( set_description ) ) {
      fprintf( ps_file, "%d rom 1 IN ID (Description ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "2 IN ID (%s ) P OD\n", PrintManager::PrintPostScriptText( set_description ));
   }

   if( primary_map != NULL ) {
      fprintf( ps_file, "%d rom 1 IN ID (Primary Map ) P OD\n", PrintManager::text_font_size );
      fprintf( ps_file, "%d bol 2 IN ID (%s ) P OD\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( primary_map->MapLabel() ));
   }

   for( int i = 0; i <= 1; i++ ) {
      fprintf( ps_file, "%d rom 1 IN ID (%s ) P OD\n", PrintManager::text_font_size, ((i == 0) ? "Root Maps" : "Plugin Maps") );
      for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
	 cse = set_maps[i].CurrentItem();
	 fprintf( ps_file, "2 IN ID %d bol (%s ) S\n", PrintManager::text_font_size, PrintManager::PrintPostScriptText( cse->SetMap()->MapLabel() ) );
	 fprintf( ps_file, "[/Dest /%s%s /Rect [106 tm 3 sub cpp tm 10 add]\n", ((i == 0) ? "Root" : "Plugin"),
		  PrintManager::PrintPostScriptText( cse->SetMap()->MapLabel(), FALSE ) );
	 fprintf( ps_file, "/Border [0 0 0] /Color [1 0 0] /InvisibleRect /I /Subtype /Link /ANN pdfmark\nL OD\n" );
	 if( PrintManager::TextNonempty( cse->comment ) ) {
	    fprintf( ps_file, "%d rom 3 IN ID (Comment ) P OD\n", PrintManager::text_font_size );
	    fprintf( ps_file, "4 IN ID (%s ) P OD\n", PrintManager::PrintPostScriptText( cse->comment ));
	 }
      }
   }
}

bool UcmSet::IsContainedIn( Cltn<Map *> *map_list )
{
   for( int i = 0; i <= 1; i++ ) {
      for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
	 if( !map_list->Includes( set_maps[i].CurrentItem()->set_map )) 
	    return FALSE;
      }
   }
   
   return TRUE;
}

bool UcmSet::IncludesMap( Map *map )
{
   int i = ((map->IsRootMap()) ? 0 : 1);

   for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() ) {
      if( set_maps[i].CurrentItem()->set_map == map )
	 return TRUE;
   }

   return FALSE;
}

void UcmSet::MarkIncludedMaps()
{
   for( int i = 0; i <= 1; i++ ) {
      for( set_maps[i].First(); !set_maps[i].IsDone(); set_maps[i].Next() )
	 set_maps[i].CurrentItem()->set_map->SetUCMSetIncluded();
   }
}

// static class functions

void UcmSet::SaveUcmSets( FILE *fp, Cltn<Map *> *map_list )
{
   bool valid_sets_exist = FALSE;
   UcmSet *ucm_set;
   
   if( ucm_sets.Size() == 0 )
      return;

   if( map_list != NULL ) { // determine if any ucm sets are contained in map_list
      for( ucm_sets.First(); !ucm_sets.IsDone(); ucm_sets.Next() ) {
	 if( ucm_sets.CurrentItem()->IsContainedIn( map_list ) ) {
	    valid_sets_exist = TRUE;
	    break;
	 }
      }
      if( !valid_sets_exist ) return;
   }

   PrintNewXMLBlock( fp, "ucm-sets" );

   for( ucm_sets.First(); !ucm_sets.IsDone(); ucm_sets.Next() ) {
      ucm_set = ucm_sets.CurrentItem();
      if( map_list != NULL ) {
	 if( !ucm_set->IsContainedIn( map_list ) ) // validate whether the map list contains
	    continue;                              // all of the ucm set maps
      }
      ucm_set->Save( fp );
   }

   PrintEndXMLBlock( fp, "ucm-sets" );
   LinebreakXML( fp );
}

void UcmSet::GeneratePostScript( FILE *ps_file )
{
   int number_sets;
   
   if( (number_sets = ucm_sets.Size()) == 0 ) 
      return;

   PrintManager::PrintHeading( "UCM Sets" );
   fprintf( ps_file, "[/Count %d /Title(UCM Sets) /OUT pdfmark\n", -1*number_sets );
   
   for( ucm_sets.First(); !ucm_sets.IsDone(); ucm_sets.Next() )
      ucm_sets.CurrentItem()->GeneratePostScriptSet( ps_file );
}

void UcmSet::AddUcmSet( UcmSet *ucm_set )
{
   UcmSet *current_set;
   
   if( ucm_sets.Size() == 0 ) 
      AllowViewingUCMSets( TRUE ); // enable menu item for editing ucm sets

   // add set in proper alphabetical order

   for( ucm_sets.First(); !ucm_sets.IsDone(); ucm_sets.Next() ) {
      current_set = ucm_sets.CurrentItem();
      if( strcmp( ucm_set->SetName(), current_set->SetName() ) < 0 ) {
	 ucm_sets.AddBefore( ucm_set, current_set );
	 return;
      }
   }
   
   ucm_sets.Add( ucm_set ); // add set at end of list if greater than all existing names
}

void UcmSet::DeleteUcmSet( UcmSet *ucm_set )
{
   ucm_sets.Delete( ucm_set );
   if( ucm_sets.Size() == 0 )
      AllowViewingUCMSets( FALSE ); // disable menu item for editing ucm sets
}

void UcmSet::ClearDirectory()
{
   while( !ucm_sets.is_empty() )
      delete ucm_sets.Detach();
   AllowViewingUCMSets( FALSE ); // disable menu item for editing ucm sets
}

void UcmSet::CreateUCMSet()
{
   UcmSet *new_set = new UcmSet;
   ViewUCMSets( new_set, TRUE );
}

void UcmSet::ReorderUcmSet( UcmSet *ucm_set )
{
   if( ucm_sets.Includes( ucm_set ) ) {
      ucm_sets.Remove( ucm_set );      
      UcmSet::AddUcmSet( ucm_set );
   }
}

bool UcmSet::UniqueSetName( const char *new_name, UcmSet *current_set )
{
   UcmSet *matching_set = NULL;

   if( !PrintManager::TextNonempty( new_name ) )
      return FALSE;
   
   for( ucm_sets.First(); !ucm_sets.IsDone(); ucm_sets.Next() ) {
      if( strequal( new_name, ucm_sets.CurrentItem()->SetName() )) {
	 matching_set = ucm_sets.CurrentItem();
	 break;
      }
   }

   if( matching_set != NULL ) {
      if( matching_set != current_set )
	 return FALSE;
   }

   return TRUE;
}

void UcmSet::ViewUCMSetList()
{
   DisplayUcmSets( &ucm_sets );
}

void UcmSet::AddCurrentMap()
{
   AddCurrentMapToSets( &ucm_sets );
}

void UcmSet::SelectExportSet( export_type etype )
{
   DisplayUcmSets( &ucm_sets, etype );  
}
