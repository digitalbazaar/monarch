/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater.basic;

import java.util.Iterator;
import java.util.Vector;

import com.db.event.EventDelegate;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.XmlElement;

/**
 * A BasicUpdateScript is one particular implementation of UpdateScript for
 * an AutoUpdater.
 * 
 * This implementation provides basic command functionality for copying/moving
 * files for a software update, etc.
 * 
 * @author Dave Longley
 */
public class BasicUpdateScript implements UpdateScript
{
   /**
    * The script as a string.
    */
   protected String mScript;
   
   /**
    * The commands that should be executed to perform the update, they are
    * stored in order of execution.
    */
   protected Vector mCommands;
   
   /**
    * The total number of items to download.
    */
   protected int mDownloadItemCount;
   
   /**
    * The total size of this update in bytes.
    */
   protected long mUpdateSize;
   
   /**
    * The processor for this script.
    */
   protected BasicUpdateScriptProcessor mProcessor;
   
   /**
    * Creates a new blank BasicUpdateScript.
    */
   public BasicUpdateScript()
   {
      this("");
   }
   
   /**
    * Creates a new BasicUpdateScript from the passed script as a String.
    * 
    * @param script the String from of the script.
    */
   public BasicUpdateScript(String script)
   {
      // store script
      mScript = script;
      
      // create the script processor
      mProcessor = new BasicUpdateScriptProcessor(this);

      // create vector for storing commands
      mCommands = new Vector();
      
      // default number of items to 0
      mDownloadItemCount = 0;
      
      // default update size to 0
      mUpdateSize = 0;
   }
   
   /**
    * Validates this script.
    * 
    * @return true if the script is valid, false if not.
    */
   public synchronized boolean validate()
   {
      boolean rval = false;
      
      // clear the commands
      mCommands.clear();
      
      // default number of items to 0
      mDownloadItemCount = 0;
      
      // default update size to 0
      mUpdateSize = 0;
      
      // parse the update script
      String script = toString();
      if(script.length() > 0)
      {
         // convert the script to an xml element and check its name and version
         XmlElement element = new XmlElement();
         if(element.convertFromXml(script) &&
            element.getName().equals("update_script") &&
            element.getAttributeValue("version").equals("1.0"))
         {
            rval = true;
            
            // parse each command
            boolean versionCommand = false;
            for(Iterator i = element.getChildren("command").iterator();
                i.hasNext() && rval;)
            {
               XmlElement commandElement = (XmlElement)i.next();

               // parse the command
               BasicUpdateScriptCommand usc = new BasicUpdateScriptCommand();
               if(usc.parseCommand(commandElement))
               {
                  mCommands.add(usc);
                  
                  if(usc.getSize() != -1)
                  {
                     // add size to total size
                     mUpdateSize += usc.getSize();
                  }
                  
                  if(usc.getName().equals("version"))
                  {
                     versionCommand = true;
                  }
                  else if(usc.getName().equals("install"))
                  {
                     // increment install item count
                     mDownloadItemCount++;
                  }
               }
               else
               {
                  rval = false;
               }
            }
            
            // ensure a version command was found
            if(!versionCommand)
            {
               rval = false;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Processes this update script. Any call to this method should cause
    * cancelled() to return false until cancel() is called.
    * 
    * @return true if the script was processed, false if it was cancelled or
    *         encountered an error.
    */
   public synchronized boolean process()
   {
      // reset the processor
      mProcessor.reset();
      
      // process the script
      return mProcessor.process();
   }
   
   /**
    * Cancels processing this update script. Any call to this method should
    * cause cancelled() to return true.
    */
   public void cancel()
   {
      // cancel processing
      mProcessor.cancelProcessing();
   }
   
   /**
    * Returns true if this script was cancelled, false if it was not. This
    * method should return false unless process() has been called followed
    * by a call to cancel(). Any subsequent call to process() should cause
    * this method to return true until cancel() is called.
    * 
    * @return true if this script has been cancelled since the last call
    *         to process().
    */
   public boolean cancelled()
   {
      // check with the processor 
      return mProcessor.processingCancelled();
   }
   
   /**
    * Reverts changes made by this script, if possible.
    * 
    * @return true if the revert was successful, false if not and the
    *         installation is now in an indetermine state.
    */
   public synchronized boolean revert()
   {
      // revert using the processor
      return mProcessor.revert();
   }
   
   /**
    * Returns true if the AutoUpdater that processed this script requires
    * a reload, false if not.
    * 
    * @return true if the AutoUpdater that processed this script requires
    *         a reload, false if not.
    */
   public boolean autoUpdaterRequiresReload()
   {
      boolean rval = false;
      
      // reload auto-updater if a shutdown or a restart is required
      if(getExitCommand() != null &&
         getExitCommand().equals("shutdown") ||
         getExitCommand().equals("restart"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the update size (in bytes).
    * 
    * @return the update size (in bytes).
    */
   public long getUpdateSize()
   {
      return mUpdateSize;
   }
   
   /**
    * Gets the number of download items.
    * 
    * @return the number of download items.
    */
   public int getDownloadItemCount()
   {
      return mDownloadItemCount;
   }
   
   /**
    * Gets the commands for this script.
    * 
    * @return the commands for this script.
    */
   public Vector getCommands()
   {
      return mCommands;
   }
   
   /**
    * Gets the command to execute when the script exits.
    * 
    * @return the command to execute when the script exits.
    */
   public String getExitCommand()
   {
      String rval = null;
      
      if(mCommands.size() > 0)
      {
         BasicUpdateScriptCommand lastCommand = 
            (BasicUpdateScriptCommand)mCommands.get(mCommands.size() - 1);
      
         rval = lastCommand.getOnSuccessArgument();
      }
      
      return rval;
   }
   
   /**
    * Gets the number of commands.
    * 
    * @return the number of commands.
    */
   public int getCommandCount()
   {
      return mCommands.size();
   }
   
   /**
    * Gets the BasicUpdateScriptProcessDelegate.
    * 
    * @return the BasicUpdateScriptProcessDelegate.
    */
   public EventDelegate getBasicUpdateScriptProcessEventDelegate()
   {
      return mProcessor.getBasicUpdateScriptProcessEventDelegate();
   }
   
   /**
    * Gets this script as a string.
    * 
    * @return this script as a string.
    */
   public String toString()
   {
      return mScript;
   }
   
   /**
    * Gets the logger for this BasicUpdateScript.
    * 
    * @return the logger for this BasicUpdateScript.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbautoupdater");
   }
}
