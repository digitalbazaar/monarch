/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/Server.h"

#include "monarch/net/ConnectionService.h"
#include "monarch/net/DatagramService.h"

#include <algorithm>

using namespace std;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;

Server::ServiceId Server::sInvalidServiceId = 0;

Server::Server() :
   mOperationRunner(NULL),
   mRunning(false),
   mMaxConnections(100),
   mCurrentConnections(0)
{
   // add first service ID
   mServiceIdFreeList.push_back(1);
}

Server::~Server()
{
   // ensure server is stopped
   Server::stop();

   // delete all port services
   for(PortServiceMap::iterator i = mPortServices.begin();
       i != mPortServices.end(); i++)
   {
      delete i->second;
   }
}

Server::ServiceId Server::addConnectionService(
   InternetAddress* a, ConnectionServicer* s, SocketDataPresenter* p,
   const char* name)
{
   ServiceId rval = sInvalidServiceId;

   mLock.lock();
   {
      // add ConnectionService
      ConnectionService* cs = new ConnectionService(this, a, s, p, name);
      rval = addPortService(cs);
      if(rval == sInvalidServiceId)
      {
         // clean up port service
         delete cs;
      }
   }
   mLock.unlock();

   return rval;
}

Server::ServiceId Server::addDatagramService(
   InternetAddress* a, DatagramServicer* s, const char* name)
{
   ServiceId rval = sInvalidServiceId;

   mLock.lock();
   {
      // add DatagramService
      DatagramService* ds = new DatagramService(this, a, s, name);
      rval = addPortService(ds);
      if(rval == sInvalidServiceId)
      {
         // clean up port service
         delete ds;
      }
   }
   mLock.unlock();

   return rval;
}

bool Server::removePortService(ServiceId id)
{
   bool rval = false;

   mLock.lock();
   {
      PortServiceMap::iterator i = mPortServices.find(id);
      if(i != mPortServices.end())
      {
         // add port service ID to front of free list
         mServiceIdFreeList.push_front(i->first);

         // remove port service from map
         PortService* ps = i->second;
         mPortServices.erase(i);

         // stop service if running
         if(isRunning())
         {
            ps->stop();
         }

         // delete port service
         delete ps;

         // removed port service
         rval = true;
      }
   }
   mLock.unlock();

   return rval;
}

bool Server::start(OperationRunner* opRunner)
{
   bool rval = true;

   mLock.lock();
   {
      if(!isRunning())
      {
         // now running
         mRunning = true;
         mOperationRunner = opRunner;

         // no connections yet
         mCurrentConnections = 0;

         // start all port services, fail if any cannot start
         for(PortServiceMap::iterator i = mPortServices.begin();
             rval && i != mPortServices.end(); i++)
         {
            rval = i->second->start();
         }

         if(!rval)
         {
            // save exception
            ExceptionRef e = Exception::get();
            Exception::clear();

            // stop all started port services
            stop();

            // reset exception as cause
            ExceptionRef ex = new Exception(
               "Could not start server. At least one port service failed.",
               "monarch.net.Server.PortServiceFailed");
            ex->setCause(e);
            Exception::set(ex);
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
         for(PortServiceMap::iterator i = mPortServices.begin();
             i != mPortServices.end(); i++)
         {
            i->second->interrupt();
         }

         // stop all port services
         for(PortServiceMap::iterator i = mPortServices.begin();
             i != mPortServices.end(); i++)
         {
            i->second->stop();
         }

         // no current connections
         mCurrentConnections = 0;

         // no longer running
         mOperationRunner = NULL;
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

inline void Server::setMaxConnectionCount(int32_t count)
{
   mMaxConnections = count;
}

inline int32_t Server::getMaxConnectionCount()
{
   return mMaxConnections;
}

inline int32_t Server::getConnectionCount()
{
   return mCurrentConnections;
}

PortService* Server::getPortService(ServiceId id)
{
   PortService* rval = NULL;

   PortServiceMap::iterator i = mPortServices.find(id);
   if(i != mPortServices.end())
   {
      rval = i->second;
   }

   return rval;
}

Server::ServiceId Server::addPortService(PortService* ps)
{
   ServiceId rval = sInvalidServiceId;

   bool added;
   if(isRunning())
   {
      // start service if server is running
      added = ps->start();
   }
   else
   {
      // no need to start service
      added = true;
   }

   if(added)
   {
      // get available ServiceId
      rval = mServiceIdFreeList.front();
      mServiceIdFreeList.pop_front();

      // add new id if list is empty
      if(mServiceIdFreeList.empty())
      {
         mServiceIdFreeList.push_back(rval + 1);
      }

      // set new port service
      mPortServices.insert(make_pair(rval, ps));
   }

   return rval;
}
