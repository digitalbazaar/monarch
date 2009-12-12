/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/AbstractConnectionPool.h"

#include <algorithm>

using namespace std;
using namespace monarch::rt;
using namespace monarch::sql;

AbstractConnectionPool::AbstractConnectionPool(
   const char* url, unsigned int poolSize) :
   mConnectionSemaphore(poolSize, true), mUrl(url)
{
   // default connection expire time to 0 (no expiration)
   mConnectionExpireTime = 0;
}

AbstractConnectionPool::~AbstractConnectionPool()
{
   // close all connections
   closeAllConnections();
}

void AbstractConnectionPool::connectionClosed(PooledConnection* connection)
{
   mListLock.lock();
   {
      // find the closed connection
      list<PooledConnection*>::iterator i =
         find(mActiveConnections.begin(), mActiveConnections.end(), connection);
      if(i != mActiveConnections.end())
      {
         // remove from active connections
         mActiveConnections.erase(i);

         // set the connection idle time
         connection->setIdleTime(System::getCurrentMilliseconds());

         // put at front of idle connections
         mIdleConnections.push_front(connection);

         // release connection permit
         mConnectionSemaphore.release();
      }
   }
   mListLock.unlock();
}

Connection* AbstractConnectionPool::getIdleConnection()
{
   PooledConnection* rval = NULL;

   // obtain connection permit
   bool acquired = true;
   if(mConnectionSemaphore.getMaxPermitCount() != 0)
   {
      acquired = mConnectionSemaphore.acquire();
   }

   if(acquired)
   {
      mListLock.lock();
      {
         while(rval == NULL && !mIdleConnections.empty())
         {
            // get last idle connection
            rval = mIdleConnections.back();
            mIdleConnections.pop_back();

            // test for connectivity
            if(!rval->isConnected())
            {
               // connection no longer connected
               // close the expired connection and delete it
               rval->closeConnection();
               delete rval;
               rval = NULL;
            }
         }

         if(rval == NULL)
         {
            // create new connection
            rval = createConnection();
         }

         if(rval != NULL)
         {
            // add connection to list of active connections
            mActiveConnections.push_back(rval);
         }
      }
      mListLock.unlock();
   }

   // now try to close any expired connections
   closeExpiredConnections();

   return rval;
}

void AbstractConnectionPool::closeExpiredConnections()
{
   // go through idle connections and remove any who have expired
   if(!mIdleConnections.empty())
   {
      mListLock.lock();
      {
         // FIXME: we might want to use a different algorithm here that
         // is quicker but doesn't necessarily clean up all expired
         // connections at once -- so that each time a request is made
         // to get a connection, not all other connections must be checked
         // in the worst case

         // find the first expired connection, all that follow are expired
         bool found = false;
         list<PooledConnection*>::iterator i = mIdleConnections.begin();
         for(; !found && i != mIdleConnections.end(); i++)
         {
            // check idle time to see if connection has expired
            found = ((*i)->getIdleTime() <=
               System::getCurrentMilliseconds() - mConnectionExpireTime);
         }

         // close and clean up all expired connections
         if(found)
         {
            for(; i != mIdleConnections.end();)
            {
               // close the expired connection and delete it
               (*i)->closeConnection();
               delete (*i);

               // remove from idle list (erase automatically advances iterator)
               i = mIdleConnections.erase(i);
            }
         }
      }
      mListLock.unlock();
   }
}

inline Connection* AbstractConnectionPool::getConnection()
{
   return getIdleConnection();
}

void AbstractConnectionPool::closeAllConnections()
{
   // lock list to close all connections
   mListLock.lock();
   {
      // remove all active connections
      for(list<PooledConnection*>::iterator i = mActiveConnections.begin();
          i != mActiveConnections.end(); i++)
      {
         PooledConnection* connection = *i;
         connection->closeConnection();
         delete connection;
      }

      // remove all idle connections
      for(list<PooledConnection*>::iterator i = mIdleConnections.begin();
          i != mIdleConnections.end(); i++)
      {
         PooledConnection* connection = *i;
         connection->closeConnection();
         delete connection;
      }

      // clear all connection lists
      mActiveConnections.clear();
      mIdleConnections.clear();
   }
   mListLock.unlock();
}

void AbstractConnectionPool::setPoolSize(unsigned int size)
{
   // lock pool to update pool size
   lock();
   {
      // try to decrease pool size by closing expired connections
      if(getConnectionCount() > size && size != 0)
      {
         closeExpiredConnections();
      }

      // set semaphore max permits
      mConnectionSemaphore.setMaxPermitCount(size);
   }
   unlock();
}

inline unsigned int AbstractConnectionPool::getPoolSize()
{
   return mConnectionSemaphore.getMaxPermitCount();
}

void AbstractConnectionPool::setConnectionExpireTime(
   uint64_t expireTime)
{
   lock();
   {
      mConnectionExpireTime = expireTime;
   }
   unlock();
}

inline uint64_t AbstractConnectionPool::getConnectionExpireTime()
{
   return mConnectionExpireTime;
}

inline unsigned int AbstractConnectionPool::getConnectionCount()
{
   return mActiveConnections.size() + mIdleConnections.size();
}

inline unsigned int AbstractConnectionPool::getActiveConnectionCount()
{
   return mActiveConnections.size();
}

inline unsigned int AbstractConnectionPool::getIdleConnectionCount()
{
   return mIdleConnections.size();
}

unsigned int AbstractConnectionPool::getExpiredConnectionCount()
{
   unsigned int expiredConnections = 0;

   mListLock.lock();
   {
      // check list for active connections
      for(list<PooledConnection*>::iterator i = mIdleConnections.begin();
          i != mIdleConnections.end(); i++)
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
   }
   mListLock.unlock();

   return expiredConnections;
}
