/***********************************************************
 *
 * File:		      collection.h
 * Authors:		      Andrew Miga
 * Created:		      March 1998
 *
 * Modification history:      Templates consolidated 11/3/98
 *
 ***********************************************************/

#ifndef COLLECTION_H
#define COLLECTION_H

#include <iostream>
#include "defines.h"
#include "listitem.h"

#define RemoveLastElement  Detach   // allows aliasing of detach right function name
#define PutStart attachl

template<class T>
class Cltn {

protected:

   LLItem<T> *pliLeft, *pliRight, *pliCurrent, *pliAdvance;
   int iLength;
  
public:

   Cltn():pliLeft(0), pliRight(0), iLength(0), pliCurrent(0), pliAdvance(0) {}
   ~Cltn();
   int length() { return( iLength ); }
   int Size() { return( iLength ); }
   int is_empty() { return( pliLeft == 0 ); } 
   void detachl();
   T RemoveFirstElement();   
   void detachr();
   T Detach();   
   T getl() { return( T(pliLeft->tData) );  }
   T getr() { return( T(pliRight->tData) ); } 
   T detachnth( int iN );
   void detach( LLItem<T> *pli );
   T Get( int iN );
   T getnth( int iN ) { return( Get( iN+1 ) ); } // old interface
   T operator[]( int iN );
   void attachr( T tNew );
   void attachb( T tNew );
   void insertnth( int iN, T tNew );
   void insert( LLItem<T> *pli, T tNew );
   Cltn<T>& attachl( T tNew );
   Cltn<T>& operator=( const Cltn<T>& lRhs );
   int in( T tItem );
   LLItem<T> *li_in( T tItem );
   Cltn<T>& join( const Cltn<T>& lRhs );

   void First() { pliCurrent = pliLeft; }
   T GetFirst();
   T GetLast();
   T GetPrevious( T tref );
   void Next();
   int IsDone() { return( pliCurrent == 0 ); }
   T CurrentItem() const { return( pliCurrent->tData ); }
   void RemoveCurrentItem();
   T NextItem() const { return( pliCurrent->pliNext->tData ); }
   T AdvanceItem();
   
   void Add( T element ) { attachr( element ); }  
   void AddB(T element ) { attachb (element); }
   bool Includes( T element );
   void AddUnique( T element ) { if( !Includes( element ) ) attachr( element ); }
   void AddBefore( T element, T reference );
   void AddAfter( T element, T reference );
   void Remove( T element );
   void Delete( T element );
   void DeleteIndex( int index );
   void Replace( T old_element, T new_element );
   void ReplaceEnd( T new_element );
   void Rotate(); // moves the last element to the front
   void Empty(); // empties a list
   void SetCurrentItem( T element ) { pliCurrent->tData = element; }

   void SplitCltn( T split_point, Cltn<T> *first, Cltn<T> *second );
   void JoinCltn( Cltn<T> *first, Cltn<T> *second );

};

#endif 
