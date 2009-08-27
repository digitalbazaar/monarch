/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3Statement.h"

#include "db/sql/sqlite3/Sqlite3Connection.h"
#include "db/sql/sqlite3/Sqlite3Row.h"

using namespace std;
using namespace db::sql;
using namespace db::sql::sqlite3;
using namespace db::rt;

Sqlite3Statement::Sqlite3Statement(const char* sql) :
   Statement(sql),
   mConnection(NULL),
   mHandle(NULL),
   mRow(NULL)
{
}

Sqlite3Statement::~Sqlite3Statement()
{
   // clean up row, if any
   if(mRow != NULL)
   {
      delete mRow;
   }

   if(mHandle != NULL)
   {
      // clean up handle
      sqlite3_finalize(mHandle);
   }
}

Connection* Sqlite3Statement::getConnection()
{
   return mConnection;
}

inline sqlite3_stmt* Sqlite3Statement::getHandle()
{
   return mHandle;
}

bool Sqlite3Statement::initialize(Sqlite3Connection* c)
{
   bool rval = true;

   mConnection = c;

   const char* tail;
   mState = sqlite3_prepare_v2(c->getHandle(), mSql, -1, &mHandle, &tail);
   if(mState != SQLITE_OK)
   {
      // exception
      ExceptionRef e = new Sqlite3Exception(c);
      e->getDetails()["sql"] = mSql;
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool Sqlite3Statement::setInt32(unsigned int param, int32_t value)
{
   bool rval = true;

   mState = sqlite3_bind_int(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception(mConnection);
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool Sqlite3Statement::setUInt32(unsigned int param, uint32_t value)
{
   bool rval = true;

   mState = sqlite3_bind_int(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception(mConnection);
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool Sqlite3Statement::setInt64(unsigned int param, int64_t value)
{
   bool rval = true;

   mState = sqlite3_bind_int64(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception(mConnection);
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool Sqlite3Statement::setUInt64(unsigned int param, uint64_t value)
{
   bool rval = true;

   mState = sqlite3_bind_int64(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception(mConnection);
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool Sqlite3Statement::setText(unsigned int param, const char* value)
{
   bool rval = true;

   // use SQLITE_STATIC to ensure the memory is not cleaned up by sqlite
   mState = sqlite3_bind_text(mHandle, param, value, -1, SQLITE_STATIC);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception(mConnection);
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool Sqlite3Statement::setInt32(const char* name, int32_t value)
{
   bool rval = false;

   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setInt32(index, value);
   }

   return rval;
}

bool Sqlite3Statement::setUInt32(const char* name, uint32_t value)
{
   bool rval = false;

   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setUInt32(index, value);
   }

   return rval;
}

bool Sqlite3Statement::setInt64(const char* name, int64_t value)
{
   bool rval = false;

   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setInt64(index, value);
   }

   return rval;
}

bool Sqlite3Statement::setUInt64(const char* name, uint64_t value)
{
   bool rval = false;

   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setUInt64(index, value);
   }

   return rval;
}

bool Sqlite3Statement::setText(const char* name, const char* value)
{
   bool rval = false;

   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setText(index, value);
   }

   return rval;
}

bool Sqlite3Statement::execute()
{
   bool rval = true;

   switch(mState)
   {
      case SQLITE_OK:
         // step to execute statement
         mState = sqlite3_step(mHandle);
         switch(mState)
         {
            case SQLITE_DONE:
               // reset to finalize statement
               mState = sqlite3_reset(mHandle);
               break;
            case SQLITE_ROW:
               // got back a row
               break;
            default:
            {
               // error stepping statement, reset sqlite3 handle because it
               // will cause a more specific error to be set... doesn't
               // matter whether we use sqlite API v1 or v2, we still need
               // this here to get specific error message
               mState = sqlite3_reset(mHandle);
               ExceptionRef e = new Sqlite3Exception(mConnection);
               Exception::set(e);
               rval = false;
               break;
            }
         }
         break;
      case SQLITE_DONE:
      case SQLITE_ROW:
      {
         // statement in bad state
         ExceptionRef e = new Exception(
            "Statement state is invalid. Did you call reset() to reuse "
            "the statement? (Connections should do this automatically).",
            "db.sql.sqlite3.BadState");
         Exception::set(e);
         rval = false;
         break;
      }
      default:
      {
         // driver error
         ExceptionRef e = new Sqlite3Exception(mConnection);
         Exception::set(e);
         rval = false;
         break;
      }
   }

   return rval;
}

Row* Sqlite3Statement::fetch()
{
   Row* rval = NULL;

   if(mRow != NULL)
   {
      // get next row
      mState = sqlite3_step(mHandle);
      switch(mState)
      {
         case SQLITE_ROW:
            // return next row
            rval = mRow;
            break;
         case SQLITE_DONE:
            // no more rows, clean up row object
            delete mRow;
            mRow = NULL;
            // reset to finalize statement
            mState = sqlite3_reset(mHandle);
            break;
         default:
         {
            // error stepping statement
            ExceptionRef e = new Sqlite3Exception(mConnection);
            Exception::set(e);
            reset();
            break;
         }
      }
   }
   else if(mState == SQLITE_ROW)
   {
      // create and return first row
      rval = mRow = new Sqlite3Row(this);
   }

   return rval;
}

bool Sqlite3Statement::reset()
{
   bool rval = true;

   // clean up existing row object
   if(mRow != NULL)
   {
      delete mRow;
      mRow = NULL;
   }

   // reset statement
   mState = sqlite3_reset(mHandle);
   if(mState != SQLITE_OK)
   {
      // driver error
      ExceptionRef e = new Sqlite3Exception(mConnection);
      Exception::set(e);
      rval = false;
   }

   return rval;
}

bool Sqlite3Statement::getRowsChanged(uint64_t& rows)
{
   // FIXME: handle exceptions
   rows = sqlite3_changes(mConnection->getHandle());
   return true;
}

uint64_t Sqlite3Statement::getLastInsertRowId()
{
   return sqlite3_last_insert_rowid(mConnection->getHandle());
}

int Sqlite3Statement::getParameterIndex(const char* name)
{
   int index = sqlite3_bind_parameter_index(mHandle, name);
   if(index == 0)
   {
      // exception, no parameter with given name found (index=0 is invalid)
      ExceptionRef e = new SqlException("Invalid parameter name.");
      e->getDetails()["name"] = name;
      Exception::set(e);
   }

   return index;
}
