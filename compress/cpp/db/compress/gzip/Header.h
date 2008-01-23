/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_gzip_Header_H
#define db_compress_gzip_Header_H

namespace db
{
namespace compress
{
namespace gzip
{

/**
 * A Header is a header for gzip-formatted data. This is the header for
 * one gzip "member".
 * 
 * It has the following format:
 * 
 * | byte 0 | byte 1 | byte 2 | byte 3 | bytes 4-7 | byte 8 | byte 9 | optional 
 * |  ID1   |  ID2   |   CM   |   FLG  |   MTIME   |   XFL  |   OS   | optional
 * 
 * ID1 (IDentification 1) = 31 (0x1f)
 * ID2 (IDentification 2) = 139 (0x8b)
 * CM (Compression Method) = bits 0-7 are reserved, CM = 8 denotes "deflate"
 * FLG (FLaGs) =
 *    bit 0 FTEXT
 *    bit 1 FHCRC
 *    bit 2 FEXTRA
 *    bit 3 FNAME
 *    bit 4 FCOMMENT
 *    bit 5 reserved
 *    bit 6 reserved
 *    bit 7 reserved
 * MTIME (Modification TIME) = the time in Unix format
 *    (seconds since 00:00:00 GMT, Jan 1, 1970).
 * XFL (eXtra FLags) =
 *    when "deflate" (CM = 8) is set, then XFL are set as follows:
 *    XFL = 2 - compressor used maximum compression, slowest algorithm
 *    XFL = 4 - compressor used fastest algorithm
 * OS (Operating System) =
 *    The type of file system the compression took place on:
 *    
 *    0 - FAT filesystem (MS-DOS, OS/2, NT/Win32)
 *    1 - Amiga
 *    2 - VMS (or OpenVMS)
 *    3 - Unix
 *    4 - VM/CMS
 *    5 - Atari TOS
 *    6 - HPFS filesystem (OS/2, NT)
 *    7 - Macintosh
 *    8 - Z-System
 *    9 - CP/M
 *   10 - TOPS-20
 *   11 - NTFS filesystem (NT)
 *   12 - QDOS
 *   13 - Acorn RISCOS
 *  255 - unknown
 * (if FEXTRA flag is set)
 * XLEN (2 bytes) the length of the extra field
 *    DATA data of length XLEN
 * (if FNAME flag is set)
 * file name, zero-terminated
 * (if FCOMMENT set)
 * file comment, zero-terminated
 * (if FHCRC set)
 * CRC16 (2 bytes)
 * 
 * Currently relevant flag information (should be updated if this
 * class sees more usage):
 * 
 * If the FHCRC bit is set in the flags then a CRC16 for the header is
 * present and occurs immediately before the compressed data. It contains
 * the two least significant bytes of the CRC32 for all the bytes in the gzip
 * header up to but not including the CRC16.
 *  
 * Gzip format information from: http://www.gzip.org/zlib/rfc-gzip.html 
 * 
 * @author Dave Longley
 */
class Header
{
public:
   /**
    * Creates a new Header.
    */
   Header();
   
   /**
    * Destructs this Header.
    */
   virtual ~Header();
};

} // end namespace gzip
} // end namespace compress
} // end namespace db
#endif
