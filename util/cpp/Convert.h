/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_Convert_H
#define db_util_Convert_H

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
   
   /**
    * A static array for converting bytes into upper-case hexadecimal strings.
    */
   static const char UPPER_HEX_CHARS[16];
   
public:
   /**
    * Converts an array of bytes into a lower-case hexadecimal string.
    * 
    * @param b the array of bytes to convert.
    * @param offset the offset at which to start converting.
    * @param length the number of bytes to convert.
    * 
    * @return the lower-case hexadecimal string. 
    */
   static std::string bytesToHex(const char* b, unsigned int length);
   
   /**
    * Converts an array of bytes into an upper-case hexadecimal string.
    * 
    * @param b the array of bytes to convert.
    * @param offset the offset at which to start converting.
    * @param length the number of bytes to convert.
    * 
    * @return the upper-case hexadecimal string. 
    */
   static std::string bytesToUpperHex(const char* b, unsigned int length);
   
   /**
    * Converts a hexadecimal string to an array of bytes. The passed
    * array of bytes must be at least hex.length() / 2 bytes long.
    * 
    * @param hex the hexadecimal string to convert.
    * @param hexLength the number of hex characters.
    * @param b the array of bytes to populate.
    * @param length to store the length of the array of bytes.
    */
   static void hexToBytes(
      const char* hex, unsigned int hexLength,
      char* bytes, unsigned int& length);
   
   /**
    * Converts a hexadecimal string to an integer.
    * 
    * @param hex the hexadecimal string to convert.
    * @param hexLength the number of hex characters.
    * 
    * @return the integer.
    */
   static int hexToInt(const char* hex, unsigned int hexLength);
   
   /**
    * Converts an integer into a lower-case hexadecimal string.
    * 
    * @param n the integer to convert.
    * 
    * @return the lower-case hexadecimal string. 
    */
   static std::string intToHex(int n);
   
   /**
    * Converts an integer into an upper-case hexadecimal string.
    * 
    * @param n the integer to convert.
    * 
    * @return the upper-case hexadecimal string. 
    */
   static std::string intToUpperHex(int n);
   
   /**
    * Converts an integer to a string.
    * 
    * @param n the integer to convert.
    * 
    * @return the string.
    */
   static std::string integerToString(long long n);
   
   /**
    * Converts a string to an integer.
    * 
    * @param s the string to convert.
    * @param n the integer to populate.
    * 
    * @return true if the string could be converted to an integer, false if not.
    */
   static bool stringToInteger(const std::string& s, long long& n);
};

} // end namespace util
} // end namespace db
#endif
