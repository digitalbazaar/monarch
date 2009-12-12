/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_OStreamOutputStream_H
#define db_io_OStreamOutputStream_H

#include "monarch/io/OutputStream.h"

#include <ostream>

namespace db
{
namespace io
{

/**
 * An OStreamOutputStream is used to write bytes with an ostream.
 *
 * @author Dave Longley
 */
class OStreamOutputStream : public OutputStream
{
protected:
   /**
    * The ostream to write with.
    */
   std::ostream* mStream;

public:
   /**
    * Creates a new OStreamOutputStream that writes with the passed ostream.
    *
    * @param stream the ostream to write with.
    */
   OStreamOutputStream(std::ostream* stream);

   /**
    * Destructs this OStreamOutputStream.
    */
   virtual ~OStreamOutputStream();

   /**
    * Writes some bytes to the stream.
    *
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool write(const char* b, int length);
};

} // end namespace io
} // end namespace db
#endif
