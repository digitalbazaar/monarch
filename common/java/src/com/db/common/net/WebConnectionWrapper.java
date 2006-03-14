/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

/**
 * This class is a base class for web connection wrappers. Any type of
 * connection that wishes to wrap a web connection to add some functionality
 * to the web connection should inherit from this class. This class provides
 * basic wrapper methods that expose an underlying web connection's
 * functionality.
 * 
 * @author Dave Longley
 */
public class WebConnectionWrapper implements WebConnection
{
   /**
    * The web connection that is wrapped by this wrapper.
    */
   protected WebConnection mWebConnection;
   
   /**
    * Creates a new web connection wrapper.
    * 
    * @param wc the web connection to wrap.
    */
   public WebConnectionWrapper(WebConnection wc)
   {
      mWebConnection = wc;
   }
   
   /**
    * Ensures the wrapped web connection gets disconnected.
    */
   protected void finalize()
   {
      mWebConnection.disconnect();
   }
   
   /**
    * Writes data to the underlying web connection.
    * 
    * @param buffer the buffer to write data from to the web connection.
    * @param offset the offset to start writing the data.
    * @param length the amount of data to write.
    * @throws IOException
    */
   public void write(byte[] buffer, int offset, int length)
   throws IOException
   {
      getWebConnection().write(buffer, offset, length);
   }

   /**
    * Reads data from the underlying web connection. The actual number of bytes
    * read may be less than the amount asked for.
    * 
    * @param buffer the buffer to read data into from the web connection.
    * @param offset the offset to start writing the data into the buffer.
    * @param length the amount of data to read.
    * @return the actual number of bytes read or -1 if there is no
    *         more data to read.
    * @throws IOException
    */
   public int read(byte[] buffer, int offset, int length)
   throws IOException
   {
      return getWebConnection().read(buffer, offset, length);
   }
   
   /**
    * Reads data from the underlying web connection. This method will block
    * until there is no more data to read (end of stream) or until the amount
    * of data asked for has been read.
    * 
    * @param buffer the buffer to read data into from the web connection.
    * @param offset the offset to start writing the data into the buffer.
    * @param length the amount of data to read.
    * @return the actual number of bytes read or -1 if the end of the
    *         stream has been reached.
    * @throws IOException
    */
   public int blockedRead(byte[] buffer, int offset, int length)
   throws IOException
   {
      return getWebConnection().blockedRead(buffer, offset, length);
   }
   
   /**
    * Unreads data back onto the underlying web connection. This method is
    * useful if a little bit of data must be read from the web connection to
    * determine how to service the connection -- but that data must then
    * be re-read later. The data can only be re-read using the underlying 
    * web connection object, it is not sent back over the physical
    * web connection.
    * 
    * @param buffer the buffer to write data from to this connection.
    * @param offset the offset to start writing the data.
    * @param length the amount of data to write.
    * @throws IOException
    */
   public void unread(byte[] buffer, int offset, int length)
   throws IOException
   {
      getWebConnection().unread(buffer, offset, length);
   }
   
   /**
    * Reads a single line from the underlying web connection that terminates in
    * a end of line character ('\n'), a carriage return ('\r'), or both an
    * end of line character and a carriage return ("\r\n" -- CRLF). This method
    * will block until there is no more data to read or until it reads a line.
    * 
    * @return the read line or null if the end of the stream was reached.
    * @throws IOException
    */
   public String readLine() throws IOException
   {
      return getWebConnection().readLine();
   }
   
   /**
    * Returns true if the underlying web connection is connected, false if it
    * is not.
    * 
    * @return true if the underlying web connection is connected, false if not.
    */
   public boolean isConnected()
   {
      return getWebConnection().isConnected();
   }
   
   /**
    * Returns true if the underlying web connection is closed, false if it is
    * not.
    * 
    * @return true if the underlying web connection is closed, false if not.
    */
   public boolean isClosed()
   {
      return getWebConnection().isClosed();
   }   
   
   /**
    * Disconnects the underlying web connection.
    */
   public void disconnect()
   {
      getWebConnection().disconnect();
   }
   
   /**
    * Gets the host for the underlying web connection.
    * 
    * @return the host for the underlying web connection.
    */
   public String getHost()
   {
      return getWebConnection().getHost();
   }
   
   /**
    * Gets the input stream used to read and unread from the underlying
    * web connection.
    * 
    * @return the input stream used to read and unread from the underlying
    *         web connection.
    * @throws IOException
    */
   public InputStream getReadStream()
   throws IOException
   {
      return getWebConnection().getReadStream();
   }
   
   /**
    * Gets the output stream used to write to the underlying web connection.
    * 
    * @return the output stream used to write to the underlying web connection.
    * @throws IOException
    */
   public OutputStream getWriteStream()
   throws IOException
   {
      return getWebConnection().getWriteStream();
   }
   
   /**
    * Gets the worker socket for the underlying web connection.
    * 
    * @return the worker socket for the underlying web connection.
    */
   public Socket getWorkerSocket()
   {
      return getWebConnection().getWorkerSocket();
   }
   
   /**
    * Sets the read timeout for the underlying web connection. This is the
    * amount of time that must pass while doing a blocking read before timing
    * out. 
    * 
    * @param timeout the read timeout in milliseconds (-1 for no timeout).
    */
   public void setReadTimeout(int timeout)
   {
      getWebConnection().setReadTimeout(timeout);
   }
   
   /**
    * Gets the local IP address for the underlying web connection.
    * 
    * @return the local IP address for the underlying web connection.
    */
   public String getLocalIP()
   {
      return getWebConnection().getLocalIP();
   }

   /**
    * Sets the remote IP address for the underlying web connection.
    * 
    * @param ip the remote IP address for the underlying web connection.
    */
   public void setRemoteIP(String ip)
   {
      getWebConnection().setRemoteIP(ip);
   }
   
   /**
    * Gets the remote IP address for the underlying web connection.
    * 
    * @return the remote IP address for the underlying web connection.
    */
   public String getRemoteIP()
   {
      return getWebConnection().getRemoteIP();
   }
   
   /**
    * Gets the local port for the underlying web connection.
    * 
    * @return the local port for the underlying web connection.
    */
   public int getLocalPort()
   {
      return getWebConnection().getLocalPort();
   }
   
   /**
    * Sets the remote port for the underlying web connection.
    * 
    * @param port the remote port for the underlying web connection.
    */
   public void setRemotePort(int port)
   {
      getWebConnection().setRemotePort(port);
   }   
   
   /**
    * Gets the remote port for the underlying web connection.
    * 
    * @return the remote port for the underlying web connection.
    */
   public int getRemotePort()
   {
      return getWebConnection().getRemotePort();
   }
   
   /**
    * Marks the underlying web connection as secure or non-secure.
    *
    * @param secure true to mark the web connection as secure, false to
    *               mark it as non-secure.
    */
   public void setSecure(boolean secure)
   {
      getWebConnection().setSecure(secure);
   }
   
   /**
    * Returns whether or not the underlying web connection has been marked as
    * secure.
    * 
    * @return true if the web connection has been marked as secure, false
    *         if it has been marked as non-secure.
    */
   public boolean isSecure()
   {
      return getWebConnection().isSecure();
   }
   
   /**
    * Gets the underlying web connection that is wrapped by this wrapper.
    * 
    * @return the underlying web connection.
    */
   public WebConnection getWebConnection()
   {
      return mWebConnection;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
