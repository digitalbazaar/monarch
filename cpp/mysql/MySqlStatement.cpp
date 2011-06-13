/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/mysql/MySqlStatement.h"

#include "monarch/sql/mysql/MySqlConnection.h"
#include "monarch/sql/mysql/MySqlRow.h"
#include "monarch/util/Data.h"

using namespace std;
using namespace monarch::sql;
using namespace monarch::sql::mysql;
using namespace monarch::rt;

MySqlStatement::MySqlStatement(const char* sql) :
   Statement(sql),
   mConnection(NULL),
   mHandle(NULL),
   mResult(NULL),
   mParamCount(0),
   mParamBindings(NULL),
   mExecuted(false),
   mFieldCount(0),
   mResultBindings(NULL),
   mRow(NULL)
{
}

MySqlStatement::~MySqlStatement()
{
   // clean up result meta-data, if any
   if(mResult != NULL)
   {
      mysql_free_result(mResult);
   }

   // clean up bindings
   if(mParamBindings != NULL)
   {
      // ensure heap data is cleaned up
      for(unsigned int i = 0; i < mParamCount; ++i)
      {
         // clean up malloc'd data
         switch(mParamBindings[i].buffer_type)
         {
            case MYSQL_TYPE_LONG:
            case MYSQL_TYPE_LONGLONG:
               free(mParamBindings[i].buffer);
               break;
            case MYSQL_TYPE_BLOB:
               free(mParamBindings[i].buffer);
               free(mParamBindings[i].length);
               break;
            default:
               // nothing to clean up
               break;
         }
      }

      // clean up all bindings
      delete [] mParamBindings;
   }
   delete [] mResultBindings;
   delete mRow;

   if(mHandle != NULL)
   {
      // clean up statement handle
      mysql_stmt_close(mHandle);
   }
}

Connection* MySqlStatement::getConnection()
{
   return mConnection;
}

inline MYSQL_STMT* MySqlStatement::getHandle()
{
   return mHandle;
}

bool MySqlStatement::initialize(MySqlConnection* c)
{
   bool rval = true;

   mConnection = c;

   // initialize handle
   mHandle = mysql_stmt_init(c->getHandle());
   if(mHandle == NULL)
   {
      // connection exception
      ExceptionRef e = c->createException();
      Exception::set(e);
      rval = false;
   }
   else
   {
      // prepare statement
      if(mysql_stmt_prepare(mHandle, mSql, strlen(mSql)) != 0)
      {
         // statement exception
         ExceptionRef e = createException();
         Exception::set(e);
         rval = false;
      }
      else
      {
         // determine number of parameters, initialize bindings
         mParamCount = mysql_stmt_param_count(mHandle);
         if(mParamCount > 0)
         {
            mParamBindings = new MYSQL_BIND[mParamCount];
            memset(mParamBindings, 0, sizeof(mParamBindings));
         }
      }
   }

   return rval;
}

bool MySqlStatement::setInt32(unsigned int param, int32_t value)
{
   bool rval = checkParamCount(param);

   if(rval)
   {
      // param is 1 higher than bind index
      --param;

      if(!mExecuted)
      {
         // we haven't executed before, so allocate space for the integer
         mParamBindings[param].buffer = malloc(sizeof(value));
      }

      // FIXME: ensure value is in the correct byte order
      //value = MO_INT32_TO_LE(value);

      // MYSQL_TYPE_LONG should be a 32-bit int INTEGER field,
      // length = 0 for ints
      // since the integer is stack-allocated, we must heap-allocate it here
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONG;
      memcpy(mParamBindings[param].buffer, &value, sizeof(value));
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = 0;
      mParamBindings[param].is_unsigned = 0;
   }

   return rval;
}

bool MySqlStatement::setUInt32(unsigned int param, uint32_t value)
{
   bool rval = checkParamCount(param);

   if(rval)
   {
      // param is 1 higher than bind index
      --param;

      if(!mExecuted)
      {
         // we haven't executed before, so allocate space for the integer
         mParamBindings[param].buffer = malloc(sizeof(value));
      }

      // ensure value is in the correct byte order
      value = MO_UINT32_TO_LE(value);

      // MYSQL_TYPE_LONG should be a 32-bit int INTEGER field,
      // length = 0 for ints
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONG;
      memcpy(mParamBindings[param].buffer, &value, sizeof(value));
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = 0;
      mParamBindings[param].is_unsigned = 1;
   }

   return rval;
}

bool MySqlStatement::setInt64(unsigned int param, int64_t value)
{
   bool rval = checkParamCount(param);

   if(rval)
   {
      // param is 1 higher than bind index
      --param;

      if(!mExecuted)
      {
         // we haven't executed before, so allocate space for the integer
         mParamBindings[param].buffer = malloc(sizeof(value));
      }

      // FIXME: ensure value is in the correct byte order
      //value = MO_INT64_TO_LE(value);

      // MYSQL_TYPE_LONGLONG should be a 64-bit int BIGINT field,
      // length = 0 for ints
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONGLONG;
      memcpy(mParamBindings[param].buffer, &value, sizeof(value));
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = 0;
      mParamBindings[param].is_unsigned = 0;
   }

   return rval;
}

bool MySqlStatement::setUInt64(unsigned int param, uint64_t value)
{
   bool rval = checkParamCount(param);

   if(rval)
   {
      // param is 1 higher than bind index
      --param;

      if(!mExecuted)
      {
         // we haven't executed before, so allocate space for the integer
         mParamBindings[param].buffer = malloc(sizeof(value));
      }

      // ensure value is in the correct byte order
      value = MO_UINT64_TO_LE(value);

      // MYSQL_TYPE_LONGLONG should be a 64-bit int BIGINT field,
      // length = 0 for ints
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONGLONG;
      memcpy(mParamBindings[param].buffer, &value, sizeof(value));
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = 0;
      mParamBindings[param].is_unsigned = 1;
   }

   return rval;
}

bool MySqlStatement::setText(unsigned int param, const char* value)
{
   return setBlob(param, value, strlen(value));
}

bool MySqlStatement::setBlob(unsigned int param, const char* value, int length)
{
   bool rval = checkParamCount(param);

   if(rval)
   {
      // param is 1 higher than bind index
      --param;
      unsigned long len = length;

      if(!mExecuted)
      {
         // we haven't executed before, allocate space for the length
         mParamBindings[param].buffer = malloc(length);
         mParamBindings[param].length = (unsigned long*)malloc(sizeof(len));
      }
      else
      {
         // re-allocate space for the data
         mParamBindings[param].buffer =
            realloc(mParamBindings[param].buffer, length);
      }

      // FIXME: ensure length is in the correct byte order
      //len = MO_UINT32_TO_LE(len);

      // MYSQL_TYPE_BLOB should be a BLOB or TEXT field
      // length is heap-allocated and cleaned up later
      mParamBindings[param].buffer_type = MYSQL_TYPE_BLOB;
      memcpy(mParamBindings[param].buffer, value, length);
      mParamBindings[param].is_null = 0;
      memcpy(mParamBindings[param].length, &len, sizeof(len));
   }

   return rval;
}

bool MySqlStatement::setInt32(const char* name, int32_t value)
{
   ExceptionRef e = new Exception(
      "MySql named parameter support not implemented.",
      "monarch.sql.mysql.MySql");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setUInt32(const char* name, uint32_t value)
{
   ExceptionRef e = new Exception(
      "MySql named parameter support not implemented.",
      "monarch.sql.mysql.MySql");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setInt64(const char* name, int64_t value)
{
   ExceptionRef e = new Exception(
      "MySql named parameter support not implemented.",
      "monarch.sql.mysql.MySql");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setUInt64(const char* name, uint64_t value)
{
   ExceptionRef e = new Exception(
      "MySql named parameter support not implemented.",
      "monarch.sql.mysql.MySql");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setText(const char* name, const char* value)
{
   ExceptionRef e = new Exception(
      "MySql named parameter support not implemented.",
      "monarch.sql.mysql.MySql");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setBlob(const char* name, const char* value, int length)
{
   ExceptionRef e = new Exception(
      "MySql named parameter support not implemented.",
      "monarch.sql.mysql.MySql");
   Exception::set(e);
   return false;
}

bool MySqlStatement::execute()
{
   bool rval = false;

   // bind parameters
   if(mysql_stmt_bind_param(mHandle, mParamBindings) != 0)
   {
      // statement exception
      ExceptionRef e = createException();
      Exception::set(e);
   }
   else if(mysql_stmt_execute(mHandle) != 0)
   {
      // statement exception
      ExceptionRef e = createException();
      Exception::set(e);
   }
   else
   {
      rval = true;

      if(!mExecuted || mResult == NULL)
      {
         // get result meta-data
         mResult = mysql_stmt_result_metadata(mHandle);
      }

      if(mResult != NULL)
      {
         if(!mExecuted || mResultBindings == NULL)
         {
            // get field count
            mFieldCount = mysql_stmt_field_count(mHandle);

            // setup result bindings
            mResultBindings = new MYSQL_BIND[mFieldCount];
            memset(mResultBindings, 0, sizeof(MYSQL_BIND) * mFieldCount);

            for(unsigned int i = 0; i < mFieldCount; ++i)
            {
               mResultBindings[i].length = &mResultBindings[i].buffer_length;
            }

            // set result bindings
            if(mysql_stmt_bind_result(mHandle, mResultBindings) != 0)
            {
               // statement exception
               ExceptionRef e = createException();
               Exception::set(e);
               rval = false;
               delete [] mResultBindings;
               mResultBindings = NULL;
            }
         }
         else
         {
            // clear result bindings
            memset(mResultBindings, 0, sizeof(MYSQL_BIND) * mFieldCount);
         }

         // pull results from server (necessary to clear pipeline for next call)
         if(mysql_stmt_store_result(mHandle) != 0)
         {
            // statement exception
            ExceptionRef e = createException();
            Exception::set(e);
            rval = false;
         }
      }

      if(rval)
      {
         // statement has now been executed at least once
         mExecuted = true;
      }
   }

   return rval;
}

Row* MySqlStatement::fetch()
{
   Row* rval = NULL;

   if(mResult != NULL)
   {
      // fetch the next row
      int rc = mysql_stmt_fetch(mHandle);
      if(rc == 1)
      {
         // exception occurred
         ExceptionRef e = createException();
         Exception::set(e);
      }
      else if(rc != MYSQL_NO_DATA)
      {
         if(mRow == NULL)
         {
            // create row as necessary
            mRow = new MySqlRow(this);

            // set fields for row
            mRow->setFields(
               mysql_fetch_fields(mResult), mFieldCount, mResultBindings);
         }

         rval = mRow;
      }
   }

   return rval;
}

bool MySqlStatement::reset()
{
   // nothing to reset
   return true;
}

bool MySqlStatement::getRowsChanged(uint64_t& rows)
{
   // FIXME: handle exceptions
   rows = mysql_stmt_affected_rows(mHandle);
   return true;
}

uint64_t MySqlStatement::getLastInsertRowId()
{
   return mysql_stmt_insert_id(mHandle);
}

Exception* MySqlStatement::createException()
{
   Exception* e = new Exception(
      mysql_stmt_error(mHandle),
      "monarch.sql.mysql.MySql");
   e->getDetails()["mysqlErrorCode"] = mysql_stmt_errno(mHandle);
   e->getDetails()["mysqlSqlState"] = mysql_stmt_sqlstate(mHandle);
   return e;
}

bool MySqlStatement::checkParamCount(unsigned int param)
{
   bool rval = true;

   if(param > mParamCount)
   {
      // exception, no parameter with given index
      ExceptionRef e = new Exception(
         "Invalid statement parameter index.",
         "monarch.sql.mysql.MySql");
      e->getDetails()["index"] = param;
      Exception::set(e);
      rval = false;
   }

   return rval;
}
