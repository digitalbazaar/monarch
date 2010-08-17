/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Base64Codec_H
#define monarch_util_Base64Codec_H

#include <string>

namespace monarch
{
namespace util
{

/**
 * Encodes and decodes data into/from Base64.
 *
 * Base64 encoding consists of taking 3 bytes (24 bits) and converting
 * it into 4 6-bit index values. Each index value corresponds to an
 * alphanumeric character. Every 76 characters, an CRLF must be
 * inserted. Decoders should ignore whitespace.
 *
 * So these bytes:
 *
 * b0: 00000001
 * b1: 00000010
 * b2: 00000011
 *
 * Would be concatenated to:
 *
 * 000000010000001000000011
 *
 * And then split up like this:
 *
 * 000000|010000|001000|000011
 *
 * An algorithm for doing this would be the following,
 * where 0x3F = 63 (6-bits):
 *
 * Index 1 = b0 >> 2
 * Index 2 = (b0 << 4 | b1 >> 4) & 0x3F
 * Index 3 = (b1 << 2 | b2 >> 6) & 0x3F
 * Index 4 = b2 & 0x3F
 *
 * Algorithm steps (and byte view):
 *
 *     1   |   2    |   3   |    4
 * 00000000
 *   000000
 *   000000|00010000
 *   000000|  010000
 *   000000|  010000|00001000
 *   000000|  010000|  001000
 *   000000|  010000|  001000|00000011
 *   000000|  010000|  001000|  000011
 *
 *   000000|  010000|  001000|  000011
 *
 * And these values would be translated into index values:
 *
 * 0, 32, 16, 3
 *
 * Which would in turn translate into:
 *
 * AgQD
 *
 * If the byte array that is being translated is not a multiple of 3, then
 * the bytes are translated and the '=' is used as padding.
 *
 * So for 2 bytes:
 *
 * 00000001
 * 00000010
 *
 * 0000000100000010
 *
 * 000000|010000|001000|=
 *
 * And for 1 byte:
 *
 * 0000001
 *
 * 000000|001000|=|=
 *
 * TODO: Generalize the API so that different maps of characters could be
 * passed in to do different encodings.
 *
 * @author Dave Longley
 */
class Base64Codec
{
public:
   /**
    * Maximum line length specified for PEM messages (RFC 1421).
    */
   static const size_t sMaxPemLength = 64;

   /**
    * Maximum line length specified for MIME messages (RFC 2045).
    */
   static const size_t sMaxMimeLength = 76;

   /**
    * Standard symbol characters '+' and '/' to use for Base64 encoding of
    * 6-bit (62-63) values.
    */
   static const char sStandardSymChars[];

   /**
    * URL safe symbol characters '-' and '_' to use for Base64 encoding of
    * 6-bit (62-63) values.
    */
   static const char sUrlSymChars[];

   /**
    * Base64 encodes data. The passed array of bytes is transformed into a
    * base64-encoded string. If maxline is greater than 0, lines will be split
    * with a "\r\n".
    *
    * @param data the byte array to encode.
    * @param length the number of bytes in the data to encode.
    * @param maxLineLength the maximum number of encoded bytes per line to use,
    *        defaults to no maximum. A common value is 76.
    * @param symChars array of length two specifying the alphabet for the
    *        standard "+/" encoding. Can be used to substitute URL and filename
    *        safe encodings such as "-_".
    *
    * @return the base64-encoded string.
    */
   static std::string encode(
      const char* data, unsigned int length, size_t maxLineLength = 0,
      const char symChars[] = sStandardSymChars);

   /**
    * Base64 encodes data using a URL and filename safe alphabet. Equivalent to
    * calling: encode(data, length, 0, sUrlSymChars).
    *
    * @param data the byte array to encode.
    * @param length the number of bytes in the data to encode.
    *
    * @return the base64-encoded string.
    */
   static std::string urlSafeEncode(const char* data, unsigned int length);

   /**
    * Base64 decodes data. The passed base64-encoded string is transformed
    * into a heap-allocated byte array of data.
    *
    * The caller of this method must free the allocated data.
    *
    * @param str the Base64-encoded string.
    * @param data a pointer to a byte array that will be set by this method.
    * @param length to store the number of decoded bytes.
    * @param symChars array of length two specifying the alphabet for the
    *        standard "+/" encoding. Can be used to substitute URL and filename
    *        safe encodings such as "-_".
    */
   static void decode(
      const char* str, char** data, unsigned int& length,
      const char symChars[] = sStandardSymChars);

   /**
    * Base64 decodes data using a URL and filename safe alphabet. Equivalent to
    * calling: decode(str, data, length, sUrlSymChars).
    *
    * @param str the Base64-encoded string.
    * @param data a pointer to a byte array that will be set by this method.
    * @param length to store the number of decoded bytes.
    */
   static void urlSafeDecode(
      const char* str, char** data, unsigned int& length);
};

} // end namespace util
} // end namespace monarch
#endif
