/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_Data_H
#define db_data_Data_H

#include <inttypes.h>

// for htonl() and friends
#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "db/util/Macros.h"

namespace db
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
#define DB_FOURCC_FROM_CHARS(a,b,c,d) \
   (fourcc_t)((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))

/**
 * Convert a four character string to a fourcc_t in host endianess.
 * 
 * @param s a string with at least four characters
 *  
 * @return a fourcc_t
 */
#define DB_FOURCC_FROM_STR(s) DB_FOURCC_FROM_CHARS((s)[0],(s)[1],(s)[2],(s)[3])

/**
 * Set a string to fourcc values.
 * 
 * @param s a fourcc_t
 * @param s a string with at least four characters
 */
#define DB_FOURCC_TO_STR(fourcc, s) DB_STMT_START { \
      (s)[0] = ( (fourcc)      & 0xff); \
      (s)[1] = (((fourcc)>>8)  & 0xff); \
      (s)[2] = (((fourcc)>>16) & 0xff); \
      (s)[3] = (((fourcc)>>24) & 0xff); \
   } DB_STMT_END

/**
 * A printf style format string.
 * 
 * printf("fourcc=%" DB_FOURCC_FORMAT "\n", DB_FOURCC_ARGS(fourcc));
 */
#define DB_FOURCC_FORMAT "c%c%c%c"

/**
 * Break down a fourcc_t into printf args.
 * 
 * printf("fourcc=%" DB_FOURCC_FORMAT "\n", DB_FOURCC_ARGS(fourcc));
 * 
 * @param fourcc a fourcc_t
 *  
 * @return a list of args
 */
#define DB_FOURCC_ARGS(fourcc) \
   (char)( (fourcc)      & 0xff), \
   (char)(((fourcc)>>8)  & 0xff), \
   (char)(((fourcc)>>16) & 0xff), \
   (char)(((fourcc)>>24) & 0xff)

/**
 * Little endian to big endian conversion.
 */
// FIXME optimize, use other macros/asm/funcs (htonl), etc
#define DB_UINT32_SWAP_LE_BE_CONSTANT(val) \
   ((((uint32_t)(val) & (uint32_t)0x000000FF) << 24) | \
    (((uint32_t)(val) & (uint32_t)0x0000FF00) << 8) | \
    (((uint32_t)(val) & (uint32_t)0x00FF0000) >> 8) | \
    (((uint32_t)(val) & (uint32_t)0xFF000000) >> 24))
#define DB_UINT32_SWAP_LE_BE(val) DB_UINT32_SWAP_LE_BE_CONSTANT(val)

#if BYTE_ORDER == LITTLE_ENDIAN
#define DB_UINT32_TO_LE(val) (val)
#elif BYTE_ORDER == BIG_ENDIAN 
#define DB_UINT32_TO_LE(val) DB_UINT32_SWAP_LE_BE(val)
#else
#error BYTE_ORDER not defined 
#endif

// symettric conversion
#define DB_UINT32_FROM_LE(val) DB_UINT32_TO_LE(val)

} // end namespace data
} // end namespace db
#endif
