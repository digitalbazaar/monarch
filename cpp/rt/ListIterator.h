/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_ListIterator_H
#define db_rt_ListIterator_H

#include "monarch/rt/Iterator.h"
#include <list>

namespace db
{
namespace rt
{

/**
 * A ListIterator is an Iterator for an stl list.
 *
 * @author Dave Longley
 */
template<class T>
class ListIterator : public Iterator<T>
{
protected:
   /**
    * An iterator that points to the current object in the list.
    */
   std::_List_iterator<T> mCurrent;

   /**
    * The stl iterator that points to the next object in the list.
    */
   std::_List_iterator<T> mNext;

   /**
    * A reference to the list.
    */
   std::list<T>* mList;

public:
   /**
    * Creates a new ListIterator for the given stl list.
    *
    * @param l the list to iterate over.
    */
   ListIterator(std::list<T>& l);

   /**
    * Destructs this ListIterator.
    */
   virtual ~ListIterator() {};

   /**
    * Gets the next object and advances the ListIterator.
    *
    * @return the next object.
    */
   virtual T& next();

   /**
    * Returns true if this ListIterator has more objects.
    *
    * @return true if this ListIterator has more objects, false if not.
    */
   virtual bool hasNext();

   /**
    * Removes the current object and advances the ListIterator.
    */
   virtual void remove();
};

template<class T>
ListIterator<T>::ListIterator(std::list<T>& l)
{
   mList = &l;
   mNext = mCurrent = mList->begin();
}

template<class T>
T& ListIterator<T>::next()
{
   mCurrent = mNext;
   mNext++;
   return *mCurrent;
}

template<class T>
bool ListIterator<T>::hasNext()
{
   return mNext != mList->end();
}

template<class T>
void ListIterator<T>::remove()
{
   mNext = mCurrent = mList->erase(mCurrent);
   mNext++;
}

} // end namespace rt
} // end namespace db
#endif
