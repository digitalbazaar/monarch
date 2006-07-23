/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater.basic;

import java.io.File;

import com.db.event.EventObject;

/**
 * This class stores information about an event that occurred during script
 * processing.
 * 
 * @author Dave Longley
 */
public class BasicUpdateScriptProcessEvent extends EventObject
{
   /**
    * Creates a new BasicUpdateScriptProcessEvent.
    * 
    * @param name the name of the event.
    * @param script the script the event is for.
    * @param command the current update script command that is being executed.
    */
   public BasicUpdateScriptProcessEvent(
      String name, BasicUpdateScript script, BasicUpdateScriptCommand command)
   {
      super(name);
      
      setUpdateScript(script);
      setUpdateScriptCommand(command);
   }
   
   /**
    * Makes a copy of this event object.
    * 
    * @return a copy of this event object.
    */
   public BasicUpdateScriptProcessEvent copy()
   {
      // copy name and command
      BasicUpdateScriptProcessEvent copy =
         new BasicUpdateScriptProcessEvent(
            getName(), getUpdateScript(), getUpdateScriptCommand());
      
      // copy data
      copy.copyDataFrom(this);
      
      return copy;
   }
   
   /**
    * Sets the BasicUpdateScript that is being executed.
    * 
    * @param script the update script that is being executed.
    */
   public void setUpdateScript(BasicUpdateScript script)
   {
      setData("script", script);
   }

   /**
    * Gets the BasicUpdateScript that is being executed.
    * 
    * @return the update script that is being executed.
    */
   public BasicUpdateScript getUpdateScript()
   {
      return (BasicUpdateScript)getData("script");
   }

   /**
    * Sets the current BasicUpdateScriptCommand that is being executed.
    * 
    * @param command the current update script command that is being executed.
    */
   public void setUpdateScriptCommand(BasicUpdateScriptCommand command)
   {
      setData("command", command);
   }

   /**
    * Gets the current BasicUpdateScriptCommand that is being executed.
    * 
    * @return the current update script command that is being executed.
    */
   public BasicUpdateScriptCommand getUpdateScriptCommand()
   {
      return (BasicUpdateScriptCommand)getData("command");
   }
   
   /**
    * Sets the command number.
    * 
    * @param number the command number. 
    */
   public void setCommandNumber(int number)
   {
      setData("command_number", number);
   }
   
   /**
    * Gets the command number.
    * 
    * @return the command number. 
    */
   public int getCommandNumber()
   {
      return getDataIntValue("command_number");
   }
   
   /**
    * Sets the download item number.
    * 
    * @param number the download item number. 
    */
   public void setDownloadItemNumber(int number)
   {
      setData("download_item_number", number);
   }
   
   /**
    * Gets the download item number.
    * 
    * @return the download item number. 
    */
   public int getDownloadItemNumber()
   {
      return getDataIntValue("download_item_number");
   }   
   
   /**
    * Sets the file that was changed.
    * 
    * @param file the file that was changed.
    */
   public void setChangedFile(File file)
   {
      setData("file", file);
   }

   /**
    * Gets the file that was changed.
    * 
    * @return the file that was changed.
    */
   public File getChangedFile()
   {
      return (File)getData("file");
   }
   
   /**
    * Sets the action involved in the change of the file.
    * 
    * @param action the action involved in the change of the file. 
    */
   public void setFileAction(String action)
   {
      setData("file_action", action);
   }
   
   /**
    * Gets the action involved in the change of the file.
    * 
    * @return the action involved in the change of the file. 
    */
   public String getFileAction()
   {
      return getDataStringValue("file_action");
   }
   
   /**
    * Sets the change in bytes downloaded for a file.
    * 
    * @param change the change in bytes downloaded for a file. 
    */
   public void setFileBytesDownloaded(long change)
   {
      setData("file_downloaded_bytes", change);
   }
   
   /**
    * Gets the change in bytes downloaded for a file.
    * 
    * @return the change in bytes downloaded for a file. 
    */
   public long getFileBytesDownloaded()
   {
      return getDataLongValue("file_downloaded_bytes");
   }   
   
   /**
    * Sets the progress involved in the change of the file in bytes.
    * 
    * @param progress the progress involved in the change of the file in bytes. 
    */
   public void setFileProgress(long progress)
   {
      setData("file_progress", progress);
   }
   
   /**
    * Gets the progress involved in the change of the file in bytes.
    * 
    * @return the progress involved in the change of the file in bytes. 
    */
   public long getFileProgress()
   {
      return getDataLongValue("file_progress");
   }
}
