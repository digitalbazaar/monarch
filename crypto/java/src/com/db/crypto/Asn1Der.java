/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.crypto;

import java.math.BigInteger;
import java.util.Iterator;
import java.util.Vector;

import sun.security.util.DerInputStream;
import sun.security.util.DerOutputStream;
import sun.security.util.ObjectIdentifier;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A class for storing data using the Abstract Syntax Notation Number One
 * format using DER (Distinguished Encoding Rules) encoding.
 *
 * Abstract Syntax Notation Number One (ASN.1) is used to define the
 * abstract syntax of information without restricting the way the
 * information is encoded for transmission. It provides a standard that
 * allows for open systems communication. ASN.1 defines the syntax of
 * information data and a number of simple data types as well as a
 * notation for describing them and specifying values for them.
 *
 * The Digital Signature Algorithm (DSA) creates public and private keys
 * that are often stored in X.509 or PKCS#X formats -- which use
 * ASN.1 (encoded in DER format). This class provides the most basic
 * functionality required to store and load DSA keys that are encoded
 * according to ASN.1. 
 *
 * The most common binary encodings for ASN.1 are BER (Basic Encoding Rules)
 * and DER (Distinguished Encoding Rules). DER is just a subset of BER
 * that has stricter requirements for how data must be encoded.
 *
 * Each ASN.1 structure has a tag (a byte identifying the ASN.1 structure
 * type) and a byte array for the value of this ASN1 structure which
 * may be data or a list of ASN.1 structures.
 *
 * Each ASN.1 structure using BER is (Tag-Length-Value):
 * 
 * | byte 0 | bytes X | bytes Y |
 * |--------|---------|----------
 * |  tag   | length  |  value  |
 *
 * ASN.1 allows for tags to be of "High-tag-number form" which allows
 * a tag to be two or more octets, but that is not supported by this
 * class. A tag is only 1 byte. Bits 1-5 give the tag number, 6 indicates
 * whether or not the ASN.1 value is constructed, and bits 7 and 8 give
 * give the class. For this particular implementation, all that is
 * important is looking up the tag identifier for universal data types
 * (having a class of 0). Below are some universal data types:
 *
 * UNIVERSAL 0 Reserved for use by the encoding rules
 * UNIVERSAL 1 Boolean type
 * UNIVERSAL 2 Integer type
 * UNIVERSAL 3 Bitstring type
 * UNIVERSAL 4 Octetstring type
 * UNIVERSAL 5 Null type
 * UNIVERSAL 6 Object identifier type
 * UNIVERSAL 7 Object descriptor type
 * UNIVERSAL 8 External type and Instance-of type
 * UNIVERSAL 9 Real type
 * UNIVERSAL 10 Enumerated type
 * UNIVERSAL 11 Embedded-pdv type
 * UNIVERSAL 12 UTF8String type
 * UNIVERSAL 13 Relative object identifier type
 * UNIVERSAL 14-15 Reserved for future editions
 * UNIVERSAL 16 Sequence and Sequence-of types
 * UNIVERSAL 17 Set and Set-of types
 * UNIVERSAL 18-22, 25-30 Character string types
 * UNIVERSAL 23-24 Time types
 *
 * The length of an ASN.1 structure is specified after the tag
 * identifier. The length may take up 1 or more bytes, it depends
 * on the length of the value of the ASN.1 structure. DER encoding
 * requires that if the ASN.1 structure has a value that has a
 * length greater than 127, more than 1 byte will be used to store
 * its length, otherwise just one byte will be used.
 *
 * In the case that the length of the ASN.1 value is less than
 * 127, 1 octet (byte) is used to store the "short form" length. The
 * 8th bit has value 0 indicating the length is "short form" and 
 * not "long form" and bits 7-1 give the length of the data. (The 8th
 * bit is the left-most, most significant bit).
 *
 * In the case that the length of the ASN.1 value is greater
 * than 127, 2 to 127 octets (bytes) are used to store the "long form"
 * length. The first byte's 8th bit is set to 1 to indicate the length
 * is "long form." Bits 7-1 give the number of additional octets. All
 * following octets are in base 256 with the most significant digit
 * first. So, for instance, if the length of a value was 257,
 * the first byte would be set to:
 *
 * 10000010 = 130 = 0x82.
 *
 * This indicates there are 2 octets (base 256) for the length. The second
 * and third bytes (the octets just mentioned) would store the length in
 * base 256:
 *
 * octet 2: 00000001 = 1 * 256^1 = 256
 * octet 3: 00000001 = 1 * 256^0 = 1
 * total = 257
 *
 * The algorithm for converting a C# integer value of 257 to base-256 is:
 *
 * int length = 257;
 * byte b1 = (byte)(length >> 8); (shift right by 256)
 * byte b2 = (byte)length;
 *
 * or for the general case:
 * int octets = x;
 * while(i <= octets)
 * b[octets - i + 1] = (byte)(length >> (8 * i));
 * 
 * FUTURE CODE: This class should be split into a separate ASN.1 class that
 * only represents the structure (syntactically) of data -- and the actual
 * data format for DER can remain in this class. Sun has closed source
 * for using DER so if we want to avoid that we need to provide the source
 * and a good API for handling ASN.1/DER. 
 *
 * @author Dave Longley
 */
public class Asn1Der
{
   /**
    * The tag byte for this ASN.1 structure.
    */
   protected byte mTag;

   /**
    * The value for this ASN.1 structure. If this structure contains
    * a list of ASN.1s, it will be used to construct this member whenever
    * converting to bytes.
    */
   protected byte[] mValue;
   
   /**
    * A list of ASN1Der structures used to construct this ASN.1 DER structure,
    * if any.
    */
   protected Vector mAsn1Ders;
   
   /**
    * The ASN1Type of None.
    */
   public static final byte ASN1_TYPE_NONE = 0;
   
   /**
    * The ASN1Type of Boolean.
    */
   public static final byte ASN1_TYPE_BOOLEAN = 1;
   
   /**
    * The ASN1Type of Integer.
    */
   public static final byte ASN1_TYPE_INTEGER = 2;

   /**
    * The ASN1Type of Bit String.
    */
   public static final byte ASN1_TYPE_BITSTRING = 3;

   /**
    * The ASN1Type of Octet String.
    */
   public static final byte ASN1_TYPE_OCTETSTRING = 4;

   /**
    * The ASN1Type of Null.
    */
   public static final byte ASN1_TYPE_NULL = 5;
   
   /**
    * The ASN1Type of OID.
    */
   public static final byte ASN1_TYPE_OID = 6;

   /**
    * The ASN1Type of Sequence.
    */
   public static final byte ASN1_TYPE_SEQUENCE = 16;

   /**
    * The ASN1Type of Set.
    */
   public static final byte ASN1_TYPE_SET = 17;

   /**
    * Creates a new ASN.1 DER structure with no set type, null value,
    * and an empty list of internal ASN.1 structures.
    */
   public Asn1Der()
   {
      this(ASN1_TYPE_NONE, null);
   }

   /**
    * Creates a new ASN.1 DER structure with the specified tag, null value,
    * and an empty list of internal ASN.1 structures.
    * 
    * @param tag the tag for this ASN.1 structure.
    */
   public Asn1Der(byte tag)
   {
      this(tag, null);
   }
   
   /**
    * Creates a new ASN.1 structure with the specified tag, the
    * specified value, and an empty list of internal ASN.1 structures.
    * 
    * @param tag the tag for this ASN.1 structure.
    * @param val the value for this ASN.1 structure.
    */
   public Asn1Der(byte tag, byte[] val)
   {
      mTag = tag;
      mValue = val;
      mAsn1Ders = new Vector();
   }
   
   /**
    * Creates a new ASN.1 structure that is converted from a byte array.
    * 
    * @param bytes the byte array to convert from.
    */
   public Asn1Der(byte[] bytes)   
   {
      this(bytes, 0);
   }
   
   /**
    * Creates a new ASN.1 structure that is converted from a byte array.
    * 
    * @param bytes the byte array to convert from.
    * @param offset the offset to start converting from.
    */
   public Asn1Der(byte[] bytes, int offset)
   {
      this();
      
      // convert from bytes
      convertFromBytes(bytes, offset);
   }
   
   /**
    * Converts this ASN.1 DER object to a byte array.
    * 
    * @return the bytes for this ASN.1 DER.
    */
   protected synchronized byte[] convertToBytes()
   {
      byte[] bytes = null;
      
      // if this ASN.1 has a value, convert it
      if(getValue() != null)
      {
         // determine "short form"/"long form" encoding based on length
         if(getValue().length <= 127)
         {
            bytes = new byte[2 + getValue().length];
            bytes[0] = getTag();
            bytes[1] = (byte)getValue().length;
            
            // copy data into bytes
            System.arraycopy(getValue(), 0, bytes, 2, getValue().length);
         }
         else
         {
            int max = 0;
         
            // currently only supports up to a 32-bit integer (4 octets)
            // (restricted because Value.Length is a 32-bit integer)
            for(int octets = 1; octets <= 4; octets++)
            {
               max = (max << 8) + 255;
               
               if(getValue().length <= max)
               {
                  bytes = new byte[2 + octets + getValue().length];
                  bytes[0] = getTag();
                  bytes[1] = (byte)(128 + octets);
                  
                  for(int i = 0; i < octets; i++)
                  {
                     bytes[octets - i + 1] =
                        (byte)(getValue().length >> (8 * i));
                  }
                  
                  // copy data into bytes
                  System.arraycopy(
                     getValue(), 0, bytes, 2 + octets, getValue().length);
                  
                  break;
               }
            }
         }
      }
      else if(getAsn1DerCount() > 0)
      {
         // convert the list of ASN.1s and store the value as the value
         // of this ASN.1 structure
         int length = 0;
         Vector values = new Vector();
         for(int i = 0; i < getAsn1DerCount(); i++)
         {
            byte[] derBytes = getAsn1Der(i).convertToBytes();
            length += derBytes.length;
            values.add(derBytes);
         }
         
         // rebuild value byte array
         int offset = 0;
         setValue(new byte[length]);
         for(Iterator i = values.iterator(); i.hasNext();)
         {
            byte[] derBytes = (byte[])i.next();
            
            // copy the derBytes to the internal value, increment offset
            System.arraycopy(derBytes, 0, getValue(), offset, derBytes.length);
            offset += derBytes.length;
         }

         // convert to bytes now that value is set
         bytes = convertToBytes();
      }
      else
      {
         // no value or list
         bytes = new byte[2];
         bytes[0] = getTag();
         bytes[1] = 0;
      }
      
      return bytes;
   }
   
   /**
    * Converts the passed byte array into this ASN.1 DER object.
    * 
    * @param bytes the bytes for this ASN.1 DER.
    * @param offset the offset to start converting at.
    * 
    * @return the number of bytes used in the conversion.
    */
   protected synchronized int convertFromBytes(byte[] bytes, int offset)
   {
      int rval = 0;
      
      // reset the tag, value, and list
      setTag(ASN1_TYPE_NONE);
      setValue(null);
      mAsn1Ders.clear();
   
      // the minimum length for an ASN.1 DER structure is 2
      if(bytes != null && bytes.length > 1)
      {
         setTag(bytes[offset]);
         int length = bytes[offset + 1] & 0xFF;
         int valueOffset = offset + 2;
         
         // see if the length is "long form"
         int octets = length - 128;
         if(octets > 0)
         {
            // an index out of range exception will be thrown if the length
            // isn't set properly
            length = 0;
            for(int i = 0; i < octets; i++)
            {
               length = length * 256 + (bytes[valueOffset++] & 0xFF);
            }
         }
         
         // set the value
         setValue(new byte[length]);
         System.arraycopy(bytes, valueOffset, getValue(), 0, length);
         
         // if the tag indicates this value was constructed from
         // other ASN.1s, then convert the list of ASN.1s
         if(isConstructed())
         {
            convertListFromBytes(getValue());
         }
         
         // return the total number of converted bytes
         rval = valueOffset - offset + length;
      }
      
      return rval;
   }
   
   /**
    * Converts the passed DER byte array representing an ASN1List.
    * 
    * @param asn1List the ASN.1 list in DER binary form.
    */
   protected synchronized void convertListFromBytes(byte[] asn1List)
   {
      // the minimum length for an ASN.1 DER structure is 2
      int offset = 0;
      while(offset < (asn1List.length - 1))
      {
         Asn1Der asn1 = new Asn1Der();
         int inc = asn1.convertFromBytes(asn1List, offset);

         // ensure that a positive number of bytes were converted
         if(inc <= 0)
         {
            break;
         }
         
         // add new ASN.1, but do not call AddASN1() because
         // the value byte array should not be rebuilt here
         mAsn1Ders.add(asn1);
         
         // increment offset
         offset += inc;
      }
   }
   
   /**
    * Adds an ASN.1 DER structure to this ASN.1's list of ASN.1 DERs.
    * 
    * The value byte array of this ASN.1 structure will be rebuilt the next
    * time the value of this ASN.1 structure is requested in binary form.
    * 
    * @param asn1 the ASN.1 to add.
    */
   public synchronized void addAsn1Der(Asn1Der asn1)
   {
      mAsn1Ders.add(asn1);
      
      // if tag not set to constructed, set it
      if(!isConstructed())
      {
         setConstructed(true);
      }
      
      // must rebuild value byte array
      setValue(null);
   }
   
   /**
    * Removes an ASN.1 DER structure from this ASN.1's list of ASN.1 DERs.
    * 
    * The value byte array of this ASN.1 structure will be rebuilt the next
    * time the value of this ASN.1 structure is requested in binary form.
    * 
    * @param asn1 the ASN.1 to remove.
    */
   public synchronized void removeAsnDer1(Asn1Der asn1)
   {
      mAsn1Ders.remove(asn1);
      
      // if count is zero now, then set tag to not constructed
      if(getAsn1DerCount() == 0)
      {
         setConstructed(false);
      }
      
      // must rebuild value byte array
      setValue(null);
   }
   
   /**
    * Sets the tag for this ASN.1 DER.
    * 
    * @param tag the tag for this ASN.1 DER.
    */
   public void setTag(byte tag)
   {
      mTag = tag;
   }
   
   /**
    * Gets the tag for this ASN.1 DER.
    * 
    * @return the tag for this ASN.1 DER.
    */
   public byte getTag()
   {
      return mTag;
   }
   
   /**
    * Gets the type for this ASN.1 DER.
    * 
    * @return the type for this ASN.1 DER.
    */
   public byte getType()
   {
      byte type = (byte)(getTag() & 0x1F);
      return type;
   }
   
   /**
    * Sets whether or not this ASN.1 is constructed from other ASN.1's.
    * 
    * @param constructed true if this ASN.1 is constructed from other ASN.1's
    *                    false if not.
    */
   public void setConstructed(boolean constructed)
   {
      if(constructed)
      {
         // turn on 6th bit -- 32 = 0x20
         setTag((byte)(getTag() | 0x20));
      }
      else
      {
         
         // and with 11011111 (0xDF) to zero out 6th bit
         // turn off 6th bit -- 255 - 32 = 224 = 0xDF
         setTag((byte)(getTag() & 0xDF));
      }
   }
   
   /**
    * Returns true if this ASN.1 is constructed from other ASN.1's
    * and false if it is not.
    * 
    * @return true if this ASN.1 is constructed, false if not.
    */
   public boolean isConstructed()
   {
      boolean rval = false;
         
      // if the tag indicates this value was constructed from
      // other ASN.1s, then convert the list of ASN.1s
      // (to check this, look at the 6th bit (0x20 = 32))
      if((getTag() & 0x20) == 0x20)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Sets the value for this ASN.1
    * 
    * @param value the value for this ASN.1
    */
   public void setValue(byte[] value)
   {
      mValue = value;
   }
   
   /**
    * Gets the value for this ASN.1
    * 
    * @return the value for this ASN.1
    */
   public byte[] getValue()
   {
      return mValue;
   }
   
   /**
    * Gets the value of this ASN.1 as a BigInt byte array.
    * 
    * @return the value of this ASN.1 as a BigInt byte array.
    */
   public byte[] getBigIntValue()
   {
      return getBigIntValue(getValue());
   }
   
   /**
    * Converts this ASN.1 to binary form.
    * 
    * @return this ASN.1 as a byte array.
    */
   public byte[] getBytes()
   {
      return convertToBytes();
   }
   
   /**
    * Gets the ASN.1 encapsulated by this ASN.1 at the specified index.
    * 
    * @param index the index of the ASN.1 to retrieve.
    * 
    * @return the ASN.1 at the specified index.
    */
   public Asn1Der getAsn1Der(int index)
   {
      return (Asn1Der)mAsn1Ders.get(index);
   }
   
   /**
    * Gets the number of ASN.1 DER's encapsulated by this ASN.1 DER.
    * 
    * @return the number of ASN.1 DER's encapsulated by this ASN.1 DER.
    */
   public int getAsn1DerCount()
   {
      return mAsn1Ders.size();
   }
   
   /**
    * Encodes an Object Identifier (OID).
    * 
    * @param oidStr the OID string to convert to a byte array.
    * 
    * @return the byte array for the OID.
    */
   public static byte[] encodeOid(String oidStr)
   {
      byte[] bytes = null;
      
      try
      {
         // TEMP CODE: sun's closed source is used here temporarily
         // until we provide a method for encoding an OID
         
         // create object identifier
         ObjectIdentifier oid = new ObjectIdentifier(oidStr);
         
         // get DER output stream
         DerOutputStream dos = new DerOutputStream();
         dos.putOID(oid);
         bytes = dos.toByteArray();
      }
      catch(Throwable t)
      {
         LoggerManager.getLogger("dbcrypto").error(
            "Exception thrown while encoding OID!");
         LoggerManager.getLogger("dbcrypto").debug(Logger.getStackTrace(t));
      }
      
      return bytes;
   }
   
   /**
    * Converts an Object Identifier (OID) to an ASN.1 DER structure.  
    * 
    * @param asn1Der the ASN.1 structure representing the OID or null if
    *                the conversion fails.
    * 
    * @return the OID string or a blank string.
    */
   public static String asn1DerToOid(Asn1Der asn1Der)
   {
      String oidStr = "";
      
      try
      {
         // TEMP CODE: sun's closed source is used here temporarily
         // until we provide a method for deecoding an OID
         
         DerInputStream dis = new DerInputStream(asn1Der.getBytes());
         
         // get object identifier
         ObjectIdentifier oid = dis.getOID();
         
         // get OID string
         oidStr = oid.toString();
      }
      catch(Throwable t)
      {
         LoggerManager.getLogger("dbcrypto").error(
            "Exception thrown while decoding OID!");
         LoggerManager.getLogger("dbcrypto").debug(Logger.getStackTrace(t));
      }
      
      return oidStr;
   }
   
   /**
    * Converts an Object Identifier (OID) to an ASN.1 DER structure.  
    * 
    * @param oidStr the OID string to convert to an ASN.1 DER structure.
    * 
    * @return the ASN.1 structure representing the OID or null if
    *         the conversion fails.
    */
   public static Asn1Der oidToAsn1Der(String oidStr)
   {
      Asn1Der asn1 = null;
      
      if(oidStr != null)
      {
         byte[] encoded = encodeOid(oidStr);
         if(encoded != null)
         {
            asn1 = new Asn1Der(encoded, 0);
         }
      }
      
      return asn1;
   }
   
   /**
    * Converts an unsigned big integer to an ASN.1 DER structure.
    * 
    * @param bigInt the big integer to convert.
    * 
    * @return the ASN.1 structure representing the big integer or
    *         null if the conversion fails.
    */
   public static Asn1Der unsignedBigIntToAsn1Der(BigInteger bigInt)
   {
      return unsignedBigIntToAsn1Der(ASN1_TYPE_INTEGER, bigInt.toByteArray());
   }   
   
   /**
    * Converts an unsigned big integer in a binary format to an
    * ASN.1 DER structure.
    * 
    * @param bigInt the big integer to convert.
    * 
    * @return the ASN.1 structure representing the big integer or
    *         null if the conversion fails.
    */
   public static Asn1Der unsignedBigIntToAsn1Der(byte[] bigInt)
   {
      return unsignedBigIntToAsn1Der(ASN1_TYPE_INTEGER, bigInt);
   }
   
   /**
    * Converts an unsigned big integer to an ASN.1 structure.
    * 
    * @param type the ASN.1 type to use.
    * @param bigInt the big integer to convert.
    * 
    * @return the ASN.1 structure representing the big integer or
    *         null if the conversion fails.
    */
   public static Asn1Der unsignedBigIntToAsn1Der(byte type, BigInteger bigInt)   
   {
      return unsignedBigIntToAsn1Der(type, bigInt.toByteArray());
   }
   
   /**
    * Converts an unsigned big integer in a binary format to
    * an ASN.1 structure.
    * 
    * @param type the ASN.1 type to use.
    * @param bigInt the big integer to convert.
    * 
    * @return the ASN.1 structure representing the big integer or
    *         null if the conversion fails.
    */
   public static Asn1Der unsignedBigIntToAsn1Der(byte type, byte[] bigInt)
   {
      Asn1Der asn1 = null;
      
      if(bigInt != null)
      {
         // if the first bit of the first byte (0x80 = 128) is set, then we
         // need to add a leading zero byte for sign
         if((bigInt[0] & 0x80) != 0)
         {
            byte[] bytes = new byte[bigInt.length + 1];
            System.arraycopy(bigInt, 0, bytes, 1, bigInt.length);
            bigInt = bytes;
         }
         
         if(type == ASN1_TYPE_OCTETSTRING)
         {
            // convert big int to octet string
            bigInt = bigIntToOctetString(bigInt);
         }
         
         asn1 = new Asn1Der(type, bigInt);
      }
      
      return asn1;
   }
   
   /**
    * Converts an array of bytes to an array of bytes that represents
    * an unsigned big int. The first byte of the passed array will be
    * stripped off if it's most significant bit is not zero.
    * 
    * @param bytes the array of bytes that represents an unsigned big int
    *              to convert.
    * 
    * @return the big int value.
    */
   public static byte[] getBigIntValue(byte[] bytes)
   {
      byte[] rval = null;
      
      if(bytes != null)
      {
         if(bytes.length == 0)
         {
            rval = bytes;
         }
         else
         {
            if(bytes[0] == 0)
            {
               if(bytes.length > 1)
               {
                  if((bytes[1] & 0x80) != 0)
                  {
                     // the first byte was added, so leave it out
                     rval = new byte[bytes.length - 1];
                     System.arraycopy(bytes, 1, rval, 0, rval.length);
                  }
                  else
                  {
                     // the first byte was not added, include it
                     rval = bytes;
                  }
               }
               else
               {
                  rval = bytes;
               }
            }
            else
            {
               rval = bytes;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the value of an octet string.
    * 
    * @param octetString the octet string.
    * 
    * @return the value of the octet string.
    */
   public static byte[] getOctetStringValue(byte[] octetString)
   {
      // convert the octet string into an ASN.1 DER and get the value
      Asn1Der asn1 = new Asn1Der(octetString, 0);
      return asn1.getValue();
   }
   
   /**
    * Converts a big integer into an ASN.1 octet string.
    * 
    * @param bigInt the big integer to convert.
    * 
    * @return the ASN.1 octet string.
    */
   public static byte[] bigIntToOctetString(byte[] bigInt)
   {
      // build an ASN.1 with type integer and get the bytes
      Asn1Der asn1 = new Asn1Der(ASN1_TYPE_INTEGER, bigInt);
      return asn1.getBytes();
   }
}
