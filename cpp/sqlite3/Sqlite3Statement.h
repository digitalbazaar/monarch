/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_sqlite3_Statement_H
#define monarch_sql_sqlite3_Statement_H

#include <sqlite3.h>

#include "monarch/sql/Statement.h"
#include "monarch/sql/sqlite3/Sqlite3Row.h"

namespace monarch
{
namespace sql
{
namespace sqlite3
{

class Sqlite3Connection;

/**
 * An sqlite3 database statement.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class Sqlite3Statement : public monarch::sql::Statement
{
protected:
   /**
    * The connection associated with this statement.
    */
   Sqlite3Connection* mConnection;

   /**
    * The C sqlite3 statement structure.
    */
   sqlite3_stmt* mHandle;

   /**
    * The current state for this statement, i.e. whether or not it
    * has been executed/whether or not a result Row is ready.
    */
   int mState;

   /**
    * The current row, if any.
    */
   Sqlite3Row* mRow;

public:
   /**
    * Creates a new Statement.
    *
    * @param sql the SQL for the statement.
    */
   Sqlite3Statement(const char* sql);

   /**
    * Destructs this Statement.
    */
   virtual ~Sqlite3Statement();

   /**
    * {@inheritDoc}
    */
   virtual Connection* getConnection();

   /**
    * Gets the sqlite3 handle for this statement.
    *
    * @return the sqlite3 handle for this statement.
    */
   virtual sqlite3_stmt* getHandle();

   /**
    * Initializes this statement for use.
    *
    * @param c the Sqlite3Connection that prepared this statement.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool initialize(Sqlite3Connection* c);

   /**
    * {@inheritDoc}
    */
   virtual bool setInt32(unsigned int param, int32_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setUInt32(unsigned int param, uint32_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setInt64(unsigned int param, int64_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setUInt64(unsigned int param, uint64_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setText(unsigned int param, const char* value);

   /**
    * {@inheritDoc}
    */
   virtual bool setBlob(unsigned int param, const char* value, int length);

   /**
    * {@inheritDoc}
    */
   virtual bool setInt32(const char* name, int32_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setUInt32(const char* name, uint32_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setInt64(const char* name, int64_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setUInt64(const char* name, uint64_t value);

   /**
    * {@inheritDoc}
    */
   virtual bool setText(const char* name, const char* value);

   /**
    * {@inheritDoc}
    */
   virtual bool setBlob(const char* name, const char* value, int length);

   /**
    * {@inheritDoc}
    */
   virtual bool execute();

   /**
    * {@inheritDoc}
    */
   virtual Row* fetch();

   /**
    * {@inheritDoc}
    */
   virtual bool reset();

   /**
    * {@inheritDoc}
    */
   virtual bool getRowsChanged(uint64_t& rows);

   /**
    * {@inheritDoc}
    */
   virtual uint64_t getLastInsertRowId();

protected:
   /**
    * Gets the parameter index for the given name.
    *
    * @param name the name to get the parameter index for.
    *
    * @return the index >= 1 or 0 if an Exception occurred.
    */
   virtual int getParameterIndex(const char* name);
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace monarch
#endif
