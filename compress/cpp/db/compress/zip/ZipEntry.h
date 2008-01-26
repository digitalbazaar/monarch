/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_zip_ZipEntry_H
#define db_compress_zip_ZipEntry_H

namespace db
{
namespace compress
{
namespace zip
{

/**
 * A ZipEntry contains information about a single file in a ZIP archive.
 * 
 * @author Dave Longley
 */
class ZipEntry
{
protected:
   // FIXME:
   
public:
   /**
    * Creates a new ZipEntry.
    */
   ZipEntry();
   
   /**
    * Destructs this ZipEntry.
    */
   virtual ~ZipEntry();
};

} // end namespace zip
} // end namespace compress
} // end namespace db
#endif
