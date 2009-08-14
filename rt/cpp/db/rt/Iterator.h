/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_Iterator_H
#define db_rt_Iterator_H

#include "db/rt/Collectable.h"

namespace db
{
namespace rt
{

/**
 * A Iterator provides an interface for enumerating over the objects of
 * some collection.
 *
 * @author Dave Longley
 */
template<class T>
class Iterator
{
public:
   /**
    * Creates a new Iterator.
    */
   Iterator() {};

   /**
    * Destructs this Iterator.
    */
   virtual ~Iterator() {};

   /**
    * Gets the next object and advances the Iterator.
    *
    * @return the next object.
    */
   virtual T& next() = 0;

   /**
    * Returns true if this Iterator has more objects.
    *
    * @return true if this Iterator has more objects, false if not.
    */
   virtual bool hasNext() = 0;

   /**
    * Removes the current object and advances the Iterator.
    */
   virtual void remove() = 0;
};

// define counted reference Iterator type
template<class T>
class IteratorRef : public Collectable< Iterator<T> >
{
public:
   IteratorRef(Iterator<T>* ptr = NULL) :
      Collectable< Iterator<T> >(ptr) {};
   virtual ~IteratorRef() {};
};

} // end namespace rt
} // end namespace db
#endif
