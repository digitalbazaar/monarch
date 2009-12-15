/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_Data_H
#define monarch_data_Data_H

#include <inttypes.h>

#include "monarch/util/Macros.h"

namespace monarch
{
namespace data
{

/**
 * Various support macros and types for data processing.
 *
 * @author David I. Lehn
 */

/**
 * Single value for storage of a FOURCC type.
 */
typedef uint32_t fourcc_t;

/**
 * Convert four characters to a fourcc_t in host endianess.
 *
 * @param a first character
 * @param b second character
 * @param c third character
 * @param d fourth character
 *
 * @return a fourcc_t
 */
#define MO_FOURCC_FROM_CHARS(a,b,c,d) \
   (fourcc_t)((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))

/**
 * Convert a four character string to a fourcc_t in host endianess.
 *
 * @param s a string with at least four characters
 *
 * @return a fourcc_t
 */
#define MO_FOURCC_FROM_STR(s) MO_FOURCC_FROM_CHARS((s)[0],(s)[1],(s)[2],(s)[3])

/**
 * Set a string to fourcc values.
 *
 * @param s a fourcc_t
 * @param s a string with at least four characters
 */
#define MO_FOURCC_TO_STR(fourcc, s) MO_STMT_START { \
      (s)[0] = ( (fourcc)      & 0xff); \
      (s)[1] = (((fourcc)>>8)  & 0xff); \
      (s)[2] = (((fourcc)>>16) & 0xff); \
      (s)[3] = (((fourcc)>>24) & 0xff); \
   } MO_STMT_END

/**
 * Compare a fourcc with a string.  This check might be faster on an expected
 * failure due to a short-circuit fail on the first character.  An alternative
 * which may be faster on expected successful compares is:
 * (aFourcc == MO_FOURCC_FROM_STR(aString))
 *
 * @param fourcc a fourcc_t
 * @param s a string with at least four characters
 *
 * @return true on a match, false otherwise
 */
#define MO_FOURCC_CMP_STR(fourcc, s) \
      (( (fourcc)      & 0xff) == (uint8_t)((s)[0]) && \
       (((fourcc)>>8)  & 0xff) == (uint8_t)((s)[1]) && \
       (((fourcc)>>16) & 0xff) == (uint8_t)((s)[2]) && \
       (((fourcc)>>24) & 0xff) == (uint8_t)((s)[3]))

/**
 * Create a mask for first N characters of a fourcc_t.
 *
 * @param n number of characters to mask.  n in range [1,4].
 *
 * @return a mask for first N characters of a fourcc_t:
 *         1 => "?---",
 *         2 => "??--",
 *         3 => "???-",
 *         4 => "????"
 */
#define MO_FOURCC_MASK(n) (~(((fourcc_t)0xffffff00) << (8*((n)-1))))

/**
 * Compare a fourcc with a string.  This check might be faster on an expected
 * failure due to a short-circuit fail on the first character.  An alternative
 * which may be faster on expected successful compares is:
 * (aFourcc == MO_FOURCC_FROM_STR(aString))
 *
 * @param fourcc a fourcc_t
 * @param s a string with at least four characters
 * @param n number of characters to compare in range [1,4]
 *
 * @return true on a match, false otherwise
 */
#define MO_FOURCC_NCMP_STR(fourcc, s, n) \
      (((fourcc) & MO_FOURCC_MASK(n)) == \
         (MO_FOURCC_FROM_STR(s) & MO_FOURCC_MASK(n)))

/**
 * A printf style format string.
 *
 * printf("fourcc=%" MO_FOURCC_FORMAT "\n", MO_FOURCC_ARGS(fourcc));
 */
#define MO_FOURCC_FORMAT "c%c%c%c"

/**
 * Break down a fourcc_t into printf args.
 *
 * printf("fourcc=%" MO_FOURCC_FORMAT "\n", MO_FOURCC_ARGS(fourcc));
 *
 * @param fourcc a fourcc_t
 *
 * @return a list of args
 */
#define MO_FOURCC_ARGS(fourcc) \
   (char)( (fourcc)      & 0xff), \
   (char)(((fourcc)>>8)  & 0xff), \
   (char)(((fourcc)>>16) & 0xff), \
   (char)(((fourcc)>>24) & 0xff)

} // end namespace data
} // end namespace monarch
#endif
