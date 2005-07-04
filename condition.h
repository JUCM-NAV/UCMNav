/***********************************************************
 *
 * File:			condition.h
 * Author:			Andrew Miga
 * Created:			July 1997
 *
 * Modification history:
 *
 ***********************************************************/


#ifndef CONDITION_H
#define CONDITION_H

#include "collection.h"
#include <stdio.h>
#include <fstream>

typedef enum { PATH_START, STUB_PATH, PATH_END, EMPTY_POINT, TIMER_POINT, WAIT_POINT, RESP } condition_type;

class ConditionManager {

public:

   ConditionManager();
   ~ConditionManager();
   
   const char * ConditionName( int condition_list );
   Cltn<char *> * ConditionList( int condition_list );
   void ConditionType( condition_type type ) { ctype = type; }
   condition_type ConditionType() { return( ctype ); }
   void AddConditions( ConditionManager &cm );
   void AddCondition( etype type, char *condition );
   void SaveXML( FILE *fp, const char *pretype, const char *posttype );
   void GeneratePostScriptDescription( FILE *ps_file );
   bool ConditionsEmpty();

   static const char * Prompt( condition_type ctype, int condition_list );
   static void SaveConditionList( FILE *fp, Cltn<char *> *list, char *condition_name );
   static void GeneratePostscriptConditionList( FILE *ps_file, Cltn<char *> *list );
  
private:
   
   Cltn<char *> conditions[2];
   condition_type ctype;

   static char ConditionNames[][37];
   static char ConditionTypes[][18];
   static char Prompts[][38];

};


#endif
