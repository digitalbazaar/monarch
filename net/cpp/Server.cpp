/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Server.h"

using namespace std;
using namespace db::modest;
using namespace db::net;

Server::Server(Kernel* k) : mRunningHandlers(true)
{
   mKernel = k;
   mMaxConnectionCount = 10000;
   mConnectionCount = 0;
}

Server::~Server()
{
   // ensure server is stopped
   stop();
   
   // delete all port handlers
   for(map<unsigned short, PortHandler*>::iterator i = mPortHandlers.begin();
       i != mPortHandlers.end(); i++)
   {
      delete i->second;
   }
}

PortHandler* Server::getPortHandler(unsigned short port)
{
   PortHandler* rval = NULL;
   
   map<unsigned short, PortHandler*>::iterator i = mPortHandlers.find(port);
   if(i != mPortHandlers.end())
   {
      rval = i->second;
   }
   
   return rval;
}

PortHandler* Server::createPortHandler(unsigned short port)
{
   PortHandler* rval = getPortHandler(port);
   if(rval == NULL)
   {
      rval = new PortHandler();
      rval->type = 0;
      rval->connectionHandler = NULL;
      mPortHandlers[port] = rval;
   }
   
   return rval;
}

void Server::addConnectionHandler(ConnectionHandler* h)
{
   lock();
   {
      // create a PortHandler
      PortHandler* ph = createPortHandler(h->getAddress()->getPort());
      ph->type = 0;
      ph->connectionHandler = h;
   }
   unlock();
}

void Server::addDatagramHandler(DatagramHandler* h)
{
   lock();
   {
      // create a PortHandler
      PortHandler* ph = createPortHandler(h->getAddress()->getPort());
      ph->type = 1;
      ph->datagramHandler = h;
   }
   unlock();
}

void Server::start()
{
   lock();
   {
      if(!isRunning())
      {
         // now running
         mRunning = true;
         mConnectionCount = 0;
         
         // start all handlers
         for(map<unsigned short, PortHandler*>::iterator i =
             mPortHandlers.begin(); i != mPortHandlers.end(); i++)
         {
            PortHandler* ph = i->second;
            Operation* op = new Operation(ph->runnable, NULL, NULL);
            mRunningHandlers.add(op);
            mKernel->getEngine()->queue(op);
         }
      }
   }
   unlock();
}

void Server::stop()
{
   lock();
   {
      if(isRunning())
      {
         // terminate all handlers
         mRunningHandlers.terminate();
         
         // no longer running
         mRunning = false;
      }
   }
   unlock();
}

bool Server::isRunning()
{
   return mRunning;
}

Kernel* Server::getKernel()
{
   return mKernel;
}

void Server::setMaxConnectionCount(unsigned int count)
{
   mMaxConnectionCount = count;
}

unsigned int Server::getMaxConnectionCount()
{
   return mMaxConnectionCount;
}

unsigned int Server::getConnectionCount()
{
   return mConnectionCount;
}
