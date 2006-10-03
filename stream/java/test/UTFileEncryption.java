/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.stream.*;
import com.db.logging.LoggerManager;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * Runs a unit test for file encryption on a transferred file.
 * 
 * FIXME: this unit test is horribly out of date.
 * 
 * @author Dave Longley
 */
public class UTFileEncryption extends Thread
{
   /**
    * The stream cryptor.
    */
   public static DESStreamCryptor smDESSC = null;
   
   /**
    * The packet size.
    */
   public static final int smPacketSize = 65536;

   /**
    * The chunk size.
    */
   public static final int smChunkSize = 65536;

   /**
    * The receive file.
    */
   public static final String smReceiveFile = "test/data/transferredfile.mp3";
   
   /**
    * The decrypted file.
    */
   public static final String smDFile = "test/data/dfile.mp3";

   /**
    * The send file.
    */
   public static final String smSendFile = "test/data/test.mp3";
   
   /**
    * The server socket.
    */
   public static ServerSocket smServerSocket;
   
   /**
    * The file offset.
    */
   public static long smOffset;
   
   /**
    * The log print stream.
    */
   public static PrintStream smLog;
   
   /**
    * The total bytes.
    */
   public static long smTotalBytes = 0;
   
   /**
    * Performs the file download.
    * 
    * @return the length of the file.
    */
   public static long downloadFile()
   {
      PrintStream log = null;
      long totalBytes = 0;

      DataInputStream dis = null;
      FileOutputStream fos = null;
      Socket clientSocket = null;

      try
      {
         // setup the log file
         log = new PrintStream(
                  new FileOutputStream("test/data/clientlog.txt"), true);

         // try to connect to the server socket to receive the file
         clientSocket = new Socket("localhost", 5148);

         // get the client's input stream
         dis = new DataInputStream(clientSocket.getInputStream());

         // get an output stream for the file (in append mode)
         fos = new FileOutputStream(smReceiveFile, true);

         // get a packet buffer for file transferring
         byte[] packet = new byte[smPacketSize];

         // for storing the number of bytes read
         int numBytes = 0;

         // keep reading from the connection until there is no more data
         while((numBytes = dis.read(packet)) != -1)
         {
            log.println("Client - bytes read: " + numBytes);

            if(numBytes > 0)
            {
               // increment total bytes read
               totalBytes += numBytes;

               log.println("Client - Writing to file...");

               // write packet to file
               fos.write(packet, 0, numBytes);
            }
         }

         log.println("Client - total bytes: " + totalBytes);

         // close file stream and connection
         fos.close();
         //bw.close();
         dis.close();
         clientSocket.close();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }

      return totalBytes;
   }

   /**
    * Requests the file download.
    */
   public static void requestDownload()
   {
      try
      {
         // open a server socket on specified port
         smServerSocket = new ServerSocket(5148);
         
         // start a download thread
         UTFileEncryption dt = new UTFileEncryption();
         dt.start();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }
   }
    
   // server side
   @Override
   public void run()
   {
      DataOutputStream dos = null;
      FileInputStream fis = null;
      Socket clientSocket = null;
      
      try
      {
         // setup the server log
         smLog = new PrintStream(
                    new FileOutputStream("test/data/serverLog.txt"), true);
         
         // set the server socket's timeout to a second
         smServerSocket.setSoTimeout(1000);
         
         // accept a client socket connection
         clientSocket = smServerSocket.accept();
         
         // set read timeout
         clientSocket.setSoTimeout(30000);
         
         // get the client's output stream to write to it
         dos = new DataOutputStream(clientSocket.getOutputStream());
         
         // get a stream to transmit the file
         fis = new FileInputStream(smSendFile);

         // get a DES stream cryptor for encrypting the file
         smDESSC = new DESStreamCryptor(smChunkSize);
         
         // get a managed input stream, reads from file stream and modifies
         // with DESStreamCryptor
         ManagedInputStream mis = new ManagedInputStream(fis,
                                                         smDESSC, smDESSC);
         
         // get a packet buffer for file transferring
         byte[] packet = new byte[smPacketSize];
         
         int numBytes = 0;
         
         // skip to the offset
         fis.skip(smOffset);
          
         // while there are still bytes available, keep reading
         while((numBytes = mis.read(packet)) != -1)
         {
            smLog.println("Server - bytes read: " + numBytes);
            
            // increment total bytes read
            smTotalBytes += numBytes;
            
            smLog.println("Server - Writing to connection...");
            
            // write to the connection
            dos.write(packet, 0, numBytes);
         }
          
         //br.close();
         fis.close();
         dos.close();
         clientSocket.close();
         smServerSocket.close();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }
      finally
      {
         smLog.println("Server - total bytes: " + smTotalBytes);
      }
   }
   
   /**
    * Runs the unit test.
    * 
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      System.out.println("\nTesting File encryption...");
      System.out.println("------------------\n");
      
      // create common logger
      LoggerManager.createLogger("dbstream");
      LoggerManager.setFile("dbstream", "dbstrean.log", false);
      
      LoggerManager.getLogger("dbstream").debug(
         "running file encryption test...");
      
      System.out.println("Removing test files if they exist.");
      try
      {
         File f = new File(smReceiveFile);
         f.delete();
         
         f = new File(smDFile);
         f.delete();
      }
      catch(Exception e)
      {
      }
      
      smOffset = 0;
      
      System.out.println("Requesting file download...");

      // request download
      requestDownload();

      // download the file
      downloadFile();
      
      System.out.println("total bytes encrypted: " +
                         smDESSC.getNumBytesEncrypted());
      
      //long fileSize = smDESSC.getNumBytesEncrypted();
      String dkey = smDESSC.getKey();
      
      smDESSC = new DESStreamCryptor(smChunkSize, dkey);
      
      // decrypt the file
      if(!smDESSC.decrypt(new File(smReceiveFile), new File(smDFile)))
      {
         System.out.println("FAILURE: File decryption failed!");
      }
      else
      {
         System.out.println("SUCCESS: File generated.");
      }
   }
}
