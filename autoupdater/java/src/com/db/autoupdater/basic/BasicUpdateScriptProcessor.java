/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater.basic;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.util.HashMap;
import java.util.Iterator;

import com.db.crypto.Cryptor;
import com.db.event.EventDelegate;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.MethodInvoker;

/**
 * A BasicUpdateScriptProcessor processes a BasicUpdateScript.
 * 
 * FIXME: A lot of the functionality in this class needs to change when
 * BasicUpdateScriptCommand is broken into individual classes that each
 * execute their own behavior. This class should be drastically simplified.
 * 
 * @author Dave Longley
 */
public class BasicUpdateScriptProcessor
{
   /**
    * The script that is to be/being processed.
    */
   protected BasicUpdateScript mScript;
   
   /**
    * Set to true to cancel processing the script, false otherwise.
    */
   protected boolean mCancelProcessing;
   
   /**
    * Stores destination files mapped to their temporary files.
    */
   protected HashMap<File, File> mDestToTemp;
   
   /**
    * A BasicUpdateScriptProcessEventDelegate for firing
    * BasicUpdateScriptProcessEvents.
    */
   protected EventDelegate mBasicUpdateScriptProcessEventDelegate;
   
   /**
    * Creates a new BasicUpdateScriptProcessor.
    * 
    * @param script the script to process.
    */
   public BasicUpdateScriptProcessor(BasicUpdateScript script)
   {
      // store the update script
      mScript = script;

      // create temp files map
      mDestToTemp = new HashMap<File, File>();
      
      // create delegate
      mBasicUpdateScriptProcessEventDelegate = new EventDelegate();
      
      // reset so the script can be processed
      reset();
   }
   
   /**
    * Fires a BasicUpdateScriptProcessEvent.
    * 
    * @param name the name of the event.
    * @param command the command currently being executed.
    * @param commandNumber the command number.
    * @param downloadItemNumber the download item number, if any.
    * @param changedFile the changed file, if any.
    * @param fileAction the action performed on the file, if any.
    * @param downloaded the change in bytes downloaded, if any.
    * @param fileProgress the progress on the file (in bytes), if any.
    */
   protected void fireBasicUpdateScriptProcessEvent(
      String name, BasicUpdateScriptCommand command, int commandNumber,
      int downloadItemNumber, File changedFile,
      String fileAction, long downloaded, long fileProgress)
   {
      // create event
      BasicUpdateScriptProcessEvent event =
         new BasicUpdateScriptProcessEvent(name, mScript, command);
      
      if(command != null)
      {
         event.setCommandNumber(commandNumber);
         event.setDownloadItemNumber(downloadItemNumber);
         event.setChangedFile(command.getRelativePath());
         event.setFileAction(fileAction);
         event.setFileBytesDownloaded(downloaded);
         event.setFileProgress(fileProgress);
      }
      
      // fire event
      getBasicUpdateScriptProcessEventDelegate().fireEvent(event);
   }
   
   /**
    * Copies one file over another.
    * 
    * @param src the source file.
    * @param dest the destination file.
    * 
    * @return true if successfully copied, false if not.
    */
   protected boolean copyFile(File src, File dest)
   {
      boolean rval = false;
      
      FileChannel in = null;
      FileChannel out = null;
      
      try
      {
         // delete the destination file
         dest.delete();
         
         // copy the file in one go
         in = new FileInputStream(src).getChannel();
         out = new FileOutputStream(dest).getChannel();
         
         if(in.transferTo(0, in.size(), out) == in.size())
         {
            rval = true;
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      finally
      {
         try
         {
            if(in != null)
            {
               in.close();
            }
            
            if(out != null)
            {
               out.close();
            }
         }
         catch(Throwable ignore)
         {
         }
      }
      
      return rval;
   }
   
   /**
    * Backs up a given file to another file in the same directory.
    * 
    * @param destFile the destination file to backup.
    * 
    * @return true if the backup was successful, false otherwise.
    */
   protected boolean backupFile(File destFile)
   {
      boolean rval = true;
      
      // only perform a backup if the destFile exists, quietly fail if it
      // doesn't
      if(destFile.exists() && destFile.isFile())
      {
         File backupFile = new File(destFile.getAbsolutePath() + ".backup");
         rval = copyFile(destFile, backupFile);
      }
      
      return rval;
   }
   
   /**
    * Deletes a directory recursively.
    * 
    * @param dir the directory to delete.
    * 
    * @exception IOException thrown if the directory's canonical path could
    *            not be determined.
    * @exception SecurityException thrown if the directory could not be
    *            deleted due to a security error.
    */
   protected void deleteDirectory(File dir)
   throws IOException
   {
      // ensure the directory is not a symbolic link before trying to
      // delete its contents
      if(dir.getAbsolutePath().equals(dir.getCanonicalPath()))
      {
         // delete the directory's contents
         File[] files = dir.listFiles();
         if(files != null)
         {
            for(File file: files)
            {
               if(file.isFile())
               {
                  // try to delete the file
                  if(!file.delete())
                  {
                     // the file could not be deleted for some reason, so
                     // try to handle it like a directory
                     deleteDirectory(file);
                  }
               }
               else
               {
                  // assume the file is a directory or symbolic link
                  deleteDirectory(file);
               }
            }
         }
      }
      
      // delete the directory, it should now be empty
      // (or it was a symbolic link)
      dir.delete();
   }

   /**
    * Performs a install command given the update script command object.
    * 
    * @param command the update script command object.
    * @param commandNumber the command number.
    * 
    * @return true if the install was successful, false otherwise.
    */
   protected boolean performInstallCommand(
      BasicUpdateScriptCommand command, int commandNumber)
   {
      boolean rval = true;
      
      // get the temp file
      File tempFile = mDestToTemp.get(command.getRelativePath());
      
      // get the full destination path
      File destFile =
         new File(getWorkingPath() + command.getRelativePath().getPath());

      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, commandNumber, 0,
         command.getRelativePath(), "backup", 0, 0);
      
      // backup file
      rval &= backupFile(destFile);
      
      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, commandNumber, 0,
         command.getRelativePath(), "backup", 0, 100);

      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, commandNumber, 0,
         command.getRelativePath(), "install", 0, 0);
      
      // copy file
      rval &= copyFile(tempFile, destFile);
      
      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command,  commandNumber, 0,
         command.getRelativePath(), "install", 0, 100);
      
      return rval;
   }
   
   /**
    * Performs the delete command given the update script command object.
    * 
    * @param command the update script command object.
    * @param path the path of the file to delete.
    * 
    * @return true if the delete was successful, false otherwise.
    */
   protected boolean performDeleteCommand(
       BasicUpdateScriptCommand command, File path)
   {
      boolean rval = false;
      
      File deleteFile = new File(getWorkingPath() + path.getPath());
      
      try
      {
         if(deleteFile.delete())
         {
            rval = true;
         }
         else
         {
            if(deleteFile.exists())
            {
               getLogger().error(getClass(), 
                  "Could not delete file! Was it a non-empty directory?: " +
                  deleteFile.getAbsolutePath());
            }
            else
            {
               // file doesn't exist already
               rval = true;
            }
         }
      }
      catch(SecurityException e)
      {
         getLogger().error(getClass(), 
            "Permission denied while attempting to delete file: "+
            deleteFile.getAbsolutePath());
         
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      
      return rval;
   }
   
   /**
    * Performs a directory creation command given the update script command
    * object.
    * 
    * @param command the update script command object.
    * @param path the path of the directory to create.
    * 
    * @return true if the delete was successful, false otherwise.
    */
   protected boolean performMkdirCommand(
      BasicUpdateScriptCommand command, File path)
   {
      boolean rval = false;
      
      File createDir = new File(getWorkingPath() + path.getPath());
      
      // attempt to create the directory
      if(!createDir.exists())
      {
         try
         {
            rval = createDir.mkdirs();
         }
         catch(SecurityException e)
         {
            getLogger().error(getClass(), 
               "Could not create directory: " + createDir.getAbsolutePath());
            
            getLogger().debug(getClass(), Logger.getStackTrace(e));
         }
      }
      else
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Performs a directory deletion command given the update script command
    * object.
    * 
    * @param command the update script command object.
    * @param path the path of the directory to remove.
    * 
    * @return true if the delete was successful, false otherwise.
    */
   protected boolean performRmdirCommand(
      BasicUpdateScriptCommand command, File path)
   {
      boolean rval = false;
      
      File deleteDir = new File(getWorkingPath() + path.getPath());
      
      if(deleteDir.isDirectory())
      {
         try
         {
            deleteDirectory(deleteDir);
            rval = true;
         }
         catch(IOException e)
         {
            getLogger().error(getClass(), 
               "IOException thrown while attempting to " +
               "delete directory: " + deleteDir.getAbsolutePath());
            getLogger().debug(getClass(), Logger.getStackTrace(e));
         }
         catch(SecurityException e)
         {
            getLogger().error(getClass(), 
               "Permission denied while attempting to " +
               "delete directory: " + deleteDir.getAbsolutePath());
            getLogger().debug(getClass(), Logger.getStackTrace(e));
         }
      }
      else if(deleteDir.isFile())
      {
         getLogger().error(getClass(),
            "Could not remove directory, it was a file: " +
            deleteDir.getAbsolutePath());
      }
      else
      {
         // may be a symbolic link, delete it
         deleteDir.delete();
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Performs the patch command given the update script command object.
    * 
    * @param command the update script command object.
    * 
    * @return true if the patch was successful, false otherwise.
    */
   protected boolean performPatchCommand(
       BasicUpdateScriptCommand command)
   {
      boolean rval = false;
      
      try
      {
         // get working directory
         File dir = new File(System.getProperty("user.dir"));
         
         // execute patch
         Process p = Runtime.getRuntime().exec(
               command.getPatchExecutable(), null, dir);
         
         // wait for patch to complete
         int exitCode = p.waitFor();
         if(exitCode == 0)
         {
            rval = true;
         }
      }
      catch(IOException e)
      {
         getLogger().error(getClass(), "Could not execute patch!,e= " + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      catch(InterruptedException e)
      {
         getLogger().error(getClass(), "Interrupted during patch execution!");
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      
      return rval;
   }   
   
   /**
    * Downloads a file to a temporary file name. The filename will be
    * stored in the temp map in case it is valid and is to be used. If
    * the temp file is invalid, it will be deleted.
    * 
    * @param command the current command.
    * @param commandNumber the command number.
    * @param downloadItemNumber the download item number.
    * 
    * @return true if the downloaded file is valid, false if not.
    */
   protected boolean downloadFile(
      BasicUpdateScriptCommand command, int commandNumber,
      int downloadItemNumber)
   {
      boolean rval = false;
      File temp = null;
      
      try
      {
         File destination = command.getRelativePath();
         String md5 = command.getMd5Digest();
         
         // see if the destination can be written to and not cancelling
         if((!destination.exists() || destination.canWrite()) &&
            !mCancelProcessing)
         {
            // get a temp file for saving to
            temp = File.createTempFile("update", ".tmp");
            temp.deleteOnExit();
            
            try
            {
               // create file downloader to download file
               FileDownloader downloader = new FileDownloader();
               
               // background execute file downloader
               MethodInvoker mi = new MethodInvoker(
                  downloader, "downloadFile", command.getUrl(), temp);
               mi.backgroundExecute();
               
               long byteChange = 0;
               long totalBytes = 0;
               long interval = 1;
               while(!downloader.isDownloadingComplete() &&
                     !mi.isInterrupted() && !mCancelProcessing)
               {
                  byteChange = downloader.getBytesDownloaded() - totalBytes;
                  totalBytes = totalBytes + byteChange;
                  
                  // fire file changed event
                  fireBasicUpdateScriptProcessEvent(
                     "fileChanged", command, commandNumber,
                     downloadItemNumber, command.getRelativePath(),
                     "download", byteChange, totalBytes);

                  // join the downloader thread
                  mi.join(interval);
               }
               
               if(mCancelProcessing)
               {
                  mi.interrupt();
                  mi.join();
               }
               else
               {
                  // send final file changed message
                  byteChange = downloader.getBytesDownloaded() - totalBytes;
                  totalBytes = totalBytes + byteChange;
                  
                  // fire file changed event
                  fireBasicUpdateScriptProcessEvent(
                     "fileChanged", command,  commandNumber,
                     downloadItemNumber, command.getRelativePath(),
                     "download", byteChange, totalBytes);
               
                  // check the MD5 sum of the file to see if it matches
                  String tempMD5 = Cryptor.getMD5ChecksumString(temp);
                  if(tempMD5.equals(md5))
                  {
                     // file MD5 matches, if file is valid add to map
                     mDestToTemp.put(destination, temp);
                        
                     // fire event
                     fireBasicUpdateScriptProcessEvent(
                        "fileDownloaded", command,  commandNumber,
                        downloadItemNumber, command.getRelativePath(),
                        "completed", 0, 100);
                        
                     rval = true;
                  }
                  else
                  {
                     // fire event
                     fireBasicUpdateScriptProcessEvent(
                        "fileChanged", command, commandNumber,
                        downloadItemNumber, command.getRelativePath(),
                        "failed", 0, 100);
                  }
               }
            }
            catch(Throwable t)
            {
               getLogger().debug(getClass(), Logger.getStackTrace(t));
               
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "fileChanged", command, commandNumber,
                  downloadItemNumber, command.getRelativePath(),
                  "failed", 0, 100);
            }
         }
         else if (!mCancelProcessing)
         {
            getLogger().error(getClass(),
               "Could not get write access for file!: " +
               destination.getAbsolutePath());
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      // remove any invalid temp file created
      if(!rval && temp != null)
      {
         temp.delete();
      }
      
      return rval;
   }
   
   /**
    * Gets the working path, ending with the file separator appropriate for
    * the current system.
    * 
    * @return the working path, ending with the file separator appropriate for
    *         the current system.
    */
   protected String getWorkingPath()
   {
      File userDir = new File(System.getProperty("user.dir"));
      String workingPath = userDir.getAbsolutePath();
      if(!workingPath.endsWith(File.separator))
      {
         workingPath += File.separator;
      }
      
      return workingPath;
   }
   
   /**
    * Resets this processor so it can process the script again.
    */
   public synchronized void reset()
   {
      // clear temp files map
      mDestToTemp.clear();
      
      // reset cancel
      mCancelProcessing = false;
   }
   
   /**
    * Processes the BasicUpdateScript.
    * 
    * @return true if the script was successfully processed, false if not.
    */
   public synchronized boolean process()
   {
      boolean rval = false;
      
      // clear temporary files
      mDestToTemp.clear();
      
      // perform the download for every file that is needed for installation
      boolean noError = true;
      int commandNumber = 1;
      int downloadItemNumber = 1;
      for(Iterator<BasicUpdateScriptCommand> i =
          mScript.getCommands().iterator();
          i.hasNext() && noError && !mCancelProcessing;)
      {
         BasicUpdateScriptCommand command = i.next();
         if(command.getName().equals("install"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "downloadingFile", command, commandNumber, downloadItemNumber, 
               null, null, 0, 0);
            
            noError &= downloadFile(command, commandNumber, downloadItemNumber);
            downloadItemNumber++;
         }
         
         commandNumber++;
      }
      
      // execute each command of the script in order
      commandNumber = 1;
      for(Iterator<BasicUpdateScriptCommand> i =
          mScript.getCommands().iterator();
          i.hasNext() && noError && !mCancelProcessing;)
      {
         BasicUpdateScriptCommand command = i.next();
         
         if(command.getName().equals("install"))
         {
            noError &= performInstallCommand(command, commandNumber);
         }
         else if(command.getName().equals("delete"))
         {
            // perform a delete for each relative path
            for(File path: command.getRelativePaths())
            {
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "fileChanged", command, commandNumber, 0,
                  path, "delete", 0, 0);

               // perform delete
               noError &= performDeleteCommand(command, path);
            
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "fileChanged", command, commandNumber, 0,
                  path, "delete", 0, 100);
            }
         }
         else if(command.getName().equals("mkdir"))
         {
            // perform a mkdir for each relative path
            for(File path: command.getRelativePaths())
            {
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "directoryChanged", command, commandNumber, 0,
                  command.getRelativePath(), "create", 0, 0);

               // perform make directory
               noError &= performMkdirCommand(command, path);
               
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "directoryChanged", command, commandNumber, 0,
                  command.getRelativePath(), "create", 0, 100);
            }
         }
         else if(command.getName().equals("rmdir"))
         {
            // perform a rmdir for each relative path
            for(File path: command.getRelativePaths())
            {
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "directoryChanged", command, commandNumber, 0,
                  command.getRelativePath(), "delete", 0, 0);

               // perform remove directory
               noError &= performRmdirCommand(command, path);
               
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "directoryChanged", command, commandNumber, 0,
                  command.getRelativePath(), "delete", 0, 100);
            }
         }
         else if(command.getName().equals("patch"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "executingPatch", command, commandNumber, 0,
               null, null, 0, 0);
            
            // execute the patch
            noError &= performPatchCommand(command);
            
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "patchExecuted", command, commandNumber, 0,
               null, null, 0, 100);
         }
         
         // fire event
         fireBasicUpdateScriptProcessEvent(
            "commandProcessed", command, commandNumber, 0, null, null, 0, 0);

         // increment command number
         commandNumber++;
      }
      
      if(noError && !mCancelProcessing)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Reverts changes made during script processing, if possible.
    * 
    * @return true if the revert was successful, false if not and the
    *         installation is now in an indetermine state.
    */
   public synchronized boolean revert()
   {
      boolean rval = false;
      
      // move the old files back, installation failed
      boolean isValid = true;
      for(File relativeDestPath: mDestToTemp.keySet())
      {
         // get the destination file
         File dest = new File(getWorkingPath() + relativeDestPath.getPath());
         
         // see if a backup file exists
         File backupFile = new File(dest.getAbsolutePath() + ".backup"); 
         if(backupFile.exists())
         {
            // delete destination file
            dest.delete();
            
            // rename backup file to destination file
            isValid &= backupFile.renameTo(dest);
         }
      }
      
      rval = isValid;
      
      return rval;
   }
   
   /**
    * Cleans up an installation after the script has been processed.
    * 
    * @return true if cleaned up, false if not.
    */
   public synchronized boolean cleanup()
   {
      boolean rval = false;
      
      // delete all the old temp files
      boolean isValid = true;
      for(File temp: mDestToTemp.values())
      {
         rval &= temp.delete();
      }

      mDestToTemp.clear();
      
      return (rval == isValid);
   }   
   
   /**
    * Cancels processing the update script.
    */
   public void cancelProcessing()
   {
      mCancelProcessing = true;
   }
   
   /**
    * Returns true if script processing was cancelled, false if it was not.
    * 
    * @return true if script processing was cancelled, false if not.
    */
   public boolean processingCancelled()
   {
      return mCancelProcessing;
   }
   
   /**
    * Gets the BasicUpdateScriptProcessDelegate.
    * 
    * @return the BasicUpdateScriptProcessDelegate.
    */
   public EventDelegate getBasicUpdateScriptProcessEventDelegate()
   {
      return mBasicUpdateScriptProcessEventDelegate;
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
