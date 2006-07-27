/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PushbackInputStream;
import java.net.Socket;
import java.net.SocketAddress;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class represents a generic web connection to a web server.
 * 
 * @author Dave Longley
 */
public class GenericWebConnection implements WebConnection
{
   /**
    * The worker socket for this web connection.
    */
   protected Socket mWorkerSocket;
   
   /**
    * The remote IP address for this web connection.
    */
   protected String mRemoteIP;
   
   /**
    * The remote port for this web connection.
    */
   protected int mRemotePort;
   
   /**
    * Whether or not this web connection has been marked as secure.
    */
   protected boolean mSecure;
   
   /**
    * The stream used to read from the web connection.
    */
   protected PushbackInputStream mReadStream;
   
   /**
    * The stream used to write to the web connection.
    */
   protected OutputStream mWriteStream;
   
   /**
    * The number of bytes read so far.
    */
   protected long mBytesRead;
   
   /**
    * The number of bytes written so far.
    */
   protected long mBytesWritten;
   
   /**
    * The read bandwidth throttler.
    */
   protected BandwidthThrottler mReadBandwidthThrottler;
   
   /**
    * The write bandwidth throttler.
    */
   protected BandwidthThrottler mWriteBandwidthThrottler;
   
   /**
    * Creates a new generic web connection.
    * 
    * @param workerSocket the worker socket for this web connection.
    */
   public GenericWebConnection(Socket workerSocket)
   {
      // store worker socket
      mWorkerSocket = workerSocket;
      
      // set IP and port
      setRemoteIP(getRemoteIP(workerSocket));
      setRemotePort(getRemotePort(workerSocket));
      
      // not secure by default
      mSecure = false;

      // no bytes read or written yet
      mBytesRead = 0;
      mBytesWritten = 0;
      
      // create the read and write throttlers
      mReadBandwidthThrottler = new BandwidthThrottler(0);
      mWriteBandwidthThrottler = new BandwidthThrottler(0);
   }
   
   /**
    * Ensures the web connection gets disconnected.
    */
   protected void finalize()
   {
      disconnect();
   }

   /**
    * Reads data from this web connection. The actual number of bytes
    * read may be less than the amount asked for.
    * 
    * @param buffer the buffer to read data into from this connection.
    * @param offset the offset to start writing the data into the buffer.
    * @param length the amount of data to read.
    * 
    * @return the actual number of bytes read or -1 if there is no
    *         more data to read.
    *         
    * @throws IOException
    */
   public int read(byte[] buffer, int offset, int length)
   throws IOException, InterruptedException
   {
      int numBytes = -1;
      
      if(Thread.currentThread().isInterrupted())
      {
         throw new InterruptedException("WebConnection read interrupted!");
      }
      
      // throttle the read
      length = getReadBandwidthThrottler().requestBytes(length);

      // do the read
      numBytes = getReadStream().read(buffer, offset, length);
      
      // increment bytes read
      if(numBytes != -1)
      {
         mBytesRead += numBytes;
      }
      
      return numBytes;
   }
   
   /**
    * Reads data from this web connection. This method will block until
    * there is no more data to read (end of stream) or until the amount
    * of data asked for has been read.
    * 
    * @param buffer the buffer to read data into from this connection.
    * @param offset the offset to start writing the data into the buffer.
    * @param length the amount of data to read.
    * 
    * @return the actual number of bytes read or -1 if the end of the
    *         stream has been reached.
    *         
    * @throws IOException, InterruptedException
    */
   public int blockedRead(byte[] buffer, int offset, int length)
   throws IOException, InterruptedException
   {
      int read = 0;
      
      // read until there is no more data to read or until all data requested
      // has been read
      int numBytes = 0;
      while(length > 0 && numBytes != -1)
      {
         if(Thread.currentThread().isInterrupted())
         {
            throw new InterruptedException(
               "WebConnection blockedRead interrupted!");
         }
         
         // read into the buffer from the offset plus the amount of
         // data read so far
         numBytes = read(buffer, offset + read, length);
         
         if(numBytes != -1)
         {
            // increment the amount of data read so far and decrement
            // the remaining amount of data to be read
            read += numBytes;
            length -= numBytes;
         }
         else if(read == 0)
         {
            // end of stream reached
            read = -1;
         }
      }
      
      return read;
   }
   
   /**
    * Unreads data back onto the web connection. This method is useful
    * if a little bit of data must be read from the web connection to
    * determine how to service the connection -- but that data must then
    * be re-read later. The data can only be re-read using this 
    * web connection object, it is not sent back over the physical
    * web connection.
    * 
    * @param buffer the buffer to write data from to this connection.
    * @param offset the offset to start writing the data.
    * @param length the amount of data to write.
    * 
    * @throws IOException
    * @throws InterruptedException
    */
   public void unread(byte[] buffer, int offset, int length)
   throws IOException, InterruptedException
   {
      if(Thread.currentThread().isInterrupted())
      {
         throw new InterruptedException("WebConnection unread interrupted!");
      }
      
      if(getReadStream() instanceof PushbackInputStream)
      {
         ((PushbackInputStream)getReadStream()).unread(buffer, offset, length);
      }
      else
      {
         // unread with read stream
         mReadStream.unread(buffer, offset, length);
      }
      
      // decrement bytes read
      mBytesRead -= length;
   }
   
   /**
    * Reads a single line from this web connection that terminates in a
    * end of line character ('\n'), a carriage return ('\r'), or both an
    * end of line character and a carriage return ("\r\n" -- CRLF). This method
    * will block until there is no more data to read or until it reads a line.
    * 
    * @return the read line or null if the end of the stream was reached.
    * 
    * @throws IOException
    * @throws InterruptedException
    */
   public String readLine() throws IOException, InterruptedException
   {
      String line = "";

      // read one character at a time
      int numBytes = -1;
      byte[] buffer = new byte[1];
      char c = ' ';
      while(c != '\n' && (numBytes = blockedRead(buffer, 0, 1)) != -1)
      {
         c = (char)buffer[0];
         
         // see if the character is a carriage return
         if(c == '\r')
         {
            // carriage return same as end of line
            c = '\n';
            
            // try to read the next character because we must
            // determine if this is a CRLF ("\r\n") special case
            numBytes = blockedRead(buffer, 0, 1);
            if(numBytes != -1)
            {
               if((char)buffer[0] != '\n')
               {
                  // unread the byte since it is not an end of line
                  unread(buffer, 0, 1);
               }
            }
         }
         else
         {
            // append the character
            line += c;
         }
      }
      
      // signals end of stream
      if(numBytes == -1 && line.equals(""))
      {
         line = null;
      }
      
      return line;
   }
   
   /**
    * Gets the number of bytes read so far.
    * 
    * @return the number of bytes read so far.
    */
   public long getBytesRead()
   {
      return mBytesRead;
   }
   
   /**
    * Writes data to this web connection.
    * 
    * @param buffer the buffer to write data from to this connection.
    * @param offset the offset in the buffer to start writing data from.
    * @param length the amount of data to write.
    * 
    * @throws IOException
    * @throws InterruptedException
    */
   public void write(byte[] buffer, int offset, int length)
   throws IOException, InterruptedException
   {
      while(length > 0)
      {
         if(Thread.currentThread().isInterrupted())
         {
            throw new InterruptedException("WebConnection write interrupted!");
         }

         // throttle the write
         int numBytes = getWriteBandwidthThrottler().requestBytes(length);

         // do the write
         getWriteStream().write(buffer, offset, numBytes);
         
         // increment offset and decrement length
         offset += numBytes;
         length -= numBytes;
         
         // increment bytes written
         mBytesWritten += numBytes;
      }
         
      // always flush ;)
      getWriteStream().flush();
   }
   
   /**
    * Gets the number of bytes written so far.
    * 
    * @return the number of bytes written so far.
    */
   public long getBytesWritten()   
   {
      return mBytesWritten;
   }
   
   /**
    * Sets the underlying web connection's BandwidthThrottler for reading.
    * 
    * @param throttler the BandwidthThrottler to use when reading.
    */
   public void setReadBandwidthThrottler(BandwidthThrottler throttler)
   {
      mReadBandwidthThrottler = throttler;
   }

   /**
    * Gets the underlying web connection's BandwidthThrottler for reading.
    * 
    * @return the underlying web connection's read BandwidthThrottler.
    */
   public BandwidthThrottler getReadBandwidthThrottler()
   {
      return mReadBandwidthThrottler;
   }
   
   /**
    * Sets the underlying web connection's BandwidthThrottler for writing.
    * 
    * @param throttler the BandwidthThrottler to use when writing.
    */
   public void setWriteBandwidthThrottler(BandwidthThrottler throttler)
   {
      mWriteBandwidthThrottler = throttler;
   }

   /**
    * Gets the underlying web connection's BandwidthThrottler for writing.
    * 
    * @return the underlying web connection's write BandwidthThrottler.
    */
   public BandwidthThrottler getWriteBandwidthThrottler()   
   {
      return mWriteBandwidthThrottler;
   }
   
   /**
    * Returns true if this web connection is connected, false if it is not.
    * 
    * @return true if this web connection is connected, false if not.
    */
   public boolean isConnected()
   {
      // default to false
      boolean rval = false;
      
      if(getWorkerSocket() != null)
      {
         if(!getWorkerSocket().isClosed())
         {
            rval = getWorkerSocket().isConnected();
         }
      }
      
      return rval;
   }
   
   /**
    * Returns true if this web connection is closed, false if it is not.
    * 
    * @return true if this web connection is closed, false if not.
    */
   public boolean isClosed()
   {
      // default to true
      boolean rval = true;
      
      if(getWorkerSocket() != null)
      {
         rval = getWorkerSocket().isClosed();
      }
      
      return rval;
   }
   
   /**
    * Disconnects this web connection.
    */
   public synchronized void disconnect()
   {
      try
      {
         if(getWorkerSocket() != null)
         {
            if(!getWorkerSocket().isClosed())
            {
               getWorkerSocket().close();
               getLogger().debug(getClass(),
                  "web connection disconnected,ip=" + getRemoteIP());
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
   }
   
   /**
    * Gets the host for this web connection.
    * 
    * @return the host for this web connection.
    */
   public String getHost()
   {
      return getWorkerSocket().getRemoteSocketAddress().toString();
   }
   
   /**
    * Gets the input stream used to read and unread from the web connection.
    * 
    * @return the input stream used to read and unread from the web connection.
    * 
    * @throws IOException
    */
   public InputStream getReadStream()
   throws IOException
   {
      if(mReadStream == null)
      {
         // create a new push back input stream
         mReadStream =
            new PushbackInputStream(getWorkerSocket().getInputStream(), 2048);
      }
      
      return mReadStream;
   }
   
   /**
    * Gets the output stream used to write to the web connection.
    * 
    * @return the output stream used to write to the web connection.
    * 
    * @throws IOException
    */
   public OutputStream getWriteStream()
   throws IOException
   {
      if(mWriteStream == null)
      {
         // get the socket's output stream
         mWriteStream = getWorkerSocket().getOutputStream();
      }
      
      return mWriteStream;
   }
   
   /**
    * Gets the worker socket for this web connection.
    * 
    * @return the worker socket for this web connection.
    */
   public Socket getWorkerSocket()
   {
      return mWorkerSocket;
   }
   
   /**
    * Sets the read timeout for this web connection. This is the amount of
    * time that must pass while doing a blocking read before timing out. 
    * 
    * @param timeout the read timeout in milliseconds (0 for no timeout).
    */
   public void setReadTimeout(int timeout)
   {
      try
      {
         if(!isClosed())
         {
            getWorkerSocket().setSoTimeout(timeout);
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
   }
   
   /**
    * Gets the local IP address for this web connection.
    * 
    * @return the local IP address for this web connection.
    */
   public String getLocalIP()
   {
      return getLocalIP(getWorkerSocket());
   }

   /**
    * Sets the remote IP address for this web connection.
    * 
    * @param ip the remote IP address for this web connection.
    */
   public void setRemoteIP(String ip)
   {
      mRemoteIP = ip;
   }
   
   /**
    * Gets the remote IP address for this web connection.
    * 
    * @return the remote IP address for this web connection.
    */
   public String getRemoteIP()
   {
      return mRemoteIP;
   }
   
   /**
    * Gets an IP address from a socket address.
    * 
    * @param address the socket address to get the IP from.
    * 
    * @return the IP address for the socket address.
    */
   public static String getIP(SocketAddress address)
   {
      String ip = "";
      
      if(address != null)
      {
         ip = address.toString();
         
         int index = ip.indexOf("/");
         if(index != -1 && ip.length() > (index + 1))
         {
            ip = ip.substring(index + 1, ip.length());

            // chop off colon, if any
            index = ip.indexOf(":");
            if(index != -1)
            {
               ip = ip.substring(0, index);
            }
         }
      }
      
      return ip;
   }
   
   /**
    * Gets the local IP address for a socket.
    * 
    * @param s the socket to get the address of.
    * 
    * @return the local IP address for the socket.
    */
   public static String getLocalIP(Socket s)
   {
      String ip = "";
      
      if(s != null)
      {
         ip = getIP(s.getLocalSocketAddress());
      }
      
      return ip;
   }
   
   /**
    * Gets the remote IP address for a socket.
    * 
    * @param s the socket to get the address of.
    * 
    * @return the remote IP address for the socket.
    */
   public static String getRemoteIP(Socket s)
   {
      String ip = "";
      
      if(s != null)
      {
         ip = getIP(s.getRemoteSocketAddress());
      }
      
      return ip;
   }
   
   /**
    * Gets the remote port for a socket.
    * 
    * @param s the socket to get the remote port for.
    * 
    * @return the remote port for a socket or 0 if one could not be determined.
    */
   public static int getRemotePort(Socket s)
   {
      int rval = 0;
      
      if(s != null)
      {
         String address = s.getRemoteSocketAddress().toString();
         
         if(address.startsWith("/") && address.length() >= 1)
         {
            address = address.substring(1, address.length());

            // chop off colon, if any
            int index = address.indexOf(":");
            if(index != -1 && index < (address.length() - 1))
            {
               try
               {
                  rval = Integer.parseInt(address.substring(index));
               }
               catch(Throwable t)
               {
               }
            }
         }
      }
      
      return rval;
   }   
   
   /**
    * Gets the local port for this web connection.
    * 
    * @return the local port for this web connection.
    */
   public int getLocalPort()
   {
      return getWorkerSocket().getLocalPort();
   }
   
   /**
    * Sets the remote port for this web connection.
    * 
    * @param port the remote port for this web connection.
    */
   public void setRemotePort(int port)
   {
      mRemotePort = port;
   }   
   
   /**
    * Gets the remote port for this web connection.
    * 
    * @return the remote port for this web connection.
    */
   public int getRemotePort()
   {
      return mRemotePort;
   }
   
   /**
    * Marks this web connection as secure or non-secure.
    *
    * @param secure true to mark this web connection as secure, false to
    *               mark it as non-secure.
    */
   public void setSecure(boolean secure)
   {
      mSecure = secure;
   }
   
   /**
    * Returns whether or not this web connection has been marked as secure.
    * 
    * @return true if this web connection has been marked as secure, false
    *         if it has been marked as non-secure.
    */
   public boolean isSecure()
   {
      return mSecure;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
