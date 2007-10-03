/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/mysql/MySqlStatement.h"
#include "db/sql/mysql/MySqlConnection.h"
#include "db/sql/mysql/MySqlRow.h"

using namespace std;
using namespace db::sql;
using namespace db::sql::mysql;
using namespace db::rt;

MySqlStatement::MySqlStatement(MySqlConnection *c, const char* sql) :
   Statement(c, sql)
{
   // initialize handle
   mHandle = mysql_stmt_init(c->mHandle);
   if(mHandle == NULL)
   {
      // connection exception
      Exception::setLast(new MySqlException((MySqlConnection*)mConnection));
   }
   else
   {
      // prepare statement
      if(mysql_stmt_prepare(mHandle, sql, strlen(sql)) != 0)
      {
         // statement exception
         Exception::setLast(new MySqlException(this));
      }
      else
      {
         // determine number of parameters, initialize bindings
         mParamCount = mysql_stmt_param_count(mHandle);
         if(mParamCount > 0)
         {
            mParamBindings = new MYSQL_BIND[mParamCount];
         }
         else
         {
            mParamBindings = NULL;
         }
      }
   }
   
   // no result bindings yet
   mFieldCount = 0;
   mResultBindings = NULL;
   
   // no current row yet
   mRow = NULL;
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
      MYSQL_BIND* bind = mParamBindings;
      
      // ensure integer buffers and string lengths are cleaned up
      for(unsigned int i = 0; i < mParamCount; i++)
      {
         // clean up integer buffer
         switch(bind[i].buffer_type)
         {
            case MYSQL_TYPE_LONG:
               delete (int*)bind[i].buffer;
               break;
            case MYSQL_TYPE_LONGLONG:
               delete (long long*)bind[i].buffer;
               break;
            case MYSQL_TYPE_BLOB:
               delete (unsigned int*)bind[i].length;
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
   
   // clean up C statement
   mysql_stmt_close(mHandle);
}

SqlException* MySqlStatement::setInt32(unsigned int param, int value)
{
   SqlException* rval = NULL;
   
   if(param > mParamCount)
   {
      // exception, no parameter with given index
      char temp[100];
      sprintf(temp, "Invalid parameter index!,index='%i'", param);
      rval = new SqlException(temp);
      Exception::setLast(rval);
   }
   else
   {
      // param is 1 higher than bind index
      param--;
      
      // MYSQL_TYPE_LONG should be a 32-bit int INTEGER field,
      // length = 0 for ints
      // since the integer is stack-allocated, we must heap-allocate it here
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONG;
      mParamBindings[param].buffer = (char*)new int(value);
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = 0;
   }
   
   return rval;
}

SqlException* MySqlStatement::setInt64(unsigned int param, long long value)
{
   SqlException* rval = NULL;
   
   if(param > mParamCount)
   {
      // exception, no parameter with given index
      char temp[100];
      sprintf(temp, "Invalid parameter index!,index='%i'", param);
      rval = new SqlException(temp);
      Exception::setLast(rval);
   }
   else
   {
      // param is 1 higher than bind index
      param--;
      
      // MYSQL_TYPE_LONGLONG should be a 64-bit int BIGINT field,
      // length = 0 for ints
      // since the integer is stack-allocated, we must heap-allocate it here
      mParamBindings[param].buffer_type = MYSQL_TYPE_LONGLONG;
      mParamBindings[param].buffer = (char*)new long long(value);
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = 0;   
   }
   
   return rval;
}

SqlException* MySqlStatement::setText(
   unsigned int param, const char* value)
{
   SqlException* rval = NULL;
   
   if(param > mParamCount)
   {
      // exception, no parameter with given index
      char temp[100];
      sprintf(temp, "Invalid parameter index!,index='%i'", param);
      rval = new SqlException(temp);
      Exception::setLast(rval);
   }
   else
   {
      // param is 1 higher than bind index
      param--;
      
      // MYSQL_TYPE_BLOB should be a BLOB or TEXT field
      // length is heap-allocated and cleaned up later
      mParamBindings[param].buffer_type = MYSQL_TYPE_BLOB;
      mParamBindings[param].buffer = (char*)value;
      mParamBindings[param].is_null = 0;
      mParamBindings[param].length = new unsigned long(strlen(value));
   }
   
   return rval;
}

SqlException* MySqlStatement::setInt32(const char* name, int value)
{
   SqlException* rval = 
     new SqlException("MySql named parameter support not implemented!");
   
//   // FIXME: might strip support for parameter names
//   int index = mysql_bind_parameter_index(mHandle, name);
//   if(index == 0)
//   {
//      // exception, no parameter with given name found
//      string msg;
//      msg.append("Invalid parameter name!,name='");
//      msg.append(name);
//      msg.append(1, '\'');
//      rval = new SqlException(msg.c_str());
//      Exception::setLast(rval);
//   }
//   else
//   {
//      rval = setInt32(index, value);
//   }
   
   return rval;
}

SqlException* MySqlStatement::setInt64(const char* name, long long value)
{
   SqlException* rval = 
     new SqlException("MySql named parameter support not implemented!");
   
//   int index = mysql_bind_parameter_index(mHandle, name);
//   if(index == 0)
//   {
//      // exception, no parameter with given name found
//      string msg;
//      msg.append("Invalid parameter name!,name='");
//      msg.append(name);
//      msg.append(1, '\'');
//      rval = new SqlException(msg.c_str());
//      Exception::setLast(rval);
//   }
//   else
//   {
//      rval = setInt64(index, value);
//   }
   
   return rval;
}

SqlException* MySqlStatement::setText(const char* name, const char* value)
{
   SqlException* rval = 
     new SqlException("MySql named parameter support not implemented!");
   
//   int index = mysql_bind_parameter_index(mHandle, name);
//   if(index == 0)
//   {
//      // exception, no parameter with given name found
//      string msg;
//      msg.append("Invalid parameter name!,name='");
//      msg.append(name);
//      msg.append(1, '\'');
//      rval = new SqlException(msg.c_str());
//      Exception::setLast(rval);
//   }
//   else
//   {
//      rval = setText(index, value);
//   }
   
   return rval;
}

SqlException* MySqlStatement::execute()
{
   SqlException* rval = NULL;
   
   // bind parameters
   if(mysql_stmt_bind_param(mHandle, mParamBindings) != 0)
   {
      // statement exception
      rval = new MySqlException(this);
      Exception::setLast(rval);
   }
   else if(mysql_stmt_execute(mHandle) != 0)
   {
      // statement exception
      rval = new MySqlException(this);
      Exception::setLast(rval);
   }
   else
   {
      // get result meta-data
      mResult = mysql_stmt_result_metadata(mHandle);
      if(mResult != NULL)
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
            rval = new MySqlException(this);
            Exception::setLast(rval);
         }
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
         Exception::setLast(new MySqlException(this));
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

SqlException* MySqlStatement::getRowsChanged(unsigned long long& rows)
{
   // FIXME: handle exceptions
   rows = mysql_stmt_affected_rows(mHandle);
   return NULL;
}

unsigned long long MySqlStatement::getLastInsertRowId()
{
   unsigned long long rval = 0;
   
   rval = mysql_stmt_insert_id(mHandle);
   
   return rval;
}
