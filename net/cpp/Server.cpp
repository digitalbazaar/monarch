/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Server.h"
#include "ConnectionService.h"
#include "DatagramService.h"

using namespace std;
using namespace db::modest;
using namespace db::net;

Server::Server(Kernel* k) : mConnectionSemaphore(10000, true)
{
   mKernel = k;
   mRunning = false;
   mConnectionCount = 0;
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
   
   lock();
   {
      // add ConnectionService
      rval = addPortService(new ConnectionService(this, a, s, p));
   }
   unlock();
   
   return rval;
}

bool Server::addDatagramService(InternetAddress* a, DatagramServicer* s)
{
   bool rval = false;
   
   lock();
   {
      // add DatagramService
      rval = addPortService(new DatagramService(this, a, s));
   }
   unlock();
   
   return rval;
}

bool Server::start()
{
   bool rval = true;
   
   lock();
   {
      if(!isRunning())
      {
         // now running
         mRunning = true;
         mConnectionCount = 0;
         
         // start all port services
         for(map<unsigned short, PortService*>::iterator i =
             mPortServices.begin(); i != mPortServices.end(); i++)
         {
            rval &= i->second->start();
         }
      }
   }
   unlock();
   
   return rval;
}

void Server::stop()
{
   lock();
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
   mConnectionSemaphore.setMaxPermitCount(count);
}

unsigned int Server::getMaxConnectionCount()
{
   return mConnectionSemaphore.getMaxPermitCount();
}

unsigned int Server::getConnectionCount()
{
   return mConnectionCount;
}
