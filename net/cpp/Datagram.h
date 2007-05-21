/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Datagram_H
#define Datagram_H

#include "Object.h"
#include "InternetAddress.h"
#include <string>

namespace db
{
namespace net
{

/**
 * A Datagram is a self-contained, independent entity of data.
 * 
 * @author Dave Longley
 */
class Datagram : public virtual db::rt::Object
{
protected:
   /**
    * The source or destination address for this Datagram.
    */
   InternetAddress* mAddress;
   
   /**
    * The data for this Datagram.
    */
   char* mData;
   
   /**
    * The length of the data in this Datagram.
    */
   unsigned int mLength;
   
   /**
    * True to clean up the internal data buffer, false not to.
    */
   bool mCleanup;
   
   /**
    * Frees the internal data for this Datagram.
    */
   virtual void freeData();
   
public:
   /**
    * Creates a new Datagram. An InternetAddress must be specified that will
    * either be used to send this Datagram or it will be populated when this
    * Datagram is used to receive data.
    * 
    * If a length for this Datagram is specified, then an internal buffer
    * will be allocated to store data of the specified length.
    * 
    * @param address the InternetAddress to associate with this Datagram.
    * @param length the length, in bytes, of this Datagram.
    */
   Datagram(InternetAddress* address, unsigned int length = 0);
   
   /**
    * Destructs this Datagram.
    */
   virtual ~Datagram();
   
   /**
    * Sets the address for this Datagram. This is either the source or
    * destination address for this Datagram.
    * 
    * @param address the address for this Datagram.
    */
   virtual void setAddress(InternetAddress* address);
   
   /**
    * Gets the address for this Datagram. This is either the source or
    * destination address for this Datagram.
    * 
    * @return the address for this Datagram.
    */
   virtual InternetAddress* getAddress();
   
   /**
    * Sets the data for this Datagram. This method will set the internal
    * data buffer to the passed data buffer. Any existing internal data
    * buffer will be freed.
    * 
    * @param data the data to use.
    * @param length the length of the data.
    * @param cleanup true to cleanup the data when the Datagram is destructed,
    *                false not to.
    */ 
   virtual void setData(char* data, unsigned int length, bool cleanup);
   
   /**
    * Assigns the data for this Datagram. This method will copy the passed
    * data to the internal data buffer for this Datagram. The copied data
    * will be freed when this Datagram is destructed.
    * 
    * @param data the data to use.
    * @param length the length of the data.
    */ 
   virtual void assignData(const char* data, unsigned int length);
   
   /**
    * Sets the length of the data for this Datagram.
    * 
    * @param length the length of the data for this Datagram.
    */
   virtual void setLength(unsigned int length);
   
   /**
    * Gets this Datagram's data.
    * 
    * @param length to store the length, in bytes, of the data.
    * 
    * @return this Datagram's data.
    */
   virtual char* getData(unsigned int& length);
      
   /**
    * Assigns the data for this Datagram to the passed string. This method will
    * copy the passed data to the internal data buffer for this Datagram. The
    * copied data will be freed when this Datagram is destructed. The
    * terminating NULL character will NOT be included.
    * 
    * @param str the string to use.
    */ 
   virtual void assignString(const std::string& str);
   
   /**
    * Gets this Datagram's data as a string.
    * 
    * @return this Datagram's data as a string.
    */
   virtual std::string getString();
};

} // end namespace net
} // end namespace db
#endif
