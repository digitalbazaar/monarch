/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_DatagramService_H
#define db_net_DatagramService_H

#include "InternetAddress.h"
#include "DatagramServicer.h"
#include "Runnable.h"

namespace db
{
namespace net
{

/**
 * A DatagramService binds to an address to communicate using Datagrams.
 * 
 * @author Dave Longley
 */
class DatagramService : public virtual db::rt::Object, public db::rt::Runnable
{
protected:
   /**
    * The address to bind to.
    */
   InternetAddress* mAddress;
   
   /**
    * The DatagramServicer to use.
    */
   DatagramServicer* mServicer;
   
public:
   /**
    * Creates a new DatagramService that uses the passed address for
    * communication.
    * 
    * @param address the address to bind to.
    * @param servicer the DatagramServicer to service datagrams with.
    */
   DatagramService(InternetAddress* address, DatagramServicer* servicer);
   
   /**
    * Destructs this DatagramService.
    */
   virtual ~DatagramService();
   
   /**
    * Runs this DatagramService.
    */
   virtual void run();
   
   /**
    * Gets the address for this DatagramService.
    * 
    * @return the address for this DatagramService.
    */
   virtual InternetAddress* getAddress();
   
   /**
    * Gets a string representation for this Runnable.
    * 
    * @param str the string to populate.
    * 
    * @return a string representation for this Runnable.
    */
   virtual std::string& toString(std::string& str);
};

} // end namespace net
} // end namespace db
#endif
