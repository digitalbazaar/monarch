/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_id3v2_FrameHeader_H
#define db_data_id3v2_FrameHeader_H

#include "db/rt/WindowsSupport.h"

#include <string>

namespace db
{
namespace data
{
namespace id3v2
{

#ifdef WIN32
#   ifdef BUILD_DB_DATA_DLL
#      define DLL_CLASS __WIN32_DLL_EXPORT
#   else
#      define DLL_CLASS __WIN32_DLL_IMPORT
#   endif
#else
#   define DLL_CLASS
#endif

/**
 * A FrameHeader stores the ID of an ID3 tag frame, its size, and some
 * flags.
 *
 * Frame headers use a regular 4-byte Big-Endian integer for frame size,
 * whereas the main ID3v2 tag header uses a synchsafe size where bit 7 of
 * each byte is set to 0. Again, tag frame headers DO NOT use "synchsafe"
 * sizes, they use regular sizes with all 8 bits available.
 *
 * @author Dave Longley
 */
class DLL_CLASS FrameHeader
{
protected:
   /**
    * The ID of the frame (4 characters + null-terminator).
    */
   char* mId;

   /**
    * Sets the description of the frame. This is useful for differentiation
    * when multiple frames with the same ID are used but contain different
    * data.
    */
   char* mDescription;

   /**
    * The size of the frame, in bytes.
    */
   int mFrameSize;

   /**
    * This flag indicates whether the frame should be preserved or discarded
    * if the frame is unknown and the tag is altered in any way.
    */
   bool mTagAlteredDiscardFrame;

   /**
    * This flag indicates whether the frame should be preserved or discarded
    * if the frame is unknown and the file, excluding the tag, is altered
    * in any way.
    */
   bool mFileAlteredDiscardFrame;

   /**
    * This flag indicates whether or not the frame should be read only.
    */
   bool mReadOnly;

   /**
    * This flag indicates whether or not the frame is zlib-compressed. If it
    * is, then 4 bytes can be found at the end of the frame header that
    * indicate the decompressed size for the frame data.
    */
   bool mCompressed;

   /**
    * This flag indicates whether or not the frame is encrypted.
    */
   bool mEncrypted;

   /**
    * This flag indicates whether or not the frame belongs to a group of
    * other frames.
    */
   bool mGrouped;

   /**
    * Sets the flags based on the first flag byte.
    *
    * @param b the first flag byte.
    */
   virtual void setFlags1(unsigned char b);

   /**
    * Gets the first flag byte.
    *
    * @return the first flag byte.
    */
   virtual unsigned char getFlagByte1();

   /**
    * Sets the flags based on the second flag byte.
    *
    * @param b the second flag byte.
    */
   virtual void setFlags2(unsigned char b);

   /**
    * Gets the second flag byte.
    *
    * @return the second flag byte.
    */
   virtual unsigned char getFlagByte2();

public:
   /**
    * The size of an ID3v2 tag frame header in bytes.
    */
   static const int HEADER_SIZE;

   /**
    * Tag altered discard/preserve flag (bit 7 of flag byte 1).
    */
   static const unsigned char TAG_ALTERED_DISCARD_FRAME_BIT;

   /**
    * File altered discard/preserve flag (bit 6 of flag byte 1).
    */
   static const unsigned char FILE_ALTERED_DISCARD_FRAME_BIT;

   /**
    * Read only flag (bit 5 of flag byte 1).
    */
   static const unsigned char READ_ONLY_BIT;

   /**
    * Compression flag (bit 7 of flag byte 2).
    */
   static const unsigned char COMPRESSION_BIT;

   /**
    * Encryption flag (bit 6 of flag byte 2).
    */
   static const unsigned char ENCRYPTION_BIT;

   /**
    * Grouping flag (bit 5 of flag byte 2).
    */
   static const unsigned char GROUPING_BIT;

   /**
    * Creates a new FrameHeader.
    *
    * @param id the ID for this header.
    */
   FrameHeader(const char* id = "");

   /**
    * Destructs this FrameHeader.
    */
   virtual ~FrameHeader();

   /**
    * Converts this tag frame header from the given byte array.
    *
    * @param b the buffer with the data to convert from.
    * @param length the number of valid bytes in the byte array.
    */
   virtual void convertFromBytes(const char* b, int length);

   /**
    * Converts this tag frame header to a byte array.
    *
    * @param b the byte array to write to.
    * @param offset the offset at which to begin writing.
    */
   virtual void convertToBytes(char* b);

   /**
    * Sets the ID of the frame.
    *
    * @param id the ID to use.
    */
   virtual void setId(const char* id);

   /**
    * Gets the ID of the frame.
    *
    * @return the ID of the frame.
    */
   virtual const char* getId();

   /**
    * Sets the description of the frame.
    *
    * @param description the description to use in UTF-8.
    */
   virtual void setDescription(const char* description);

   /**
    * Gets the description of the frame.
    *
    * @return the description of the frame.
    */
   virtual const char* getDescription();

   /**
    * Sets the size of the frame (total frame length - header size of 10).
    *
    * @param size the size, in bytes, of the frame, excluding the header.
    */
   virtual void setFrameSize(int size);

   /**
    * Gets the size of the frame (total frame length - header size of 10).
    *
    * @return the size, in bytes, of the frame, excluding the header.
    */
   virtual int getFrameSize();

   /**
    * Sets whether or not the frame should be discarded if it is unknown
    * and the tag is altered.
    *
    * @param discard true to discard, false not to.
    */
   virtual void setTagAlteredDiscardFrame(bool discard);

   /**
    * Gets whether or not the frame should be discarded if it is unknown
    * and the tag is altered.
    *
    * @return true to discard, false not to.
    */
   virtual bool getTagAlteredDiscardFrame();

   /**
    * Sets whether or not the frame should be discarded if it is unknown
    * and the file, but not the tag, is altered.
    *
    * @param discard true to discard, false not to.
    */
   virtual void setFileAlteredDiscardFrame(bool discard);

   /**
    * Gets whether or not the frame should be discarded if it is unknown
    * and the file, but not the tag, is altered.
    *
    * @return true to discard, false not to.
    */
   virtual bool getFileAlteredDiscardFrame();

   /**
    * Sets whether or not the frame is read only.
    *
    * @param readOnly true if the frame is read only, false if not.
    */
   virtual void setReadOnly(bool readOnly);

   /**
    * Gets whether or not the frame is read only.
    *
    * @return true if the frame is read only, false if not.
    */
   virtual bool isReadOnly();

   /**
    * Sets whether or not the frame is compressed using zlib.
    *
    * @param compressed true if the frame is compressed, false if not.
    */
   virtual void setCompressed(bool compressed);

   /**
    * Gets whether or not the frame is compressed using zlib.
    *
    * @return true if the frame is compressed, false if not.
    */
   virtual bool isCompressed();

   /**
    * Sets whether or not the frame is encrypted.
    *
    * @param encrypted true if the frame is encrypted, false if not.
    */
   virtual void setEncrypted(bool encrypted);

   /**
    * Gets whether or not the frame is encrypted.
    *
    * @return true if the frame is encrypted, false if not.
    */
   virtual bool isEncrypted();

   /**
    * Sets whether or not the frame is grouped.
    *
    * @param grouped true if the frame is grouped, false if not.
    */
   virtual void setGrouped(bool grouped);

   /**
    * Gets whether or not the frame is grouped.
    *
    * @return true if the frame is grouped, false if not.
    */
   virtual bool isGrouped();

   /**
    * Converts this ID3 tag frame header to a human-readable string.
    *
    * @param str the string to populate.
    *
    * @return the human readable string.
    */
   virtual std::string& toString(std::string& str);

   /**
    * Converts the given integer into a 32-bit (4 byte) byte array. The
    * byte-order is Big Endian.
    *
    * @param integer the integer to convert.
    * @param b the byte array to write the 4 bytes to.
    */
   static void convertIntToBytes(int integer, char* b);

   /**
    * Converts a 32-bit (4 byte) byte array into an integer. The byte-order
    * is Big Endian.
    *
    * @param b the byte array to read from.
    *
    * @return the converted size.
    */
   static int convertBytesToInt(const char* b);
};

} // end namespace id3v2
} // end namespace data
} // end namespace db
#endif
