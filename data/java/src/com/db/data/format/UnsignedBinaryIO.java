/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.ByteArrayInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * UnsignedBinary has utility methods for allowing reading and writing
 * unsigned binary data. 
 * 
 * @author Dave Longley
 */
public class UnsignedBinaryIO
{
   /**
    * Reads unsigned byte that has the most significant bit first (left). This
    * is the default way in which bytes are read in Java.
    * 
    * The byte must be stored as an int because Java's bytes are unsigned.
    * 
    * @param buffer the buffer to read from.
    * @param offset the offset to start reading from.
    * @param length the number of valid bytes in the buffer.
    * 
    * @return the unsigned byte.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static int readUnsignedByte(byte[] buffer, int offset, int length)
   throws IOException
   {
      ByteArrayInputStream bais =
         new ByteArrayInputStream(buffer, offset, length);
      return readUnsignedByte(bais);
   }
   
   /**
    * Reads an unsigned short that is stored in the Intel or
    * Little Endian byte order. That is, least significant byte first.
    * 
    * An unsigned short is stored in 2 bytes: 00 00
    * 
    * The short must be read as an int because Java's shorts are unsigned.
    * 
    * @param buffer the buffer to read from.
    * @param offset the offset to start reading from.
    * @param length the number of valid bytes in the buffer.
    * 
    * @return the unsigned short.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static int readUnsignedShort(byte[] buffer, int offset, int length)
   throws IOException
   {
      ByteArrayInputStream bais =
         new ByteArrayInputStream(buffer, offset, length);
      return readUnsignedShort(bais);
   }
   
   /**
    * Reads unsigned integer (4 bytes) that is stored in the Intel or
    * Little Endian byte order. That is, least significant byte first.
    * 
    * An unsigned int is stored in 4 bytes: 00 00 00 00
    * 
    * The integer must be stored as a long because Java uses unsigned
    * integers for "int". 
    * 
    * @param buffer the buffer to read from.
    * @param offset the offset to start reading from.
    * @param length the number of valid bytes in the buffer.
    * 
    * @return the unsigned integer.
    * 
    * @throws IOException
    */
   public static long readUnsignedInt(byte[] buffer, int offset, int length)
   throws IOException
   {
      ByteArrayInputStream bais =
         new ByteArrayInputStream(buffer, offset, length);
      return readUnsignedInt(bais);
   }
   
   /**
    * Reads unsigned byte that has the most significant bit first (left). This
    * is the default way in which bytes are read in Java.
    * 
    * The byte must be stored as an int because Java's bytes are unsigned.
    * 
    * @param is the input stream to read from.
    * 
    * @return the unsigned byte.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static int readUnsignedByte(InputStream is) throws IOException
   {
      // read the byte
      int b = is.read();
      
      // ensure we aren't at the end of the stream
      if(b == -1)
      {
         // throw an exception because this method should only be
         // called when there is more data to be read
         throw new EOFException();
      }
      
      return b;
   }
   
   /**
    * Reads an unsigned short that is stored in the Intel or
    * Little Endian byte order. That is, least significant byte first.
    * 
    * An unsigned short is stored in 2 bytes: 00 00
    * 
    * The short must be read as an int because Java's shorts are unsigned.
    * 
    * @param is the input stream to read from.
    * 
    * @return the unsigned short.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static int readUnsignedShort(InputStream is) throws IOException
   {
      int rval = 0;
      
      // read the first unsigned byte
      int b1 = readUnsignedByte(is);
      
      // read the second unsigned byte
      int b2 = readUnsignedByte(is);
      
      // shift the second byte to the left, it is more significant
      // OR it with the first byte to combine them
      rval = (b2 << 8) | b1;
      
      return rval;
   }
   
   /**
    * Reads unsigned integer (4 bytes) that is stored in the Intel or
    * Little Endian byte order. That is, least significant byte first.
    * 
    * An unsigned int is stored in 4 bytes: 00 00 00 00
    * 
    * The integer must be stored as a long because Java uses unsigned
    * integers for "int". 
    * 
    * @param is the input stream to read the integer from.
    * 
    * @return the unsigned integer.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static long readUnsignedInt(InputStream is) throws IOException
   {
      long rval = 0;
      
      // read the first unsigned short
      long s1 = readUnsignedShort(is);
      
      // read the second unsigned short
      long s2 = readUnsignedShort(is);
      
      // shift the second short to the left, it is more significant
      // OR it with the first byte to combine them
      rval = s2 << 16 | s1;
      
      return rval;
   }
   
   /**
    * Writes an unsigned byte.
    * 
    * The byte passed as an int because Java's bytes are unsigned.
    * 
    * The byte is written to the passed byte array.
    * 
    * @param ubyte the unsigned byte to write.
    * @param buffer the buffer to write the unsigned byte to.
    * @param offset the offset at which to write the unsigned byte.
    */
   public static void writeUnsignedByte(int ubyte, byte[] buffer, int offset)
   {
      buffer[offset] = (byte)(ubyte & 0xff);   
   }
   
   /**
    * Writes an unsigned short in the Intel or Little Endian byte order.
    * That is, least significant byte first.
    * 
    * An unsigned short is 2 bytes: 00 00
    * 
    * The short passed as an int because Java's shorts are unsigned.
    * 
    * The short is written to the passed byte array.
    * 
    * @param ushort the unsigned short to write.
    * @param buffer the buffer to write the unsigned short to.
    * @param offset the offset at which to write the unsigned short.
    */
   public static void writeUnsignedShort(int ushort, byte[] buffer, int offset)
   {
      // get the less significant byte for the short
      buffer[offset] = (byte)(ushort & 0xff);
      
      // get the more significant byte for the short
      buffer[offset + 1] = (byte)((ushort >> 8) & 0xff);
   }   
   
   /**
    * Writes an unsigned integer (4 bytes) in the Intel or Little Endian
    * byte order. That is, least significant byte first.
    * 
    * An unsigned int is 4 bytes: 00 00 00 00
    * 
    * The integer must be passed as a long because Java uses unsigned
    * integers for "int".
    * 
    * The integer is written to the passed byte array.
    * 
    * @param uint the unsigned int to write.
    * @param buffer the buffer to write the unsigned int to.
    * @param offset the offset at which to write the unsigned int.
    */
   public static void writeUnsignedInt(long uint, byte[] buffer, int offset)
   {
      // write the least significant short
      writeUnsignedShort((int)(uint & 0xffff), buffer, offset);
      
      // write the most significant short
      writeUnsignedShort((int)((uint >> 16) & 0xffff), buffer, offset + 2);
   }
   
   /**
    * Writes an unsigned byte.
    * 
    * The byte passed as an int because Java's bytes are unsigned.
    * 
    * The byte is written to the passed byte array.
    * 
    * @param ubyte the unsigned byte to write.
    * @param os the OutputStream to write to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static void writeUnsignedByte(int ubyte, OutputStream os)
   throws IOException
   {
      os.write((byte)(ubyte & 0xff));
   }
   
   /**
    * Writes an unsigned short in the Intel or Little Endian byte order.
    * That is, least significant byte first.
    * 
    * An unsigned short is 2 bytes: 00 00
    * 
    * The short passed as an int because Java's shorts are unsigned.
    * 
    * The short is written to the passed byte array.
    * 
    * @param ushort the unsigned short to write.
    * @param os the OutputStream to write to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static void writeUnsignedShort(int ushort, OutputStream os)
   throws IOException
   {
      // get the less significant byte for the short
      os.write((byte)(ushort & 0xff));
      
      // get the more significant byte for the short
      os.write((byte)((ushort >> 8) & 0xff));
   }   
   
   /**
    * Writes an unsigned integer (4 bytes) in the Intel or Little Endian
    * byte order. That is, least significant byte first.
    * 
    * An unsigned int is 4 bytes: 00 00 00 00
    * 
    * The integer must be passed as a long because Java uses unsigned
    * integers for "int".
    * 
    * The integer is written to the passed byte array.
    * 
    * @param uint the unsigned int to write.
    * @param os the OutputStream to write to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static void writeUnsignedInt(long uint, OutputStream os)
   throws IOException
   {
      // write the least significant short
      writeUnsignedShort((int)(uint & 0xffff), os);
      
      // write the most significant short
      writeUnsignedShort((int)((uint >> 16) & 0xffff), os);
   }
}
