/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataInspector_H
#define db_data_DataInspector_H

namespace db
{
namespace data
{

/**
 * A DataInspector is used to inspect data for some implementation specific
 * attributes.
 * 
 * @author Dave Longley
 */
class DataInspector
{
public:
   /**
    * Creates a new DataInspector.
    */
   DataInspector() {};
   
   /**
    * Destructs this DataInspector.
    */
   virtual ~DataInspector() {};
   
   /**
    * Inspects the data in the passed buffer for some implementation
    * specific attributes. This method returns the number of bytes that
    * were successfully inspected such that the passed buffer can safely
    * clear that number of bytes.
    * 
    * An inspector should treat calls to this method as if the data in
    * the passed buffer is consecutive in nature (i.e. read from a stream).
    * 
    * This method may return a number of bytes that is actually greater
    * than the number passed in, if the DataInspector can determine it does
    * not need to inspect them directly.
    * 
    * @param b the buffer with data to inspect.
    * @param length the number of bytes in the buffer.
    * 
    * @return the number of bytes inspected by this DataInspector and that
    *         should not be passed to it again.
    */
   virtual int inspectData(const char* b, int length) = 0;
};

} // end namespace data
} // end namespace db
#endif
