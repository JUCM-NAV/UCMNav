/***********************************************************
 *
 * File:		      collection.cc
 * Authors:		      Andrew Miga
 * Created:		      March 1998
 *
 * Modification history:      Templates consolidated 11/3/98
 *
 ***********************************************************/

#include <iostream>
#include "collection.h"

/*
   *		Class:		Cltn 
   *		Function:	destructor
   */

template<class T>
Cltn<T>::~Cltn() {
   while( !is_empty() )
      detachr();

   pliLeft = NULL;
   pliRight = NULL;
   pliCurrent = NULL;
   pliAdvance= NULL;
   iLength = -9999;
} 

template<class T> void
Cltn<T>::Empty() {
   while( !is_empty() )
      detachr();
} 

/*
   *		Class:		Cltn 
   *		Function:	detachl
   */

template<class T> void
Cltn<T>::detachl() {
   LLItem<T> *pliTemp = pliLeft;

   pliLeft = pliLeft->pliNext;

   if( !pliLeft )
      pliRight = pliLeft;
   else
      pliLeft->pliPrev = 0;

   delete pliTemp;
   iLength--;
} 


/*
   *		Class:		Cltn 
   *		Function:	RemoveFirstElement
   *                            detaches left element of list and returns contents
   */

template<class T> T
Cltn<T>::RemoveFirstElement() {
   LLItem<T> *pliTemp = pliLeft;
   T tTemp(pliTemp->tData);

   pliLeft = pliLeft->pliNext;

   if( !pliLeft )
      pliRight = pliLeft;
   else
      pliLeft->pliPrev = 0;

   delete pliTemp;
   iLength--;

   return( tTemp );
} 

   
/*
   *		Class:		Cltn 
   *		Function:	detachr
   */

template<class T> void
Cltn<T>::detachr() {
   LLItem<T> *pliTemp = pliRight;

   pliRight = pliRight->pliPrev;
    
   if( !pliRight )
      pliLeft = pliRight;
   else
      pliRight->pliNext = 0;

   delete pliTemp;
   iLength--;
}

/*
   *		Class:		Cltn 
   *		Function:	Detach
   *                            detaches right element of list and returns contents
   */
   
template<class T> T
Cltn<T>::Detach() {
   LLItem<T> *pliTemp = pliRight;
   T tTemp(pliTemp->tData);

   pliRight = pliRight->pliPrev;
    
   if( !pliRight )
      pliLeft = pliRight;
   else
      pliRight->pliNext = 0;

   delete pliTemp;

   iLength--;

   return( tTemp );
}  


/*
   *		Class:		Cltn 
   *		Function:	detachnth
   */

template<class T> T
Cltn<T>::detachnth( int iN )
{
   LLItem<T> *pliWalk = pliLeft;    
   int i=0;

   while( pliWalk ) {
      if( i == iN ) {
	 if( pliWalk->pliPrev )
	    pliWalk->pliPrev->pliNext = pliWalk->pliNext;

	 if( pliWalk->pliNext )
	    pliWalk->pliNext->pliPrev = pliWalk->pliPrev;

	 if( pliWalk == pliLeft )
	    pliLeft = pliWalk->pliNext;

	 if( pliWalk == pliRight )
	    pliRight = pliWalk->pliPrev;

	 T tTemp(pliWalk->tData);

	 delete pliWalk;

	 iLength--;

	 return( tTemp );
      }
      pliWalk = pliWalk->pliNext;
      i++;
   }

   return( T(pliWalk->tData) );
}

/*
   *		Class:		Cltn 
   *		Function:	detach
   *		Created June 17 1996
   */

template<class T> void 
Cltn<T>::detach( LLItem<T> *pli )
{      
   if( pli->pliPrev )
      pli->pliPrev->pliNext = pli->pliNext;

   if( pli->pliNext )
      pli->pliNext->pliPrev = pli->pliPrev;

   if( pli == pliLeft )
      pliLeft = pli->pliNext;

   if( pli == pliRight ) {
      pliRight = pli->pliPrev;
      pliCurrent = NULL;
   }

   delete pli;
   iLength--;           
}  

/*
   *		Class:		Cltn 
   *		Function:	Get
   */

template<class T> T
Cltn<T>::Get( int iN )
{
   LLItem<T> *pliWalk = pliLeft;

   iN--;
   int i=0;

   while( pliWalk ) {
      if( i == iN )
	 return( T(pliWalk->tData) );
      pliWalk = pliWalk->pliNext;
      i++;
   }

   return( T(pliWalk->tData) );   
}

/*
   *		Class:		Cltn 
   *		Function:	operator[]
   */

template<class T> T
Cltn<T>::operator[]( int iN )
{
   LLItem<T> *pliWalk = pliLeft;

   int i=0;

   while( pliWalk ) {
      if( i == iN )
	 return( T(pliWalk->tData) );
      pliWalk = pliWalk->pliNext;
      i++;
   }

   return( T(pliWalk->tData) );
    
}

/*
   *		Class:		Cltn 
   *		Function:	attachr
   */

template<class T> void
Cltn<T>::attachr( T tNew )
{
   LLItem<T> *pliTemp = new LLItem<T>( tNew );
    
   if( is_empty() ) {
      pliLeft = pliRight = pliTemp;
   } else {
      pliRight->pliNext = pliTemp;
      pliTemp->pliPrev = pliRight;
      pliRight = pliTemp;
   }

   iLength++;
}

template<class T> void
Cltn<T>::attachb( T tNew )
{
   LLItem<T> *pliTemp = new LLItem<T>( tNew );
    
   if( is_empty() ) {
      pliLeft = pliRight = pliTemp;
   } else {
      pliTemp->pliNext = pliLeft;
      pliLeft = pliTemp;
   }

   iLength++;
}

/*
   *		Class:		Cltn 
   *		Function:	insertnth
   */

template<class T> void
Cltn<T>::insertnth( int iN, T tNew ) {
   LLItem<T> *pliTemp = new LLItem<T>( tNew );
   LLItem<T> *pliWalk = pliLeft;

   int i=0;

   if( iN < 0 ) {
      attachl( tNew );
      return;
   }

   while( pliWalk ) {
      if( i == iN ) {
	 pliTemp->pliNext = pliWalk->pliNext;
	 pliTemp->pliPrev = pliWalk;

	 if( pliWalk->pliNext )
	    pliWalk->pliNext->pliPrev = pliTemp;
	 pliWalk->pliNext = pliTemp;
	 if( pliWalk == pliRight )
	    pliRight = pliTemp;
	 iLength++;
	 return;
      }
      pliWalk = pliWalk->pliNext;
      i++;
   }

   attachr( tNew );

}

/*
   *		Class:		Cltn 
   *		Function:	insert
   */

template<class T> void
Cltn<T>::insert( LLItem<T> *pli, T tNew )
{
   LLItem<T> *pliTemp = new LLItem<T>( tNew );

   pliTemp->pliNext = pli->pliNext;
   pliTemp->pliPrev = pli;

   if( pli->pliNext ) 
      pli->pliNext->pliPrev = pliTemp;
	
   pli->pliNext = pliTemp;
   iLength++;

}

/*
   *		Class:		Cltn 
   *		Function:	attachl
   */

template<class T> Cltn<T>&
Cltn<T>::attachl( T tNew ) {
   LLItem <T> *pliTemp = new LLItem<T>( tNew );
    
   if( is_empty() ) {
      pliLeft = pliRight = pliTemp;
   } else {
      pliLeft->pliPrev = pliTemp;
      pliTemp->pliNext = pliLeft;
      pliLeft = pliTemp;
   }

   iLength++;

   return( *this );
}

/*
   *		Class:		Cltn 
   *		Function:	operator=
   */

template<class T> Cltn<T>&
Cltn<T>::operator=( const Cltn<T>& lRhs ) {
   LLItem<T> *pliWalk = lRhs.pliLeft;

   while( !is_empty() )
      detachl();

   while( pliWalk ) {
      attachr( pliWalk->tData );
      pliWalk = pliWalk->pliNext;
   }

   return( *this );
}  

/*
   *		Class:		Cltn 
   *		Function:	in
   *
   *	Added second parameter to use for splitting
   */

template<class T> int
Cltn<T>::in( T tItem ) {
   LLItem<T> *pliWalk = pliLeft;
   int i = 0;

   while( pliWalk ) {
      if( pliWalk->tData == tItem )
	 return( i );
      pliWalk = pliWalk->pliNext;
      i++;
   }

   return( -1 );
}

/*
   *		Class:		Cltn 
   *		Function:	in
   *
   *	Added second parameter to use for splitting
   */

template<class T> LLItem<T> *
Cltn<T>::li_in( T tItem ) {
   LLItem<T> *pliWalk = pliLeft;
   int i = 0;

   while( pliWalk ) {
      if( pliWalk->tData == tItem )
	 return( pliWalk );
      pliWalk = pliWalk->pliNext;
      i++;
   }

   return( pliWalk );
}


/*
   *		Class:		Cltn 
   *		Function:	join
   */

template<class T> Cltn<T>&
Cltn<T>::join( const Cltn<T>& lRhs ) {
   LLItem<T> *pliWalk = lRhs.pliLeft;

   while( pliWalk ) {
      attachr( pliWalk->tData );
      pliWalk = pliWalk->pliNext;
   }

   iLength += lRhs.iLength;

   return( *this );
}
  
template<class T> T
Cltn<T>::GetFirst()
{
   pliCurrent = pliLeft;
   return ( pliCurrent->tData );
}

template<class T> T
Cltn<T>::GetLast()
{
   pliCurrent = pliRight;
   return ( pliCurrent->tData );
}

template<class T> T
Cltn<T>::GetPrevious( T tref )
{
   LLItem<T> *pli_prev = NULL, *pliWalk = pliLeft;
   
   while( pliWalk ) {
      if( pliWalk->tData == tref )
	 return( pli_prev->tData );
      pli_prev = pliWalk;
      pliWalk = pliWalk->pliNext;
   }   
	 return( pli_prev->tData );
}


template<class T> void
Cltn<T>::Next(){
   if( pliCurrent ) 
      pliCurrent = pliCurrent->pliNext;
   else
      pliCurrent = pliAdvance;
}

template<class T> void
Cltn<T>::RemoveCurrentItem()
{
   pliAdvance = pliCurrent->pliNext;
   detach( pliCurrent );
   pliCurrent = NULL;
}

template<class T> T
Cltn<T>::AdvanceItem()
{
   if( pliCurrent ) 
      pliCurrent = pliCurrent->pliNext;
   return( pliCurrent->tData );
}

template<class T> void
Cltn<T>::AddBefore( T element, T reference )
{
  
   if( pliLeft ){
    
      for ( pliCurrent = pliLeft; pliCurrent != 0; pliCurrent = pliCurrent->pliNext ){
	 if ( pliCurrent->tData == reference ){
	    if( (pliCurrent = pliCurrent->pliPrev) != NULL )
	       insert( pliCurrent, element );
	    else
	       attachl( element );
	    return;
	 }
      }
   }
   else
      std::cerr << "Element does not exist in collection.\n";
    
}
  
template<class T> void
Cltn<T>::AddAfter( T element, T reference )
{
  
   if( pliLeft ){
    
      for ( pliCurrent = pliLeft; pliCurrent != 0; pliCurrent = pliCurrent->pliNext ){
	 if ( pliCurrent->tData == reference ){
	    if( pliCurrent->pliNext )
	       insert( pliCurrent, element );
	    else
	       attachr( element );
	    return;
	 }
      }
   }
   else
      std::cerr << "Element does not exist in collection.\n";
    
}
 
// assume function Includes has returned true prior to calling Remove.  Simply
// remove the current element at pliCurrent.  Use code from detachnth replacing pliWalk with pliCurrent
// to create detach method in List.h.
  
template<class T> void
Cltn<T>::Remove( T element )
{
   if ( pliCurrent->tData == element )
      detach( pliCurrent );
}
  
template<class T> bool
Cltn<T>::Includes( T element )
{  
   if( pliLeft ){
    
      for ( pliCurrent = pliLeft; pliCurrent != 0; pliCurrent = pliCurrent->pliNext ){
	 if ( pliCurrent->tData == element )
	    return TRUE;
      }
   }
   else
      return FALSE; // as collection is empty it cannot include anything
    
   return FALSE;
}

template<class T> void
Cltn<T>::Delete( T element )
{
   if( Includes( element ) )
      Remove( element );
}

template<class T> void
Cltn<T>::DeleteIndex( int index )
{
   LLItem<T> *pliWalk = pliLeft;
   int i = 0;

   index--;

   while( pliWalk ) {
      if( i == index ) {
	 detach( pliWalk );
	 return;
      }
      pliWalk = pliWalk->pliNext;
      i++;
   }
}

template<class T> void
Cltn<T>::Replace( T old_element, T new_element )
{  
   if( Includes( old_element ) )
      pliCurrent->tData = new_element;
   else
      attachr( new_element );
}

template<class T> void
Cltn<T>::ReplaceEnd( T new_element )
{
   if( pliRight )
      pliRight->tData = new_element;
}

template<class T> void
Cltn<T>::Rotate()
{
   LLItem<T> *pli_first = pliRight;
   pliRight = pli_first->pliPrev;
   pliRight->pliNext = NULL;
   pli_first->pliNext = pliLeft;
   pliLeft->pliPrev = pli_first;
   pli_first->pliPrev = NULL;
   pliLeft = pli_first;   
}

template<class T> void
Cltn<T>::SplitCltn( T split_point, Cltn<T> *first, Cltn<T> *second ){
  
   LLItem<T> *split_item = li_in( split_point ); 
   int split_position = in( split_point );
   first->iLength = split_position;
   second->iLength = iLength - split_position;
    
   first->pliLeft = pliLeft;
   first->pliRight = split_item->pliPrev;
   first->pliRight->pliNext = NULL;
    
   second->pliLeft = split_item;
   second->pliLeft->pliPrev = NULL;
   second->pliRight = pliRight;
    
   pliLeft = NULL;	// avoids deletion problems
    
}

template<class T> void
Cltn<T>::JoinCltn( Cltn<T> *first, Cltn<T> *second ) {

   pliLeft = first->pliLeft;
   first->pliRight->pliNext = second->pliLeft;
   second->pliLeft->pliPrev = first->pliRight;
   pliRight = second->pliRight;
   iLength = first->iLength + second->iLength;
  
   first->pliLeft = NULL;	// avoids deletion problems
   second->pliLeft = NULL;

}
