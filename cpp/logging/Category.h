/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_logging_Category_H
#define monarch_logging_Category_H

#include <map>
#include <utility>
#include <list>

#include "monarch/util/Macros.h"

// Forward declaration
namespace monarch
{
   namespace logging
   {
      class Category;
   }
}

extern monarch::logging::Category* MO_DEFAULT_CAT;
extern monarch::logging::Category* MO_ALL_CAT;

namespace monarch
{
namespace logging
{

/**
 * A logging category.
 *
 * @author David I. Lehn
 */
class Category
{
protected:
   /**
    * The id string of the logger.  See Category() for a full description.
    */
   char* mId;

   /**
    * The human readable name of the id of the logger.  See Category() for
    * a full description.
    */
   char* mName;

   /**
    * A description of this category.  Used for help messages.
    */
   char* mDescription;

   /**
    * ANSI escape codes for this category.  Often used to colorize the id
    * string in logging messages.
    */
   char* mAnsiEscapeCodes;

public:
   /**
    * Create a new Category.
    *
    * @param id a string identifier for this category.  The id should be in the
    *        form DOMAIN_CLASS[_SUBCLASS...].  DOMAIN is a major project id
    *        such as "DB" for Digital Bazaar Core or "BM" for Bitmunk.  CLASS
    *        is a sub-system such as MAIL, NET, RT, etc.  Break down the class
    *        further into sub-classes if needed.  There is no parenting of
    *        categories (yet).  May be NULL.
    * @param name a more human readable name for this logger compared to the
    *        id.  Will show up as "<?>" if set to NULL.
    * @param description a description of this category.  Used for help
    *        messages.  May be NULL.
    */
   Category(const char* id, const char* name, const char* description);

   /**
    * Destructs the Category.
    */
   virtual ~Category();

   /**
    * Initializes the static categories. This static method is called by
    * Logging::initialize() which MUST be called during application start-up.
    */
   static void initialize();

   /**
    * Frees static categories. This static method is called from
    * Logging::cleanup() and MUST be called during application tear-down.
    */
   static void cleanup();

   /**
    * Set the category short name.
    *
    * @param name the category short name.
    */
   virtual void setId(const char* id);

   /**
    * Gets the category short name.
    *
    * @return the category short name or NULL if not set.
    */
   virtual const char* getId();

   /**
    * Set the category name.
    *
    * @param name the category name.
    */
   virtual void setName(const char* name);

   /**
    * Gets the category name.
    *
    * @return the category name or "<?>" if not set.
    */
   virtual const char* getName();

   /**
    * Set the category description.
    *
    * @param name the category description.
    */
   virtual void setDescription(const char* description);

   /**
    * Gets the category description.
    *
    * @return the category description or NULL if not set.
    */
   virtual const char* getDescription();

   /**
    * Set the logging ANSI escape codes.
    *
    * @param name the logging ANSI escape codes.
    */
   virtual void setAnsiEscapeCodes(const char* ansi);

   /**
    * Gets the logging ANSI escape codes.
    *
    * @return the logging ANSI escape codes or an empty string.
    */
   virtual const char* getAnsiEscapeCodes();
};

} // end namespace logging
} // end namespace monarch
#endif
