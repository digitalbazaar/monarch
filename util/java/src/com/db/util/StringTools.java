/*
 * Copyright (c) 2006-2008 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Collection;

/**
 * This class provides static methods for manipulating strings in useful ways.
 * 
 * @author Dave Longley
 */
public class StringTools
{
   /**
    * Lowercase hexadecimal characters for fast lookups.
    */
   public static final char[] HEX_CHARS =
   {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f'
   };
   
   /**
    * Splits the passed string according to the given regular expression
    * and puts the split parts into the passed collection.
    * 
    * @param str the string to split.
    * @param regex the regular expression to use.
    * @param collection the collection to put the split parts into.
    */
   public static void split(
      String str, String regex, Collection<String> collection)
   {
      String[] split = str.split(regex);
      for(int i = 0; i < split.length; i++)
      {
         collection.add(split[i]);
      }
   }
   
   /**
    * Glues the objects in the specified collection together using the
    * specified separator. Each object's string representation is obtained
    * by calling String.valueOf(object).
    * 
    * @param collection the collection to glue together.
    * @param separator the separator to use.
    * 
    * @return the string of glued objects from the collection.
    */
   public static String glue(Collection<?> collection, String separator)
   {
      StringBuffer sb = new StringBuffer();
      
      for(Object obj: collection)
      {
         if(sb.length() > 0)
         {
            sb.append(separator);
         }
         
         sb.append(String.valueOf(obj));
      }
      
      return sb.toString();
   }
   
   /**
    * Glues the objects in the specified array together using the
    * specified separator. Each object's string representation is obtained
    * by calling String.valueOf(object).
    * 
    * @param array the array to glue together.
    * @param separator the separator to use.
    * 
    * @return the string of glued objects from the collection.
    */
   public static String glue(Object[] array, String separator)
   {
      StringBuffer sb = new StringBuffer();
      
      for(int i = 0; i < array.length; i++)
      {
         if(i > 0)
         {
            sb.append(separator);
         }
         
         sb.append(String.valueOf(array[i]));
      }
      
      return sb.toString();
   }
   
   /**
    * Glues the characters in the specified array together using the
    * specified separator.
    * 
    * @param array the array to glue together.
    * @param separator the separator to use.
    * 
    * @return the string of glued objects from the collection.
    */
   public static String glue(char[] array, String separator)
   {
      StringBuffer sb = new StringBuffer();
      
      for(int i = 0; i < array.length; i++)
      {
         if(i > 0)
         {
            sb.append(separator);
         }
         
         sb.append(array[i]);
      }
      
      return sb.toString();
   }
   
   /**
    * Creates a string of repeated content where each content string is
    * separated by a separation string. An example of the use of this method
    * is for generating a string of question marks ('?') for a large
    * parameterized SQL query. 
    * 
    * @param content the content string to use.
    * @param count the number of content strings to glue together. 
    * @param separator the separator string to use.
    *  
    * @return the string of glued content.
    */
   public static String glue(String content, int count, String separator)   
   {
      // determine the string buffer size
      int size = (content.length() + (count - 1) * separator.length()) * count;
      StringBuffer sb = new StringBuffer(size);
      
      for(int i = 0; i < count; i++)
      {
         if(i > 0)
         {
            sb.append(separator);
         }
         
         sb.append(content);
      }
      
      return sb.toString();      
   }
   
   /**
    * Removes a particular string of content from a glued together string of
    * contents that are separated by some separator string. For instance,
    * this method can be used to remove the string "1234" from any of the given
    * strings:
    * 
    * STRING BEFORE REMOVE -> STRING AFTER REMOVE
    * "1234,56723,231321,31223123" -> "56723,231321,31223123"
    * "56723,1234,123213,31223123" -> "56723,123213,31223123"
    * "231312,321325,564512,1234" -> "231312,321325,564512"
    * "1234,123456,1234567" -> "123456,1234567"
    * "123,1234,123456" -> "123456,1234567"
    * "1234" -> ""
    * "1241314,412124241" -> "1241314,412124241"
    * "123,122,1234" -> "123,122"
    * 
    * i.e. String str = removeContent("123,122,1234", ",", "1234"); 
    * 
    * The string "1234" and only the string "1234" (along with any necessary
    * separators) will be removed from any of the above strings.
    * 
    * @param gluedString the glued string of contents.
    * @param separator the separator string.
    * @param remove the string to remove.
    * 
    * @return the new string minus one instance of the removed content.
    */
   public static String removeContentOnce(
      String gluedString, String separator, String remove)
   {
      StringBuffer sb = new StringBuffer(gluedString);
      
      // see if there are any separators in the glued string
      int index = sb.indexOf(separator);
      if(index != -1)
      {
         // there are separators, so first look for (remove + separator)
         // at the beginning
         index = sb.indexOf(remove + separator);
         if(index != -1 && index != 0)
         {
            index = -1;
         }

         // if not found at the beginning...
         if(index == -1)
         {
            // next look for (separator + remove + separator) in the middle
            index = sb.indexOf(separator + remove + separator);
         }
         
         // if not found in the middle...
         if(index == -1)
         {
            // next look for (separator + remove) at the end
            index = sb.indexOf(separator + remove,
               sb.length() - separator.length() - remove.length());
         }
         
         // see if an occurrence was found
         if(index != -1)
         {
            // remove the occurrence
            sb.replace(
               index, index + separator.length() + remove.length(), "");
         }
      }
      else
      {
         // see if the glued string equals the remove string
         if(gluedString.equals(remove))
         {
            // clear the string buffer
            sb.setLength(0);
         }
      }
      
      return sb.toString();
   }
   
   /**
    * Performs the same function as removeContentOnce() multiple times until
    * the remove content is no longer present in the passed glued together
    * string of contents.
    * 
    * @param gluedString the glued string of contents.
    * @param separator the separator string.
    * @param remove the string to remove.
    * 
    * @return the new string that does not contain the removed content.
    */
   public static String removeContent(
      String gluedString, String separator, String remove)
   {
      StringBuffer sb = new StringBuffer(gluedString);
      
      // see if there are any separators in the glued string
      int index = sb.indexOf(separator);
      while(index != -1)
      {
         // there are separators, so first look for (remove + separator)
         // at the beginning
         index = sb.indexOf(remove + separator);
         if(index != -1 && index != 0)
         {
            index = -1;
         }

         // if not found at the beginning...
         if(index == -1)
         {
            // next look for (separator + remove + separator) in the middle
            index = sb.indexOf(separator + remove + separator);
         }
         
         // if not found in the middle...
         if(index == -1)
         {
            // next look for (separator + remove) at the end
            index = sb.indexOf(separator + remove,
               sb.length() - separator.length() - remove.length());
         }
         
         // see if an occurrence was found
         if(index != -1)
         {
            // remove the occurrence
            sb.replace(
               index, index + separator.length() + remove.length(), "");
         }
      }
      
      // see if the glued string equals the remove string
      if(gluedString.equals(remove))
      {
         // clear the string buffer
         sb.setLength(0);
      }
      
      return sb.toString();      
   }
   
   /**
    * Converts an array of bytes into a lower-case hexadecimal string.
    * 
    * @param bytes the array of bytes to convert.
    * 
    * @return the lower-case hexadecimal string.
    */
   public static String bytesToHex(byte[] bytes)
   {
      int b;
      StringBuilder hex = new StringBuilder(bytes.length * 2);
      for(int i = 0; i < bytes.length; i++)
      {
         // hexadecimal uses 2 digits, each with 16 values (or 4 bits):
         
         // convert the top 4 bits
         b = bytes[i];
         hex.append(HEX_CHARS[(b >> 4) & 0xff]);
         
         // convert the bottom 4 bits
         hex.append(HEX_CHARS[(b & 0x0f)]);
      }
      
      return hex.toString();
   }
   
   /**
    * Converts a nibble (1 high or low hex character) to a byte.
    * 
    * @param hex the nibble (high or low).
    * 
    * @return the byte value.
    * 
    * @exception IOException thrown if the nibble was not a valid hex character.
    */
   public static int nibbleToByte(char hex) throws IOException
   {
      int rval;
      
      if(hex >= '0' && hex <= '9')
      {
         rval = (hex - '0');
      }
      else if(hex >= 'A' && hex <= 'F')
      {
         rval = hex - 'A' + 0xa;
      }
      else if(hex >= 'a' && hex <= 'f')
      {
         rval = hex - 'a' + 0xa;
      }
      else
      {
         throw new IOException(
            "Could not convert nibble to byte! Nibble was not valid hex.");
      }
      
      return rval & 0xff;
   }
   
   /**
    * Converts 2 hex nibbles (high and low bits) to a byte.
    * 
    * @param high the high bits.
    * @param low the low bits.
    * 
    * @return the byte.
    * 
    * @exception IOException thrown if the nibble was not a valid hex character.
    */
   public static byte hexToByte(char high, char low) throws IOException
   {
      int rval = nibbleToByte(low);
      int temp = nibbleToByte(high);
      rval += (temp << 4) & 0xff;
      return (byte)(rval & 0xff);
   }
   
   /**
    * Converts a hexadecimal string to an array of bytes.
    * 
    * If the hex string contains an odd number of characters, an initial 0 is
    * assumed.
    * 
    * An error can occur if the input is outside the character range
    * [0-9A-Fa-f].
    * 
    * @param hex the hexadecimal string to convert.
    * @param baos the ByteArrayOutputStream to populate.
    * 
    * @exception IOException if an IO error occurs.
    */
   public static void hexToBytes(String hex, ByteArrayOutputStream baos)
      throws IOException
   {
      int i;
      if(hex.length() % 2 == 0)
      {
         // even # of characters
         i = 0;
      }
      else
      {
         // odd # of characters, prepend initial '0' and convert first char
         i = 1;
         baos.write(hexToByte('0', hex.charAt(0)));
      }
      
      // convert the rest
      for(; i < hex.length(); i += 2)
      {
         baos.write(hexToByte(hex.charAt(i), hex.charAt(i + 1)));
      }
   }
}
