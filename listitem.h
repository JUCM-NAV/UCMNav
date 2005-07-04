/***********************************************************
 *
 * File:			listitem.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:	November 1998
 *
 ***********************************************************/

#ifndef LL_ITEM_H
#define LL_ITEM_H

template<class T> class Cltn;

#include "definitions.h"

template<class T>
class LLItem {

 private:
  T tData;
  LLItem<T>* pliPrev;
  LLItem<T>* pliNext;

  LLItem( const T &tNew ):tData( tNew ), pliPrev( 0 ), pliNext( 0 ) {}
  ~LLItem() {}
  
  friend class Cltn<T>;
};

#endif
