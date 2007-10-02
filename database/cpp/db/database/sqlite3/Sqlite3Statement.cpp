/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/sqlite3/Sqlite3Statement.h"
#include "db/database/sqlite3/Sqlite3Connection.h"
#include "db/database/sqlite3/Sqlite3Row.h"

using namespace std;
using namespace db::database;
using namespace db::database::sqlite3;
using namespace db::rt;

Sqlite3Statement::Sqlite3Statement(Sqlite3Connection *c, const char* sql) :
   Statement(c, sql)
{
   // FIXME: switch to sqlite3_prepare_v2 when appropriate
   const char* tail;
   mState = sqlite3_prepare(c->mHandle, sql, -1, &mHandle, &tail);
   if(mState != SQLITE_OK)
   {
      // exception
      Exception::setLast(new Sqlite3Exception((Sqlite3Connection*)mConnection));
   }
   
   // no current row yet
   mRow = NULL;
}

Sqlite3Statement::~Sqlite3Statement()
{
   // clean up row, if any
   if(mRow != NULL)
   {
      delete mRow;
   }
   
   // clean up C statement
   sqlite3_finalize(mHandle);
}

DatabaseException* Sqlite3Statement::setInt32(unsigned int param, int value)
{
   DatabaseException* rval = NULL;
   
   mState = sqlite3_bind_int(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      rval = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(rval);
   }
   
   return rval;
}

DatabaseException* Sqlite3Statement::setInt64(
   unsigned int param, long long value)
{
   DatabaseException* rval = NULL;
   
   mState = sqlite3_bind_int64(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      rval = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(rval);
   }
   
   return rval;
}

DatabaseException* Sqlite3Statement::setText(
   unsigned int param, const char* value)
{
   DatabaseException* rval = NULL;
   
   // use SQLITE_STATIC to ensure the memory is not cleaned up by sqlite
   mState = sqlite3_bind_text(mHandle, param, value, -1, SQLITE_STATIC);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      rval = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(rval);
   }
   
   return rval;
}

DatabaseException* Sqlite3Statement::setInt32(const char* name, int value)
{
   DatabaseException* rval = NULL;
   
   int index = sqlite3_bind_parameter_index(mHandle, name);
   if(index == 0)
   {
      // exception, no parameter with given name found
      string msg;
      msg.append("Invalid parameter name!,name='");
      msg.append(name);
      msg.append(1, '\'');
      rval = new DatabaseException(msg.c_str());
      Exception::setLast(rval);
   }
   else
   {
      rval = setInt32(index, value);
   }
   
   return rval;
}

DatabaseException* Sqlite3Statement::setInt64(const char* name, long long value)
{
   DatabaseException* rval = NULL;
   
   int index = sqlite3_bind_parameter_index(mHandle, name);
   if(index == 0)
   {
      // exception, no parameter with given name found
      string msg;
      msg.append("Invalid parameter name!,name='");
      msg.append(name);
      msg.append(1, '\'');
      rval = new DatabaseException(msg.c_str());
      Exception::setLast(rval);
   }
   else
   {
      rval = setInt64(index, value);
   }
   
   return rval;
}

DatabaseException* Sqlite3Statement::setText(
   const char* name, const char* value)
{
   DatabaseException* rval = NULL;
   
   int index = sqlite3_bind_parameter_index(mHandle, name);
   if(index == 0)
   {
      // exception, no parameter with given name found
      string msg;
      msg.append("Invalid parameter name!,name='");
      msg.append(name);
      msg.append(1, '\'');
      rval = new DatabaseException(msg.c_str());
      Exception::setLast(rval);
   }
   else
   {
      rval = setText(index, value);
   }
   
   return rval;
}

DatabaseException* Sqlite3Statement::execute()
{
   DatabaseException* rval = NULL;
   
   if(mState == SQLITE_OK)
   {
      // step to execute statement
      mState = sqlite3_step(mHandle);
      
      // ensure state is set to SQLITE_OK for first row
      if(mState == SQLITE_ROW)
      {
         mState = SQLITE_OK;
      }
   }
   else
   {
      rval = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(rval);
   }
   
   return rval;
}

Row* Sqlite3Statement::fetch()
{
   Row* rval = NULL;
   
   if(mState == SQLITE_ROW)
   {
      mState = sqlite3_step(mHandle);
      switch(mState)
      {
         case SQLITE_ROW:
            if(mRow == NULL)
            {
               // create row as necessary
               mRow = new Sqlite3Row(this);
            }
            rval = mRow;
            break;
         case SQLITE_DONE:
            // no more rows
            break;
         default:
            // set exception
            Exception::setLast(
               new Sqlite3Exception((Sqlite3Connection*)mConnection));
            break;
      }
   }
   else if(mState == SQLITE_OK)
   {
      if(mRow == NULL)
      {
         // create row as necessary
         mRow = new Sqlite3Row(this);
      }
      
      // return row and update state for next row
      rval = mRow;
      mState = SQLITE_ROW;
   }
   
   return rval;
}

DatabaseException* Sqlite3Statement::getRowsChanged(unsigned long long& rows)
{
   // FIXME: handle exceptions
   rows = sqlite3_changes(((Sqlite3Connection*)mConnection)->mHandle);
   return NULL;
}

unsigned long long Sqlite3Statement::getLastInsertRowId()
{
   unsigned long long rval = 0;
   
   rval = sqlite3_last_insert_rowid(((Sqlite3Connection*)mConnection)->mHandle);
   
   return rval;
}
