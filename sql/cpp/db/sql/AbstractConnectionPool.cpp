/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/AbstractConnectionPool.h"

using namespace std;
using namespace db::rt;
using namespace db::sql;

AbstractConnectionPool::AbstractConnectionPool(
   const char* url, unsigned int poolSize) :
   mUrl(url), mConnectionSemaphore(poolSize, true)
{
   // default JobThread expire time to 0 (no expiration)
   mConnectionExpireTime = 0;
}

AbstractConnectionPool::~AbstractConnectionPool()
{
   // close all connections
   closeAllConnections();
}

Connection* AbstractConnectionPool::getIdleConnection()
{
   Connection* rval = NULL;
   
   // FIXME: semi-complicated solution for only going through the list once
   // we might want to use the less complicated but possibly slower method
   
   mListLock.lock();
   {
      if(!mConnections.empty())
      {
         // make list to hold expired connections
         list<Connection*> expiredConnections;
         
         // grab the first idle connection
         for(list<PooledConnection*>::iterator i = mConnections.begin();
             i != mConnections.end(); i++)
         {
            // get the connection
            PooledConnection* connection = *i;
            
            // find non-active connections
            if(connection->getIdleTime() != 0)
            {
               if(connection->getIdleTime() <=
                  System::getCurrentMilliseconds() - mConnectionExpireTime)
               {
                  // mark as expired
                  expiredConnections.push_back(connection);
               }
               else
               {
                  // idle connection
                  if(rval == NULL)
                  {
                     // save idle connection if we haven't already
                     rval = connection;
                  }
               }
            }
         }
         
         // made it through list, check if we have an idle connection to use
         if(rval == NULL)
         {
            if(!expiredConnections.empty())
            {
               // use the first expired connection
               rval = expiredConnections.front();
               expiredConnections.pop_front();
               
               // FIXME: remove the rest of the expired connections
               
            }
            else
            {
               // no idle or expired connections, create a connection to use
               rval = createConnection();
               mConnections.push_back((PooledConnection*)rval);
            }
         }
      }
      else
      {
         // create new connection and add to connection list
         rval = createConnection();
         mConnections.push_back((PooledConnection*)rval);
      }
   }
   mListLock.unlock();
   
   return rval;
}

void AbstractConnectionPool::closeExpiredConnections()
{
   // FIXME:
   // get list lock, go through idle connections and remove any who have expired
   if(!mConnections.empty())
   {
      list<PooledConnection*> expiredConnections;
      
      mListLock.lock();
      {
         for(list<PooledConnection*>::iterator i = mConnections.begin();
             i != mConnections.end();)
         {
            // get the connection
            PooledConnection* connection = *i;
            
            // check idle time to see if connection has expired
            if(connection->getIdleTime() <=
               System::getCurrentMilliseconds() - mConnectionExpireTime)
            {
               // put in expired list (erase automatically advances iterator)
               i = mConnections.erase(i);
               expiredConnections.push_back(connection);
            }
            else
            {
               // manually increment iterator
               i++;
            }
         }
      }
      mListLock.unlock();
      
      if(!expiredConnections.empty())
      {
         for(list<PooledConnection*>::iterator i = expiredConnections.begin();
             i != expiredConnections.end(); i++)
         {
            // close the expired connections and then delete them
            (*i)->closeConnection();
            
            delete (*i);
         }
      }
   } 
}

Connection* AbstractConnectionPool::getConnection()
{
   Connection* rval = NULL;
   
   // FIXME: implement me
   
   return rval;
}

void AbstractConnectionPool::closeAllConnections()
{
   // lock list to close all connections
   mListLock.lock();
   {
      for(list<PooledConnection*>::iterator i = mConnections.begin();
          i != mConnections.end();)
      {
         PooledConnection* connection = *i;
         connection->closeConnection();
         i = mConnections.erase(i);
         delete connection;
      }
      
      // clear all connections
      mConnections.clear();
   }
   mListLock.unlock();
}

void AbstractConnectionPool::setPoolSize(unsigned int size)
{
   // lock pool to update pool size
   lock();
   {
      // Note: connections are created lazily so if the connection pool size
      // here is greater than the number of connections currently in
      // the pool, those connections will be created as they are needed
      // hence, we do not need to adjust for increases in the pool size
      // only for decreases
      
      // remove connections as necessary
      if(mConnections.size() > size && size != 0)
      {
         closeExpiredConnections();
      }
      
      // set semaphore permits
      mConnectionSemaphore.setMaxPermitCount(size);
   }
   unlock();
}

unsigned int AbstractConnectionPool::getPoolSize()
{
   return mConnectionSemaphore.getMaxPermitCount();
}

void AbstractConnectionPool::setConnectionExpireTime(
   unsigned long long expireTime)
{
   lock();
   {
      mConnectionExpireTime = expireTime;
   }
   unlock();
}

unsigned long long AbstractConnectionPool::getConnectionExpireTime()
{
   return mConnectionExpireTime;
}

unsigned int AbstractConnectionPool::getConnectionCount()
{
   return mConnections.size();
}

unsigned int AbstractConnectionPool::getActiveConnectionCount()
{
   unsigned int activeConnections = 0;
   
   // FIXME: do I need to lock on read-only?
   
   // check list for active connections
   for(list<PooledConnection*>::iterator i = mConnections.begin();
       i != mConnections.end(); i++)
   {
      // get the connection
      PooledConnection* connection = *i;
      
      // check if active connection
      if(connection->getIdleTime() == 0)
      {
         activeConnections++;
      }
   }
   
   return activeConnections;
}

unsigned int AbstractConnectionPool::getIdleConnectionCount()
{
   unsigned int idleConnections = 0;
   
   // FIXME: do I need to lock on read-only?
   
   // check list for idle connections
   for(list<PooledConnection*>::iterator i = mConnections.begin();
       i != mConnections.end(); i++)
   {
      // get the connection
      PooledConnection* connection = *i;
      
      // check if idle connection
      if(connection->getIdleTime() >
         System::getCurrentMilliseconds() - mConnectionExpireTime)
      {
         idleConnections++;
      }
   }
   
   return idleConnections;
}

unsigned int AbstractConnectionPool::getExpiredConnectionCount()
{
   unsigned int expiredConnections = 0;
   
   // FIXME: do I need to lock on read-only?
   
   // check list for active connections
   for(list<PooledConnection*>::iterator i = mConnections.begin();
       i != mConnections.end(); i++)
   {
      // get the connection
      PooledConnection* connection = *i;
      
      // check if expired connection
      if(connection->getIdleTime() <=
         System::getCurrentMilliseconds() - mConnectionExpireTime)
      {
         expiredConnections++;
      }
   }
   
   return expiredConnections;
}
