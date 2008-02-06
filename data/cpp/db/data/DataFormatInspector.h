/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataFormatInspector_H
#define db_data_DataFormatInspector_H

#include "db/data/DataInspector.h"
#include "db/util/DynamicObject.h"

#include <string>

namespace db
{
namespace data
{

/**
 * A DataFormatInspector inspects data by checking to see if its format is
 * recognized or not.
 * 
 * @author Dave Longley
 */
class DataFormatInspector : public DataInspector
{
public:
   /**
    * The type of data formats.
    */
   typedef enum DataFormatType
   {
      // type not yet determined
      Unknown = 0,
      // zip, tar, ...
      Archive,
      // pdf, doc, ...
      Document,
      // png, jpg, gif, ...
      Image,
      // mpeg, ogg vorbis, wma, ..
      Audio,
      // mpeg, avi, asf, mov, ...
      Video,
      // ID3, etc
      Metadata
   };
   
public:
   /**
    * Creates a new DataFormatInspector.
    */
   DataFormatInspector() {};
   
   /**
    * Destructs this DataFormatInspector.
    */
   virtual ~DataFormatInspector() {};
   
   /**
    * Returns whether or not this inspector is "data-satisfied." The inspector
    * is data-satisfied when it has inspected enough data to determine whether
    * or not the format of the data is recognized.
    * 
    * @return true if this inspector has inspected enough data to determine
    *         if the format is or is not recognized, false if not.
    */
   virtual bool isDataSatisfied() = 0;
   
   /**
    * Returns whether or not this inspector has recognized the format of
    * the data it has been inspecting.
    * 
    * @return true if the data format is recognized, false if not.
    */
   virtual bool isFormatRecognized() = 0;
   
   /**
    * Sets whether or not this inspector should keep inspecting data after
    * successfully recognizing the data format, false if not.
    * 
    * @param inspect true to keep inspecting, false not to.
    */
   virtual void setKeepInspecting(bool inspect) = 0;
   
   /**
    * Gets whether or not this inspector should keep inspecting data after
    * successfully recognizing the data format, false if not.
    * 
    * @return true to keep inspecting, false not to.
    */
   virtual bool keepInspecting() = 0;
   
   /**
    * Gets the total number of bytes inspected so far.
    * 
    * @return the number of bytes inspected so far.
    */
   virtual unsigned long long getBytesInspected() = 0;
   
   /**
    * Gets the type specific details of this stream.
    * 
    * While isFormatRecognized() is false the only valid keys in this object
    * are:
    * "type" (db::data::DataFormatInspector::DataFormatType) => false
    * "mime-type" (string) => "application/octet-stream"
    * 
    * Once isFormatRecognized() returns true then "type" and "mime-type" should
    * be set to something more appropraite.  Further keys are set depending on
    * the type, mime-type, and contents of the stream.
    * 
    * Depending on the stream the format details may be updated when more data
    * is inspected.
    * 
    * @return the type specific details of this stream.
    */
   virtual db::util::DynamicObject getFormatDetails() = 0;
};

} // end namespace data
} // end namespace db
#endif
