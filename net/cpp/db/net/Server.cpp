/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/Server.h"

#include "db/net/ConnectionService.h"
#include "db/net/DatagramService.h"

using namespace std;
using namespace db::modest;
using namespace db::net;

Server::Server(OperationRunner* opRunner) : mConnectionSemaphore(1000, true)
{
   mOperationRunner = opRunner;
   mRunning = false;
}

Server::~Server()
{
   // ensure server is stopped
   stop();
   
   // delete all port services
   for(map<unsigned short, PortService*>::iterator i = mPortServices.begin();
       i != mPortServices.end(); i++)
   {
      delete i->second;
   }
}

PortService* Server::getPortService(unsigned short port)
{
   PortService* rval = NULL;
   
   map<unsigned short, PortService*>::iterator i = mPortServices.find(port);
   if(i != mPortServices.end())
   {
      rval = i->second;
   }
   
   return rval;
}

bool Server::addPortService(PortService* ps)
{
   bool rval = false;
   
   // get old port service
   PortService* old = getPortService(ps->getAddress()->getPort());
   if(old != NULL)
   {
      // stop and delete old port service
      old->stop();
      delete old;
   }
   
   // set new port service
   mPortServices[ps->getAddress()->getPort()] = ps;
   
   // start service if server is running
   if(isRunning())
   {
      rval = ps->start();
   }
   else
   {
      // no need to start service
      rval = true;
   }
   
   return rval;
}

bool Server::addConnectionService(
   InternetAddress* a, ConnectionServicer* s, SocketDataPresenter* p)
{
   bool rval = false;
   
   mLock.lock();
   {
      // add ConnectionService
      rval = addPortService(new ConnectionService(this, a, s, p));
   }
   mLock.unlock();
   
   return rval;
}

bool Server::addDatagramService(InternetAddress* a, DatagramServicer* s)
{
   bool rval = false;
   
   mLock.lock();
   {
      // add DatagramService
      rval = addPortService(new DatagramService(this, a, s));
   }
   mLock.unlock();
   
   return rval;
}

bool Server::start()
{
   bool rval = true;
   
   mLock.lock();
   {
      if(!isRunning())
      {
         // now running
         mRunning = true;
         
         // start all port services
         for(map<unsigned short, PortService*>::iterator i =
             mPortServices.begin(); i != mPortServices.end(); i++)
         {
            rval = rval && i->second->start();
         }
      }
   }
   mLock.unlock();
   
   return rval;
}

void Server::stop()
{
   mLock.lock();
   {
      if(isRunning())
      {
         // interrupt all port services
         for(map<unsigned short, PortService*>::iterator i =
             mPortServices.begin(); i != mPortServices.end(); i++)
         {
            i->second->interrupt();
         }
         
         // stop all port services
         for(map<unsigned short, PortService*>::iterator i =
             mPortServices.begin(); i != mPortServices.end(); i++)
         {
            i->second->stop();
         }
         
         // release any used connection permits
         int used = mConnectionSemaphore.usedPermits();
         if(used > 0)
         {
            mConnectionSemaphore.release(used);
         }
         
         // no longer running
         mRunning = false;
      }
   }
   mLock.unlock();
}

inline bool Server::isRunning()
{
   return mRunning;
}

inline OperationRunner* Server::getOperationRunner()
{
   return mOperationRunner;
}

inline void Server::setMaxConnectionCount(unsigned int count)
{
   mConnectionSemaphore.setMaxPermitCount(count);
}

inline unsigned int Server::getMaxConnectionCount()
{
   return mConnectionSemaphore.getMaxPermitCount();
}

inline unsigned int Server::getConnectionCount()
{
   return mConnectionSemaphore.usedPermits();
}
