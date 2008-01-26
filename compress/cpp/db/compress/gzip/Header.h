/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_gzip_Header_H
#define db_compress_gzip_Header_H

#include "db/io/ByteBuffer.h"

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
 * FIXME: The current implementation has no support for storing a file name
 * or file comment to write out. It can, however, read that information in.
 * 
 * @author Dave Longley
 */
class Header
{
protected:
   /**
    * A ByteBuffer for storing an "extra field."
    */
   db::io::ByteBuffer mExtraField;
   
   /**
    * True if the FNAME flag is set, meaning an ISO 8859-1 (LATIN-1) filename
    * that is null-terminated is present. This filename has no directory
    * components.
    */
   bool mHasFilename;
   
   /**
    * The filename, if any.
    */
   char* mFilename;
   
   /**
    * True if the FCOMMENT flag is set, meaning an ISO 8859-1 (LATIN-2)
    * human-readable comment about the file is present. Line feeds ('\n')
    * are used for line breaks.
    */
   bool mHasFileComment;
   
   /**
    * The file comment, if any.
    */
   char* mFileComment;
   
   /**
    * True if the FHCRC flag is set.
    */
   bool mHasCrc;
   
   /**
    * Stores the crc for this header.
    */
   unsigned short mCrc;
   
   /**
    * The file system flag.
    */
   unsigned char mFileSystemFlag;
   
   /**
    * The first identification byte for a gzip header.
    */
   static const int GZIP_ID1 = 0x1f;
   
   /**
    * The second identification byte for a gzip header.
    */
   static const int GZIP_ID2 = 0x8b;
   
   /**
    * The FTEXT flag.
    */
   static const unsigned char GZIP_FTEXT = 0x01;
   
   /**
    * The FHCRC flag.
    */
   static const unsigned char GZIP_FHCRC = 0x02;
   
   /**
    * The FEXTRA flag.
    */
   static const unsigned char GZIP_FEXTRA = 0x04;
   
   /**
    * The FNAME flag.
    */
   static const unsigned char GZIP_FNAME = 0x08; 
   
   /**
    * The FCOMMENT flag.
    */
   static const unsigned char GZIP_FCOMMENT = 0x16;
   
public:
   /**
    * Creates a new Header.
    */
   Header();
   
   /**
    * Destructs this Header.
    */
   virtual ~Header();
   
   /**
    * Tries to convert this header from an array of bytes.
    * 
    * @param b the array of bytes to convert from.
    * @param length the number of bytes in the array to convert from.
    * 
    * @return the number of extra bytes required to convert this header from
    *         the passed array of bytes, 0 if no more bytes are required,
    *         and -1 if an exception occurred.
    */
   virtual int convertFromBytes(char* b, int length);
   
   /**
    * Writes this header to the passed ByteBuffer, resizing it if necessary.
    * 
    * @param b the ByteBuffer to write to.
    */
   virtual void convertToBytes(db::io::ByteBuffer* b);
   
   /**
    * Sets the FHCRC flag. If true, this header uses a CRC, if false, it does not.
    * 
    * @param flag true to turn on the FHCRC flag, false to shut it off.
    */
   virtual void setHasCrc(bool flag);
   
   /**
    * Resets the header's CRC.
    */
   virtual void resetCrc();
   
   /**
    * Sets the file system flag.
    * 
    * @param flag the file system flag to use.
    */
   virtual void setFileSystemFlag(unsigned char flag);
   
   /**
    * Gets the filename, if any.
    * 
    * @return the filename, if any.
    */
   virtual const char* getFilename();
   
   /**
    * Gets the file comment, if any.
    * 
    * @return the file comment, if any.
    */
   virtual const char* getFileComment();
   
   /**
    * Gets the size of this header in bytes.
    * 
    * @return the size of this header in bytes.
    */
   virtual int getSize();
};

} // end namespace gzip
} // end namespace compress
} // end namespace db
#endif
