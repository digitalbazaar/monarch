/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Data_H
#define monarch_util_Data_H

#include <inttypes.h>

// for htonl() and friends
#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace monarch
{
namespace util
{

/**
 * Various support macros and types for data processing.
 *
 * @author David I. Lehn
 */

/**
 * 16 bit little endian to big endian conversion.
 */
// FIXME optimize, use other macros/asm/funcs (htonl), etc
#define MO_UINT16_SWAP_LE_BE_CONSTANT(val) \
   ((((uint16_t)(val) & UINT16_C(0x00FF)) << 8) | \
    (((uint16_t)(val) & UINT16_C(0xFF00)) >> 8))
#define MO_UINT16_SWAP_LE_BE(val) MO_UINT16_SWAP_LE_BE_CONSTANT(val)

/**
 * 32 bit little endian to big endian conversion.
 */
// FIXME optimize, use other macros/asm/funcs (htonl), etc
#define MO_UINT32_SWAP_LE_BE_CONSTANT(val) \
   ((((uint32_t)(val) & UINT32_C(0x000000FF)) << 24) | \
    (((uint32_t)(val) & UINT32_C(0x0000FF00)) << 8) | \
    (((uint32_t)(val) & UINT32_C(0x00FF0000)) >> 8) | \
    (((uint32_t)(val) & UINT32_C(0xFF000000)) >> 24))
#define MO_UINT32_SWAP_LE_BE(val) MO_UINT32_SWAP_LE_BE_CONSTANT(val)

/**
 * 64 bit little endian to big endian conversion.
 */
// FIXME optimize, use other macros/asm/funcs (htonl), etc
#define MO_UINT64_SWAP_LE_BE_CONSTANT(val) \
   ((((uint64_t)(val) & UINT64_C(0x00000000000000FF)) << 56) | \
    (((uint64_t)(val) & UINT64_C(0x000000000000FF00)) << 40) | \
    (((uint64_t)(val) & UINT64_C(0x0000000000FF0000)) << 24) | \
    (((uint64_t)(val) & UINT64_C(0x00000000FF000000)) <<  8) | \
    (((uint64_t)(val) & UINT64_C(0x000000FF00000000)) >>  8) | \
    (((uint64_t)(val) & UINT64_C(0x0000FF0000000000)) >> 24) | \
    (((uint64_t)(val) & UINT64_C(0x00FF000000000000)) >> 40) | \
    (((uint64_t)(val) & UINT64_C(0xFF00000000000000)) >> 56))
#define MO_UINT64_SWAP_LE_BE(val) MO_UINT64_SWAP_LE_BE_CONSTANT(val)

#if BYTE_ORDER == LITTLE_ENDIAN
#define MO_UINT16_TO_LE(val) (val)
#define MO_UINT32_TO_LE(val) (val)
#define MO_UINT64_TO_LE(val) (val)
#define MO_UINT16_TO_BE(val) MO_UINT16_SWAP_LE_BE(val)
#define MO_UINT32_TO_BE(val) MO_UINT32_SWAP_LE_BE(val)
#define MO_UINT64_TO_BE(val) MO_UINT64_SWAP_LE_BE(val)
#elif BYTE_ORDER == BIG_ENDIAN
#define MO_UINT16_TO_LE(val) MO_UINT16_SWAP_LE_BE(val)
#define MO_UINT32_TO_LE(val) MO_UINT32_SWAP_LE_BE(val)
#define MO_UINT64_TO_LE(val) MO_UINT64_SWAP_LE_BE(val)
#define MO_UINT16_TO_BE(val) (val)
#define MO_UINT32_TO_BE(val) (val)
#define MO_UINT64_TO_BE(val) (val)
#else
#error BYTE_ORDER not defined
#endif

// symmetric conversion
#define MO_UINT16_FROM_LE(val) MO_UINT16_TO_LE(val)
#define MO_UINT32_FROM_LE(val) MO_UINT32_TO_LE(val)
#define MO_UINT64_FROM_LE(val) MO_UINT64_TO_LE(val)
#define MO_UINT16_FROM_BE(val) MO_UINT16_TO_BE(val)
#define MO_UINT32_FROM_BE(val) MO_UINT32_TO_BE(val)
#define MO_UINT64_FROM_BE(val) MO_UINT64_TO_BE(val)

} // end namespace util
} // end namespace monarch
#endif
