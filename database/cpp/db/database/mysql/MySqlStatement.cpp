/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/mysql/MySqlStatement.h"
#include "db/database/mysql/MySqlConnection.h"
#include "db/database/mysql/MySqlRow.h"

using namespace std;
using namespace db::database;
using namespace db::database::mysql;
using namespace db::rt;

MySqlStatement::MySqlStatement(MySqlConnection *c, const char* sql) :
   Statement(c, sql)
{
   // initialize handle
   mHandle = mysql_stmt_init(NULL);
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
         // get result meta-data
         mResult = mysql_stmt_result_metadata(mHandle);
         
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
   
   // clean up bindings
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
               delete (unsigned long*)bind[i].length;
               break;
            default:
               // nothing to clean up
               break;
         }
      }
      
      // clean up all bindings
      delete [] mParamBindings;
   }
   
   // clean up row, if any
   if(mRow != NULL)
   {
      delete mRow;
   }
   
   // clean up C statement
   mysql_stmt_close(mHandle);
}

DatabaseException* MySqlStatement::setInt32(unsigned int param, int value)
{
   DatabaseException* rval = NULL;
   
   if(param > mParamCount)
   {
      // exception, no parameter with given index
      char temp[100];
      sprintf(temp, "Invalid parameter index!,index='%i'", param);
      rval = new DatabaseException(temp);
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

DatabaseException* MySqlStatement::setInt64(unsigned int param, long long value)
{
   DatabaseException* rval = NULL;
   
   if(param > mParamCount)
   {
      // exception, no parameter with given index
      char temp[100];
      sprintf(temp, "Invalid parameter index!,index='%i'", param);
      rval = new DatabaseException(temp);
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

DatabaseException* MySqlStatement::setText(
   unsigned int param, const char* value)
{
   DatabaseException* rval = NULL;
   
   if(param > mParamCount)
   {
      // exception, no parameter with given index
      char temp[100];
      sprintf(temp, "Invalid parameter index!,index='%i'", param);
      rval = new DatabaseException(temp);
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

DatabaseException* MySqlStatement::setInt32(const char* name, int value)
{
   DatabaseException* rval = NULL;
   
//   // FIXME: might strip support for parameter names
//   int index = mysql_bind_parameter_index(mHandle, name);
//   if(index == 0)
//   {
//      // exception, no parameter with given name found
//      string msg;
//      msg.append("Invalid parameter name!,name='");
//      msg.append(name);
//      msg.append(1, '\'');
//      rval = new DatabaseException(msg.c_str());
//      Exception::setLast(rval);
//   }
//   else
//   {
//      rval = setInt32(index, value);
//   }
   
   return rval;
}

DatabaseException* MySqlStatement::setInt64(const char* name, long long value)
{
   DatabaseException* rval = NULL;
   
//   int index = mysql_bind_parameter_index(mHandle, name);
//   if(index == 0)
//   {
//      // exception, no parameter with given name found
//      string msg;
//      msg.append("Invalid parameter name!,name='");
//      msg.append(name);
//      msg.append(1, '\'');
//      rval = new DatabaseException(msg.c_str());
//      Exception::setLast(rval);
//   }
//   else
//   {
//      rval = setInt64(index, value);
//   }
   
   return rval;
}

DatabaseException* MySqlStatement::setText(const char* name, const char* value)
{
   DatabaseException* rval = NULL;
   
//   int index = mysql_bind_parameter_index(mHandle, name);
//   if(index == 0)
//   {
//      // exception, no parameter with given name found
//      string msg;
//      msg.append("Invalid parameter name!,name='");
//      msg.append(name);
//      msg.append(1, '\'');
//      rval = new DatabaseException(msg.c_str());
//      Exception::setLast(rval);
//   }
//   else
//   {
//      rval = setText(index, value);
//   }
   
   return rval;
}

DatabaseException* MySqlStatement::execute()
{
   DatabaseException* rval = NULL;
   
   if(mysql_stmt_execute(mHandle) != 0)
   {
      // statement exception
      rval = new MySqlException(this);
      Exception::setLast(rval);
   }
   
   return rval;
}

Row* MySqlStatement::fetch()
{
   Row* rval = NULL;
   
   if(mResult != NULL)
   {
      // fetch the next row
      MYSQL_ROW row = mysql_fetch_row(mResult);
      if(row != NULL)
      {
         if(mRow == NULL)
         {
            // create row as necessary
            mRow = new MySqlRow(this);
            rval = mRow;
         }
         
         // set row data
         mRow->setData(
            row, mysql_num_fields(mResult), mysql_fetch_lengths(mResult),
            mysql_fetch_fields(mResult));
      }
      else if(mysql_errno(((MySqlConnection*)mConnection)->mHandle) != 0)
      {
         // set exception
         Exception::setLast(
            new MySqlException((MySqlConnection*)mConnection));
      }
   }
   
   return rval;
}

DatabaseException* MySqlStatement::getRowsChanged(unsigned long long& rows)
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
