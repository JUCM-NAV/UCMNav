/***********************************************************
 *
 * File:			data.h
 * Author:			Andrew Miga
 * Created:			November 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef DATA_H
#define DATA_H

#include "collection.h"
#include <stdio.h>
#include <string.h>

typedef enum { DATA_STORES, ACCESS_MODES } editing_mode;

class Responsibility;
class DataReference;

class DataStoreItem {  // data class for each store or access mode, used by DataStoreDirectory

public:

   DataStoreItem( const char *dsi ); // constructors, accept user inputted string as parameters
   DataStoreItem( const char *dsi, int id );
   ~DataStoreItem() {}

   int Identifier() { return( identifier ); } // returns integer identifier
   char *Item() { return( item ); } // access modes for string representing item
   void Item( const char *new_item ) { strncpy( item, new_item, 19 ); }

   static void ResetCount() { item_count = 0; max_id = -1; } // resets counters for elements
   static int GetItemCount() { return ( item_count ); }    // Apr2005 gM: required for map import to avoid conflict with constant IDs
   
private:
   
   char item[20]; // string entered by user representing data store or access mode
   int identifier; // internal integer identifier used to match elements with references

   static int item_count, max_id; // counter for items, current maximum reference
};

class DataStoreDirectory {

public:

   ~DataStoreDirectory() {}
   static DataStoreDirectory * Instance(); // returns pointer to sole instance, creates instance on first call
   
   void List(); // list stores or modes in list box
   void DisplayDataStores( Responsibility *responsibility ); // displays available stores in responsibility data usage list box
   void DisplayAccessModes(); // displays available access modes in responsibility data usage list box
   void ViewDataStores(); // invokes data store dialog box
   void SetMode( editing_mode mode ) { emode = mode; } // set method for editing mode
   char * Item( int index ) { return( data_stores[emode].Get(index)->Item() ); } // access methods for elements
   char * Item( editing_mode mode, int identifier );
   DataStoreItem *DSItem( int index ) { return( data_stores[emode].Get(index) ); }
   DataStoreItem *DSItem( editing_mode mode, int index ) { return( data_stores[mode].Get(index) ); }
   void AddItem( const char *item ) { data_stores[emode].Add( new DataStoreItem( item ) ); }
   void AddItem( const char *item, int identifier ) { data_stores[emode].Add( new DataStoreItem( item, identifier ) ); }
   void AddItem( editing_mode mode, const char *item, int identifier )
   { data_stores[mode].Add( new DataStoreItem( item, identifier ) ); }
   void EditItem( const char *item, int index ) { this->DSItem( index )->Item( item ); }
   void DeleteItem( int index ); // deletes the item at index
   void ClearDirectory(); // removes all elements from directory
   bool IsReferenceValid( DataReference *dr ); // determines if an element still exists
   void SaveXML( FILE *fp ); // saves the directory contents in XML format
   void GeneratePostScriptDescription( FILE *ps_file );

protected:

   Cltn<DataStoreItem *> data_stores[2]; // lists of data stores and access modes
   
private:

   DataStoreDirectory() { emode = DATA_STORES; } // constructor, sets initial editing mode
   editing_mode emode; // the elements being currently edited ( data stores / access modes )
   
};

#endif
