/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Convert_H
#define monarch_util_Convert_H

#include <string>

namespace monarch
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
    * array of bytes must be at least ceil(hex.length() / 2) bytes long.
    *
    * If the hex string contains an odd number of characters, an initial 0 is
    * assumed.
    *
    * An error can occur if the input is outside the character range
    * [0-9A-Fa-f].
    *
    * @param hex the hexadecimal string to convert.
    * @param hexLength the number of hex characters.
    * @param b the array of bytes to populate.
    * @param length to store the length of the array of bytes.
    *
    * @return true on success, false on error and an exception will be set.
    */
   static bool hexToBytes(
      const char* hex, unsigned int hexLength,
      char* bytes, unsigned int& length);

   /**
    * Converts a hexadecimal string to an integer. The string is parsed in
    * big endian order.
    *
    * If the hex string contains an odd number of characters, an initial 0 is
    * assumed.
    *
    * An error can occur if the input is outside the character range
    * [0-9A-Fa-f] or if the result value woudl exceed the maxium unsigned int
    * value.
    *
    * @param hex the hexadecimal string to convert.
    * @param hexLength the number of hex characters.
    * @param value the value to fill.
    * @param length the number of converted bytes
    *
    * @return true on success, false on error and an exception will be set.
    */
   static bool hexToInt(
      const char* hex, unsigned int hexLength,
      unsigned int &value);

   /**
    * Converts an integer into a lower-case hexadecimal string.
    *
    * @param n the integer to convert.
    *
    * @return the lower-case hexadecimal string.
    */
   static std::string intToHex(unsigned int n);

   /**
    * Converts an integer into an upper-case hexadecimal string.
    *
    * @param n the integer to convert.
    *
    * @return the upper-case hexadecimal string.
    */
   static std::string intToUpperHex(unsigned int n);

   // Note: Commented out because the C functions are simple enough --
   // just use them and they are faster.
//   /**
//    * Converts an integer to a string.
//    *
//    * @param n the integer to convert.
//    *
//    * @return the string.
//    */
//   static std::string integerToString(long long n);
//
//   /**
//    * Converts a string to an integer.
//    *
//    * @param s the string to convert.
//    * @param n the integer to populate.
//    * @param base the base to use, defaulting to 10.
//    *
//    * @return true if the string could be converted to an integer, false if not.
//    */
//   static bool stringToInteger(const char* s, long long& n, int base = 10);
};

} // end namespace util
} // end namespace monarch
#endif
