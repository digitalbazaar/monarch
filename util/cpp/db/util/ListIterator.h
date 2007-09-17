/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_ListIterator_H
#define db_util_ListIterator_H

#include "db/util/Iterator.h"
#include <list>

namespace db
{
namespace util
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
    * The stl iterator for the list.
    */
   std::_List_iterator<T> mIterator;
   
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
   mIterator = l.begin();
}

template<class T>
T& ListIterator<T>::next()
{
   T* rval = &(*mIterator);
   mIterator++;
   return *rval;
}

template<class T>
bool ListIterator<T>::hasNext()
{
   return mIterator != mList->end();
}

template<class T>
void ListIterator<T>::remove()
{
   mIterator = mList->erase(mIterator);
}

} // end namespace util
} // end namespace db
#endif
