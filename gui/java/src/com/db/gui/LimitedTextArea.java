/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import javax.swing.JTextArea;
import javax.swing.text.BadLocationException;
import javax.swing.text.Document;
import javax.swing.text.Element;
import javax.swing.text.ElementIterator;

/**
 * A LimitedTextArea is a text area that displays a document that is
 * wrapped by a LimitedDocument that can have a set maximum length.
 * 
 * It would have been great to just implement javax.swing.text.Document and
 * create a "LimitedDocument" class (or similar), however, JTextArea doesn't
 * work correctly with such an implementation. It must make some assumptions
 * that don't hold true for every implementation of the Document interface.
 * 
 * Simply building a wrapper class that implements javax.swing.text.Document
 * and extends a PlainDocument via aggregation (without any changes whatsoever)
 * does not produce the same results as simply using a PlainDocument that is
 * not wrapped in a JTextArea.
 * 
 * A different implementation that allows for any Documents to be used with
 * this area is preferrable in the future.
 * 
 * @author Dave Longley
 */
public class LimitedTextArea extends JTextArea
{
   /**
    * The maximum length for the text in this text area.
    */
   protected int mMaxTextLength;
   
   /**
    * Creates a new LimitedTextArea with a plain document and no set maximum
    * text length.
    */
   public LimitedTextArea()
   {
      this(-1);
   }
   
   /**
    * Creates a new LimitedTextArea with a plain document and the specified
    * maximum text length.
    * 
    * @param maxLength the maximum amount of text for this text area, a value
    *                  of -1 indicates no capacity.
    */
   public LimitedTextArea(int maxLength)
   {
      // set maximum text length
      setMaxTextLength(maxLength);
   }
   
   /**
    * Limits the length of the underlying document to the maximum text length.
    */
   protected void limitDocumentLength()
   {
      // see if there is a maximum text length
      if(getMaxTextLength() != -1)
      {
         // get the document
         Document doc = getDocument();
         
         // cap the document text if necessary
         if(doc.getLength() > getMaxTextLength())
         {
            // get the amount of text to remove
            int count = doc.getLength() - getMaxTextLength();
            
            // iterate over the elements in the document, increasing the
            // index until it reaches the count
            int index = 0;
            ElementIterator iterator = new ElementIterator(doc);
            Element element = null;
            while((element = iterator.next()) != null && index < count)
            {
               index = element.getEndOffset();
            }
            
            try
            {
               // remove the text up to the index (or the end of the document)
               doc.remove(0, Math.min(doc.getLength(), index));
            }
            catch(BadLocationException ignore)
            {
               // location is not bad
            }
         }
      }
   }
   
   /**
    * Inserts the specified text at the specified position.
    * 
    * @param str the text to insert.
    * @param pos the position at which to insert >= 0.
    * 
    * @exception IllegalArgumentException if pos is an invalid position in
    *                                     the model.
    */
   public void insert(String str, int pos)
   {
      Document doc = getDocument();
      if(doc != null)
      {
         // insert the string
         super.insert(str, pos);

         // limit the document length
         limitDocumentLength();
      }
   }

   /**
    * Appends the given text to the end of the document.     
    *
    * @param str the text to append.
    */
   public void append(String str)
   {
      Document doc = getDocument();
      if(doc != null)
      {
         // append the string
         super.append(str);
         
         // limit the document length
         limitDocumentLength();
      }
   }

   /**
    * Replaces text from the indicated start to end position with the
    * new text specified.
    *
    * @param str the text to use as the replacement.
    * @param start the start position >= 0.
    * @param end the end position >= start.
    * 
    * @exception IllegalArgumentException if part of the range is an
    *                                     invalid position in the model.
    */
   public void replaceRange(String str, int start, int end)
   {
      Document doc = getDocument();
      if(doc != null)
      {
         // insert the string
         super.replaceRange(str, start, end);

         // limit the document length
         limitDocumentLength();         
      }      
   }
   
   /**
    * Sets the maximum length of the text in this text area.
    * 
    * @param max the maximum length of the text in this text area, a value of
    *            -1 indicates no maximum.
    */
   public void setMaxTextLength(int max)
   {
      mMaxTextLength = Math.max(-1, max);
   }
   
   /**
    * Gets the maximum length of the text in this text area.
    * 
    * @return the maximum length of the text in this text area, a value of -1
    *         indicates no maximum.
    */
   public int getMaxTextLength()
   {
      return mMaxTextLength;
   }
}
