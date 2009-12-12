/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_HashTable_H
#define monarch_rt_HashTable_H

#include "monarch/rt/Atomic.h"
#include "monarch/rt/HazardPtrList.h"

namespace monarch
{
namespace rt
{

/**
 * Defines a hash code function. This function produces a hash code from a key.
 */
template<typename _K>
struct HashFunction
{
   int operator()(const _K& k) const;
};

/**
 * Defines an equality function. This function returns true if two keys are
 * equal, false if they are not.
 */
template<typename _K>
struct EqualsFunction
{
   bool operator()(const _K& k1, const _K& k2) const;
};

/**
 * The default equals function.
 */
template<typename _K>
struct DefaultEqualsFunction : public EqualsFunction<_K>
{
   bool operator()(const _K& k1, const _K& k2) const
   {
      return k1 == k2;
   };
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
 * This HashTable is stored as a linked-list of entry blocks. Each entry block
 * is referred to as an EntryList. Each entry block constitutes a single
 * hash table. There are multiple EntryLists so that this HashTable can be
 * resized. To enable this behavior in a lock-free manner, a combination
 * of hazard pointers and compare-and-swap operations are used.
 *
 * Here are two scenarios of two threads working concurrently, one that is
 * moving EntryLists into a garbage list (GC) and another that is trying to
 * use one of those EntryLists (GET). The first scenario demonstrates a
 * successful acquisition of an EntryList that is now garbage, and the second
 * shows a successful miss on acquiring that EntryList (which will result in
 * the next EntryList being checked instead). Note that in the first case, this
 * is not an error, the garbage EntryList will instruct the thread to look at
 * the next EntryList via some state internal to it and its memory will not be
 * freed by any thread performing garbage collection:
 *
 * Scenario 1
 * ----------
 * GET: Get a blank hazard pointer.
 * GET: Set the hazard pointer to EntryList X.
 * GET: Ensure X is still in the valid list (it is).
 * GET: Proceed to use X.
 * GC: Move X onto a garbage list.
 * GC: Check X's reference count (it is 0).
 * GC: Scan the list of hazard pointers for X (it is found).
 * GET: Increase reference count on X (now 1, saving a future GC scan time).
 * GC: Does not collect X.
 *
 * Scenario 2
 * ----------
 * GET: Get a blank hazard pointer.
 * GET: Set the hazard pointer to EntryList X.
 * GC: Move X onto a garbage list.
 * GET: Ensure X is still in the valid list (it is not).
 * GET: Loop back and get a different EntryList Y.
 * GC: Check X's reference count (it is 0).
 * GC: Scan the list of hazard pointers for X (it is NOT found).
 * GC: Collect X.
 *
 * To be more specific about how the garbage collection algorithm works:
 *
 * First, using a loop and compare-and-swap, remove N EntryLists from the
 * shared garbage list and place them in a private temporary list A. Next,
 * iterate over that list checking each EntryList for a reference count of 0.
 * If the reference count is higher than 0, move the EntryList into another
 * temporary list B. If the reference count is 0, scan the hazard pointer list
 * for the EntryList. If it is not found, free the EntryList. If it is found,
 * append the EntryList to B. When A is empty, check the valid list for
 * EntryLists that are now considered garbage. Note that this will require
 * getting a reference to the EntryList first so that competing GC threads will
 * not free it while checking the garbage flag. If an EntryList is garbage,
 * then CAS remove it from the valid list. Then append it to B. When all
 * valid EntryLists have been checked, CAS prepend B onto the shared garbage
 * list.
 *
 * Note: Reference counts are only kept for EntryLists. Entries are protected
 * by hazard pointers. Also, the code is written such that the reference count
 * for an EntryList that an Entry belongs to is always 1 if the Entry is being
 * accessed. Assumptions that this will continue to hold true are made to
 * simplify the code.
 *
 * @author Dave Longley
 */
template<typename _K, typename _V, typename _H,
typename _E = DefaultEqualsFunction<_K> >
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
   struct EntryList;
   struct Entry
   {
      // types of entries
      enum Type
      {
         Value = 0,
         Sentinel,
         Tombstone
      };

      // data in the entry
#ifdef WIN32
      /* MS Windows requires any variable written to in an atomic operation
         to be aligned to the address size of the CPU. */
      volatile Type type __attribute__ ((aligned(4)));
#else
      volatile Type type;
#endif
      _K k;
      int h;
      _V* v;
      EntryList* owner;
      Entry* next;
   };

   /**
    * An entry list has an array of entries, a capacity, and a pointer to
    * the next, newer EntryList (or NULL).
    */
   struct EntryList
   {
      /* Note: Reference count for an EntryList, note that an assumption is made
         here that since an entry has to be heap-allocated and 32-bit aligned,
         so will the first value of the struct be automatically 32-bit aligned,
         but we put the special alignment in here anyway. */
      volatile aligned_uint32_t refCount;
#ifdef WIN32
      /* MS Windows requires any variable written to in an atomic operation
         to be aligned to the address size of the CPU. */
      volatile Entry* garbageEntries __attribute__ ((aligned(4)));
#else
      volatile Entry* garbageEntries;
#endif
#ifdef WIN32
      /* MS Windows requires any variable written to in an atomic operation
         to be aligned to the address size of the CPU. */
      volatile Entry* freeEntries __attribute__ ((aligned(4)));
#else
      volatile Entry* freeEntries;
#endif
      Entry** entries;
      int capacity;
      int length;
      EntryList* next;
      bool old;
      EntryList* garbageNext;
   };

   /**
    * The first EntryList.
    */
#ifdef WIN32
   /* MS Windows requires any variable written to in an atomic operation
      to be aligned to the address size of the CPU. */
   volatile EntryList* mHead __attribute__ ((aligned(4)));
#else
   volatile EntryList* mHead;
#endif

   /**
    * The first garbage EntryList.
    */
#ifdef WIN32
   /* MS Windows requires any variable written to in an atomic operation
      to be aligned to the address size of the CPU. */
   volatile EntryList* mGarbageHead __attribute__ ((aligned(4)));
#else
   volatile EntryList* mGarbageHead;
#endif

   /**
    * A hazard pointer list for protecting access to entry lists.
    */
   HazardPtrList mHazardPtrs;

   /**
    * The function for producing hash codes from keys.
    */
   _H mHashFunction;

   /**
    * The function for comparing keys for equality.
    */
   _E mEqualsFunction;

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
    * @param replace true to replace an existing value, false to abort if
    *        an existing value is found.
    *
    * @return true if the value was put in the table, false if not.
    */
   virtual bool put(const _K& k, const _V& v, bool replace = true);

   /**
    * Gets the value that is mapped to the passed key.
    *
    * @param k the key to get the value for.
    * @param v the value to be set.
    *
    * @return true if the key was found, false if not.
    */
   virtual bool get(const _K& k, _V& v);

   /**
    * Removes the value that is mapped to the passed key.
    *
    * @param k the key to remove the value for.
    *
    * @return true if the value removed, false if the key did not exist.
    */
   virtual bool remove(const _K& k);

   /**
    * Removes all key value pairs from this HashTable.
    */
   virtual void clear();

   /**
    * Gets the current length of this HashTable.
    *
    * @return the length of this HashTable.
    */
   virtual int length();

protected:
   /**
    * Creates an empty EntryList.
    *
    * @param capacity the capacity for the EntryList.
    *
    * @return the new EntryList.
    */
   virtual EntryList* createEntryList(int capacity);

   /**
    * Frees an EntryList.
    *
    * @param el the EntryList to free.
    */
   virtual void freeEntryList(EntryList* el);

   /**
    * Creates a Value Entry.
    *
    * @param el the EntryList to create the Entry for.
    * @param key the Entry key.
    * @param value the Entry value.
    *
    * @return the new Entry.
    */
   virtual Entry* createEntry(EntryList* el, const _K& key, const _V& value);

   /**
    * Frees an Entry.
    *
    * @param e the Entry to free.
    */
   virtual void freeEntry(Entry* e);

   /**
    * Increases the reference count on the next entry list. An assumption
    * is made that the previous entry list already has a reference count
    * greater than 0.
    *
    * @param ptr the hazard pointer to use.
    * @param prev the previous entry list, NULL to use the head.
    *
    * @return the next EntryList (can be NULL).
    */
   virtual EntryList* refNextEntryList(HazardPtr* ptr, EntryList* prev);

   /**
    * Decreases the reference count on the given entry list.
    *
    * @param el the entry list to decrease the reference count on.
    */
   virtual void unrefEntryList(EntryList* el);

   /**
    * Sets the given hazard pointer at the Entry at the given index. The
    * hazard pointer must be set to NULL to unprotect the Entry.
    *
    * @param ptr the hazard pointer to use.
    * @param el the EntryList to get the Entry in.
    * @param idx the index of the Entry.
    *
    * @return the Entry (can be NULL).
    */
   virtual Entry* protectEntry(HazardPtr* ptr, EntryList* el, int idx);

   /**
    * A helper function that gets the most current EntryList at
    * the time at which this function was called. The reference count for
    * the returned EntryList will be incremented and therefore must be
    * decremented later by the caller.
    *
    * @param ptr the hazard pointer to use.
    *
    * @return the most current EntryList.
    */
   virtual EntryList* getCurrentEntryList(HazardPtr* ptr);

   /**
    * Replaces an old entry with a new one, if the old one hasn't changed.
    *
    * @param el the EntryList to update.
    * @param idx the index of the old entry.
    * @param eOld the old Entry.
    * @param eNew the new Entry.
    *
    * @return true if the replacement was successful, false if the old
    *         entry changed before the replacement could take place.
    */
   virtual bool replaceEntry(
      EntryList* el, int idx, Entry* eOld, Entry* eNew);

   /**
    * Maps a key to a value in this HashTable.
    *
    * @param k the key.
    * @param v the value.
    * @param replace true to replace an existing value, false to abort if
    *        an existing value is found.
    * @param ptr the HazardPtr to use.
    *
    * @return true if the value was put in the table, false if not.
    */
   virtual bool put(const _K& k, const _V& v, bool replace, HazardPtr* ptr);

   /**
    * Gets the Entry that is mapped to the passed key. The owner EntryList
    * must be unref'd.
    *
    * @param ptr the HazardPtr to use.
    * @param k the key to get the value for.
    *
    * @return the Entry or NULL if none exists.
    */
   virtual Entry* getEntry(HazardPtr* ptr, const _K& k);

   /**
    * Resizes the table.
    *
    * @param ptr the hazard pointer to use.
    * @param el the current EntryList.
    * @param capacity the new capacity to use.
    */
   virtual void resize(HazardPtr* ptr, EntryList* el, int capacity);

   /**
    * Marks and/or collects garbage EntryLists.
    *
    * @param ptr the hazard pointer to use.
    */
   virtual void collectGarbage(HazardPtr* ptr);
};

template<typename _K, typename _V, typename _H, typename _E>
HashTable<_K, _V, _H, _E>::HashTable(int capacity) :
   mGarbageHead(NULL)
{
   // create first EntryList
   mHead = createEntryList(capacity);
}

template<typename _K, typename _V, typename _H, typename _E>
HashTable<_K, _V, _H, _E>::HashTable(const HashTable& copy) :
   mGarbageHead(NULL)
{
   // create the first EntryList
   mHead = createEntryList(copy.mHead->capacity);

   // acquire a hazard pointer in the copy
   HashTable& c = const_cast<HashTable&>(copy);
   HazardPtr* cPtr = c.mHazardPtrs.acquire();

   // get a hazard pointer for this table
   HazardPtr* ptr = mHazardPtrs.acquire();

   // iterate over every entry list in copy, putting every value
   EntryList* el = c.refNextEntryList(cPtr, NULL);
   while(el != NULL)
   {
      for(int i = 0; i < el->capacity; i++)
      {
         Entry* e = c.protectEntry(cPtr, el, i);
         if(e != NULL)
         {
            // if the entry type is Value, put it into this table
            if(e->type == Entry::Value)
            {
               // copy key and value and unprotect entry
               _K key = e->k;
               _V value = *(e->v);
               cPtr->value = NULL;
               put(key, value, false, ptr);
            }
            else
            {
               // unprotect entry
               cPtr->value = NULL;
            }
         }
      }

      // get the next entry list, drop reference to old list
      EntryList* next = c.refNextEntryList(cPtr, el);
      c.unrefEntryList(el);
      el = next;
   }

   // release the hazard pointers
   c.mHazardPtrs.release(cPtr);
   mHazardPtrs.release(ptr);
}

template<typename _K, typename _V, typename _H, typename _E>
HashTable<_K, _V, _H, _E>::~HashTable()
{
   // clean up all valid entry lists
   EntryList* el = const_cast<EntryList*>(mHead);
   while(el != NULL)
   {
      EntryList* tmp = el;
      el = el->next;
      freeEntryList(tmp);
   }

   // clean up all garbage lists
   el = const_cast<EntryList*>(mGarbageHead);
   while(el != NULL)
   {
      EntryList* tmp = el;
      el = el->garbageNext;
      freeEntryList(tmp);
   }
}

template<typename _K, typename _V, typename _H, typename _E>
HashTable<_K, _V, _H, _E>& HashTable<_K, _V, _H, _E>::operator=(
   const HashTable& rhs)
{
   // remove all entries from this table
   clear();

   // acquire a hazard pointer in the rhs
   HashTable& r = const_cast<HashTable&>(rhs);
   HazardPtr* rPtr = r.mHazardPtrs.acquire();

   // get a hazard pointer for this table
   HazardPtr* ptr = mHazardPtrs.acquire();

   // iterate over every entry list in rhs, putting every value
   EntryList* el = r.refNextEntryList(rPtr, NULL);
   while(el != NULL)
   {
      for(int i = 0; i < el->capacity; i++)
      {
         Entry* e = r.protectEntry(rPtr, el, i);
         if(e != NULL)
         {
            // if the entry type is Value, put it into this table
            if(e->type == Entry::Value)
            {
               // copy key and value and unprotect entry
               _K key = e->k;
               _V value = *(e->v);
               rPtr->value = NULL;
               put(key, value, false, ptr);
            }
            else
            {
               // unprotect entry
               rPtr->value = NULL;
            }
         }
      }

      // get the next entry list, drop reference to old list
      EntryList* next = r.refNextEntryList(rPtr, el);
      r.unrefEntryList(el);
      el = next;
   }

   // release the hazard pointers
   r.mHazardPtrs.release(rPtr);
   mHazardPtrs.release(ptr);

   return *this;
}

template<typename _K, typename _V, typename _H, typename _E>
bool HashTable<_K, _V, _H, _E>::put(const _K& k, const _V& v, bool replace)
{
   bool rval = false;

   // do put with an acquired hazard pointer
   HazardPtr* ptr = mHazardPtrs.acquire();
   rval = put(k, v, replace, ptr);
   mHazardPtrs.release(ptr);

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
bool HashTable<_K, _V, _H, _E>::get(const _K& k, _V& v)
{
   bool rval = false;

   // acquire a hazard pointer
   HazardPtr* ptr = mHazardPtrs.acquire();

   Entry* e = getEntry(ptr, k);
   if(e != NULL)
   {
      // get value, unprotect entry and unref its list
      v = *(e->v);
      EntryList* owner = e->owner;
      mHazardPtrs.release(ptr);
      unrefEntryList(owner);
      rval = true;
   }
   else
   {
      // release unused hazard pointer
      mHazardPtrs.release(ptr);
   }

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
bool HashTable<_K, _V, _H, _E>::remove(const _K& k)
{
   bool rval = false;

   // acquire a hazard pointer
   HazardPtr* ptr = mHazardPtrs.acquire();

   // loop until a value is set to a tombstone or the entry is not found
   bool done = false;
   while(!done)
   {
      // get the entry
      Entry* e = getEntry(ptr, k);
      if(e != NULL)
      {
         // set type to Tombstone (it must have been Value to be returned
         // from get())
         if(Atomic::compareAndSwap(&e->type, Entry::Value, Entry::Tombstone))
         {
            // decrease list's length
            Atomic::decrementAndFetch(&e->owner->length);
            rval = done = true;
         }

         // unreference entry and its list
         EntryList* owner = e->owner;
         ptr->value = NULL;
         unrefEntryList(owner);
      }
      else
      {
         done = true;
      }
   }

   // release the hazard pointer
   mHazardPtrs.release(ptr);

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
void HashTable<_K, _V, _H, _E>::clear()
{
   // acquire a hazard pointer
   HazardPtr* ptr = mHazardPtrs.acquire();

   // iterate over every entry list, removing every value
   EntryList* el = refNextEntryList(ptr, NULL);
   while(el != NULL)
   {
      for(int i = 0; i < el->capacity; i++)
      {
         // loop until a value is set to a tombstone or the entry is not found
         bool done = false;
         while(!done)
         {
            Entry* e = protectEntry(ptr, el, i);
            if(e != NULL)
            {
               // if the entry type is Value, set it to Tombstone
               if(e->type == Entry::Value)
               {
                  // set type to tombstone
                  if(Atomic::compareAndSwap(
                     &e->type, Entry::Value, Entry::Tombstone))
                  {
                     // decrease list's length
                     Atomic::decrementAndFetch(&e->owner->length);
                     done = true;
                  }
               }
               // any other type means we're done with this entry
               else
               {
                  done = true;
               }

               // unprotect entry
               ptr->value = NULL;
            }
            else
            {
               // no entry to clear
               done = true;
            }
         }
      }

      // get the next entry list, drop reference to old list
      EntryList* next = refNextEntryList(ptr, el);
      unrefEntryList(el);
      el = next;
   }

   // release the hazard pointer
   mHazardPtrs.release(ptr);
}

template<typename _K, typename _V, typename _H, typename _E>
int HashTable<_K, _V, _H, _E>::length()
{
   int rval = 0;

   // acquire a hazard pointer
   HazardPtr* ptr = mHazardPtrs.acquire();

   // iterate over every entry list, adding lengths
   EntryList* el = refNextEntryList(ptr, NULL);
   while(el != NULL)
   {
      // add length
      rval += el->length;

      // get the next entry list, drop reference to old list
      EntryList* next = refNextEntryList(ptr, el);
      unrefEntryList(el);
      el = next;
   }

   // release the hazard pointer
   mHazardPtrs.release(ptr);

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
struct HashTable<_K, _V, _H, _E>::EntryList*
HashTable<_K, _V, _H, _E>::createEntryList(int capacity)
{
   EntryList* el = static_cast<EntryList*>(
      Atomic::mallocAligned(sizeof(EntryList)));
   el->refCount = 0;
   el->garbageEntries = NULL;
   el->freeEntries = NULL;
   el->capacity = capacity;
   el->length = 0;
   el->entries = static_cast<Entry**>(calloc(capacity, sizeof(Entry*)));
   el->next = NULL;
   el->old = false;
   el->garbageNext = NULL;
   return el;
};

template<typename _K, typename _V, typename _H, typename _E>
void HashTable<_K, _V, _H, _E>::freeEntryList(EntryList* el)
{
   // free all live entries
   for(int i = 0; i < el->capacity; i++)
   {
      Entry* e = el->entries[i];
      if(e != NULL)
      {
         freeEntry(e);
      }
   }

   // free all garbage entries
   {
      Entry* e = const_cast<Entry*>(el->garbageEntries);
      while(e != NULL)
      {
         Entry* tmp = e;
         e = e->next;
         freeEntry(tmp);
      }
   }

   // free all free list entries
   {
      Entry* e = const_cast<Entry*>(el->freeEntries);
      while(e != NULL)
      {
         Entry* tmp = e;
         e = e->next;
         freeEntry(tmp);
      }
   }

   // free entries array
   free(el->entries);

   // free list
   Atomic::freeAligned(el);
};

template<typename _K, typename _V, typename _H, typename _E>
struct HashTable<_K, _V, _H, _E>::Entry*
HashTable<_K, _V, _H, _E>::createEntry(
   EntryList* el, const _K& key, const _V& value)
{
   Entry* e = NULL;

   // try to reuse an Entry from the free list
   if(el->freeEntries != NULL)
   {
      Entry* head;
      do
      {
         head = const_cast<Entry*>(el->freeEntries);
      }
      while(!Atomic::compareAndSwap(&el->freeEntries, head, (Entry*)NULL));

      if(head != NULL)
      {
         e = head;
      }
   }

   if(e == NULL)
   {
      // create a new Entry, none were found on the free list
      e = static_cast<Entry*>(malloc(sizeof(Entry)));
      e->v = new _V(value);
   }
   else
   {
      *(e->v) = value;
   }

   e->type = Entry::Value;
   e->k = key;
   e->h = mHashFunction(key);
   e->owner = el;
   e->next = NULL;

   return e;
};

template<typename _K, typename _V, typename _H, typename _E>
void HashTable<_K, _V, _H, _E>::freeEntry(Entry* e)
{
   if(e->v != NULL)
   {
      delete e->v;
   }
   free(e);
}

template<typename _K, typename _V, typename _H, typename _E>
struct HashTable<_K, _V, _H, _E>::EntryList*
HashTable<_K, _V, _H, _E>::refNextEntryList(HazardPtr* ptr, EntryList* prev)
{
   EntryList* rval = NULL;

   if(prev == NULL)
   {
      do
      {
         // attempt to protect the head EntryList with a hazard pointer
         ptr->value = const_cast<EntryList*>(mHead);
      }
      // ensure the head hasn't changed
      while(ptr->value != mHead);

      // set return value
      rval = static_cast<EntryList*>(ptr->value);
   }
   else
   {
      /* The previous list is protected with a reference count and its
         next pointer will only be investigated if we're looking to find
         the end of the list or if we've been guaranteed (i.e. we found
         a Sentinel Entry in the current list) to find a next EntryList.
         Therefore, we don't need to do any special checks here. */
      rval = prev->next;
   }

   if(rval != NULL)
   {
      // increment reference count, clear hazard pointer
      Atomic::incrementAndFetch(&rval->refCount);
      ptr->value = NULL;
   }

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
void HashTable<_K, _V, _H, _E>::unrefEntryList(EntryList* el)
{
   // decrement reference count
   Atomic::decrementAndFetch(&el->refCount);
}

template<typename _K, typename _V, typename _H, typename _E>
struct HashTable<_K, _V, _H, _E>::Entry*
HashTable<_K, _V, _H, _E>::protectEntry(HazardPtr* ptr, EntryList* el, int idx)
{
   Entry* rval = NULL;

   do
   {
      // attempt to protect the Entry with a hazard pointer
      ptr->value = el->entries[idx];
   }
   // ensure the Entry hasn't changed
   while(ptr->value != el->entries[idx]);

   // set return value
   rval = static_cast<Entry*>(ptr->value);

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
struct HashTable<_K, _V, _H, _E>::EntryList*
HashTable<_K, _V, _H, _E>::getCurrentEntryList(HazardPtr* ptr)
{
   EntryList* rval = NULL;

   /* Note: A call to refNextEntryList(ptr, NULL) will never return NULL
      per the design of this HashTable. There is always a head EntryList. */

   // get the tail
   rval = refNextEntryList(ptr, NULL);
   while(rval->next != NULL)
   {
      // get the next entry list, drop reference to old list
      EntryList* next = refNextEntryList(ptr, rval);
      unrefEntryList(rval);
      rval = next;
   }

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
bool HashTable<_K, _V, _H, _E>::replaceEntry(
   EntryList* el, int idx, Entry* eOld, Entry* eNew)
{
   bool rval = false;

   rval = Atomic::compareAndSwap(el->entries + idx, eOld, eNew);
   if(rval)
   {
      if(eOld == NULL || eOld->type == Entry::Tombstone)
      {
         // increment list length
         Atomic::incrementAndFetch(&el->length);
      }

      // isolate the old garbage list, making it private to this thread
      Entry* head = NULL;
      if(el->garbageEntries != NULL)
      {
         // if we fail to isolate the garbage list, then move on, someone
         // else is handling it in another thread
         head = const_cast<Entry*>(el->garbageEntries);
         if(!Atomic::compareAndSwap(
            &el->garbageEntries, head, (Entry*)NULL))
         {
            // let the other thread handle the garbage entries
            head = NULL;
         }
      }

      /* Move unused entries in the old list into a private free list for
         entries. That private list will be prepended to the shared free list
         when we're finished here. Build a new list from the entries that are
         still in use. Keep track of the tail of the new list so we can append
         eOld to it when we're finished. */
      Entry* e = head;
      Entry* tail;
      Entry* next;
      Entry* freeHead;
      Entry* freeTail;
      head = tail = freeHead = freeTail = NULL;
      while(e != NULL)
      {
         // save next and then clear it
         next = e->next;
         e->next = NULL;

         /* Note: The next garbage entry is not in use if there's no hazard
            pointer to it. Since the entry has been removed from any shared
            list, no other threads can start trying to access it. There can
            only be those threads that were already accessing it with the
            protection of a hazard pointer. */
         if(!mHazardPtrs.isProtected(e))
         {
            if(freeHead == NULL)
            {
               // start the new free list
               freeHead = freeTail = e;
            }
            else
            {
               // append to the new free list
               freeTail->next = e;
               freeTail = e;
            }
         }
         else if(head == NULL)
         {
            // start the new garbage list
            head = tail = e;
         }
         else
         {
            // append to the new garbage list
            tail->next = e;
            tail = e;
         }

         // go to next garbage entry
         e = next;
      }

      // append old entry to the new garbage list
      if(eOld != NULL)
      {
         if(tail == NULL)
         {
            // start the new garbage list
            head = tail = eOld;
         }
         else
         {
            // append to the new garbage lits
            tail->next = eOld;
            tail = eOld;
         }
      }

      // prepend private free list to the shared free list
      if(freeHead != NULL)
      {
         Entry* oldHead;
         do
         {
            oldHead = const_cast<Entry*>(el->freeEntries);
            freeTail->next = oldHead;
         }
         while(!Atomic::compareAndSwap(&el->freeEntries, oldHead, freeHead));
      }

      // prepend private garbage list to the shared garbage list
      if(head != NULL)
      {
         Entry* oldHead;
         do
         {
            oldHead = const_cast<Entry*>(el->garbageEntries);
            tail->next = oldHead;
         }
         while(!Atomic::compareAndSwap(&el->garbageEntries, oldHead, head));
      }
   }

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
bool HashTable<_K, _V, _H, _E>::put(
   const _K& k, const _V& v, bool replace, HazardPtr* ptr)
{
   bool rval = false;

   /* Steps:

      1. Enter a spin loop that runs until the value is inserted.
      2. Create the entry for the table (if it hasn't been created yet).
      3. Use the newest entries array.
      4. Find the index to insert at.
      5. Do a CAS to insert.
      6. CAS may fail if another thread slipped in and modified the value
         before us, in which case we optimize by dropping the update.

      Note: We may need to resize the table or we may discover that an old
      entry is a sentinel, in which case we must insert into a new table.
   */

   Entry* eNew = NULL;

   // enter a spin loop that keep trying to insert while we haven't attempted
   // an insert yet
   bool inserted = false;
   bool insertAttempted = false;
   while(!insertAttempted)
   {
      // use the newest entries array and max index
      EntryList* el = getCurrentEntryList(ptr);
      int maxIdx = el->capacity - 1;

      // create value entry as necessary
      if(eNew == NULL)
      {
         eNew = createEntry(el, k, v);
      }
      else
      {
         // update EntryList owner of the Entry
         eNew->owner = el;
      }

      // enter another spin loop to keep trying to insert while:
      // 1. we haven't decided we need a new table
      // 2. we haven't attempted an insert
      bool mustResize = false;
      bool useNewTable = false;
      int i = eNew->h & maxIdx;
      while(!useNewTable && !insertAttempted)
      {
         // get any existing entry directly from the current list
         Entry* eOld = protectEntry(ptr, el, i);
         if(eOld == NULL)
         {
            // there is no existing entry so try to insert ...
            // replace entry may fail because some other thread just
            // barely beat us here ... but we optimize by saying we
            // actually lost the fight and were overwritten
            replaceEntry(el, i, eOld, eNew);
            inserted = insertAttempted = true;
         }
         else if(eOld->type == Entry::Sentinel)
         {
            // a sentinel entry tells us that there is a newer table where
            // we have to do the insert
            useNewTable = true;
         }
         // existing entry is replaceable value or a tombstone
         else
         {
            // if the hashes match and the keys are equal, then we can replace
            // the existing entry
            if(eOld->h == eNew->h && mEqualsFunction(eOld->k, k))
            {
               // we can only replace the existing entry if its a tombstone
               // or if the replace flag is setting allowing us to replace
               // any existing value
               if(eOld->type == Entry::Tombstone || replace)
               {
                  // replace entry may fail because some other thread just
                  // barely beat us here ... but we optimize by saying we
                  // actually lost the fight and were overwritten
                  replaceEntry(el, i, eOld, eNew);
                  inserted = true;
               }
               insertAttempted = true;
            }
            else if(i == maxIdx)
            {
               // keys did not match and we've reached the end of the table, so
               // we must use a new table
               useNewTable = true;
               mustResize = true;
            }
            else
            {
               // keys did not match, so we found a collision, increase the
               // index by 1 and try again
               i++;
            }
         }

         // unprotect old entry
         ptr->value = NULL;
      }

      if(inserted)
      {
         // insert was successful
         rval = true;
      }
      else if(mustResize)
      {
         // resize and then try insert again
         // FIXME: use a better capacity increase formula
         int capacity = (int)(el->capacity * 1.5 + 1);
         if(capacity % 2 == 0)
         {
            // ensure capacity is always odd so that when we are reprobing
            // we won't strip odd indexes
            capacity++;
         }
         resize(ptr, el, capacity);
      }

      // unreference the current list
      unrefEntryList(el);
   }

   // FIXME: run garbage collection how often? also run in get()?
   collectGarbage(ptr);

   if(!rval)
   {
      // clean up created entry
      freeEntry(eNew);
   }

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
struct HashTable<_K, _V, _H, _E>::Entry*
HashTable<_K, _V, _H, _E>::getEntry(HazardPtr* ptr, const _K& k)
{
   Entry* rval = NULL;

   /* Search for an entry with a key that matches or determine there is no
      entry with the given key. If we find an entry that matches the hash
      for our key, but the key does not match, then we must reprobe. We do
      so by incrementing our index by 1 since we handle collisions when
      inserting by simply increasing the index by 1 until we find an empty
      slot. */

   /* Note: The next pointer in any EntryList is only initialized to NULL,
      never later set to it. A value of NULL indicates that the EntryList
      is the last one in this HashTable. Any other value indicates that
      there is another EntryList with newer values. If an older list has
      a Sentinel Entry, then there is guaranteed to be another list.
      Therefore, there is a guarantee that following a non-NULL next pointer
      will always result in finding a newer list, even if the current list
      has been marked as garbage and is waiting for all references to it to
      be removed so that it can be collected. */

   /* Iterate through each EntryList until a non-Sentinel is found. If an
      old Value entry is found, we copy it to the new list. If an Entry with
      a matching key is found, we return the Entry if it is a Value and we
      return NULL if it is a Tombstone.
      Note: The entry and its list will have their reference counts increased,
      protecting them from being freed. */
   bool done = false;
   int hash = mHashFunction(k);
   EntryList* el = refNextEntryList(ptr, NULL);
   while(!done && el != NULL)
   {
      // check the entries for the given index, capped at maxIdx
      bool listDone = false;
      int maxIdx = el->capacity - 1;
      for(int i = hash & maxIdx; !listDone && i < el->capacity; i++)
      {
         Entry* e = protectEntry(ptr, el, i);
         if(e == NULL)
         {
            // there is no such entry in this list
            listDone = true;
         }
         else if(e->type != Entry::Sentinel)
         {
            /* If we found a value in an old list then we're responsible for
               attempting to copy that value to the most current list. We
               attempt to do a put here, but we don't care if we fail. If we
               fail, then someone else has already put the value or an even
               newer value into the current list for us. We simply mark the
               old entry as a Sentinel. */
            if(e->type == Entry::Value && el->old)
            {
               /* Note: Do not reuse the existing hazard pointer, it is being
                  used to protect the current entry. */
               put(e->k, *(e->v), false);

               // set entry type to a sentinel
               if(Atomic::compareAndSwap(
                  &e->type, Entry::Value, Entry::Sentinel))
               {
                  // decrement list length
                  Atomic::decrementAndFetch(&el->length);
               }
            }
            /* If the hashes match and the keys are equal, then we found the
               value we want or a tombstone. If they don't, we'll reprobe via
               the for-loop. */
            else if(e->h == hash && mEqualsFunction(e->k, k))
            {
               // if the entry is a value, return it
               if(e->type == Entry::Value)
               {
                  rval = e;
                  done = true;
               }
               listDone = true;
            }
         }

         // only unprotect the entry if we aren't returning it
         if(rval == NULL && e != NULL)
         {
            ptr->value = NULL;
         }
      }

      if(rval == NULL)
      {
         // get the next entry list, drop reference to the old list
         EntryList* next = refNextEntryList(ptr, el);
         unrefEntryList(el);
         el = next;
      }
   }

   return rval;
}

template<typename _K, typename _V, typename _H, typename _E>
void HashTable<_K, _V, _H, _E>::resize(
   HazardPtr* ptr, EntryList* el, int capacity)
{
   /* Note: When we call resize(), other threads might also be trying to resize
      at the same time. Therefore, we allocate a new EntryList and then try to
      CAS it onto the current EntryList. If it fails, then someone else has
      already done the resize for us and we should deallocate the EntryList
      we created and return. If it succeeds, we appended our EntryList as the
      newest EntryList. Note that we could not have appended our EntryList onto
      an EntryList that was marked as garbage because any EntryList that is
      marked as garbage must have a next pointer that is not NULL because that
      would make it the newest list, which is never garbage. In either case,
      the old current EntryList is marked as an old list. By marking the list
      as old, subsequent calls to put() know to mark entries in the old list
      as Sentinels.

      Keep in mind that when resize() is called from put(), it is from within
      a loop that will keep trying to put() the key/value pair into the table
      until it is successful. If this resize fails for any reason, another
      resize will have succeeded to provide room for the put() or this will
      be called again until it succeeds.
    */

   // only bother resizing if our current list isn't already old
   if(!el->old)
   {
      // create the new entry list
      EntryList* newList = createEntryList(capacity);

      // try to swap in the new list
      if(Atomic::compareAndSwap(&el->next, (EntryList*)NULL, newList))
      {
         // success, mark the old current list as old
         el->old = true;
      }
      else
      {
         // failure, free the list we allocated
         freeEntryList(newList);
      }
   }
}

template<typename _K, typename _V, typename _H, typename _E>
void HashTable<_K, _V, _H, _E>::collectGarbage(HazardPtr* ptr)
{
   // temporarily isolate the entire garbage list to this thread
   EntryList* privateHead = NULL;
   EntryList* privateTail = NULL;
   if(mGarbageHead != NULL)
   {
      // only do garbage collection if we successfully isolate the garbage
      // list, otherwise we optimize this out as someone else happens to
      // be doing collection at the same time as we are
      EntryList* head = const_cast<EntryList*>(mGarbageHead);
      if(Atomic::compareAndSwap(&mGarbageHead, head, (EntryList*)NULL))
      {
         // keep up to max garbage lists in a private list and get the tail for
         // garbage to be added back to the shared list
         EntryList* tail = NULL;
         const int max = 3;
         EntryList* next = head;
         int i = 0;
         while(next != NULL)
         {
            if(i < max)
            {
               if(privateHead == NULL)
               {
                  privateHead = head;
               }
               privateTail = head;
               head = head->garbageNext;
               privateTail->garbageNext = NULL;
               i++;
            }

            if(next->garbageNext == NULL)
            {
               // found tail
               tail = next;
            }
            next = next->garbageNext;
         }

         // prepend remaining garbage back onto the shared list
         if(head != NULL)
         {
            EntryList* oldHead;
            do
            {
               oldHead = const_cast<EntryList*>(mGarbageHead);
               tail->garbageNext = oldHead;
            }
            while(!Atomic::compareAndSwap(&mGarbageHead, oldHead, head));
         }
      }
   }

   /* Use a hazard pointer to protect the first EntryList (we optimize out
      incrementing the reference count here). If it is old, has a length of 0,
      and a ref count of 0, then we can mark it as garbage. This is safe to do.
      Suppose another thread is just about to increment the ref count or does
      so while we're marking this list as garbage. Since the old flag was set
      before ref count was incremented, the put() call gaurantees that no new
      entries will be written to the EntryList. It can only be read from. */
   // attempt to protect the head EntryList with a hazard pointer
   ptr->value = const_cast<EntryList*>(mHead);
   // ensure the head hasn't changed, if it has, someone else is handling GC
   // and we don't bother with it
   if(ptr->value == mHead)
   {
      EntryList* el = static_cast<EntryList*>(ptr->value);
      if(el->old && el->length == 0 && el->refCount == 0)
      {
         // if we fail to remove the list from the head, then someone else has
         // done our work for us
         if(Atomic::compareAndSwap(&mHead, el, el->next))
         {
            // we are responsible for marking the list as garbage, so move it
            // to our private garbage list
            if(privateTail == NULL)
            {
               // start the private list
               privateHead = privateTail = el;
            }
            else
            {
               // append to the private list
               privateTail->garbageNext = el;
               privateTail = el;
            }
         }
      }
   }
   // clear hazard pointer
   ptr->value = NULL;

   // clean up the private garbage list as much as possible
   EntryList* next = privateHead;
   while(next != NULL)
   {
      // we can clean up the list if its reference count is 0, it is not
      // protected by the hazard pointer list, and then check again to
      // ensure the ref count hasn't increased during the protection check
      if(next->refCount == 0 &&
         !mHazardPtrs.isProtected(next) &&
         next->refCount == 0)
      {
         // update private head and tail
         if(privateHead == privateTail)
         {
            privateHead = privateTail = NULL;
         }
         else
         {
            privateHead = next->garbageNext;
         }

         // save next entry list
         EntryList* tmp = next;
         next = next->garbageNext;

         // free entry list
         freeEntryList(tmp);
      }
   }

   // prepend remaining private garbage back onto the shared list
   if(privateHead != NULL)
   {
      EntryList* oldHead;
      do
      {
         oldHead = const_cast<EntryList*>(mGarbageHead);
         privateTail->garbageNext = oldHead;
      }
      while(!Atomic::compareAndSwap(&mGarbageHead, oldHead, privateHead));
   }
}

} // end namespace rt
} // end namespace monarch
#endif
