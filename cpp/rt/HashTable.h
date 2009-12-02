/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_HashTable_H
#define db_rt_HashTable_H

#include "db/rt/Atomic.h"
#include "db/rt/HazardPtrList.h"

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
 * This HashTable is stored as a linked-list of entry blocks. Each entry block
 * is referred to as an EntryList. Each entry block constitutes a single
 * hash table. There are multiple EntryLists so that this HashTable can be
 * resized. During a resize, operations to get/put on the HashTable will
 * need to iterate over this list. When a resize is complete, old EntryLists
 * will be freed. To enable this behavior in a lock-free manner, a combination
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
 * GET: Entry X is still in the valid list (it is not).
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
   struct EntryList;
   struct Entry
   {
      // types of entries
      enum Type
      {
         Value,
         Sentinel,
         Tombstone
      };

      // data in the entry
      Type type;
      _K k;
      int h;
      _V* v;
      EntryList* owner;
      Entry* garbageNext;
   };

   /**
    * An entry list has an array of entries, a capacity, and a pointer to
    * the next, newer EntryList (or NULL).
    */
   struct EntryList
   {
      /* Note: Reference count for an entry, note that an assumption is made
         here that since an entry has to be heap-allocated and 32-bit aligned,
         so will the first value of the struct be automatically 32-bit aligned,
         but we put the special alignment in here anyway. */
      aligned_uint32_t refCount;
#ifdef WIN32
      /* MS Windows requires any variable written to in an atomic operation
         to be aligned to the address size of the CPU. */
      volatile Entry* garbageEntries __attribute__ ((aligned(4)));
#else
      volatile Entry* garbageEntries;
#endif
      Entry** entries;
      int capacity;
      int length;
      EntryList* next;
      bool old;
      bool garbage;
      EntryList* garbageNext;
   };

   /**
    * The entry first list.
    */
#ifdef WIN32
   /* MS Windows requires any variable written to in an atomic operation
      to be aligned to the address size of the CPU. */
   volatile EntryList* mHead __attribute__ ((aligned(4)));
#else
   volatile EntryList* mHead;
#endif

   /**
    * A hazard pointer list for protecting access to entry lists.
    */
   HazardPtrList mHazardPtrs;

   /**
    * The maximum capacity of the hash table, meaning the maximum number of
    * entries that could be stored in it at present. The capacity will grow
    * as necessary.
    */
   int mCapacity;

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
    * @param key the Entry key.
    * @param value the Entry value.
    *
    * @return the new Entry.
    */
   virtual Entry* createValue(const _K& key, const _V& value);

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
    * @param el the previous entry list, NULL to use the head.
    *
    * @return the next EntryList (can be NULL).
    */
   virtual EntryList* refNextEntryList(HazardPtr* ptr, EntryList* prev);

   /**
    * Decreases the reference count on the given entry list.
    *
    * @param el the entry list to decrease the reference count on.
    */
   virtual void unrefEntryList(Entry* el);

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
    * Gets the entry at the given index in the table. The EntryList owner of
    * the Entry must be unref'd when the Entry is no longer in use.
    *
    * @param ptr the hazard pointer to use.
    * @param idx the index of the entry to retrieve.
    *
    * @return the entry or NULL if no such entry exists.
    */
   virtual Entry* getEntry(HazardPtr* ptr, int idx);

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
    * @param k the key to get the value for.
    * @param e the entry with the value.
    *
    * @return true if the key was found, false if not.
    */
   virtual bool get(const _K& k, Entry* e);

   /**
    * Resizes the table.
    *
    * @param ptr the hazard pointer to use.
    * @param el the current EntryList.
    * @param capacity the new capacity to use.
    */
   virtual void resize(HazardPtr* ptr, EntryList* el, int capacity);
};

template<typename _K, typename _V, typename _H>
HashTable<_K, _V, _H>::HashTable(int capacity) :
   mCapacity(capacity)
{
   // create first EntryList
   mHead = createEntryList(capacity);
}

template<typename _K, typename _V, typename _H>
HashTable<_K, _V, _H>::HashTable(const HashTable& copy) :
   mCapacity(copy.mCapacity)
{
   // create the first EntryList
   mHead = createEntryList(mCapacity);

   // FIXME: iterate over the copy and put all of its entries
}

template<typename _K, typename _V, typename _H>
HashTable<_K, _V, _H>::~HashTable()
{
   // clean up all entry lists
   EntryList* el = mHead;
   while(el != NULL)
   {
      EntryList* tmp = el;
      el = el->next;
      freeEntryList(tmp);
   }
}

template<typename _K, typename _V, typename _H>
HashTable<_K, _V, _H>& HashTable<_K, _V, _H>::operator=(
   const HashTable& rhs)
{
   // FIXME: clear this table

   // FIXME: iterate over the copy and put all of its entries

   return *this;
}

template<typename _K, typename _V, typename _H>
bool HashTable<_K, _V, _H>::put(const _K& k, const _V& v, bool replace)
{
   bool rval = false;

   // do put with an acquired hazard pointer
   HazardPtr* ptr = mHazardPtrs.acquire();
   rval = put(k, v, replace, ptr);
   mHazardPtrs.release(ptr);

   return rval;
}

template<typename _K, typename _V, typename _H>
bool HashTable<_K, _V, _H>::get(const _K& k, _V& v)
{
   bool rval = false;

   Entry* e;
   rval = get(k, e);
   if(rval)
   {
      // get value, unreference entry's list
      v = *(e->v);
      unrefEntryList(e->owner);
   }

   return rval;
}

template<typename _K, typename _V, typename _H>
bool HashTable<_K, _V, _H>::remove(const _K& k)
{
   bool rval = false;

   // loop until a value is set to a tombstone or the entry is not found
   bool done = false;
   while(!done)
   {
      // get the entry
      Entry* e;
      if(get(k, e))
      {
         // set type to tombstone
         if(Atomic::compareAndSwap(&e->type, Entry::Value, Entry::Tombstone))
         {
            // decrease list's length
            Atomic::decrementAndFetch(&e->owner->length);
            rval = done = true;
         }

         // unreference entry's list
         unrefEntryList(e->owner);
      }
      else
      {
         done = true;
      }
   }

   return rval;
}

template<typename _K, typename _V, typename _H>
int HashTable<_K, _V, _H>::length()
{
   int rval = 0;

   // acquire a hazard pointer
   HazardPtr* ptr = mHazardPtrs.acquire();

   // iterate over every entry list, adding lengths
   EntryList* el = refNextEntryList(ptr, NULL);
   while(el->next != NULL)
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

template<typename _K, typename _V, typename _H>
struct HashTable<_K, _V, _H>::EntryList*
HashTable<_K, _V, _H>::createEntryList(int capacity)
{
   EntryList* el = static_cast<EntryList*>(
      Atomic::mallocAligned(sizeof(EntryList)));
   el->refCount = 0;
   el->garbageEntries = NULL;
   el->capacity = capacity;
   el->length = 0;
   el->entries = static_cast<Entry**>(malloc(capacity));
   memset(el->entries, NULL, capacity);
   el->next = NULL;
   el->old = false;
   el->garbage = false;
   el->garbageNext = NULL;
   return el;
};

template<typename _K, typename _V, typename _H>
void HashTable<_K, _V, _H>::freeEntryList(EntryList* el)
{
   // free all live entries
   for(int i = 0; i < el->capacity; i++)
   {
      Entry* e = el->entries[i];
      if(e != NULL)
      {
         if(e->v != NULL)
         {
            delete e->v;
         }
         free(e);
      }
   }

   // free all garbage entries
   for(Entry* e = el->garbageEntries; e != NULL; e = e->garbageNext)
   {
      if(e->v != NULL)
      {
         delete e->v;
      }
      free(e);
   }

   // free list
   Atomic::freeAligned(el);
};

template<typename _K, typename _V, typename _H>
struct HashTable<_K, _V, _H>::Entry*
HashTable<_K, _V, _H>::createValue(const _K& key, const _V& value)
{
   Entry* e = static_cast<Entry*>(malloc(sizeof(Entry)));
   e->type = Entry::Value;
   e->k = key;
   e->h = mHashFunction(key);
   e->v = new _V(value);
   e->garbageNext = NULL;
   return e;
};

template<typename _K, typename _V, typename _H>
struct HashTable<_K, _V, _H>::EntryList*
HashTable<_K, _V, _H>::refNextEntryList(HazardPtr* ptr, EntryList* prev)
{
   EntryList* rval = NULL;

   if(prev == NULL)
   {
      do
      {
         // attempt to protect the head EntryList with a hazard pointer
         ptr->value = mHead;
      }
      // ensure the head hasn't changed
      while(ptr->value != mHead);
   }
   else
   {
      /* The previous list is protected with a reference count and its
         next pointer will only be investigated if we're looking to find
         the end of the list or if we've been guaranteed (i.e. we found
         a Sentinel Entry in the current list) to find a next EntryList.
         Therefore, we don't need to do any special checks here. */
      rval = ptr->next;
   }

   if(ptr->value != NULL)
   {
      // set return value and increment reference count
      rval = static_cast<EntryList*>(ptr->value);
      Atomic::incrementAndFetch(&rval->refCount);
   }

   return rval;
}

template<typename _K, typename _V, typename _H>
void HashTable<_K, _V, _H>::unrefEntryList(Entry* el)
{
   // decrement reference count
   Atomic::decrementAndFetch(&el->refCount);
}

template<typename _K, typename _V, typename _H>
struct HashTable<_K, _V, _H>::EntryList*
HashTable<_K, _V, _H>::getCurrentEntryList(HazardPtr* ptr)
{
   EntryList* rval = NULL;

   /* Note: A call to refNextEntryList(ptr, NULL) will never return NULL
      per the design of this HashTable. There is always a head EntryList. */

   // get the tail
   EntryList* el = refNextEntryList(ptr, NULL);
   while(el->next != NULL)
   {
      // get the next entry list, drop reference to old list
      EntryList* next = refNextEntryList(ptr, el);
      unrefEntryList(el);
      el = next;
   }

   return rval;
}

template<typename _K, typename _V, typename _H>
bool HashTable<_K, _V, _H>::replaceEntry(
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

      // FIXME: we need a way to clean up this garbage list other than
      // just whenever the owner EntryList gets cleaned up ... every time
      // a value is replaced for the same key, a garbage entry is going
      // to be created

      // move old entry to garbage list
      if(eOld != NULL)
      {
         do
         {
            eOld->garbageNext = el->garbageEntries;
         }
         while(!Atomic::compareAndSwap(
               el->garbageEntries, eOld->garbageNext, eOld));
      }
   }

   return rval;
}

template<typename _K, typename _V, typename _H>
struct HashTable<_K, _V, _H>::Entry*
HashTable<_K, _V, _H>::getEntry(HazardPtr* ptr, int idx)
{
   Entry* rval = NULL;

   /* Note: The next pointer in any EntryList is only initialized to NULL,
      never later set to it. A value of NULL indicates that the EntryList
      is the last one in this HashTable. Any other value indicates that
      there is another EntryList with newer values. If an older list has
      a Sentinel Entry, then there is guaranteed to be another list. Therefore,
      there is a guarantee that following a non-NULL next pointer will always
      result in finding a newer list, even if the current list has been marked
      as garbage and is waiting for all references to it to be removed so
      that it can be collected. */

   /* Iterate through each EntryList until a non-old Value entry is found.
      Note: The entry's owner will have its reference count increased,
      protecting it from being freed. */
   bool found = false;
   EntryList* el = refNextEntryList(ptr, NULL);
   while(!found && el != NULL)
   {
      // check the entries for the given index
      if(idx < el->capacity)
      {
         Entry* e = el->entries[idx];
         if(e != NULL && e->type == Entry::Value)
         {
            if(!e->old)
            {
               // the list is not old, so we found an up-to-date value
               found = true;
               rval = e;
            }
            else
            {
               /* We found a value in an old list so we're responsible for
                  attempting to copy that value to the most current list.
                  We attempt to do a put here, but we don't care if we
                  fail. If we fail, then someone else has already put
                  the value or an even newer value into the current list for
                  us. We simply mark the old entry as a Sentinel.
                */
               put(e->k, *(e->v), false, ptr);

               // set entry type to a sentinel
               if(Atomic::compareAndSwap(
                  &e->type, Entry::Value, Entry::Tombstone))
               {
                  // decrement list length
                  Atomic::decrementAndFetch(el->length);
               }
            }
         }
      }

      if(!found)
      {
         // get the next entry list, drop reference to the old list
         EntryList* next = refNextEntryList(ptr, el);
         unrefEntryList(el);
         el = next;
      }
   }

   return rval;
}

template<typename _K, typename _V, typename _H>
bool HashTable<_K, _V, _H>::put(
   const _K& k, const _V& v, bool replace, HazardPtr* ptr)
{
   bool rval = false;

   /* Steps:

      1. Create the entry for the table.
      2. Enter a spin loop that runs until the value is inserted.
      3. Use the newest entries array.
      4. Find the index to insert at.
      5. Do a CAS to insert.
      6. CAS may fail if another thread slipped in and modified the value
         before us, so loop and try again.

      Note: We may need to resize the table or we may discover that an old
      entry is a sentinel, in which case we must insert into a new table.
    */

   // create a value entry
   Entry* eNew = createValue(k, v);

   // enter a spin loop that keep trying to insert while:
   // 1. we haven't inserted yet AND
   // 2. we're replacing existing values OR we haven't attempted an insert
   bool inserted = false;
   bool insertAttempted = false;
   while(!inserted && (replace || !insertAttempted))
   {
      // use the newest entries array and its max index
      EntryList* el = getCurrentEntryList(ptr);
      int maxIdx = el->capacity - 1;

      // set EntryList owner of the Entry
      eNew->owner = el;

      // enter another spin loop to keep trying to insert while:
      // 1. we haven't decided we need a new table
      // 2. we haven't inserted AND
      // 3. we're replacing existing values OR we haven't attempted an insert
      bool useNewTable = false;
      int i = eNew->h;
      while(!useNewTable && !inserted && (replace || !insertAttempted))
      {
         // limit index to the max index
         i &= maxIdx;

         // get any existing entry directly from the current list
         Entry* eOld = el->entries[i];
         if(eOld == NULL)
         {
            // there is no existing entry so try to insert
            inserted = replaceEntry(el + i, eOld, eNew);
            insertAttempted = true;
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
            // if the entry key is at the same memory address or if the hashes
            // match and the keys match, then we can replace the existing
            // entry
            if(&(eOld->k) == &k || (eOld->h == eNew->h && eOld->k == k))
            {
               inserted = replaceEntry(el + i, eOld, eNew);
               insertAttempted = true;
            }
            else if(i == maxIdx)
            {
               // keys did not match and we've reached the end of the table, so
               // we must use a new table
               useNewTable = true;
            }
            else
            {
               // keys did not match, so we found a collision, increase the
               // index by 1 and try again
               i++;
            }
         }
      }

      if(inserted)
      {
         // insert was successful
         rval = true;
      }
      else if(useNewTable)
      {
         // resize and then try insert again
         // FIXME: use a better capacity increase formula
         int capacity = (int)(el->capacity * 1.5 + 1);
         resize(ptr, el, capacity);
      }

      // unreference the current list
      unrefEntryList(el);
   }

   // FIXME: run garbage collection? how often?

   return rval;
}

template<typename _K, typename _V, typename _H>
bool HashTable<_K, _V, _H>::get(const _K& k, Entry* e)
{
   bool rval = false;

   // acquire a hazard pointer
   HazardPtr* ptr = mHazardPtrs.acquire();

   /* Search our entries array looking to see if we have a key that matches or
      if the entry for the given key does not exist. If we find an entry that
      matches the hash for our key, but the key does not match, then we must
      reprobe. We do so by incrementing our index by 1 since we handle
      collisions when inserting by simply increasing the index by 1 until we
      find an empty slot.
    */
   int hash = mHashFunction(k);
   bool done = false;
   for(int i = hash; !done; i++)
   {
      // limit index to capacity of table
      i &= (mCapacity - 1);

      // get entry at index
      Entry* e = getEntry(ptr, i);
      if(e != NULL)
      {
         // if the entry key is at the same memory address or if the hashes
         // match and the keys match, then we found the value we want
         if(&(e->k) == &k || (e->h == hash && e->k == k))
         {
            rval = done = true;
         }
         else if(i == mCapacity - 1)
         {
            // keys did not match and we've reached the end of the table, so
            // the entry does not exist
            done = true;
         }

         if(!rval)
         {
            // unreference entry's list
            unrefEntryList(e->owner);
         }
      }
      else
      {
         // no such entry
         done = true;
      }
   }

   // release the hazard pointer
   mHazardPtrs.release(ptr);

   return rval;
}

template<typename _K, typename _V, typename _H>
void HashTable<_K, _V, _H>::resize(HazardPtr* ptr, EntryList* el, int capacity)
{
   /* Note: When we call resize(), other threads might also be trying to
      resize at the same time. Therefore, we allocate a new EntryList and
      then try to CAS it onto the current EntryList. If it fails, then someone
      else has already done the resize for us and we should deallocate the
      EntryList we created and return. If it succeeds, then we either appended
      our EntryList onto another EntryList that was marked as garbage while
      we were doing work (which is ok, it will be collected later), or we
      appended our EntryList as the most current valid EntryList. In either
      case, the old current EntryList is marked as an old list. By marking
      the list as old, subsequent calls to put() know to mark entries in
      the old list as Sentinels.

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
      if(Atomic::compareAndSwap(el->next, NULL, newList))
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
/*
template<typename _K, typename _V, typename _H>
void HashTable<_K, _V, _H>::collectGarbage(HazardPtr* ptr)
{
   // 12-02-2009: figure this out
   // FIXME: iterate over EntryLists, mark "old" lists that have only sentinels
   // and tombstones as garbage
   // FIXME: should other entries be moved into the new list? how costly will
   // this operation be? should we call it from get? should we only move at
   // most one entry? will this work cleanly with multiple intermediate old
   // lists?
}
*/

} // end namespace rt
} // end namespace db
#endif
