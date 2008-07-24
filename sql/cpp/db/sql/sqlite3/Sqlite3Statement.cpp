/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3Statement.h"

#include "db/sql/sqlite3/Sqlite3Connection.h"
#include "db/sql/sqlite3/Sqlite3Row.h"

using namespace std;
using namespace db::sql;
using namespace db::sql::sqlite3;
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
      ExceptionRef e = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(e, false);
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
   
   // clean up handle
   sqlite3_finalize(mHandle);
}

int Sqlite3Statement::getParameterIndex(const char* name)
{
   int index = sqlite3_bind_parameter_index(mHandle, name);
   
   if(index == 0)
   {
      // exception, no parameter with given name found
      int length = strlen(name) + 40;
      char temp[length];
      snprintf(temp, length, "Invalid parameter name!,name='%s'", name);
      ExceptionRef e = new SqlException(temp);
      Exception::setLast(e, false);
   }
   
   return index;
}

bool Sqlite3Statement::setInt32(unsigned int param, int value)
{
   bool rval = true;
   
   mState = sqlite3_bind_int(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
}

bool Sqlite3Statement::setUInt32(
   unsigned int param, unsigned int value)
{
   bool rval = true;
   
   mState = sqlite3_bind_int(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
}

bool Sqlite3Statement::setInt64(
   unsigned int param, long long value)
{
   bool rval = true;
   
   mState = sqlite3_bind_int64(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
}

bool Sqlite3Statement::setUInt64(
   unsigned int param, unsigned long long value)
{
   bool rval = true;
   
   mState = sqlite3_bind_int64(mHandle, param, value);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
}

bool Sqlite3Statement::setText(
   unsigned int param, const char* value)
{
   bool rval = true;
   
   // use SQLITE_STATIC to ensure the memory is not cleaned up by sqlite
   mState = sqlite3_bind_text(mHandle, param, value, -1, SQLITE_STATIC);
   if(mState != SQLITE_OK)
   {
      // exception, could not bind parameter
      ExceptionRef e = new Sqlite3Exception((Sqlite3Connection*)mConnection);
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
}

bool Sqlite3Statement::setInt32(const char* name, int value)
{
   bool rval = true;
   
   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setInt32(index, value);
   }
   
   return rval;
}

bool Sqlite3Statement::setUInt32(const char* name, unsigned int value)
{
   bool rval = true;
   
   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setUInt32(index, value);
   }
   
   return rval;
}

bool Sqlite3Statement::setInt64(const char* name, long long value)
{
   bool rval = true;
   
   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setInt64(index, value);
   }
   
   return rval;
}

bool Sqlite3Statement::setUInt64(
   const char* name, unsigned long long value)
{
   bool rval = true;
   
   int index = getParameterIndex(name);
   if(index > 0)
   {
      rval = setUInt64(index, value);
   }
   
   return rval;
}

bool Sqlite3Statement::setText(
   const char* name, const char* value)
{
   bool rval = true;
   
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
         if(mState == SQLITE_DONE)
         {
            // reset statement for future use
            mState = sqlite3_reset(mHandle);
         }
         else if(mState != SQLITE_ROW)
         {
            // error stepping statement (version 1 of api requires reset)
            sqlite3_reset(mHandle);
            ExceptionRef e =
               new Sqlite3Exception((Sqlite3Connection*)mConnection);
            Exception::setLast(e, false);
            rval = false;
         }
         break;
      case SQLITE_DONE:
      case SQLITE_ROW:
         // clean up existing row object
         if(mRow != NULL)
         {
            delete mRow;
            mRow = NULL;
         }
         
         // reset statement and execute again
         mState = sqlite3_reset(mHandle);
         Sqlite3Statement::execute();
         break;
      default:
         {
            // statement in bad state
            ExceptionRef e =
               new Sqlite3Exception((Sqlite3Connection*)mConnection);
            Exception::setLast(e, false);
            rval = false;
         }
         break;
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
            break;
         default:
            {
               // clean up row object
               delete mRow;
               mRow = NULL;
               
               // error stepping statement (version 1 of api requires reset)
               sqlite3_reset(mHandle);
               ExceptionRef e =
                  new Sqlite3Exception((Sqlite3Connection*)mConnection);
               Exception::setLast(e, false);
            }
            break;
      }
   }
   else if(mState == SQLITE_ROW)
   {
      // create and return first row
      rval = mRow = new Sqlite3Row(this);
   }
   
   return rval;
}

bool Sqlite3Statement::getRowsChanged(unsigned long long& rows)
{
   // FIXME: handle exceptions
   rows = sqlite3_changes(((Sqlite3Connection*)mConnection)->mHandle);
   return true;
}

unsigned long long Sqlite3Statement::getLastInsertRowId()
{
   return sqlite3_last_insert_rowid(
      ((Sqlite3Connection*)mConnection)->mHandle);
}
