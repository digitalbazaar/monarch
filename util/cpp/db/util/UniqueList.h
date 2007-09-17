/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_UniqueList_H
#define db_util_UniqueList_H

#include "db/util/ListIterator.h"

namespace db
{
namespace util
{

/**
 * A UniqueList consists of a list of unique objects. The objects will be
 * compared on their equality operator: operator==.
 * 
 * @author Dave Longley
 */
template<typename T>
class UniqueList
{
protected:
   /**
    * The underlying list data structure.
    */
   std::list<T> mList;
   
public:
   /**
    * Creates a new UniqueList.
    */
   UniqueList() {};
   
   /**
    * Destructs this UniqueList.
    */
   virtual ~UniqueList() {};
   
   /**
    * Adds an object to this list, if it isn't already in the list.
    * 
    * @param obj the object to add.
    * 
    * @return true if the object was added, false if it was not.
    */
   virtual bool add(const T& obj);
   
   /**
    * Removes an object from this list, if it is in the list.
    * 
    * @param obj the object to remove.
    * 
    * @return true if the object was removed, false if it not.
    */
   virtual bool remove(const T& obj);
   
   /**
    * Gets the Iterator for this list. It must be deleted after use.
    * 
    * @return the Iterator for the list.
    */
   virtual Iterator<T>* getIterator();
};

template<typename T>
bool UniqueList<T>::add(const T& obj)
{
   bool rval = true;
   
   Iterator<T>* i = getIterator();
   while(rval && i->hasNext())
   {
      rval = !(i->next() == obj);
   }
   
   if(rval)
   {
      mList.push_back(obj);
   }
   
   return rval;
}

template<typename T>
bool UniqueList<T>::remove(const T& obj)
{
   bool rval = false;
   
   Iterator<T>* i = getIterator();
   while(!rval && i->hasNext())
   {
      if(i->next() == obj)
      {
         i->remove();
         rval = true;
      }
   }
   
   return rval;
}

template<typename T>
Iterator<T>* UniqueList<T>::getIterator()
{
   return new ListIterator<T>(mList);
}

} // end namespace util
} // end namespace db
#endif
