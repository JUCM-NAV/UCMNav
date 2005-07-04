/***********************************************************
 *
 * File:			label.h
 * Author:			Andrew Miga
 * Created:			April 1997
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef LABEL_H
#define LABEL_H

#include "defines.h"
#include <stdio.h>
#include <stdlib.h>

class Map;

class Label {

public:
   
   Label( const char *new_label, Map *parent ); // constructor, includes initial label
   Label( int label_id, const char *new_label, Map *parent ); // constructor for file loading
   ~Label();

   const char * TextLabel() const { return( text_label ); } // access methods for path label
   void TextLabel( const char *new_label );

   void IncrementCount() { reference_count++; } // increments reference count
   void DecrementCount(); // decrements reference count, deletes object if zero
   void SaveXML( FILE *fp );  // implements Save method, in XML format
   int GetNumber()  { return( label_number ); } // returns integer identifier
   int LoadIdentifier() { return( load_number ); }
   
   static void ResetLabelCount() { number_labels = 0; } // resets global count of labels
   
private:

   char text_label[DISPLAYED_LABEL+1];  // user given path label
   int label_number, load_number, // current and past integer identifiers for label object
      reference_count; // refernce count of object
   Map *parent_map;

   static int number_labels; // global count of number of labels

};


#endif



