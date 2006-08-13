/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A class for storing data using the Abstract Syntax Notation Number One
   /// format using DER (Distinguished Encoding Rules) encoding.
   /// </summary>
   ///
   /// <remarks>
   /// Abstract Syntax Notation Number One (ASN.1) is used to define the
   /// abstract syntax of information without restricting the way the
   /// information is encoded for transmission. It provides a standard that
   /// allows for open systems communication. ASN.1 defines the syntax of
   /// information data and a number of simple data types as well as a
   /// notation for describing them and specifying values for them.
   ///
   /// The Digital Signature Algorithm (DSA) creates public and private keys
   /// that are often stored in X.509 or PKCS#X formats -- which use
   /// ASN.1 (encoded in DER format). This class provides the most basic
   /// functionality required to store and load DSA keys that are encoded
   /// according to ASN.1. 
   ///
   /// The most common binary encodings for ASN.1 are BER (Basic Encoding Rules)
   /// and DER (Distinguished Encoding Rules). DER is just a subset of BER
   /// that has stricter requirements for how data must be encoded.
   ///
   /// Each ASN.1 structure has a tag (a byte identifying the ASN.1 structure
   /// type) and a byte array for the value of this ASN1 structure which
   /// may be data or a list of ASN.1 structures.
   ///
   /// Each ASN.1 structure using BER is (Tag-Length-Value):
   /// 
   /// | byte 0 | bytes X | bytes Y |
   /// |--------|---------|----------
   /// |  tag   | length  |  value  |
   ///
   /// ASN.1 allows for tags to be of "High-tag-number form" which allows
   /// a tag to be two or more octets, but that is not supported by this
   /// class. A tag is only 1 byte. Bits 1-5 give the tag number, 6 indicates
   /// whether or not the ASN.1 value is constructed, and bits 7 and 8 give
   /// give the class. For this particular implementation, all that is
   /// important is looking up the tag identifier for universal data types
   /// (having a class of 0). Below are some universal data types:
   ///
   /// UNIVERSAL 0 Reserved for use by the encoding rules
   /// UNIVERSAL 1 Boolean type
   /// UNIVERSAL 2 Integer type
   /// UNIVERSAL 3 Bitstring type
   /// UNIVERSAL 4 Octetstring type
   /// UNIVERSAL 5 Null type
   /// UNIVERSAL 6 Object identifier type
   /// UNIVERSAL 7 Object descriptor type
   /// UNIVERSAL 8 External type and Instance-of type
   /// UNIVERSAL 9 Real type
   /// UNIVERSAL 10 Enumerated type
   /// UNIVERSAL 11 Embedded-pdv type
   /// UNIVERSAL 12 UTF8String type
   /// UNIVERSAL 13 Relative object identifier type
   /// UNIVERSAL 14-15 Reserved for future editions
   /// UNIVERSAL 16 Sequence and Sequence-of types
   /// UNIVERSAL 17 Set and Set-of types
   /// UNIVERSAL 18-22, 25-30 Character string types
   /// UNIVERSAL 23-24 Time types
   ///
   /// The length of an ASN.1 structure is specified after the tag
   /// identifier. The length may take up 1 or more bytes, it depends
   /// on the length of the value of the ASN.1 structure. DER encoding
   /// requires that if the ASN.1 structure has a value that has a
   /// length greater than 127, more than 1 byte will be used to store
   /// its length, otherwise just one byte will be used.
   ///
   /// In the case that the length of the ASN.1 value is less than
   /// 127, 1 octet (byte) is used to store the "short form" length. The
   /// 8th bit has value 0 indicating the length is "short form" and 
   /// not "long form" and bits 7-1 give the length of the data. (The 8th
   /// bit is the left-most, most significant bit).
   ///
   /// In the case that the length of the ASN.1 value is greater
   /// than 127, 2 to 127 octets (bytes) are used to store the "long form"
   /// length. The first byte's 8th bit is set to 1 to indicate the length
   /// is "long form." Bits 7-1 give the number of additional octets. All
   /// following octets are in base 256 with the most significant digit
   /// first. So, for instance, if the length of a value was 257,
   /// the first byte would be set to:
   ///
   /// 10000010 = 130 = 0x82.
   ///
   /// This indicates there are 2 octets (base 256) for the length. The second
   /// and third bytes (the octets just mentioned) would store the length in
   /// base 256:
   ///
   /// octet 2: 00000001 = 1 * 256^1 = 256
   /// octet 3: 00000001 = 1 * 256^0 = 1
   /// total = 257
   ///
   /// The algorithm for converting a C# integer value of 257 to base-256 is:
   ///
   /// int length = 257;
   /// byte b1 = (byte)(length >> 8); (shift right by 256)
   /// byte b2 = (byte)length;
   ///
   /// or for the general case:
   /// int octets = x;
   /// while(i <= octets)
   /// b[octets - i + 1] = (byte)(length >> (8 * i));
   /// </remarks>
   ///
   /// <author>Dave Longley</author>
   public class ASN1
   {
      /// <summary>
      /// The tag byte for this ASN.1 structure. 
      /// </summary>
      protected byte mTag;
      
      /// <summary>
      /// The value for this ASN.1 structure. If this structure contains
      /// a list of ASN.1s, it will be used to construct this member whenever
      /// converting to bytes.
      /// </summary>
      protected byte[] mValue;
      
      /// <summary>
      /// A list of ASN1 structures used to construct this ASN.1 structure,
      /// if any.
      /// </summary>
      protected ArrayList mASN1s;
      
      /// <summary>
      /// A list of supported ASN.1 types.
      /// </summary>
      public enum ASN1Type : byte
      {
         None        = 0,
         Boolean     = 1,
         Integer     = 2,
         BitString   = 3,
         OctetString = 4,
         Null        = 5,
         OID         = 6,
         Sequence    = 16,
         Set         = 17
      }   
      
      /// <summary>
      /// Creates a new ASN.1 structure with no set type, null value,
      /// and an empty list of internal ASN.1 structures.
      /// </summary>
      public ASN1() :
         this(ASN1Type.None, null)
      {
      }
      
      /// <summary>
      /// Creates a new ASN1 structure with the specified type, null value,
      /// and an empty list of internal ASN.1 structures.
      /// </summary>
      ///
      /// <param name="type">the type for this ASN.1 structure.</param>
      public ASN1(ASN1Type type) :
         this(type, null)
      {
      }       
      
      /// <summary>
      /// Creates a new ASN.1 structure with the specified tag, the
      /// specified value, and an empty list of internal ASN.1 structures.
      /// </summary>
      ///
      /// <param name="type">the type for this ASN.1 structure.</param>
      /// <param name="val">the value for this ASN.1 structure.</param>
      public ASN1(ASN1Type type, byte[] val)
      {
         mTag = (byte)type;
         mValue = val;
         mASN1s = new ArrayList();
      }
      
      /// <summary>
      /// Creates a new ASN1 structure with the specified tag, null value,
      /// and an empty list of internal ASN.1 structures.
      /// </summary>
      ///
      /// <param name="tag">the tag for this ASN.1 structure.</param>
      public ASN1(byte tag) :
         this(tag, null)
      {
      }
      
      /// <summary>
      /// Creates a new ASN.1 structure with the specified tag, the
      /// specified value, and an empty list of internal ASN.1 structures.
      /// </summary>
      ///
      /// <param name="tag">the tag for this ASN.1 structure.</param>
      /// <param name="val">the value for this ASN.1 structure.</param>
      public ASN1(byte tag, byte[] val)
      {
         mTag = tag;
         mValue = val;
         mASN1s = new ArrayList();
      }
      
      /// <summary>
      /// Creates a new ASN.1 structure that is converted from a byte array.
      /// </summary>
      ///
      /// <param name="bytes">the byte array to convert from.</param>
      public ASN1(byte[] bytes) :
         this()
      {
         ConvertFromBytes(bytes, 0);
      }
      
      /// <summary>
      /// Converts this ASN1 object to a byte array.
      /// </summary>
      ///
      /// <remarks></remarks>
      ///
      /// <returns>the bytes for this ASN.1.</returns>
      [MethodImpl(MethodImplOptions.Synchronized)]
      protected virtual byte[] ConvertToBytes()
      {
         byte[] bytes = null;
         
         // if this ASN.1 has a value, convert it
         if(Value != null)
         {
            // determine "short form"/"long form" encoding based on length
            if(Value.Length <= 127)
            {
               bytes = new byte[2 + Value.Length];
               bytes[0] = Tag;
               bytes[1] = (byte)Value.Length;
               
               // copy data into bytes
               Array.Copy(Value, 0, bytes, 2, Value.Length);
            }
            else
            {
               int max = 0;
            
               // currently only supports up to a 32-bit integer (4 octets)
               // (restricted because Value.Length is a 32-bit integer)
               for(int octets = 1; octets <= 4; octets++)
               {
                  max = (max << 8) + 255;
               
                  if(Value.Length <= max)
                  {
                     bytes = new byte[2 + octets + Value.Length];
                     bytes[0] = Tag;
                     bytes[1] = (byte)(128 + octets);
                     
                     for(int i = 0; i < octets; i++)
                     {
                        bytes[octets - i + 1] = (byte)(Value.Length >> (8 * i));
                     }
                     
                     // copy data into bytes
                     Array.Copy(Value, 0, bytes, 2 + octets, Value.Length);
                     
                     break;
                  }
               }
            }
         }
         else if(mASN1s.Count > 0)
         {
            // convert the list of ASN.1s and store the value as the value
            // of this ASN.1 structure
            int length = 0;
            ArrayList values = new ArrayList();
            foreach(ASN1 asn1 in mASN1s)
            {
               byte[] asn1Bytes = asn1.ConvertToBytes();
               length += asn1Bytes.Length;
               values.Add(asn1Bytes);
            }
            
            // rebuild value byte array
            int offset = 0;
            Value = new byte[length];
            foreach(byte[] asn1Bytes in values)
            {
               // copy the asn1Bytes to the internal value, increment offset
               Array.Copy(asn1Bytes, 0, Value, offset, asn1Bytes.Length);
               offset += asn1Bytes.Length;
            }

            // convert to bytes now that value is set
            bytes = ConvertToBytes();
         }
         else
         {
            // no value or list
            bytes = new byte[2];
            bytes[0] = Tag;
            bytes[1] = 0;
         }
         
         return bytes;
      }
      
      /// <summary>
      /// Converts the passed byte array into this ASN.1 object.
      /// </summary>
      ///
      /// <param name="bytes">the bytes for this ASN.1.</param>
      /// <param name="offset">the offset to start converting at.</param>
      /// <returns>the number of bytes used in the conversion.</returns>
      [MethodImpl(MethodImplOptions.Synchronized)]
      protected virtual int ConvertFromBytes(byte[] bytes, int offset)
      {
         int rval = 0;
      
         // reset the tag, value, and list
         Tag = (byte)ASN1Type.None;
         Value = null;
         mASN1s.Clear();
      
         // the minimum length for an ASN.1 structure is 2
         if(bytes != null && bytes.Length > 1)
         {
            Tag = bytes[offset];
            int length = bytes[offset + 1];
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
                  length = length * 256 + bytes[valueOffset++];
               }
            }
            
            // set the value
            Value = new byte[length];
            Array.Copy(bytes, valueOffset, Value, 0, length);
            
            // if the tag indicates this value was constructed from
            // other ASN.1s, then convert the list of ASN.1s
            if(Constructed)
            {
               ConvertListFromBytes(Value);
            }
            
            // return the total number of converted bytes
            rval = valueOffset - offset + length;
         }
         
         return rval;
      }
      
      /// <summary>
      /// Converts the passed byte array representing an ASN1List.
      /// </summary>
      ///
      /// <param name="asn1List">the ASN.1 list in binary form.</param>
      [MethodImpl(MethodImplOptions.Synchronized)]
      protected virtual void ConvertListFromBytes(byte[] asn1List)
      {
         // the minimum length for an ASN.1 structure is 2
         int offset = 0;
         while(offset < (asn1List.Length - 1))
         {
            ASN1 asn1 = new ASN1();
            int inc = asn1.ConvertFromBytes(asn1List, offset);

            // ensure that a positive number of bytes were converted
            if(inc <= 0)
            {
               break;
            }
            
            // add new ASN.1, but do not call AddASN1() because
            // the value byte array should not be rebuilt here
            mASN1s.Add(asn1);
            
            // increment offset
            offset += inc;
         }
      }
      
      /// <summary>
      /// Adds an ASN.1 structure to this ASN.1's list of ASN.1s.
      /// </summary>
      /// 
      /// <remarks>
      /// The value byte array of this ASN.1 structure will be rebuilt the
      /// next time the value of this ASN.1 structure is requested in
      /// binary form.
      /// </remarks>
      ///
      /// <param name="asn1">the ASN.1 to add.</param>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual void AddASN1(ASN1 asn1)
      {
         mASN1s.Add(asn1);
         
         // if tag not set to constructed, set it
         if(!Constructed)
         {
            Constructed = true;
         }
         
         // must rebuild value byte array
         Value = null;
      }
      
      /// <summary>
      /// Removes an ASN.1 structure from this ASN.1's list of ASN1s.
      /// </summary>
      ///
      /// <remarks>
      /// The value byte array of this ASN.1 structure will be rebuilt the
      /// next time the value of this ASN.1 structure is requested in
      /// binary form.
      /// </remarks>
      /// 
      /// <param name="asn1">the ASN1 to remove.</param>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual void RemoveASN1(ASN1 asn1)
      {
         mASN1s.Remove(asn1);
         
         // if count is zero now, then set tag to not constructed
         if(mASN1s.Count == 0)
         {
            // and with 11011111 (0xDF) to zero out 6th bit
            Tag = (byte)(Tag & 0xDF);
         }
         
         // must rebuild value byte array
         Value = null;
      }
      
      /// <summary>
      /// Gets/Sets the tag for this ASN.1.
      /// </summary>
      public virtual byte Tag
      {
         get
         {
            return mTag;
         }
         
         set
         {
            mTag = value;
         }
      }
      
      /// <summary>
      /// Gets the type for this ASN.1.
      /// </summary>
      public virtual ASN1Type Type
      {
         get
         {
            byte type = (byte)(Tag & 0x1F);
            return (ASN1Type)type;
         }
      }
      
      /// <summary>
      /// Returns true if this ASN.1 is constructed from other ASN.1's
      /// and false if it is not.
      /// </summary>
      public virtual bool Constructed
      {
         get
         {
            bool rval = false;
            
            // if the tag indicates this value was constructed from
            // other ASN.1s, then convert the list of ASN.1s
            // (to check this, look at the 6th bit (0x20 = 32))
            if((Tag & 0x20) == 0x20)
            {
               rval = true;
            }
            
            return rval;
         }
         
         set
         {
            if(value)
            {
               // turn on 6th bit -- 32 = 0x20
               Tag = (byte)(Tag | 0x20);
            }
            else
            {
               // turn off 6th bit -- 255 - 32 = 224 = 0xDF
               Tag = (byte)(Tag & 0xDF);
            }
         }
      }
      
      /// <summary>
      /// Gets/Sets the value for this ASN.1.
      /// </summary>
      public virtual byte[] Value
      {
         get
         {
            return mValue;
         }
         
         set
         {
            mValue = value;
         }
      }
      
      /// <summary>
      /// Gets the value of this ASN.1 as a BigInt byte array.
      /// </summary>
      public virtual byte[] BigIntValue
      {
         get
         {
            return GetBigIntValue(Value);
         }
      }
      
      /// <summary>
      /// Gets the ASN.1 in binary form.
      /// </summary>
      ///
      /// <returns>the a byte array that represents this ASN1.
      public virtual byte[] Bytes
      {
         get
         {
            return ConvertToBytes();
         }
      }
      
      /// <summary>
      /// Gets the ASN.1 encapsulated by this ASN.1 at the specified index.
      /// </summary>
      /// 
      /// <param name="index">the index of the ASN.1 to retrieve.</param>
      /// <returns>the ASN.1 at the specified index.</returns>
      public virtual ASN1 this[int index]
      {
         get
         {
            return (ASN1)mASN1s[index];
         }
      }
      
      /// <summary>
      /// Gets the number of ASN.1's encapsulated by this ASN.1
      /// </summary>
      public virtual int Count
      {
         get
         {
            return mASN1s.Count;
         }
      }
      
      /// <summary>
      /// Converts an Object Identifier (OID) to an ASN.1 structure.
      /// </summary>
      /// 
      /// <param name="oid">the OID to convert to an ASN.1 structure.</param>
      /// <returns>the ASN.1 structure representing the OID or null if
      /// the conversion fails.</returns>
      public static ASN1 OIDToASN1(string oid)
      {
         ASN1 asn1 = null;
         
         if(oid != null)
         {
            byte[] encoded = CryptoConfig.EncodeOID(oid);
            asn1 = new ASN1(encoded);
         }
         
         return asn1;
      }

      /// <summary>
      /// Converts an unsigned big integer in a binary format to
      /// an ASN.1 structure.
      /// </summary>
      ///
      /// <param name="bigInt">the big integer to convert.</param>
      /// <returns>the ASN.1 structure representing the big integer or
      /// null if the conversion fails.</returns>
      public static ASN1 UnsignedBigIntToASN1(byte[] bigInt)
      {
         return UnsignedBigIntToASN1(ASN1Type.Integer, bigInt);
      }
      
      /// <summary>
      /// Converts an unsigned big integer in a binary format to
      /// an ASN.1 structure.
      /// </summary>
      ///
      /// <param name="type">the ASN.1 type to use.</param>
      /// <param name="bigInt">the big integer to convert.</param>
      /// <returns>the ASN.1 structure representing the big integer or
      /// null if the conversion fails.</returns>
      public static ASN1 UnsignedBigIntToASN1(ASN1Type type, byte[] bigInt)
      {
         ASN1 asn1 = null;
         
         if(bigInt != null)
         {
            // if the first bit of the first byte (0x80 = 128) is set, then we
            // need to add a leading zero byte for sign
            if((bigInt[0] & 0x80) != 0)
            {
               byte[] bytes = new byte[bigInt.Length + 1];
               Array.Copy(bigInt, 0, bytes, 1, bigInt.Length);
               bigInt = bytes;
            }
            
            if(type == ASN1Type.OctetString)
            {
               // convert big int to octet string
               bigInt = BigIntToOctetString(bigInt);
            }
            
            asn1 = new ASN1(type, bigInt);
         }
         
         return asn1;
      }
      
      /// <summary>
      /// Converts an array of bytes to an array of bytes that represents
      /// an unsigned big int. The first byte of the passed array will be
      /// stripped off if it's most significant bit is not zero.
      /// </summary>
      public static byte[] GetBigIntValue(byte[] bytes)
      {
         byte[] rval = null;
         
         if(bytes != null)
         {
            if(bytes.Length == 0)
            {
               rval = bytes;
            }
            else
            {
               if(bytes[0] == 0)
               {
                  if(bytes.Length > 1)
                  {
                     if((bytes[1] & 0x80) != 0)
                     {
                        // the first byte was added, so leave it out
                        rval = new byte[bytes.Length - 1];
                        Array.Copy(bytes, 1, rval, 0, rval.Length);
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
      
      /// <summary>
      /// Gets the value of an octet string.
      /// </summary>
      /// 
      /// <param name="octetString">the octet string.</param>
      /// <returns>the value of the octet string.</returns>
      public static byte[] GetOctetStringValue(byte[] octetString)
      {
         // convert the octet string into an ASN.1 and get the value
         ASN1 asn1 = new ASN1(octetString);
         return asn1.Value;
      }
      
      /// <summary>
      /// Converts a big integer into an octet string.
      /// </summary>
      /// 
      /// <param name="bigint">the big integer to convert.</param>
      /// <returns>the octet string.</param>
      public static byte[] BigIntToOctetString(byte[] bigInt)
      {
         // build an ASN.1 with type integer and get the bytes
         ASN1 asn1 = new ASN1(ASN1Type.Integer, bigInt);
         return asn1.Bytes;
      }
   }
}
