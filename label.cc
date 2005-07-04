
/***********************************************************
 *
 * File:			label.cc
 * Author:			Andrew Miga
 * Created:			April 1997
 *
 * Modification history:
 *
 ***********************************************************/


#include "label.h"
#include "map.h"
#include "xml_generator.h"
#include <string.h>

int Label::number_labels = 0;

Label::Label( const char *new_label, Map *parent )
{
   text_label[DISPLAYED_LABEL] = 0;
   strncpy( text_label, new_label, DISPLAYED_LABEL );
   parent_map = parent;
   parent_map->RegisterLabel( this );
   label_number = number_labels++;
   reference_count = 0;
}

Label::Label( int label_id, const char *new_label, Map *parent )
{
   load_number = label_id;
   text_label[DISPLAYED_LABEL] = 0;
   strncpy( text_label, new_label, DISPLAYED_LABEL );
   parent_map = parent;
   parent_map->RegisterLabel( this );
   label_number = number_labels++;
   reference_count = 0;
}

Label::~Label()
{
   parent_map->PurgeLabel( this );
}

void Label::TextLabel( const char *new_label )
{
   if( new_label )
      strncpy( text_label, new_label, DISPLAYED_LABEL );
}

void Label::DecrementCount()
{
   if( --reference_count == 0 )
      delete this;
}

void Label::SaveXML( FILE *fp )
{
   char buffer[400];

   sprintf( buffer, "path-label path-label-id=\"pl%d\" label=\"%s\" /", label_number, PrintDescription( text_label ) );
   PrintXML( fp, buffer );   
}
