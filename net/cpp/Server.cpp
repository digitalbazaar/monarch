/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Server.h"

using namespace std;
using namespace db::modest;
using namespace db::net;

Server::Server(Kernel* k) : mRunningServices(true)
{
   mKernel = k;
   mRunning = false;
   mMaxConnectionCount = 10000;
   mConnectionCount = 0;
}
#include <iostream>
Server::~Server()
{
   cout << "~Server()" << endl;
   
   // ensure server is stopped
   stop();
   
   cout << "~Server() ensured stop()" << endl;
   
   // delete all port services
   for(map<unsigned short, PortService*>::iterator i = mPortServices.begin();
       i != mPortServices.end(); i++)
   {
      cout << "deleting port service" << endl;
      // delete Runnable service and actual PortService
      delete i->second->service;
      delete i->second;
      cout << "port service deleted." << endl;
   }
   
   cout << "~Server() all port services deleted." << endl;
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

PortService* Server::createPortService(unsigned short port)
{
   PortService* rval = getPortService(port);
   if(rval == NULL)
   {
      // create new port service
      rval = new PortService();
      mPortServices[port] = rval;
   }
   else
   {
      if(rval->service != NULL)
      {
         if(isRunning())
         {
            // stop old service
            rval->operation->interrupt();
            rval->operation->waitFor();
            mRunningServices.prune();
         }
         
         // delete service
         delete rval->service;
      }
   }
   
   return rval;
}

void Server::startPortService(PortService* ps)
{
   cout << "starting port service" << endl;
   ps->operation = new Operation(ps->service, NULL, NULL);
   mRunningServices.add(ps->operation);
   mKernel->getEngine()->queue(ps->operation);
}

void Server::addConnectionService(
   InternetAddress* a, ConnectionServicer* s, SocketDataPresenter* p)
{
   lock();
   {
      // create a PortService with a ConnectionService
      PortService* ps = createPortService(a->getPort());
      ps->service = new ConnectionService(this, a, s, p);
      
      // start service if server is running
      if(isRunning())
      {
         startPortService(ps);
      }
   }
   unlock();
}

void Server::addDatagramService(InternetAddress* a, DatagramServicer* s)
{
   lock();
   {
      // create a PortService with a DatagramService
      PortService* ps = createPortService(a->getPort());
      ps->service = new DatagramService(a, s);
      
      // start service if server is running
      if(isRunning())
      {
         startPortService(ps);
      }
   }
   unlock();
}

void Server::start()
{
   lock();
   {
      if(!isRunning())
      {
         cout << "starting up port services" << endl;
         
         // now running
         mRunning = true;
         mConnectionCount = 0;
         
         // start all services
         for(map<unsigned short, PortService*>::iterator i =
             mPortServices.begin(); i != mPortServices.end(); i++)
         {
            startPortService(i->second);
         }
         
         cout << "port services started." << endl;
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
         cout << "terminating all port services" << endl;
         
         // terminate all services
         mRunningServices.terminate();
         
         cout << "all port services terminated" << endl;
         
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
