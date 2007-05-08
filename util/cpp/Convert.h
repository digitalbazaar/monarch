/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Convert_H
#define Convert_H

#include <string>

namespace db
{
namespace util
{

/**
 * The Convert class provides methods for converting basic data types.
 * 
 * @author Dave Longley
 */
class Convert
{
private:
   /**
    * Creates a new Convert object.
    */
   Convert() {};
   
   /**
    * A static array for converting bytes into hexadecimal strings.
    */
   static const char HEX_CHARS[16];
   
public:
   /**
    * Converts an array of bytes into a hexadecimal string.
    * 
    * @param b the array of bytes to convert.
    * @param offset the offset at which to start converting.
    * @param length the number of bytes to convert. 
    */
   static std::string bytesToHex(const char* b, unsigned int length);
};

} // end namespace util
} // end namespace db
#endif
