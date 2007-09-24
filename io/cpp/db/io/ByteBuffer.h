/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_ByteBuffer_H
#define db_io_ByteBuffer_H

#include "db/io/InputStream.h"
#include "db/io/OutputStream.h"

namespace db
{
namespace io
{

/**
 * A ByteBuffer holds an internal array of bytes that can be dynamically
 * resized, filled, and emptied.
 * 
 * @author Dave Longley
 */
class ByteBuffer
{
protected:
   /**
    * The internal byte buffer. 
    */
   char* mBuffer;
   
   /**
    * The capacity for the byte buffer.
    */
   int mCapacity;
   
   /**
    * The current offset in the buffer.
    */
   int mOffset;
   
   /**
    * The number of valid bytes in the buffer.
    */
   int mLength;
   
   /**
    * True if the internal buffer should be cleaned up, false if not.
    */
   bool mCleanup;
   
   /**
    * Cleans up the internal byte buffer.
    */
   virtual void cleanupBytes();
   
public:
   /**
    * Creates a new ByteBuffer with the specified starting size.
    * 
    * @param capacity the capacity of the ByteBuffer in bytes.
    */
   ByteBuffer(int capacity = 0);
   
   /**
    * Creates a new ByteBuffer that wraps the passed buffer of bytes.
    * 
    * @param b the buffer of bytes to wrap.
    * @param offset the offset at which valid bytes begin.
    * @param length the number of valid bytes.
    * @param cleanup true to handle clean up of the memory, false not to.
    */
   ByteBuffer(char* b, int offset, int length, bool cleanup = false);
   
   /**
    * Copies the data in the passed ByteBuffer into this one.
    * 
    * @param b the ByteBuffer to copy.
    */
   ByteBuffer(const ByteBuffer& copy);
   
   /**
    * Destructs this ByteBuffer.
    */
   virtual ~ByteBuffer();
   
   /**
    * Allocates enough space in the current buffer for the passed number of
    * bytes.
    * 
    * @param length the number of bytes that need to be written to this buffer.
    * @param resize true to resize the buffer as is necessary, false not to. 
    */
   virtual void allocateSpace(int length, bool resize);
   
   /**
    * Resizes the ByteBuffer to the given capacity. Any existing data that
    * cannot fit in the new capacity will be truncated.
    * 
    * @param capacity the new capacity, in bytes, for this buffer.
    */
   virtual void resize(int capacity);
   
   /**
    * Puts data from the passed buffer into this buffer.
    * 
    * @param b the buffer with data to put into this buffer.
    * @param length the number of bytes to put into this buffer.
    * @param resize true to automatically resize this buffer if the passed
    *               number of bytes will not otherwise fit.
    * 
    * @return the actual number of bytes put into this buffer, which may be
    *         less than the number of bytes requested if this buffer is full.
    */
   virtual int put(const char* b, int length, bool resize);
   
   /**
    * Puts data copied from the passed ByteBuffer into this buffer. The passed
    * ByteBuffer is *not* altered.
    * 
    * @param b the ByteBuffer to copy data from.
    * @param length the number of bytes to copy.
    * @param resize true to automatically resize this buffer if the passed
    *               number of bytes will not otherwise fit.
    * 
    * @return the actual number of bytes put into this buffer, which may be
    *         less than the number of bytes requested if this buffer is full
    *         or if the passed ByteBuffer does not have enough bytes.
    */
   virtual int put(ByteBuffer* b, int length, bool resize);
   
   /**
    * Reads data from the passed input stream and puts it into this buffer.
    * 
    * This method will block until at one byte can be read from the input
    * stream, unless this buffer is already full, or until the end of the
    * stream has been reached.
    * 
    * Exception::getLast() will be set if an IO error occurs.
    * 
    * @param is the input stream to read from.
    * 
    * @return the number of bytes read from the input stream and put into
    *         this buffer, 0 if the end of the input stream was reached,
    *         and -1 if an exception occurred.
    */
   virtual int put(InputStream* is);
   
   /**
    * Gets data out of this buffer and puts it into the passed buffer. This
    * method will increment the internal pointer of this buffer by the number
    * of bytes retrieved.
    * 
    * @param b the buffer to put the retrieved data into.
    * @param length the maximum number of bytes to get.
    * 
    * @return the actual number of bytes retrieved, which may be 0 if this
    *         buffer is empty.
    */
   virtual int get(char* b, int length);
   
   /**
    * Gets data out of this buffer and puts it into the passed ByteBuffer. This
    * method will increment the internal pointer of this buffer and the
    * passed buffer by the number of bytes retrieved.
    * 
    * @param b the ByteBuffer to put the retrieved data into.
    * @param length the maximum number of bytes to get.
    * @param resize true to resize the passed buffer as necessary, false not to.
    * 
    * @return the actual number of bytes retrieved, which may be 0 if this
    *         buffer is empty or if the passed buffer is full.
    */
   virtual int get(ByteBuffer* b, int length, bool resize);
   
   /**
    * Gets data out of this buffer and writes it to the passed output stream.
    * This method will increment the internal pointer of this buffer by the
    * number of bytes retrieved.
    * 
    * Exception::getLast() will be set if an IO error occurs.
    * 
    * @param os the OutputStream to write the retrieved data to.
    * 
    * @return the actual number of bytes retrieved, which may be 0 if this
    *         buffer is empty.
    */
   virtual int get(OutputStream* os);
   
   /**
    * Clears the specified amount of data out of this buffer (from the
    * beginning). If more data is requested to be cleared than there is data,
    * all data will be cleared.
    * 
    * @param length the amount of data to clear from this buffer.
    * 
    * @return the actual amount of data cleared.
    */
   virtual int clear(int length);
   
   /**
    * Clears all data from this buffer.
    * 
    * @return the actual amount of data cleared.
    */
   virtual int clear();
   
   /**
    * Trims data from the end of this buffer without resizing it.
    * 
    * @param length the number of bytes to trim off the end of this buffer.
    * 
    * @return the actual number of bytes trimmed.
    */
   virtual int trim(int length);
   
   /**
    * Resets the offset for this ByteBuffer by the specified length. This will
    * move the internal offset pointer backwards.
    * 
    * @param length the maximum number of bytes to move the offset back.
    * 
    * @return the actual number of bytes the offset was moved backwards.
    */
   virtual int reset(int length);
   
   /**
    * Gets the capacity of this buffer.
    * 
    * @return the capacity of this buffer.
    */
   virtual int getCapacity() const;
   
   /**
    * Sets the internal buffer to the same buffer used by the passed
    * ByteBuffer. No data is copied.
    * 
    * @param b the buffer to use.
    * @param cleanup true if the passed buffer should be cleaned up by
    *                this object, false if not.
    */
   virtual void setBytes(ByteBuffer* b, bool cleanup);
   
   /**
    * Sets the internal buffer.
    * 
    * @param b the buffer to use.
    * @param offset the offset at which the valid bytes start in the buffer.
    * @param length the number of valid bytes in the buffer.
    * @param cleanup true if the passed buffer should be cleaned up by
    *                this object, false if not.
    */
   virtual void setBytes(char* b, int offset, int length, bool cleanup);
   
   /**
    * Gets the bytes in this buffer. This method will return the underlying
    * byte array. The offset at which valid bytes begin can be retrieved by
    * calling offset().
    * 
    * @return the bytes in this buffer.
    */
   virtual char* bytes() const;
   
   /**
    * Gets the bytes in this buffer starting at the valid offset for this
    * buffer.
    * 
    * @return the bytes in this buffer starting at the valid offset.
    */
   virtual char* data() const;
   
   /**
    * Gets the offset at which the bytes start in this buffer.
    * 
    * @return the offset at which the bytes start in this buffer.
    */
   virtual int offset() const;
   
   /**
    * Gets the amount of used space in this buffer.
    * 
    * @return the amount of used space in this buffer.
    */
   virtual int length() const;
   
   /**
    * Gets the amount of free space in this buffer.
    * 
    * @return the amount of free space in this buffer, in bytes.
    */
   virtual int getFreeSpace() const;
   
   /**
    * Returns true if this buffer is full.
    * 
    * @return true if this buffer is full, false if it is not.
    */
   virtual bool isFull() const;
   
   /**
    * Returns true if this buffer is empty.
    * 
    * @return true if this buffer is empty, false if it is not.
    */
   virtual bool isEmpty() const;
};

} // end namespace io
} // end namespace db
#endif
