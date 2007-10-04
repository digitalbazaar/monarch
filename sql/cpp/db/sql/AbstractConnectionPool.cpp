/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/AbstractConnectionPool.h"

using namespace std;
using namespace db::rt;
using namespace db::sql;

AbstractConnectionPool::AbstractConnectionPool(
   const char* url, unsigned int poolSize) :
   mConnectionSemaphore(poolSize, true), mUrl(url) 
{
   // default JobThread expire time to 0 (no expiration)
   mConnectionExpireTime = 0;
}

AbstractConnectionPool::~AbstractConnectionPool()
{
   // close all connections
   closeAllConnections();
}

void AbstractConnectionPool::connectionClosed(PooledConnection* connection)
{
   // find the closed connection
   bool closed = false;
   for(list<PooledConnection*>::iterator i = mActiveConnections.begin();
       !closed && i != mActiveConnections.end();)
   {
      if(*i == connection)
      {
         mListLock.lock();
         {
            // remove from active connections
            mActiveConnections.erase(i);
            
            // set the connection idle time
            connection->setIdleTime(System::getCurrentMilliseconds());
            
            // put at end of idle connections
            mIdleConnections.push_back(connection);
            
            // release connection permit
            mConnectionSemaphore.release();
         }
         mListLock.unlock();
      }
   }
}

Connection* AbstractConnectionPool::getIdleConnection()
{
   PooledConnection* rval = NULL;
   
   if(!mIdleConnections.empty())
   {
      mListLock.lock();
      {
         // get first idle connection
         rval = mIdleConnections.front();
         mIdleConnections.pop_front();
         mActiveConnections.push_back(rval);
      }
      mListLock.unlock();
   }
   else
   {
      // obtain connection permit
      bool acquired = true;
      if(mConnectionSemaphore.getMaxPermitCount() != 0)
      {
         acquired = (mConnectionSemaphore.acquire() == NULL);
      }
      
      if(acquired)
      {
         // create new connection & add to active connections
         rval = createConnection();
         if(rval != NULL)
         {
            mListLock.lock();
            {
               mActiveConnections.push_back(rval);
            }
            mListLock.unlock();
         }
      }
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
         for(list<PooledConnection*>::iterator i = mIdleConnections.begin();
             i != mIdleConnections.end();)
         {
            // get the connection
            PooledConnection* connection = *i;
            
            // check idle time to see if connection has expired
            if(connection->getIdleTime() <=
               System::getCurrentMilliseconds() - mConnectionExpireTime)
            {
               // put in expired list (erase automatically advances iterator)
               i = mIdleConnections.erase(i);
               
               // close the expired connections and then delete them
               (*i)->closeConnection();
               delete (*i);
            }
            else
            {
               // manually increment iterator
               i++;
            }
         }
      }
      mListLock.unlock();
   } 
}

Connection* AbstractConnectionPool::getConnection()
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
   return mActiveConnections.size() + mIdleConnections.size();
}

unsigned int AbstractConnectionPool::getActiveConnectionCount()
{
   return mActiveConnections.size();
}

unsigned int AbstractConnectionPool::getIdleConnectionCount()
{
   return mIdleConnections.size();
}

unsigned int AbstractConnectionPool::getExpiredConnectionCount()
{
   unsigned int expiredConnections = 0;
   
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
   
   return expiredConnections;
}
