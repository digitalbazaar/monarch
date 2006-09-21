/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.Socket;

import com.db.logging.LoggerManager;
import com.db.logging.Logger;
import com.db.util.PortFileLock;
import com.db.util.PortFileLockConnectionHandler;
import com.db.util.SslPortFileLock;

/**
 * A unit test for the FileLock class.
 * 
 * @author Dave Longley
 */
public class UTFileLock
{
   /**
    * Runs the FileLock test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbutil", "ut-filelock.log", false);
         LoggerManager.setFileVerbosity("dbutil", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbutil", Logger.ERROR_VERBOSITY);
         
         String filename = "lockfile.lock";
         
         // get a file lock
         PortFileLock fileLock = new SslPortFileLock(filename);
         
         // set the connection handler
         TestPortFileLockConnectionHandler handler =
            new TestPortFileLockConnectionHandler(fileLock);
         fileLock.setConnectionHandler(handler);
         
         // lock the file
         if(fileLock.tryLock())
         {
            System.out.println("file lock acquired");
         }
         else
         {
            System.out.println("couldn't get lock ... piping: \"UNLOCK ME\"");
            Socket socket = fileLock.getCommSocket();
            if(socket != null)
            {
               byte[] buffer = new String("UNLOCK ME").getBytes();
               socket.getOutputStream().write(buffer);
               socket.close();
            }
            else
            {
               System.out.println("couldn't get comm socket!");
            }
         }
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A test PortFileLockConnectionHandler.
    * 
    * @author Dave Longley
    */
   public static class TestPortFileLockConnectionHandler
   implements PortFileLockConnectionHandler
   {
      /**
       * The PortFileLock this handler is for.
       */
      protected PortFileLock mLock;
      
      /**
       * Creates a new TestPortFileLockConnectionHandler.
       * 
       * @param lock the PortFileLock this handler is for.
       */
      public TestPortFileLockConnectionHandler(PortFileLock lock)
      {
         mLock = lock;
      }
      
      /**
       * Handles a connection to a PortFileLock.
       * 
       * @param socket the connected worker socket.
       */
      public void handlePortFileLockConnection(Socket socket)      
      {
         try
         {
            // read from the socket
            BufferedReader reader = new BufferedReader(
               new InputStreamReader(socket.getInputStream()));
            
            String line = reader.readLine();
            System.out.println("RECEIVED=" + line);
            
            // close connection
            socket.close();
            
            if(line != null && line.equals("UNLOCK ME"))
            {
               mLock.unlock();
               System.out.println("file lock released.");
            }
            else
            {
               System.out.println("file lock still held.");
            }
         }
         catch(Throwable t)
         {
            t.printStackTrace();
         }
      }
   }
}
