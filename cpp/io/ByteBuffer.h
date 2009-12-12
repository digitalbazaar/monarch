/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_ByteBuffer_H
#define db_io_ByteBuffer_H

#include "monarch/io/InputStream.h"
#include "monarch/io/OutputStream.h"

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
   unsigned char* mBuffer;

   /**
    * The capacity for the byte buffer.
    */
   int mCapacity;

   /**
    * A pointer to the current offset in the buffer.
    */
   unsigned char* mOffset;

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
    * @param capacity the total capacity of the buffer.
    * @param cleanup true to handle clean up of the memory, false not to.
    */
   ByteBuffer(
      char* b, int offset, int length, int capacity, bool cleanup);

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
    * Frees all memory, that is marked as freeable (via "cleanup"), associated
    * with this ByteBuffer. This ByteBuffer's capacity, offset, and length
    * will all be set to zero.
    */
   virtual void free();

   /**
    * Allocates enough space in the current buffer for the passed number of
    * bytes. Keep in mind that if resize is called, the underlying memory will
    * automatically become managed by this ByteBuffer and the previously
    * pointed to underlying byte array will not be cleaned up unless it was
    * specified so previously.
    *
    * This call is also useful for manually defragmenting this buffer before
    * writing to its underlying bytes directly. If some of the data that was
    * written to this buffer has already been read out of it, then the data
    * between the beginning of this buffer and its current offset will be
    * included in the return value of freeSpace(). This means that if you
    * try to write freeSpace() bytes to the underlying bytes in this buffer
    * directly (i.e. via write(uend(), ...) or write(end(), ...)), then you
    * will experience a buffer-overrun. To avoid this, this buffer can be
    * manually defragmented by calling allocateSpace() with the resize parameter
    * set to false. Whatever amount you pass in for the length parameter will
    * become available up to freeSpace(). This is done by moving the data
    * to the beginning of the buffer as needed. Keep in mind that this will
    * cause any old bytes before the current offset to become unavailable.
    *
    * A simpler way to say the above is that if you call:
    *
    * allocateSpace(freeSpace(), false)
    *
    * Then the valid data will be placed at the beginning of the buffer if
    * it wasn't there already.
    *
    * If resize is false and length is larger than freeSpace(), then only
    * freeSpace() will be available.
    *
    * @param length the number of bytes that need to be written to this buffer.
    * @param resize true to resize the buffer as is necessary, false not to.
    */
   virtual void allocateSpace(int length, bool resize);

   /**
    * Resizes the ByteBuffer to the given capacity. Any existing data that
    * cannot fit in the new capacity will be truncated. Keep in mind that, the
    * underlying memory will automatically become managed by this ByteBuffer
    * and the previously pointed to underlying byte array will not be cleaned
    * up unless it was specified so previously.
    *
    * @param capacity the new capacity, in bytes, for this buffer.
    */
   virtual void resize(int capacity);

   /**
    * Reallocates the space for data in this buffer, possibly at a new memory
    * location. Any existing managed data will be free'd.
    *
    * If copy is true and the existing data is larger than the given capacity,
    * then it will be truncated.
    *
    * @param capacity the new capacity for this buffer.
    * @param copy true to copy the existing data, false not to.
    */
   virtual void reAllocate(int capacity = 0, bool copy = false);

   /**
    * Puts a particular byte into this buffer n times.
    *
    * @param b the byte to put into this buffer.
    * @param n the number of times to put b into this buffer.
    * @param resize true to automatically resize this buffer if the
    *               bytes will not otherwise fit.
    *
    * @return the actual number of bytes put into this buffer, which may be
    *         less than n if this buffer is full.
    */
   virtual int putByte(unsigned char b, int n, bool resize);

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
    * stream has been reached. If you want this method to block until the
    * buffer is full or until a specified amount is read into it, then
    * call fill() instead.
    *
    * Exception::get() will be set if an IO error occurs.
    *
    * @param is the input stream to read from.
    * @param length the maximum number of bytes to read, 0 to use this buffer's
    *               free space as the maximum to read.
    *
    * @return the number of bytes read from the input stream and put into
    *         this buffer, 0 if the end of the input stream was reached,
    *         and -1 if an exception occurred.
    */
   virtual int put(InputStream* is, int length = 0);

   /**
    * Tries to fill this buffer with data from the passed InputStream.
    *
    * This method will block until the specified number of bytes is read,
    * the end of the input stream is reached, or until the buffer is full.
    *
    * Exception::get() will be set if an IO error occurs.
    *
    * @param is the input stream to read from.
    * @param length the number of bytes to read, 0 to read as much as can fit
    *               in the buffer.
    *
    * @return the number of bytes read from the input stream and put into
    *         this buffer, 0 if the end of the input stream was reached,
    *         and -1 if an exception occurred.
    */
   virtual int fill(InputStream* is, int length = 0);

   /**
    * Gets a single byte out of this buffer. This method will increment the
    * internal pointer of this buffer if a byte was available.
    *
    * @param b the byte to fill with the next one from the buffer.
    *
    * @return 1 if a byte was retrieved, 0 if not because the buffer was empty.
    */
   virtual int getByte(unsigned char& b);

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
    * Exception::get() will be set if an IO error occurs.
    *
    * @param os the OutputStream to write the retrieved data to.
    *
    * @return the actual number of bytes retrieved, which may be 0 if this
    *         buffer is empty or -1 if there was an exception.
    */
   virtual int get(OutputStream* os);

   /**
    * Clears the specified amount of data out of this buffer (from the
    * beginning). If more data is requested to be cleared than there is data,
    * all data will be cleared.
    *
    * This method is not guaranteed to be reversible, use advanceOffset() if
    * you may want to call reset() later.
    *
    * Note: The underlying bytes will not be altered by this call, only
    * the internal offset pointer will be moved forward and the length
    * decreased by the specified length parameter. If all data is cleared,
    * then the internal offset pointer and length will be set to 0.
    *
    * @param length the amount of data to clear from this buffer, a negative
    *               number will have no effect.
    *
    * @return the actual amount of data cleared.
    */
   virtual int clear(int length);

   /**
    * Clears all data from this buffer.
    *
    * Note: The underlying bytes will not be altered by this call, only
    * the internal offset pointer and length will be reset to 0.
    *
    * @return the actual amount of data cleared.
    */
   virtual int clear();

   /**
    * Moves the internal offset pointer forward and decreases the length of
    * this buffer by the specified parameter. The internal offset pointer
    * will not be set past the capacity of this buffer.
    *
    * This method has the effect of reading bytes out of this buffer without
    * actually copying them anywhere. The underlying bytes in this buffer
    * will not be affected, and a call to reset() can undo this operation.
    *
    * This method is particularly useful if the underlying bytes have been
    * read from via this buffer's accessors and this buffer now needs to
    * be updated to account for those changes.
    *
    * @param length the amount to advance the offset, a negative number will
    *        have no effect.
    *
    * @return the actual amount the offset increased.
    */
   virtual int advanceOffset(int length);

   /**
    * Resets the offset for this ByteBuffer by the specified length. This will
    * move the internal offset pointer backwards. The length of this buffer
    * will be increased by the amount the offset moves backwards.
    *
    * Note: This will not cause the next put() call to place data in a
    * different location than it would have if you hadn't called reset(). Data
    * will still be put at offset() + length() or end(), it's just that
    * after this call, offset will be less and length will be greater.
    *
    * If you want to put data at an earlier spot in the buffer, then you
    * should be decreasing the length of the buffer, not its offset. To do so,
    * call trim().
    *
    * @param length the maximum number of bytes to move the offset back, a
    *               negative number will have no effect.
    *
    * @return the actual number of bytes the offset was moved backwards.
    */
   virtual int reset(int length);

   /**
    * Trims data from the end of this ByteBuffer without resizing it. This will
    * decrease the length of this ByteBuffer. The internal offset that
    * points to the start of the valid bytes will be unaffected.
    *
    * This method is useful for re-writing (overwriting) data in a buffer and
    * then extending it again via extend().
    *
    * @param length the number of bytes to trim off the end of this buffer, a
    *               negative number will have no effect.
    *
    * @return the actual number of bytes trimmed.
    */
   virtual int trim(int length);

   /**
    * Extends the length of this ByteBuffer without resizing its capacity. This
    * will increase the length of this ByteBuffer. The internal offset that
    * points to the start of valid bytes will be unaffected.
    *
    * If this method is to be called because an external method is going to
    * first add valid bytes to this ByteBuffer by writing directly to its
    * underlying byte array, then allocateSpace() should be called before
    * that external method is called. That will ensure that there is enough
    * memory allocated in the underlying buffer to store what will be written.
    *
    * The extend() method will not allocate any space or shift any bytes around
    * to make room for the requested length.
    *
    * @param length the number of bytes to increase the length of this buffer
    *               by, up to its free space minus its current offset -- a
    *               negative number will have no effect.
    *
    * @return the actual number of bytes the length was increased by.
    */
   virtual int extend(int length);

   /**
    * Gets the first byte in this ByteBuffer and increments the internal
    * pointer. Do not call this method if this ByteBuffer is empty.
    *
    * @return the first byte in this ByteBuffer.
    */
   virtual unsigned char next();

   /**
    * Gets the capacity of this buffer.
    *
    * @return the capacity of this buffer.
    */
   virtual int capacity() const;

   /**
    * Sets the internal buffer to the same buffer used by the passed
    * ByteBuffer. No data is copied.
    *
    * @param b the buffer to use.
    * @param cleanup true if the data in the passed buffer should be cleaned
    *                up by this object, false if not.
    */
   virtual void setBytes(ByteBuffer* b, bool cleanup);

   /**
    * Sets the internal buffer.
    *
    * @param b the buffer to use.
    * @param offset the offset at which the valid bytes start in the buffer.
    * @param length the number of valid bytes in the buffer.
    * @param capacity the total capacity of the buffer.
    * @param cleanup true if the passed buffer should be cleaned up by
    *                this object, false if not.
    */
   virtual void setBytes(
      char* b, int offset, int length, int capacity, bool cleanup);

   /**
    * Gets the bytes in this buffer. This method will return the underlying
    * byte array. The offset at which valid bytes begin can be retrieved by
    * calling offset().
    *
    * @return the bytes in this buffer.
    */
   virtual char* bytes() const;

   /**
    * Gets the unsigned bytes in this buffer. This method will return the
    * underlying byte array. The offset at which valid bytes begin can be
    * retrieved by calling offset().
    *
    * @return the unsigned bytes in this buffer.
    */
   virtual unsigned char* ubytes() const;

   /**
    * Gets the bytes in this buffer starting at the valid offset for this
    * buffer.
    *
    * @return the bytes in this buffer starting at the valid offset.
    */
   virtual char* data() const;

   /**
    * Gets the bytes in this buffer as unsigned bytes starting at the valid
    * offset for this buffer.
    *
    * @return the unsigned bytes in this buffer starting at the valid offset.
    */
   virtual unsigned char* udata() const;

   /**
    * Gets the bytes in this buffer starting at the valid offset for this
    * buffer + the number of valid bytes in the buffer. This method is
    * useful for manually appending data to this buffer (which is typically
    * followed by a call to extend() afterwards).
    *
    * @return the first invalid byte at the end of this buffer.
    */
   virtual char* end() const;

   /**
    * Gets the bytes in this buffer starting at the valid offset for this
    * buffer + the number of valid bytes in the buffer. This method is
    * useful for manually appending data to this buffer (which is typically
    * followed by a call to extend() afterwards).
    *
    * @return the first invalid unsigned byte at the end of this buffer.
    */
   virtual unsigned char* uend() const;

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
   virtual int freeSpace() const;

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

   /**
    * Returns true if the underlying byte array is managed by this
    * ByteBuffer (meaning it is heap-allocated and will be deleted upon
    * the destruction of this ByteBuffer).
    *
    * @return true if the underlying byte array is managed by this ByteBuffer.
    */
   virtual bool isManaged() const;
};

// typedef for reference-counted ByteBuffer
typedef monarch::rt::Collectable<ByteBuffer> ByteBufferRef;

} // end namespace io
} // end namespace db
#endif
