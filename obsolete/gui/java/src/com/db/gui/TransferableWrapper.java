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
 * A TransferableWrapper is wrapper for objects so they can be considered
 * Transferable and a wrapper for Transferables so that objects can be
 * retrieved from them.
 * 
 * @author Dave Longley
 */
public class TransferableWrapper implements Transferable
{
   /**
    * The object to wrap as a Transferable.
    */
   protected Object mObject;

   /**
    * Creates a new TransferableWrapper for the passed object with the
    * passed source component.
    *
    * @param obj the object to wrap as Transferable.
    */
   public TransferableWrapper(Object obj)
   {
      // store object
      mObject = obj;
   }
   
   /**
    * Creates a new TransferableWrapper for the passed Transferrable.
    *
    * @param transferable the Transferable to wrap.
    */
   public TransferableWrapper(Transferable transferable)
   {
      try
      {
         // get the object from the passed transferable
         DataFlavor df = new DataFlavor(DataFlavor.javaJVMLocalObjectMimeType);
         if(transferable.isDataFlavorSupported(df))
         {
            // store object
            mObject = transferable.getTransferData(df);
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "An exception was thrown while getting the transferable object " +
            "from the passed transferable!,exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
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
         DataFlavor df = new DataFlavor(DataFlavor.javaJVMLocalObjectMimeType);
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
      else
      {
         throw new UnsupportedFlavorException(flavor);
      }
      
      return rval;
   }
   
   /**
    * Gets the wrapped object.
    * 
    * @return the wrapped object.
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
