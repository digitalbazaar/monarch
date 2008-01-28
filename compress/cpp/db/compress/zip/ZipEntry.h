/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_zip_ZipEntry_H
#define db_compress_zip_ZipEntry_H

#include "db/rt/Collectable.h"

namespace db
{
namespace compress
{
namespace zip
{

/**
 * A ZipEntryImpl provides the implementation for a reference-counted ZipEntry.
 * 
 * A ZipEntry is typedef'ed below.
 * 
 * @author Dave Longley
 */
class ZipEntryImpl
{
protected:
   // FIXME:
   
public:
   /**
    * Creates a new ZipEntryImpl.
    */
   ZipEntryImpl();
   
   /**
    * Destructs this ZipEntryImpl.
    */
   virtual ~ZipEntryImpl();
};

/**
 * A ZipEntry contains information about a single file in a ZIP archive.
 */
typedef db::rt::Collectable<ZipEntryImpl> ZipEntry;

} // end namespace zip
} // end namespace compress
} // end namespace db
#endif
