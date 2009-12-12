/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_InspectorInputStream_H
#define db_data_InspectorInputStream_H

#include "monarch/io/FilterInputStream.h"
#include "monarch/io/ByteBuffer.h"
#include "monarch/data/DataInspector.h"
#include "monarch/util/StringTools.h"

#include <map>
#include <list>
#include <cstring>

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
class InspectorInputStream : public monarch::io::FilterInputStream
{
protected:
   /**
    * A structure for maintaining information about a particular DataInspector.
    */
   struct DataInspectorMetaData
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
   typedef std::map<
      const char*, DataInspectorMetaData, monarch::util::StringComparator>
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
   monarch::io::ByteBuffer mReadBuffer;

   /**
    * Stores the number of bytes that have been inspected and can be
    * released from the read buffer.
    */
   int mAvailableBytes;

   /**
    * Set to true if the input stream finished early because all
    * inspectors are finished and have keepInspecting set to false.
    */
   bool mFinished;

   /**
    * Set to true if this input stream should read the underlying
    * input stream fully even if its inspectors are finished, false if not.
    */
   bool mReadFully;

public:
   /**
    * Creates a new InspectorInputStream that inspects data in the passed
    * InputStream.
    *
    * @param is the underlying InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    */
   InspectorInputStream(monarch::io::InputStream* is, bool cleanup = false);

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
    * @return the DataInspector with the given name or NULL if none exists.
    */
   virtual DataInspector* getInspector(const char* name);

   /**
    * Gets all of the DataInspectors from this stream.
    *
    * @param inspectors a list to populate with all of the DataInspectors
    *                   from this stream.
    */
   virtual void getInspectors(std::list<DataInspector*>& inspectors);

   /**
    * Scans the entire input stream, calling read() on it until it returns
    * 0 or -1.
    *
    * @param total if not NULL, will return the total number of bytes read.
    *
    * @return true if the entire stream was read, false if an exception
    *         occurred.
    */
   virtual bool inspect(uint64_t* total = NULL);

   /**
    * Sets whether or not this InspectorInputStream should read the underlying
    * stream in its entirely even if none of its DataInspectors are still
    * inspecting. By default, this behavior is on.
    *
    * @param on true to fully read the underlying stream, false not to.
    */
   virtual void setReadFully(bool on);
};

} // end namespace data
} // end namespace db
#endif
