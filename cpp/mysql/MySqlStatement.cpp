/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/mysql/MySqlStatement.h"

#include "monarch/sql/mysql/MySqlConnection.h"
#include "monarch/sql/mysql/MySqlRow.h"

using namespace std;
using namespace db::sql;
using namespace db::sql::mysql;
using namespace db::rt;

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

   // clean up param bindings
   if(mParamBindings != NULL)
   {
      // ensure integer buffers and string lengths are cleaned up
      for(unsigned int i = 0; i < mParamCount; i++)
      {
         // clean up integer buffer
         switch(mParamBindings[i].buffer_type)
         {
            case MYSQL_TYPE_LONG:
               delete (int*)mParamBindings[i].buffer;
               break;
            case MYSQL_TYPE_LONGLONG:
               delete (long long*)mParamBindings[i].buffer;
               break;
            case MYSQL_TYPE_BLOB:
               delete (unsigned long*)mParamBindings[i].length;
               break;
            default:
               // nothing to clean up
               break;
         }
      }

      // clean up all bindings
      delete [] mParamBindings;
   }

   // clean up result bindings
   if(mResultBindings != NULL)
   {
      delete [] mResultBindings;
   }

   // clean up row, if any
   if(mRow != NULL)
   {
      delete mRow;
   }

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
      ExceptionRef e = new MySqlException(c);
      Exception::set(e);
      rval = false;
   }
   else
   {
      // prepare statement
      if(mysql_stmt_prepare(mHandle, mSql, strlen(mSql)) != 0)
      {
         // statement exception
         ExceptionRef e = new MySqlException(this);
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
      param--;

      if(mExecuted)
      {
         // clean up previously set parameter
         delete (int*)mParamBindings[param].buffer;
         //memset(mParamBindings, 0, sizeof(mParamBindings));
      }

      // MYSQL_TYPE_LONG should be a 32-bit int INTEGER field,
      // length = 0 for ints
      // since the integer is stack-allocated, we must heap-allocate it here
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONG;
      mParamBindings[param].buffer = (char*)new int(value);
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
      param--;

      if(mExecuted)
      {
         // clean up previously set parameter
         delete (int*)mParamBindings[param].buffer;
         //memset(mParamBindings, 0, sizeof(mParamBindings));
      }

      // MYSQL_TYPE_LONG should be a 32-bit int INTEGER field,
      // length = 0 for ints
      // since the integer is stack-allocated, we must heap-allocate it here
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONG;
      mParamBindings[param].buffer = (char*)new unsigned int(value);
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
      param--;

      if(mExecuted)
      {
         // clean up previously set parameter
         delete (long long*)mParamBindings[param].buffer;
         //memset(mParamBindings, 0, sizeof(mParamBindings));
      }

      // MYSQL_TYPE_LONGLONG should be a 64-bit int BIGINT field,
      // length = 0 for ints
      // since the integer is stack-allocated, we must heap-allocate it here
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONGLONG;
      mParamBindings[param].buffer = (char*)new long long(value);
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
      param--;

      if(mExecuted)
      {
         // clean up previously set parameter
         delete (long long*)mParamBindings[param].buffer;
         //memset(mParamBindings, 0, sizeof(mParamBindings));
      }

      // MYSQL_TYPE_LONGLONG should be a 64-bit int BIGINT field,
      // length = 0 for ints
      // since the integer is stack-allocated, we must heap-allocate it here
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONGLONG;
      mParamBindings[param].buffer = (char*)new unsigned long long(value);
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = 0;
      mParamBindings[param].is_unsigned = 1;
   }

   return rval;
}

bool MySqlStatement::setText(unsigned int param, const char* value)
{
   bool rval = checkParamCount(param);

   if(rval)
   {
      // param is 1 higher than bind index
      param--;

      if(mExecuted)
      {
         // clean up previously set parameter
         delete (unsigned long*)mParamBindings[param].length;
         //memset(mParamBindings, 0, sizeof(mParamBindings));
      }

      // MYSQL_TYPE_BLOB should be a BLOB or TEXT field
      // length is heap-allocated and cleaned up later
      mParamBindings[param].buffer_type = MYSQL_TYPE_BLOB;
      mParamBindings[param].buffer = (char*)value;
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = new unsigned long(strlen(value));
   }

   return rval;
}

bool MySqlStatement::setInt32(const char* name, int32_t value)
{
   ExceptionRef e =
     new SqlException("MySql named parameter support not implemented.");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setUInt32(const char* name, uint32_t value)
{
   ExceptionRef e =
     new SqlException("MySql named parameter support not implemented.");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setInt64(const char* name, int64_t value)
{
   ExceptionRef e =
     new SqlException("MySql named parameter support not implemented.");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setUInt64(const char* name, uint64_t value)
{
   ExceptionRef e =
     new SqlException("MySql named parameter support not implemented.");
   Exception::set(e);
   return false;
}

bool MySqlStatement::setText(const char* name, const char* value)
{
   ExceptionRef e =
     new SqlException("MySql named parameter support not implemented.");
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
      ExceptionRef e = new MySqlException(this);
      Exception::set(e);
   }
   else if(mysql_stmt_execute(mHandle) != 0)
   {
      // statement exception
      ExceptionRef e = new MySqlException(this);
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

            for(unsigned int i = 0; i < mFieldCount; i++)
            {
               mResultBindings[i].length = &mResultBindings[i].buffer_length;
            }

            // set result bindings
            if(mysql_stmt_bind_result(mHandle, mResultBindings) != 0)
            {
               // statement exception
               ExceptionRef e = new MySqlException(this);
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
            ExceptionRef e = new MySqlException(this);
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
         ExceptionRef e = new MySqlException(this);
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

bool MySqlStatement::checkParamCount(unsigned int param)
{
   bool rval = true;

   if(param > mParamCount)
   {
      // exception, no parameter with given index
      ExceptionRef e = new SqlException(
         "Invalid statement parameter index.");
      e->getDetails()["index"] = param;
      Exception::set(e);
      rval = false;
   }

   return rval;
}
