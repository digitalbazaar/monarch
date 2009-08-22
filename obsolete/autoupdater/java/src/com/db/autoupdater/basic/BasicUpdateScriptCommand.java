/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater.basic;

import java.io.File;
import java.net.URL;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.XmlElement;

/**
 * A basic UpdateScriptCommand is an atomic BasicUpdateScript command.
 * 
 * This class can be used to parse and verify that a particular command
 * in a BasicUpdateScript is valid.
 * 
 * FIXME: This class needs to be broken up into an abstract base class
 * and individual derived classes that parse out their specific commands
 * and execute the appropriate behavior. Work on this has begun and has
 * been backed up. The work stopped because there are more pressing issues.
 * 
 * @author Manu Sporny
 * @author Dave Longley
 */
public class BasicUpdateScriptCommand
{
   /**
    * The name of the command.
    */
   protected String mName;
   
   /**
    * The arguments for the command.
    */
   protected List<String> mArguments;
      
   /**
    * The URL associated with the command.
    */
   protected URL mUrl;
      
   /**
    * The MD5 digest associated with the command.
    */
   protected String mMd5Digest;
   
   /**
    * The size associated with the command.
    */
   protected long mSize;
   
   /**
    * The relative file paths associated with the command.
    */
   protected List<File> mRelativePaths;
   
   /**
    * The executable, and its arguments, for a patch.
    */
   protected String[] mPatchExecutable;
   
   /**
    * A message to display.
    */
   protected String mMessage;
   
   /**
    * The on_success argument if it exists.
    */
   protected String mOnSuccessArgument;

   /**
    * Creates a BasicUpdateScriptCommand.
    */
   public BasicUpdateScriptCommand()
   {
      // create arguments and relative paths vectors
      mArguments = new Vector<String>();
      mRelativePaths = new Vector<File>();
   }
   
   /**
    * Resets the information for this command.
    */
   protected void reset()
   {
      mName = "";
      mArguments.clear();

      mUrl = null;
      mMd5Digest = "";
      mSize = -1;
      mRelativePaths.clear();
      mPatchExecutable = null;
      mMessage = "";
      mOnSuccessArgument = "";
   }
   
   /**
    * Parses an install command.
    * 
    * @return true if successfully parsed, false if not.
    */
   protected boolean parseInstallCommand()
   {
      boolean rval = false;
      
      try
      {
         // There should be 4 arguments:
         //
         // 0. the URL for the file to download
         // 1. the size of the file in bytes
         // 2. the MD5 digest for the file
         // 3. the final destination (relative path) for the file
         
         mUrl = new URL(mArguments.get(0));
         mSize = Long.parseLong(mArguments.get(1));
         mMd5Digest = mArguments.get(2);
         mRelativePaths.add(new File(mArguments.get(3)));
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Install command in update script is invalid.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Parses a delete command.
    * 
    * @return true if successfully parsed, false if not.
    */
   protected boolean parseDeleteCommand()
   {
      boolean rval = false;
      
      try
      {
         // There should be 1-n arguments:
         //
         // 0-n. the relative path of the file to delete
         
         if(mArguments.size() > 0)
         {
            for(String path: mArguments)
            {
               mRelativePaths.add(new File(path));
            }
            
            rval = true;
         }
         else
         {
            getLogger().error(getClass(),
               "Delete command in update script is invalid.");
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Delete command in update script is invalid.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Parses a mkdir command.
    * 
    * @return true if successfully parsed, false if not.
    */
   protected boolean parseMakeDirectoryCommand()
   {
      boolean rval = false;
      
      try
      {
         // There should be 1-n arguments:
         //
         // 0-n. the relative path of the directory to create
         
         if(mArguments.size() > 0)
         {
            for(String path: mArguments)
            {
               mRelativePaths.add(new File(path));
            }
            
            rval = true;
         }
         else
         {
            getLogger().error(getClass(), 
               "Make directory command in update script is invalid.");
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Make directory command in update script is invalid.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Parses a rmdir command.
    * 
    * @return true if successfully parsed, false if not.
    */
   protected boolean parseRemoveDirectoryCommand()
   {
      boolean rval = false;
      
      try
      {
         // There should be 1-n arguments:
         //
         // 0-n. the relative path of the directory to create
         
         if(mArguments.size() > 0)
         {
            for(String path: mArguments)
            {
               mRelativePaths.add(new File(path));
            }
            
            rval = true;
         }
         else
         {
            getLogger().error(getClass(), 
               "Remove directory command in update script is invalid.");
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Remove directory command in update script is invalid.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Parses a patch command.
    * 
    * @return true if successfully parsed, false if not.
    */
   protected boolean parsePatchCommand()
   {
      boolean rval = false;
      
      try
      {
         // There should be 1-n arguments:
         //
         // 1. the patch executable command to execute
         // n. the arguments for the executable
         
         if(mArguments.size() > 0)
         {
            mPatchExecutable = new String[mArguments.size()];
            mArguments.toArray(mPatchExecutable);
            rval = true;
         }
         else
         {
            getLogger().error(getClass(),
               "Patch command must have at least 1 argument!");
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Patch command in update script is invalid.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Parses a message command.
    * 
    * @return true if successfully parsed, false if not.
    */
   protected boolean parseMessageCommand()
   {
      boolean rval = false;
      
      try
      {
         // There should be 1 argument:
         //
         // 0. the message to display
         
         mMessage = mArguments.get(0);
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Message command in update script is invalid.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Parses a on_success command.
    * 
    * @return true if successfully parsed, false if not.
    */
   protected boolean parseOnSuccessCommand()
   {
      boolean rval = false;
      
      try
      {
         // There should be 1 argument:
         //
         // 0. restart|shutdown|manually_download
         
         mOnSuccessArgument = mArguments.get(0);
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "On success command in update script is invalid.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Parses a command from an XmlElement.
    * 
    * The BNF for the possible commands is as follows:
    * 
    * <pre>
    * FILE := (install IARGS | delete DARGS | mkdir DARGS | rmdir DARGS |
    *          patch PARGS | message MARGS)* on_success? OARGS
    * 
    * LINE := COMMAND
    * 
    * COMMAND := (install IARGS | delete DARGS | 
    *             mkdir DARGS | rmdir DARGS | patch PARGS |
    *             message MARGS | on_success OARGS)
    * 
    * URL := FILE_OR_HTTP_URL_STRING
    * SIZE := LONG
    * MD5SUM := STRING
    * RELATIVE_PATH := RELATIVE_FILE_PATH
    * 
    * MARGS := STRING
    * 
    * IARGS := URL SIZE MD5SUM RELATIVE_PATH
    * 
    * DARGS := RELATIVE_PATH*
    * 
    * PARGS := STRING+
    * 
    * OARGS := (restart|shutdown)
    * </pre>
    * 
    * MARGS are message arguments.
    * DARGS are directory arguments.
    * PARGS are patch arguments.
    * OARGS are on success arguments.
    * 
    * @param element the xml element to parse the command from.
    * 
    * @return true if the command was parsed successfully, false if not.
    */
   public boolean parseCommand(XmlElement element)
   {
      boolean rval = false;
      
      // reset command information
      reset();
      
      try
      {
         if(element.getName().equals("command"))
         {
            // get the name of the command
            mName = element.getAttributeValue("name");
            
            // parse the arguments for the command, put them in a temporary map
            HashMap<Integer, String> map = new HashMap<Integer, String>();
            boolean argumentsValid = true;
            for(Iterator<XmlElement> i = element.getChildren().iterator();
                i.hasNext() && argumentsValid;)
            {
               XmlElement argumentElement = i.next();
               
               // get the order attribute
               int order = argumentElement.getAttributeIntValue("order");
               
               if(!map.containsKey(order))
               {
                  map.put(order, argumentElement.getValue());
               }
               else
               {
                  // duplicate argument -- arguments are invalid
                  argumentsValid = false;
               }
            }
            
            // proceed if the arguments are valid
            if(argumentsValid)
            {
               for(int i = 0; i < map.size(); i++)
               {
                  // add the arguments in order
                  String argument = map.get(i);
                  mArguments.add(argument);
               }
               
               // parse specific command
               if(getName().equals("install"))
               {
                  rval = parseInstallCommand();
               }
               else if(getName().equals("delete"))
               {
                  rval = parseDeleteCommand();
               }
               else if(getName().equals("mkdir"))
               {
                  rval = parseMakeDirectoryCommand();
               }
               else if(getName().equals("rmdir"))
               {
                  rval = parseRemoveDirectoryCommand();
               }
               else if(getName().equals("patch"))
               {
                  rval = parsePatchCommand();
               }
               else if(getName().equals("message"))
               {
                  rval = parseMessageCommand();
               }
               else if(getName().equals("on_success"))
               {
                  rval = parseOnSuccessCommand();
               }
               else
               {
                  getLogger().error(getClass(),
                     "Unknown command: " + getName());
               }
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Exception thrown while parsing a command in update script!" +
            ",exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Gets the name of the command.
    * 
    * @return the name of the command.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the URL associated with the command.
    * 
    * @return the URL associated with the command, null if the command does
    *         not exist.
    */
   public URL getUrl()
   {
      return mUrl;
   }
   
   /**
    * Gets the MD5 digest associated with the command.
    * 
    * @return the MD5 digest associated with the command if it exists, a
    *         blank string is returned if no MD5 digest is associated with it.
    */
   public String getMd5Digest()
   {
      return mMd5Digest;
   }
   
   /**
    * Gets the size associated with the command.
    * 
    * @return the size of the file associated with the command, -1 if 
    *         there is no size associated with the command.
    */
   public long getSize()
   {
      return mSize;
   }
   
   /**
    * Gets the first relative path associated with the command.
    * 
    * @return the first relative path associated with the command.
    */
   public File getRelativePath()
   {
      File rval = null;
      
      if(mRelativePaths.size() > 0)
      {
         rval = mRelativePaths.get(0);
      }
      
      return rval;
   }
   
   /**
    * Gets the relative file paths associated with the command.
    * 
    * @return a list of File objects that have the relative file paths
    *         associated with the command.
    */
   public List<File> getRelativePaths()
   {
      return mRelativePaths;
   }
   
   /**
    * Gets the patch executable.
    *
    * @return the patch executable.
    */
   public String[] getPatchExecutable()
   {
      return mPatchExecutable;
   }
   
   /**
    * Gets a message to display.
    *
    * @return a message to display.
    */
   public String getMessage()
   {
      return mMessage;
   }
   
   /**
    * Gets the on success argument if it exists.
    * 
    * @return the on success argument if it exists.
    */
   public String getOnSuccessArgument()
   {
      return mOnSuccessArgument;
   }
   
   /**
    * Gets the logger for this BasicUpdateScriptCommand.
    * 
    * @return the logger for this BasicUpdateScriptCommand.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbautoupdater");
   }
}
