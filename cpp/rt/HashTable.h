/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_HashTable_H
#define db_rt_HashTable_H

#if 0

#ifdef WIN32
#include <malloc.h>
#endif

namespace db
{
namespace rt
{

/**
 * Defines a hash code function. This function produces a hash code from a key.
 */
// FIXME: define a default hash function?
template<typename _K>
struct HashFunction
{
   int operator()(const _K& k) const;
};

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
template<typename _K, typename _V, typename _H>
class HashTable
{
protected:
   /**
    * An entry is a single slot in the hash table. It can hold either:
    *
    * 1. A key, a hash, and value.
    * 2. A sentinel that indicates a key is stored in a newer entries array.
    * 3. A tombstone that indicates a removed key.
    */
   struct Entry
   {
      // types of entries
      enum Type
      {
         Value,
         Sentinel,
         Tombstone
      };

      // reference count for an entry, note that an assumption is made here
      // that since an entry has to be heap-allocated and 32-bit aligned, so
      // will the first value of the struct be automatically 32-bit aligned,
      // but we put the special alignment in here anyway
#ifdef WIN32
      // FIXME: The compiler attribute is provided to ensure the count is
      // 32-bit aligned so that it doesn't break fragile windows atomic
      // code.
      volatile unsigned int refCount __attribute__ ((aligned (32)));
#else
      volatile unsigned int refCount;
#endif

      // data in the entry
      Type type;
      _K k;
      int h;
      union
      {
         _V v;
         bool dead;
      };
   };

   /**
    * An entry list has an array of entries, a capacity, and a pointer to
    * the next, newer EntryList (or NULL).
    *
    * FIXME: might need to store references to itself as a void pointer
    * to be destroyed?
    */
   struct EntryList
   {
      // reference count for an entry, note that an assumption is made here
      // that since an entry has to be heap-allocated and 32-bit aligned, so
      // will the first value of the struct be automatically 32-bit aligned,
      // but we put the special alignment in here anyway
#ifdef WIN32
      // FIXME: The compiler attribute is provided to ensure the count is
      // 32-bit aligned so that it doesn't break fragile windows atomic
      // code.
      volatile unsigned int refCount __attribute__ ((aligned(4)));
#else
      volatile unsigned int refCount;
#endif

      Entry** entries;
      int capacity;

#ifdef WIN32
      // FIXME: The compiler attribute is provided to ensure this is
      // 32-bit aligned so that it doesn't break fragile windows atomic
      // code.
      volatile EntryList* next __attribute__ ((aligned(4)));
#else
      volatile EntryList* next;
#endif
   };

   /**
    * The entry list.
    */
   EntryList* mEntryList;

   /**
    * The maximum capacity of the hash table, meaning the maximum number of
    * entries that could be stored in it at present. The capacity will grow
    * as necessary.
    */
   int mCapacity;

   /**
    * The current length of the hash table, meaning the number of entries
    * stored in it.
    */
   int mLength;

   /**
    * The function for producing hash codes from keys.
    */
   _H mHashFunction;

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
   virtual void put(const _K& k, const _V& v);

   /**
    * Gets the value that is mapped to the passed key.
    *
    * @param k the key to get the value for.
    *
    * @return the value or NULL if the value does not exist.
    */
   virtual _V* get(const _K& k);

protected:
   /**
    * Allocates memory that is aligned such that it can be safely used in a
    * Compare-And-Swap algorithm.
    *
    * @param size the size of the memory to allocate.
    *
    * @return a pointer to the allocated memory.
    */
   virtual void* allocateCasableMemory(size_t size);

   /**
    * Frees some CAS-able memory.
    *
    * @param ptr the pointer to the CAS-able memory to free.
    */
   virtual void freeCasableMemory(void* ptr);

   /**
    * Performs an atomic insert by using Compare And Swap (CAS).
    *
    * @param dst the destination to write the new value to.
    * @param oldVal the old value.
    * @param newVal the new value.
    *
    * @return true if successful, false if not.
    */
   virtual bool atomicInsert(void* dst, void* oldVal, void* newVal);

   /**
    * Creates a value entry.
    *
    * @param key the entry key.
    * @param value the entry value.
    *
    * @return the new entry.
    */
   virtual Entry* createValue(const _K& key, const _V& value);

   /**
    * Creates a sentinel entry.
    *
    * @param key the entry key.
    *
    * @return the new entry.
    */
   virtual Entry* createSentinel(const _K& key);

   /**
    * Creates a tombstone entry.
    *
    * @param key the entry key.
    *
    * @return the new entry.
    */
   virtual Entry* createTombStone(const _K& key);

   /**
    * Increases the ref count on an entry.
    *
    * @param e the entry.
    */
   virtual void refEntry(Entry* e);

   /**
    * Decreases the ref count on an entry.
    *
    * @param e the entry.
    */
   virtual void unrefEntry(Entry* e);

   /**
    * Increases the ref count on an entry list.
    *
    * @param el the entry list.
    */
   virtual void refEntryList(Entry* el);

   /**
    * Decreases the ref count on an entry list.
    *
    * @param el the entry list.
    */
   virtual void unrefEntryList(Entry* el);

   /**
    * Atomically inserts an entry into an entries list and returns true
    * if successful.
    *
    * @param el the entries list to insert into.
    * @param idx the index to insert at.
    * @param eOld the old entry to replace.
    * @param eNew the new entry to insert.
    *
    * @return true if successful, false if not.
    */
   virtual bool insertEntry(Entry** el, int idx, Entry* eOld, Entry* eNew);

   /**
    * Gets the entry at the given index and increases its reference count. Make
    * sure to call unrefEntry() when finished.
    *
    * @param idx the index of the entry to retrieve.
    *
    * @return the entry or NULL if no such entry exists.
    */
   virtual Entry* getEntry(int idx);

   /**
    * Resizes the table.
    *
    * @param capacity the new capacity to use.
    */
   virtual void resize(int capacity);
};

template<typename _K, typename _V, typename _H>
HashTable<_K, _V, _H>::HashTable(int capacity) :
   mCapacity(capacity),
   mLength(0)
{
   // FIXME:
}

template<typename _K, typename _V, typename _H>
HashTable<_K, _V, _H>::HashTable(const HashTable& copy)
{
   // FIXME:
}

template<typename _K, typename _V, typename _H>
HashTable<_K, _V, _H>::~HashTable()
{
   // FIXME:
}

template<typename _K, typename _V, typename _H>
HashTable<_K, _V, _H>& HashTable<_K, _V, _H>::operator=(
   const HashTable& rhs)
{
   // FIXME:

   return *this;
}

template<typename _K, typename _V, typename _H>
void HashTable<_K, _V, _H>::put(const _K& k, const _V& v)
{
   /* Steps:
    *
    * 1. Create the entry for the table.
    * 2. Enter a spin loop that runs until the value is inserted.
    * 3. Use the newest entries array.
    * 4. Find the index to insert at.
    * 5. Do a CAS to insert.
    * 6. CAS may fail if another thread slipped in and modified the value
    *    before us, so loop and try again.
    *
    * Note: We may need to resize the table or we may discover that an old
    * entry is a sentinel, in which case we must insert into a new table.
    */

   // create a value entry
   Entry* e = createValue(k, v);

   // enter a spin loop that runs until the entry has been inserted
   bool inserted = false;
   while(!inserted)
   {
      // use the newest entries array (last in the linked list)
      EntryList* el = mEntryList;
      for(; el->next != NULL; el = el->next);

      // enter another spin loop to keep trying to insert the entry until
      // we do or we decide that we must insert into a newer table
      bool useNewTable = false;
      int idx = eNew->h;
      while(!inserted && !useNewTable)
      {
         // limit index to capacity of table
         i &= (mCapacity - 1);

         // get any existing entry
         Entry* eOld = getEntry(i);
         if(eOld == NULL)
         {
            // there is no existing entry so try to insert
            inserted = atomicInsert(el, i, eOld, eNew);
         }
         else if(eOld->type == Entry::Sentinel)
         {
            // a sentinel entry tells us that there is a newer table where
            // we have to do the insert
            useNewTable = true;
         }
         // existing entry is replaceable value or tombstone
         else
         {
            // if the entry key is at the same memory address or if the hashes
            // match and the keys match, then we can replace the existing
            // entry
            if(&(eOld->k) == &k || (eOld->h == eNew->h && eOld->k == k))
            {
               inserted = atomicInsert(el, i, eOld, eNew);
            }
            else if(i == mCapacity - 1)
            {
               // keys did not match and we've reached the end of the table, so
               // we must use a new table
               useNewTable = true;
            }
            else
            {
               // keys did not match, so we found a collision, increase the
               // index by 1 and try again
               idx++;
            }
         }

         // unreference the old entry
         unrefEntry(eOld);
      }

      if(useNewTable)
      {
         // see if there is a newer table ... if not, then we must resize
         // to create one
         if(entries == mLastEntryList)
         {
            // resize and then try insert again
            // FIXME: use a better capacity increase formula
            resize((int)(mCapacity * 1.5 + 1));
         }
      }
   }
}

template<typename _K, typename _V, typename _H>
_V* HashTable<_K, _V, _H>::get(const _K& k)
{
   _V* rval = NULL;

   /* Search our entries array looking to see if we have a key that matches or
    * if the entry for the given key does not exist. If we find an entry that
    * matches the hash for our key, but the key does not match, then we must
    * reprobe. We do so by incrementing our index by 1 since we handle
    * collisions when inserting by simply increasing the index by 1 until we
    * find an empty slot.
    */
   int hash = mHashFunction(k);
   bool done = false;
   for(int i = hash; !done; i++)
   {
      // limit index to capacity of table
      i &= (mCapacity - 1);

      // get entry at index
      Entry* e = getEntry(i);
      if(e != NULL)
      {
         // if the entry key is at the same memory address or if the hashes
         // match and the keys match, then we found the value we want
         if(&(e->k) == &k || (e->h == hash && e->k == k))
         {
            rval = &(e->v);
            done = true;
         }
         else if(i == mCapacity - 1)
         {
            // keys did not match and we've reached the end of the table, so
            // the entry does not exist
            done = true;
         }
      }
      else
      {
         // no such entry
         done = true;
      }

      // unreference entry
      unrefEntry(e);
   }

   return rval;
}

template<typename _K, typename _V, typename _H>
void* HashTable<_K, _V, _H>::allocateCasableMemory(size_t size)
{
#ifdef WIN32
   // must align on a 32-bit boundary because windows is silly
   // this can be removed once non-windows specific atomics are used
   return _aligned_malloc(size, 32);
#else
   return malloc(size);
#endif
}

template<typename _K, typename _V, typename _H>
void HashTable<_K, _V, _H>::freeCasableMemory(void* ptr)
{
   if(ptr != NULL)
   {
#ifdef WIN32
      _aligned_free(ptr);
#else
      free(ptr);
#endif
   }
}

template<typename _K, typename _V, typename _H>
bool HashTable<_K, _V, _H>::atomicInsert(
   void* dst, void* oldVal, void* newVal)
{
#ifdef WIN32
   return (InterlockedCompareExchange(dst, newVal, oldVal) == oldVal);
#else
   return __sync_bool_compare_and_swap(dst, oldVal, newVal);
#endif
}

template<typename _K, typename _V, typename _H>
Entry* HashTable<_K, _V, _H>::createValue(const _K& key, const _V& value)
{
   Entry* e = static_cast<Entry*>(allocateCasableMemory(sizeof(Entry)));
   e->refCount = 1;
   e->type = Value;
   e->k = key;
   e->h = mHashFunction(key);
   e->v = value;
};

template<typename _K, typename _V, typename _H>
Entry* HashTable<_K, _V, _H>::createSentinel(const _K& key)
{
   Entry* e = static_cast<Entry*>(allocateCasableMemory(sizeof(Entry)));
   e->refCount = 1;
   e->type = Sentinel;
   e->k = key;
   e->h = mHashFunction(key);
   e->dead = false;
};

template<typename _K, typename _V, typename _H>
Entry* HashTable<_K, _V, _H>::createTombstone(const _K& key)
{
   Entry* e = static_cast<Entry*>(allocateCasableMemory(sizeof(Entry)));
   e->refCount = 1;
   e->type = Tombstone;
   e->k = key;
   e->h = mHashFunction(key);
   e->dead = true;
};

template<typename _K, typename _V, typename _H>
bool HashTable<_K, _V, _H>::insertEntry(
   Entry** el, int idx, Entry* eOld, Entry* eNew)
{
   return atomicInsert(el + idx, eOld, eNew);
}

template<typename _K, typename _V, typename _H>
Entry* HashTable<_K, _V, _H>::getEntry(int idx)
{
   Entry* rval = NULL;

   // FIXME: we need to ensure that if we have access to an entries array
   // here, it will not be destroyed by another thread that is performing
   // a resize ... so we need to do some kind of reference counting ...
   // and in addition to that, we need to ensure that once we leave this
   // function the entry will not be destroyed ... so we need to reference
   // count that as well...

   // FIXME: references to entries should not pose a problem concerning
   // memory clean up ... if someone has a pointer to an entry, then they
   // must already have a reference to the list it resides in ... which
   // will guarantee that an entry within that list cannot be destroyed
   // once they drop the reference to that list they will already have a
   // reference to the entry, again guaranteeing it won't be destroyed
   // prematurely ... however ...
   // entry list destruction is a problem in the current design ...
   // when are entry lists destroyed? when no one has any reference to
   // them any more ... when does that happen? when no one is looking in
   // an old entry list for something any longer ...
   // what if someone is searching through the entry lists ... and grabs
   // a pointer to an entry list ... and then that entry list is dropped
   // from the linked list of entry lists and its reference is destroyed
   // ... when we go to increment the reference on the pointer that is
   // looking at the list, we will explode

   // FIXME: could add a "hazard pointer" to each thread ... that must
   // be checked when doing clean up ... to use it:
   // Thread A: myHazardPtr = &x;
   // Thread A: ensure *myHazardPtr == *(&x) (CAS?) ... FAIL, x could be gone
   // Thread B: save old x as y
   // Thread B: set x to NULL
   // Thread B: decrement y->ref via atomic, get result
   // Thread B: if result == 0 AND no thread has stored the address of x
   //           then delete x->object (delete fails in this case)
   // Thread A: increment x->ref
   // Thread A: store x
   // Thread A: clear myHazardPtr
   //
   // What happens when you try to de-reference x? FAIL


   // iterate through each entries list until a non-sentinel entry is found
   bool found = false;
   for(EntryList* el = mEntryList; !found && el != NULL; el = el->next)
   {
      for(int i = 0; i < el->capacity; i++)
      {
         Entry* e = el->entries[idx];
         if(e->type != Sentinel)
         {
            found = true;
            rval = e;
         }
      }
   }

   return rval;
}

template<typename _K, typename _V, typename _H>
void HashTable<_K, _V, _H>::resize(int capacity)
{
   // get the last entries array (last in linked list)
   EntryList* el = mEntryList;
   for(; el->next != NULL; el = el->next);

   // FIXME: ignore double copies for now? don't worry about the extra
   // work generated when two threads just so happen to want to resize at
   // the same time? ... at least it's unlikely right now as we only resize
   // on put() and we check to see if someone else has already started
   // resizing first

   // if the capacity of the list is smaller than the requested capacity
   // then create a new one
   if(el->capacity < capacity)
   {
      // create the new entry list
      EntryList* newList = new EntryList;

      EntryList newList;
      newList->entries = malloc();
      atomicInsert(dst, oldList, newList);
   }


   // FIXME: use a better capacity increase formula

   // create table with the


   // FIXME: do not copy tombstones
}


} // end namespace rt
} // end namespace db
#endif

#endif
