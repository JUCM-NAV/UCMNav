/***********************************************************
 *
 * File:			listitem2.h
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

#ifndef LISTITEM_H
#define LISTITEM_H

template<class T> class List;
template<class T> class ListIterator;

template<class T>
class ListItem {
 private:
  T tData;
  ListItem<T> *pliNext, *pliPrev;

  ListItem( const T &tNew ):tData( tNew ) { pliNext = pliPrev = 0; };
  
  friend class List<T>;
  friend class ListIterator<T>;
};

#endif
