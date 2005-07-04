/***********************************************************
 *
 * File:			data.cc
 * Author:			Andrew Miga
 * Created:			November 1997
 *
 * Modification history:
 *
 ***********************************************************/

extern "C" {
#include "forms.h"
#include "ucmnavui.h"
}

#include "data.h"
#include "responsibility.h"
#include "print_mgr.h"

static DataStoreDirectory *SoleDataStoreDirectory; // class variable for sole instance of directory object

int DataStoreItem::item_count = 0;
int DataStoreItem::max_id = -1;

extern void EditDataStores();
extern FD_DataStores *pfddsd;
extern FD_DataStoreUsage *pfddsu;

static char type_names[][15] = {

   "Data Stores",
   "Access Modes" };

DataStoreItem::DataStoreItem( const char *dsi )
{
   item[19] = 0;
   strncpy( item, dsi, 19 );
   identifier = item_count++;
}

DataStoreItem::DataStoreItem( const char *dsi, int id )
{
   item[19] = 0;
   strncpy( item, dsi, 19 );
   identifier = id;

   if( id > max_id ) {  // avoid reallocating the same identifier
      max_id = id;
      item_count = max_id+1;
   }

}

DataStoreDirectory * DataStoreDirectory::Instance()
{
   if( SoleDataStoreDirectory == NULL )
      SoleDataStoreDirectory = new DataStoreDirectory();

   return( SoleDataStoreDirectory );
}

void DataStoreDirectory::List()
{
   fl_clear_browser( pfddsd->DSBrowser );

   for( data_stores[emode].First(); !data_stores[emode].IsDone(); data_stores[emode].Next() )
      fl_addto_browser( pfddsd->DSBrowser, data_stores[emode].CurrentItem()->Item() );
}

void DataStoreDirectory::ViewDataStores()
{
   EditDataStores();
}

void DataStoreDirectory::DeleteItem( int index )
{
   DataStoreItem *dsi = DSItem( index );
   data_stores[emode].Delete( dsi );
   delete dsi;
}

void DataStoreDirectory::ClearDirectory()
{
   for( int i = 0; i < 2; i++ ) {
      while( !data_stores[i].is_empty() )
	 delete data_stores[i].Detach();
   }
   DataStoreItem::ResetCount();
}

void DataStoreDirectory::SaveXML( FILE *fp )
{
   DataStoreItem *dsi;
   char buffer[300];
   
   if( (data_stores[DATA_STORES].Size() != 0) ||
       (data_stores[ACCESS_MODES].Size() != 0) ) {

      PrintNewXMLBlock( fp, "data-store-directory" );
      
      for( data_stores[DATA_STORES].First(); !data_stores[DATA_STORES].IsDone(); data_stores[DATA_STORES].Next() ) {
	 dsi = data_stores[DATA_STORES].CurrentItem();
	 sprintf( buffer, "data-store data-store-id=\"ds%d\" data-store-item=\"%s\" /", dsi->Identifier(), PrintDescription( dsi->Item() ) );
	 PrintXML( fp, buffer );
      }

      for( data_stores[ACCESS_MODES].First(); !data_stores[ACCESS_MODES].IsDone(); data_stores[ACCESS_MODES].Next() ) {
	 dsi = data_stores[ACCESS_MODES].CurrentItem();
	 sprintf( buffer, "access-mode access-mode-id=\"am%d\" access-mode-item=\"%s\" /", dsi->Identifier(), PrintDescription( dsi->Item() ) );
	 PrintXML( fp, buffer );
      }

      PrintEndXMLBlock( fp, "data-store-directory" );
      LinebreakXML( fp );

   }
}

void DataStoreDirectory::GeneratePostScriptDescription( FILE *ps_file )
{
   if( (data_stores[DATA_STORES].Size() == 0) && (data_stores[ACCESS_MODES].Size() == 0) )
      return;

   PrintManager::PrintHeading( "Data Store Specification" );
   fprintf( ps_file, "[/Title(Data Store Specification) /OUT pdfmark\n" );
   
   for( int i = 0; i < 2; i++ ) {
      fprintf( ps_file, "%d bol (%s ) P L\n1 IN ID %d rom\n", PrintManager::text_font_size, type_names[i], PrintManager::text_font_size );
      for( data_stores[i].First(); !data_stores[i].IsDone(); data_stores[i].Next() )
	 fprintf( ps_file, "(%s ) P\n", data_stores[i].CurrentItem()->Item() );
      fprintf( ps_file, "OD L\n" );
   }
}

void DataStoreDirectory::DisplayDataStores( Responsibility *responsibility )
{
   DataStoreItem *dsi;
   char buffer[40];
   int color = 11;
   
   fl_freeze_form( pfddsu->DataStoreUsage );
   
   fl_clear_browser( pfddsu->AvailableData );
   for( data_stores[DATA_STORES].First(); !data_stores[DATA_STORES].IsDone(); data_stores[DATA_STORES].Next() ) {
      
      dsi = data_stores[DATA_STORES].CurrentItem();
      if( responsibility->ReferencesData( dsi->Identifier() )) {
	 sprintf( buffer, "@N@C%d%s", color, dsi->Item() );
	 fl_addto_browser( pfddsu->AvailableData, buffer );
      }
      else	  
	 fl_addto_browser( pfddsu->AvailableData, dsi->Item() );
   }
   
   fl_unfreeze_form( pfddsu->DataStoreUsage );
}

void DataStoreDirectory::DisplayAccessModes()
{
   fl_freeze_form( pfddsu->DataStoreUsage );

   fl_clear_browser( pfddsu->AccessModes );
   for( data_stores[ACCESS_MODES].First(); !data_stores[ACCESS_MODES].IsDone(); data_stores[ACCESS_MODES].Next() ) 
      fl_addto_browser( pfddsu->AccessModes, data_stores[ACCESS_MODES].CurrentItem()->Item() );
   
   fl_unfreeze_form( pfddsu->DataStoreUsage );
}

char * DataStoreDirectory::Item( editing_mode mode, int identifier )
{
   for( data_stores[mode].First(); !data_stores[mode].IsDone(); data_stores[mode].Next() ) {
      if( data_stores[mode].CurrentItem()->Identifier() == identifier )
	 return( data_stores[mode].CurrentItem()->Item() );
   }
   return 0;
}

bool DataStoreDirectory::IsReferenceValid( DataReference *dr )
{
   bool data_found = FALSE, access_found = FALSE;
   int data_id = dr->Data();
   int access_id = dr->Access();

   for( data_stores[DATA_STORES].First(); !data_stores[DATA_STORES].IsDone(); data_stores[DATA_STORES].Next() ) {
      if( data_stores[DATA_STORES].CurrentItem()->Identifier() == data_id ) {
	 data_found = TRUE;
	 break;
      }
   }

   if( !data_found )
      return( FALSE );

   for( data_stores[ACCESS_MODES].First(); !data_stores[ACCESS_MODES].IsDone(); data_stores[ACCESS_MODES].Next() ) {
      if( data_stores[ACCESS_MODES].CurrentItem()->Identifier() == access_id ) {
	 access_found = TRUE;
	 break;
      }
   }

   if( !access_found )
      return( FALSE );
   else
      return( TRUE );

}
