/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_id3v2_TagHeaderv2_H
#define monarch_data_id3v2_TagHeaderv2_H

namespace monarch
{
namespace data
{
namespace id3v2
{

/**
 * An Id3v2 tag header. The currently supported version.revision is 3.0. That
 * means that ID3v2.3.0 tags are supported.
 *
 * The format of the header is such:
 *
 * | bytes 0-2 | byte 3  |  byte 4  | byte 5  | byte 6-9 |
 * -------------------------------------------------------
 * |   "ID3"   | version | revision |  flags  |   size   |
 *
 * In hex form:
 *
 * 49 44 33 yy yy xx zz zz zz zz
 *
 * ID3v2/file identifier      "ID3"
 * ID3v2 version              $03 00
 * ID3v2 flags                %abc00000
 * ID3v2 size             4 * %0xxxxxxx
 *
 * The first three bytes of the header are always "ID3" which indicates
 * that the data represents an ID3v2 tag. The next two bytes provide the
 * version (only 3.0 is currently supported -- this is the most wide-spread
 * version).
 *
 * @author Dave Longley
 */
class TagHeader
{
protected:
   /**
    * The version of the ID3 tag.
    */
   unsigned char mVersion;

   /**
    * The revision of the ID3 tag.
    */
   unsigned char mRevision;

   /**
    * Set to true if the unsynchronized flag is set.
    */
   bool mUnsynchronizedFlag;

   /**
    * Set to true if the extended header flag is set.
    */
   bool mExtendedHeaderFlag;

   /**
    * Set to true if the experimental flag is set.
    */
   bool mExperimentalFlag;

   /**
    * The size of the ID3 tag. This is the sum of the size of the
    * extended header and the frames (and padding).
    */
   int mTagSize;

   /**
    * Returns a byte with its bits set according to the header flags.
    *
    * @return the flag byte of this header.
    */
   unsigned char getFlagByte();

public:
   /**
    * Supported version.
    */
   static const unsigned char sSupportedVersion;

   /**
    * Supported revision.
    */
   static const unsigned char sSupportedRevision;

   /**
    * The size of an ID3v2 header in bytes.
    */
   static const int sHeaderSize;

   /**
    * The maximum tag size in bytes.
    */
   static const int sMaxTagSize;

   /**
    * Unsynchronized flag (bit 7).
    */
   static const unsigned char sUnsynchronizedBit;

   /**
    * Extended header flag (bit 6).
    */
   static const unsigned char sExtendedHeaderBit;

   /**
    * Experimental flag (bit 5).
    */
   static const unsigned char sExperimentalBit;

   /**
    * Creates a new TagHeader.
    */
   TagHeader();

   /**
    * Destructs this TagHeader.
    */
   virtual ~TagHeader();

   /**
    * Converts the header into a 10 byte array.
    *
    * @param b the byte array to write the header to, must be at least 10
    *          bytes long.
    */
   virtual void convertToBytes(char* b);

   /**
    * Converts the header from a byte array with at least 10 bytes.
    *
    * @param b the byte array to convert from.
    *
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b);

   /**
    * Sets the version.
    *
    * @param version the version to use.
    */
   virtual void setVersion(unsigned char version);

   /**
    * Gets the version.
    *
    * @return the version.
    */
   virtual unsigned char getVersion();

   /**
    * Sets the revision of the version.
    *
    * @param revision the revision to use.
    */
   virtual void setRevision(unsigned char revision);

   /**
    * Gets the revision of the version.
    *
    * @return the revision of the version.
    */
   virtual unsigned char getRevision();

   /**
    * Sets the flags based on the passed byte value.
    *
    * @param b the flag byte to use.
    */
   virtual void setFlags(unsigned char b);

   /**
    * Sets the unsychronized flag.
    *
    * @param flag the boolean value to set the flag to.
    */
   virtual void setUnsynchronizedFlag(bool flag);

   /**
    * Gets the unsynchronized flag.
    *
    * @return the status of the unsynchronized flag.
    */
   virtual bool getUnsynchronizedFlag();

   /**
    * Sets the extended header flag.
    *
    * @param flag the boolean value to set the flag to.
    */
   virtual void setExtendedHeaderFlag(bool flag);

   /**
    * Gets the extended header flag.
    *
    * @return the status of the extended header flag.
    */
   virtual bool getExtendedHeaderFlag();

   /**
    * Sets the experimental flag.
    *
    * @param flag the boolean value to set the flag to.
    */
   virtual void setExperimentalFlag(bool flag);

   /**
    * Gets the experimental flag.
    *
    * @return the status of the experimental flag.
    */
   virtual bool getExperimentalFlag();

   /**
    * Sets the tag size. This is the extended header size plus the frame sizes.
    *
    * @param tagSize the tag size to use.
    */
   virtual void setTagSize(int tagSize);

   /**
    * Gets the tag size. This is the extended header size plus the frame sizes.
    *
    * @return the tag size, which is the extended header plus the frames.
    */
   virtual int getTagSize();

   /**
    * Converts the given integer into a 32-bit (4 byte) synchsafe byte array.
    *
    * Each byte has its most significant bit is cleared to allow for
    * unsynchronization (so that an unrecognized ID3 tag will not be
    * played as audio data). The byte-order is Big Endian.
    *
    * @param integer the integer to convert.
    * @param b the byte array to write the 4 bytes to.
    */
   static void convertIntToSynchsafeBytes(int integer, char* b);

   /**
    * Converts a 32-bit (4 byte) synchsafe byte array into an integer. The
    * byte-order is Big Endian and the most significant bit of each byte
    * is cleared to allow for unsynchronization (so that an unrecognized
    * ID3 tag will not be played as audio data).
    *
    * @param b the byte array to read from.
    *
    * @return the converted size.
    */
   static int convertSynchsafeBytesToInt(const char* b);
};

} // end namespace id3v2
} // end namespace data
} // end namespace monarch
#endif
