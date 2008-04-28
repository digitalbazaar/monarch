/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Arrays;

/**
 * A JsonWriter provides an interface for serializing objects to
 * JSON (JavaScript Object Notation) (RFC 4627).
 * 
 * A JsonWriter writes out a whole object at once and can be used again.
 * The compact setting should be used to minimize extra whitespace when not
 * needed.
 * 
 * @author David I. Lehn
 * @author Dave Longley
 */
public class JsonWriter
{
   /**
    * Compact mode to minimize whitespace.
    */
   protected boolean mCompact;
   
   /**
    * The starting indentation level. 
    */
   protected int mIndentLevel;
   
   /**
    * The number of spaces per indentation level.
    */
   protected int mIndentSpaces;
   
   /**
    * Creates a new JsonWriter.
    */
   public JsonWriter()
   {
      // Initialize to compact representation
      setCompact(true);
      setIndentation(0, 3);
   }
   
   /**
    * Writes out indentation.  None if in compact mode.
    * 
    * @param os the OutputStream to write to.
    * @param level indentation level.
    * 
    * @exception IOException thrown if an IO error occurs. 
    */
   protected void writeIndentation(OutputStream os, int level)
      throws IOException
   {
      int indent = mCompact ? 0 : (level * mIndentSpaces);
      
      // write out indentation
      if(indent > 0)
      {
         byte[] temp = new byte[indent];
         byte c = ' ';
         Arrays.fill(temp, c);
         os.write(temp, 0, indent);
      }
   }
   
   /**
    * Recursively serializes an object to JSON using the passed DynamicObject.
    * 
    * @param dyno the DynamicObject to serialize.
    * @param os the OutputStream to write the JSON to.
    * @param level current level of indentation (-1 to initialize with default).
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   protected void write(DynamicObject dyno, OutputStream os, int level)
      throws IOException
   {
      if(level < 0)
      {
         level = mIndentLevel;
      }
      
      if(dyno == null)
      {
         os.write(new String("null").getBytes("ASCII"));
      }
      else
      {
         switch(dyno.getType())
         {
            case String:
               {
                  String temp = dyno.getString();
                  int length = temp.length();
                  
                  // UTF-8 has a maximum of 7-bytes per character when
                  // encoded in json format
                  StringBuilder encoded = new StringBuilder(length * 7 + 2);
                  encoded.append('"');
                  for(int i = 0; i < length; i++)
                  {
                     char c = temp.charAt(i);
                     if((c >= 0x5d /* && c <= 0x10FFFF */) ||
                        (c >= 0x23 && c <= 0x5B) ||
                        (c == 0x21) ||
                        (c == 0x20))
                     {
                        // TODO: check this handles UTF-* properly
                        encoded.append(c);
                     }
                     else
                     {
                        encoded.append('\\');
                        switch(c)
                        {
                           case '"': /* 0x22 */
                           case '\\': /* 0x5C */
                              encoded.append(c);
                              break;
                           // '/' is in the RFC but not required to be escaped
                           //case '/': /* 0x2F */
                           //   encoded[n++] = '/';
                           //   break;
                           case '\b': /* 0x08 */
                              encoded.append('b');
                              break;
                           case '\f': /* 0x0C */
                              encoded.append('f');
                              break;
                           case '\n': /* 0x0A */
                              encoded.append('n');
                              break;
                           case '\r': /* 0x0D */
                              encoded.append('r');
                              break;
                           case '\t': /* 0x09 */
                              encoded.append('t');
                              break;
                           default:
                              encoded.append(String.format("u%04x", (int)c));
                              break;
                        }
                     }
                  }
                  
                  // end string serialization and write encoded string
                  encoded.append('"');
                  os.write(encoded.toString().getBytes("ASCII"));
               }
               break;
            case Boolean:
            case Number:
               {
                  os.write(dyno.getString().getBytes("ASCII"));
               }
               break;
            case Map:
               {
                  // start map serialization
                  if(mCompact)
                  {
                     byte b = '{';
                     os.write(b);
                  }
                  else
                  {
                     byte[] b = {'{','\n'};
                     os.write(b);
                  }
                  
                  // serialize each map member
                  byte b;
                  byte[] compactEndName = {'"',':'};
                  byte[] endName = {'"',' ',':',' '};
                  DynamicObjectIterator i = dyno.getIterator();
                  while(i.hasNext())
                  {
                     DynamicObject next = i.next();
                     
                     // serialize indentation and start serializing member name
                     writeIndentation(os, level + 1);
                     b = '"';
                     os.write(b);
                     os.write(i.getName().getBytes("UTF-8"));
                     
                     // end serializing member name, serialize member value
                     if(mCompact)
                     {
                        os.write(compactEndName);
                     }
                     else
                     {
                        os.write(endName);
                     }
                     
                     // write object
                     write(next, os, level + 1);
                     
                     // serialize delimiter if appropriate
                     if(i.hasNext())
                     {
                        b = ',';
                        os.write(b);
                     }
                     
                     // add formatting if appropriate
                     if(!mCompact)
                     {
                        b = '\n';
                        os.write(b);
                     }
                  }
                  
                  // end map serialization
                  writeIndentation(os, level);
                  b = '}';
                  os.write(b);
               }
               break;
            case Array:
               {
                  // start array serialization
                  if(mCompact)
                  {
                     byte b = '[';
                     os.write(b);
                  }
                  else
                  {
                     byte[] b = {'[','\n'};
                     os.write(b);
                  }
                  
                  // serialize each array element
                  byte b;
                  DynamicObjectIterator i = dyno.getIterator();
                  while(i.hasNext())
                  {
                     // serialize indentation and array value
                     writeIndentation(os, level + 1);
                     write(i.next(), os, level + 1);
                     
                     // serialize delimiter if appropriate
                     if(i.hasNext())
                     {
                        b = ',';
                        os.write(b);
                     }
                     
                     // add formatting if appropriate
                     if(!mCompact)
                     {
                        b = '\n';
                        os.write(b);
                     }
                  }
                  
                  // end array serialization
                  writeIndentation(os, level);
                  b = ']';
                  os.write(b);
               }
               break;
         }
      }
   }
   
   /**
    * Serializes an object to JSON using the passed DynamicObject.
    * 
    * @param dyno the DynamicObject to serialize.
    * @param os the OutputStream to write the JSON to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public void write(DynamicObject dyno, OutputStream os)
      throws IOException
   {
      BufferedOutputStream bos = new BufferedOutputStream(os);
      write(dyno, bos, mIndentLevel);
      bos.flush();
   }
   
   /**
    * Sets the starting indentation level and the number of spaces
    * per indentation level.
    * 
    * @param level the starting indentation level.
    * @param spaces the number of spaces per indentation level.
    */
   public void setIndentation(int level, int spaces)
   {
      mIndentLevel = level;
      mIndentSpaces = spaces;
   }
   
   /**
    * Sets the writer to use compact mode and not output unneeded whitespace.
    * 
    * @param compact true to minimize whitespace, false not to.
    */
   public void setCompact(boolean compact)
   {
      mCompact = compact;
   }
}
