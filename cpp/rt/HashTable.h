/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_HashTable_H
#define db_rt_HashTable_H

namespace db
{
namespace rt
{

/**
 * A HashTable is a lock-free hash table. Its implementation is based on
 * a presentation by Dr. Cliff Click at Google Tech Talks on March 28, 2007
 * called: "Advanced Topics in Programming Series: A Lock-Free Hash Table".
 *
 * This hash table can be used by multiple threads at once and does not require
 * any locking. To achieve this, it relies upon the atomic compare-and-swap
 * operation.
 *
 * @author Dave Longley
 */
// FIXME: make a third template value that is an object that implements the
// hash code function
template<typename K, typename V>
class HashTable
{
protected:
   /**
    * The array of values, indexed by hash code.
    */
   V* mValues;

   /**
    * The maximum capacity of the hash table.
    */
   int mCapacity;

   /**
    * The current length of the hash table, meaning the number of values
    * stored in it.
    */
   int mLength;

   /**
    * The hash code function. This function produces a hash code from a key.
    */
   typedef int (*HashCodeFunction)(const K& k);
   HashCodeFunction mHashCodeFunction;

public:
   /**
    * Creates a new HashTable with the given initial capacity.
    *
    * @param capacity the initial capacity to use.
    */
   HashTable(int capacity = 10);

   /**
    * Creates a copy of a HashTable.
    *
    * @param copy the HashTable to copy.
    */
   HashTable(const HashTable& copy);

   /**
    * Destructs this HashTable.
    */
   virtual ~HashTable();

   /**
    * Sets this HashTable equal to another one. This will clear any existing
    * entries in this HashTable and then fill it with the entries from the
    * passed HashTable.
    *
    * @param rhs the HashTable to set this one equal to.
    *
    * @return a reference to this HashTable.
    */
   virtual HashTable& operator=(const HashTable& rhs);

   /**
    * Maps a key to a value in this HashTable.
    *
    * @param k the key.
    * @param v the value.
    */
   // FIXME: consider only implementing operators
   virtual void put(const K& k, const V& v);

   /**
    * Gets the value that is mapped to the passed key.
    *
    * @param k the key to get the value for.
    *
    * @return the value or NULL if the value does not exist.
    */
   virtual V* get(const K& k);
};

template<typename K, typename V>
HashTable<K, V>::HashTable(int capacity) :
   mValues(NULL),
   mCapacity(capacity),
   mLength(0)
{
   // FIXME:
}

template<typename K, typename V>
HashTable<K, V>::HashTable(const HashTable& copy)
{
   // FIXME:
}

template<typename K, typename V>
HashTable<K, V>::~HashTable()
{
   // FIXME:
}

template<typename K, typename V>
HashTable<K, V>& HashTable<K, V>::operator=(const HashTable& rhs)
{
   // FIXME:
}

template<typename K, typename V>
void HashTable<K, V>::put(const K& k, const V& v)
{
   // FIXME:
}

template<typename K, typename V>
V* HashTable<K, V>::get(const K& k)
{
   // FIXME:
}

} // end namespace rt
} // end namespace db
#endif
