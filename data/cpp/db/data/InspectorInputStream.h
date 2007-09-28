/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_InspectorInputStream_H
#define db_data_InspectorInputStream_H

#include "db/io/FilterInputStream.h"
#include "db/io/ByteBuffer.h"
#include "db/data/DataInspector.h"

#include <map>
#include <list>

namespace db
{
namespace data
{

/**
 * An InspectorInputStream uses DataInspectors to collect information about
 * the data that is read.
 * 
 * @author Dave Longley
 */
class InspectorInputStream : public db::io::FilterInputStream
{
protected:
   /**
    * A NameComparator compares the names for the data inspector map.
    */
   typedef struct NameComparator
   {
      /**
       * Compares two inspector names using a string compare, returning true
       * if the first name is less than the second, false if not.
       * 
       * @param name1 the first name.
       * @param name2 the second name.
       * 
       * @return true if the name1 < name2, false if not.
       */
      bool operator()(const char* name1, const char* name2) const
      {
         return strcmp(name1, name2) < 0;
      }
   };
   
   /**
    * A structure for maintaining information about a particular DataInspector.
    */
   typedef struct DataInspectorMetaData
   {
      /**
       * The DataInspector.
       */
      DataInspector* inspector;
      
      /**
       * The current number of bytes that have been inspected by the associated
       * inspector in the shared read buffer.
       */
      int inspectedBytes;
      
      /**
       * True if the inspector should be cleanup when this meta data
       * is erased, false if not.
       */
      bool cleanupInspector;
   };
   
   /**
    * The data inspectors (stored along with their meta-data).
    */
   typedef std::map<const char*, DataInspectorMetaData, NameComparator>
      InspectorMap;
   InspectorMap mInspectors;
   
   /**
    * Stores the data inspectors that are waiting to inspect the currently
    * buffered data.
    */
   typedef std::list<DataInspectorMetaData*> InspectorList;
   InspectorList mWaiting;
   
   /**
    * An internal buffer for storing data read from the underlying stream.
    */
   db::io::ByteBuffer mReadBuffer;
   
   /**
    * Stores the number of bytes that have been inspected and can be
    * released from the read buffer.
    */
   int mAvailableBytes;
   
public:
   /**
    * Creates a new InspectorInputStream that inspects data in the passed
    * InputStream.
    * 
    * @param is the underlying InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    */
   InspectorInputStream(db::io::InputStream* is, bool cleanup = false);
   
   /**
    * Creates a new InspectorInputStream.
    */
   InspectorInputStream();
   
   /**
    * Destructs this InspectorInputStream.
    */
   virtual ~InspectorInputStream();
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);
   
   /**
    * Adds an inspector to this stream.
    * 
    * @param name the name for the inspector.
    * @param di the DataInspector to add.
    * @param cleanup true if the inspector's memory should be managed by
    *                this stream, false if not.
    * 
    * @return true if the inspector was added, false if it was not because
    *         another inspector with the same name was already added.
    */
   virtual bool addInspector(const char* name, DataInspector* di, bool cleanup);
   
   /**
    * Removes an inspector from this stream.
    * 
    * @param name the name of the DataInspector to remove.
    * 
    * @return true if the inspector was removed.
    */
   virtual bool removeInspector(const char* name);
   
   /**
    * Gets a DataInspector from this stream by its name.
    * 
    * @param name the name of the DataInspector to get.
    * 
    * @return the DataInspector with the given name or null if none exists.
    */
   virtual DataInspector* getInspector(const char* name);
   
   /**
    * Gets all of the DataInspectors from this stream.
    * 
    * @param inspectors a list to populate with all of the DataInspectors
    *                   from this stream.
    */
   virtual void getInspectors(std::list<DataInspector*>& inspectors);
};

} // end namespace data
} // end namespace db
#endif
