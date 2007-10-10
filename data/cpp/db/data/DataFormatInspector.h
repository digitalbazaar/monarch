/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataFormatInspector_H
#define db_data_DataFormatInspector_H

#include "db/data/DataInspector.h"

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
   virtual long getBytesInspected() = 0;
   
   /**
    * Gets a string identifier for the format that was detected.
    * 
    * @param str the string to populate with the format that was detected.
    * 
    * @return a string identifier for the format that was detected.
    */
   virtual std::string& getFormat(std::string& str) = 0;
   
   /**
    * Gets a custom readable report on the data inspection.
    * 
    * @param str the string to populate.
    * 
    * @return a custom readable report on the data inspection.
    */
   virtual std::string& getInspectionReport(std::string& str) = 0;
};

} // end namespace data
} // end namespace db
#endif
