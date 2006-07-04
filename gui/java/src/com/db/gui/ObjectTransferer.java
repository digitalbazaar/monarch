/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.io.IOException;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * An ObjectTransferer is an object that is used to transfer an Object. 
 * 
 * @author Dave Longley
 */
public class ObjectTransferer implements Transferable
{
   /**
    * The object to transfer.
    */
   protected Object mObject;
   
   /**
    * Creates a new ObjectTransferer that can transfer the passed object.
    *
    * @param obj the object to transfer.
    */
   public ObjectTransferer(Object obj)
   {
      // store object to transfer
      mObject = obj;
   }
   
   /**
    * Returns an array of DataFlavor objects indicating the flavors the data 
    * can be provided in. The array should be ordered according to
    * preference for providing the data (from most richly descriptive to
    * least descriptive).
    * 
    * @return an array of data flavors in which this data can be transferred.
    */
   public DataFlavor[] getTransferDataFlavors()
   {
      DataFlavor[] flavors = new DataFlavor[0];
      
      try
      {
         // add JVM local object mime type
         DataFlavor df =
            new DataFlavor(DataFlavor.javaJVMLocalObjectMimeType);
         flavors = new DataFlavor[]{df};
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), "could not create DnD data flavor!");
      }
      
      return flavors;
   }

   /**
    * Returns whether or not the specified data flavor is supported for
    * this object.
    * 
    * @param flavor the requested flavor for the data.
    * 
    * @return boolean indicating whether or not the data flavor is supported.
    */
   public boolean isDataFlavorSupported(DataFlavor flavor)
   {
      boolean rval = false;
      
      DataFlavor[] flavors = getTransferDataFlavors();
      for(int i = 0; i < flavors.length; i++) 
      {
         if(flavor.isMimeTypeEqual(flavor))
         {
            rval = true;
            break;
         }
      }
      
      return rval;
   }

   /**
    * Returns an object which represents the data to be transferred. The
    * class of the object returned is defined by the representation class
    * of the flavor.
    *
    * @param flavor the requested flavor for the data.
    *
    * @exception IOException if the data is no longer available
    *                        in the requested flavor.
    * @exception UnsupportedFlavorException if the requested data flavor is
    *                                       not supported.
    */
   public Object getTransferData(DataFlavor flavor)
      throws UnsupportedFlavorException, IOException      
   {
      Object rval = null;
      
      if(isDataFlavorSupported(flavor))
      {
         // return the object
         rval = getObject();
      }
      
      return rval;
   }
   
   /**
    * Gets the object to transfer/that was transferred.
    * 
    * @return the object to transfer/that was transferred.
    */
   public Object getObject()
   {
      return mObject;
   }
   
   /**
    * Gets the logger for this object.
    *
    * @return the logger for this object.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbgui");
   }
}
