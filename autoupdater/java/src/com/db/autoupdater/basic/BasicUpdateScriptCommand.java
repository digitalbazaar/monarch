/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater.basic;

import java.io.File;
import java.net.URL;
import java.util.Iterator;
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
   protected Vector mArguments;
      
   /**
    * A string denoting the version of the software.
    */
   protected String mVersion;

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
   protected Vector mRelativePaths;
   
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
      mArguments = new Vector();
      mRelativePaths = new Vector();
   }
   
   /**
    * Resets the information for this command.
    */
   protected void reset()
   {
      mName = "";
      mArguments.clear();

      mVersion = "";
      mUrl = null;
      mMd5Digest = "";
      mSize = -1;
      mRelativePaths.clear();
      mMessage = "";
      mOnSuccessArgument = "";
   }
   
   /**
    * Parses a version command.
    * 
    * @return true if successfully parsed, false if not.
    */
   protected boolean parseVersionCommand()
   {
      boolean rval = false;
      
      try
      {
         // There should be 1 argument:
         //
         // 0. version
         mVersion = (String)mArguments.get(0);
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Version command in update script is invalid.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
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
         
         mUrl = new URL((String)mArguments.get(0));
         mSize = Long.parseLong((String)mArguments.get(1));
         mMd5Digest = (String)mArguments.get(2);
         mRelativePaths.add(new File((String)mArguments.get(3)));
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
            for(Iterator i = mArguments.iterator(); i.hasNext();)
            {
               String path = (String)i.next();
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
         // There should be 1 argument:
         //
         // 0. the relative path of the directory to create
         
         mRelativePaths.add(new File((String)mArguments.get(0)));
         rval = true;
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
         // There should be 1 argument:
         //
         // 0. the relative path of the directory to remove
         
         mRelativePaths.add(new File((String)mArguments.get(0)));
         rval = true;
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
         
         mMessage = (String)mArguments.get(0);
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
         
         mOnSuccessArgument = (String)mArguments.get(0);
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
    * FILE := version VARGS
    *         (install IARGS | delete DARGS | mkdir DARGS | rmdir DARGS |
    *          message MARGS)* on_success? OARGS
    * 
    * LINE := COMMAND
    * 
    * COMMAND := (version VARGS | message MARGS | install IARGS | delete DARGS | 
    *             mkdir DARGS | rmdir DARGS | on_success OARGS)
    * 
    * N := DECIMAL_NUMBER
    * VERSION_NUMBER := (N.N|N.N.N)
    * VARGS := VERSION_NUMBER
    * 
    * URL := FILE_OR_HTTP_URL_STRING
    * SIZE := N
    * MD5SUM := STRING
    * RELATIVE_PATH := RELATIVE_FILE_PATH
    * 
    * MARGS := STRING
    * 
    * IARGS := URL SIZE MD5SUM RELATIVE_PATH
    * 
    * DARGS := RELATIVE_PATH*
    * 
    * OARGS := (restart|shutdown|manually_download)
    * </pre>
    * 
    * VARGS are version arguments.
    * MARGS are message arguments.
    * DARGS are directory arguments.
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
      
      if(element.getName().equals("command"))
      {
         // get the name of the command
         mName = element.getAttributeValue("name");
         
         // parse the arguments for the command
         for(Iterator i = element.getChildren().iterator(); i.hasNext();)
         {
            XmlElement argumentElement = (XmlElement)i.next();
            mArguments.add(argumentElement.getValue());
         }
         
         // parse specific command
         if(getName().equals("version"))
         {
            rval = parseVersionCommand();
         }
         else if(getName().equals("install"))
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
         else if(getName().equals("message"))
         {
            rval = parseMessageCommand();
         }
         else if(getName().equals("on_success"))
         {
            rval = parseOnSuccessCommand();
         }
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
    * Gets the version number associated with the command.
    * 
    * @return a string denoting the software version.
    */
   public String getVersion()
   {
      return mVersion;
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
         rval = (File)mRelativePaths.get(0);
      }
      
      return rval;
   }
   
   /**
    * Gets the relative file paths associated with the command.
    * 
    * @return a vector of File objects that have the relative file paths
    *         associated with the command.
    */
   public Vector getRelativePaths()
   {
      return mRelativePaths;
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
