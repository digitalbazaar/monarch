/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_Utf8Codec_H
#define db_util_Utf8Codec_H

#include "db/rt/Exception.h"
#include <string>

namespace db
{
namespace util
{

/**
 * Encodes and decodes data into/from UTF-8.
 * 
 * UTF-8 (8-bit UCS/Unicode Transformation Format) is a Unicode variable-length
 * character encoding.
 * 
 * FIXME: description of how it works here
 * FIXME: description of the algorithm for converting to/from here
 * 
 * @author Dave Longley
 */
class Utf8Codec
{
protected:

public:
   /**
    * Encodes the passed data into UTF-8. The data will be appended to
    * the passed out string.
    * 
    * @param ce the character encoding for the passed data.
    * @param data the byte array to encode.
    * @param length the number of bytes in the data to encode.
    * @param out the string to store the UTF-8 encoded output in.
    * 
    * @return an Exception if one occurred, NULL if not.
    */
   static db::rt::Exception* encode(
      const char* ce, const char* data, unsigned int length, std::string& out);
   
   /**
    * Decodes the passed UTF-8 encoded string to the passed character
    * encoding. The converted data will be placed in a heap-allocated
    * array of bytes that must be freed by the caller of this method.
    *  
    * @param str the UTF-8 encoded string.
    * @param ce the character encoding to decode to.
    * @param data a pointer to a byte array that will be set by this method.
    * @param length to store the number of decoded bytes.
    * 
    * @return an Exception if one occurred, NULL if not.
    */
   static db::rt::Exception* decode(
      const char* str, const char* ce, char** data, unsigned int& length);
};

} // end namespace util
} // end namespace db
#endif
