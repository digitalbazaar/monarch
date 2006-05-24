/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

import java.io.File;

import com.db.common.EventObject;

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
    * Sets the action involved in the change of the file.
    * 
    * @return the action involved in the change of the file. 
    */
   public String getFileAction()
   {
      return getDataStringValue("file_action");
   }
   
   /**
    * Sets the progress involved in the change of the file.
    * 
    * @param progress the progress involved in the change of the file. 
    */
   public void setFileProgress(int progress)
   {
      setData("file_progress", progress);
   }
   
   /**
    * Sets the progress involved in the change of the file.
    * 
    * @return the progress involved in the change of the file. 
    */
   public int getFileProgress()
   {
      return getDataIntValue("file_progress");
   }
}
