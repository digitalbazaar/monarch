/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

/**
 * This interface represents a web connection to a web server.
 * 
 * @author Dave Longley
 */
public interface WebConnection
{
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
    * @throws InterruptedException
    */
   public int read(byte[] buffer, int offset, int length)
   throws IOException, InterruptedException;
   
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
    * @throws IOException
    * @throws InterruptedException
    */
   public int blockedRead(byte[] buffer, int offset, int length)
   throws IOException, InterruptedException;
   
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
   throws IOException, InterruptedException;
   
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
   public String readLine() throws IOException, InterruptedException;
   
   /**
    * Gets the number of bytes read so far.
    * 
    * @return the number of bytes read so far.
    */
   public long getBytesRead();
   
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
   throws IOException, InterruptedException;
   
   /**
    * Gets the number of bytes written so far.
    * 
    * @return the number of bytes written so far.
    */
   public long getBytesWritten();
   
   /**
    * Sets this web connection's BandwidthThrottler for reading.
    * 
    * @param throttler the BandwidthThrottler to use when reading.
    */
   public void setReadBandwidthThrottler(BandwidthThrottler throttler);

   /**
    * Gets this web connection's BandwidthThrottler for reading.
    * 
    * @return this web connection's read BandwidthThrottler.
    */
   public BandwidthThrottler getReadBandwidthThrottler();
   
   /**
    * Sets this web connection's BandwidthThrottler for writing.
    * 
    * @param throttler the BandwidthThrottler to use when writing.
    */
   public void setWriteBandwidthThrottler(BandwidthThrottler throttler);

   /**
    * Gets this web connection's BandwidthThrottler for writing.
    * 
    * @return this web connection's write BandwidthThrottler.
    */
   public BandwidthThrottler getWriteBandwidthThrottler();

   /**
    * Returns true if this web connection is connected, false if it is not.
    * 
    * @return true if this web connection is connected, false if not.
    */
   public boolean isConnected();
   
   /**
    * Returns true if this web connection is closed, false if it is not.
    * 
    * @return true if this web connection is closed, false if not.
    */
   public boolean isClosed();
   
   /**
    * Disconnects this web connection.
    */
   public void disconnect();
   
   /**
    * Gets the host for this web connection.
    * 
    * @return the host for this web connection.
    */
   public String getHost();
   
   /**
    * Gets the input stream used to read and unread from the web connection.
    * 
    * @return the input stream used to read and unread from the web connection.
    * 
    * @throws IOException
    */
   public InputStream getReadStream()
   throws IOException;
   
   /**
    * Gets the output stream used to write to the web connection.
    * 
    * @return the output stream used to write to the web connection.
    * 
    * @throws IOException
    */
   public OutputStream getWriteStream()
   throws IOException;
   
   /**
    * Gets the worker socket for this web connection.
    * 
    * @return the worker socket for this web connection.
    */
   public Socket getWorkerSocket();
   
   /**
    * Sets the read timeout for this web connection. This is the amount of
    * time that must pass while doing a blocking read before timing out. 
    * 
    * @param timeout the read timeout in milliseconds (0 for no timeout).
    */
   public void setReadTimeout(int timeout);
   
   /**
    * Gets the local IP address for this web connection.
    * 
    * @return the local IP address for this web connection.
    */
   public String getLocalIP();

   /**
    * Sets the remote IP address for this web connection.
    * 
    * @param ip the remote IP address for this web connection.
    */
   public void setRemoteIP(String ip);
   
   /**
    * Gets the remote IP address for this web connection.
    * 
    * @return the remote IP address for this web connection.
    */
   public String getRemoteIP();
   
   /**
    * Gets the local port for this web connection.
    * 
    * @return the local port for this web connection.
    */
   public int getLocalPort();
   
   /**
    * Sets the remote port for this web connection.
    * 
    * @param port the remote port for this web connection.
    */
   public void setRemotePort(int port);
   
   /**
    * Gets the remote port for this web connection.
    * 
    * @return the remote port for this web connection.
    */
   public int getRemotePort();
   
   /**
    * Marks this web connection as secure or non-secure.
    *
    * @param secure true to mark this web connection as secure, false to
    *               mark it as non-secure.
    */
   public void setSecure(boolean secure);
   
   /**
    * Returns whether or not this web connection has been marked as secure.
    * 
    * @return true if this web connection has been marked as secure, false
    *         if it has been marked as non-secure.
    */
   public boolean isSecure();
}
