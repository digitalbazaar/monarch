/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_json_JsonLd_H
#define monarch_data_json_JsonLd_H

#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace data
{
namespace json
{

/**
 * The JsonLd class provides APIs for working with JSON-LD objects.
 *
 * @author Dave Longley
 */
class JsonLd
{
protected:
   /**
    * Creates a new JsonLd.
    */
   JsonLd();

public:
   /**
    * Destructs this JsonLd.
    */
   virtual ~JsonLd();

   /**
    * Normalizes a JSON-LD object.
    *
    * @param in the JSON-LD object to normalize.
    * @param out to be set to the normalized JSON-LD object.
    *
    * @return true on success, false on failure with exception set.
    */
   static bool normalize(
      monarch::rt::DynamicObject& in, monarch::rt::DynamicObject& out);

   /**
    * Removes the context from a JSON-LD object.
    *
    * @param in the JSON-LD object to remove the context from.
    * @param out to be set to the context-neutral JSON-LD object.
    *
    * @return true on success, false on failure with exception set.
    */
   static bool removeContext(
      monarch::rt::DynamicObject& in, monarch::rt::DynamicObject& out);

   /**
    * Adds the given context to the given context-neutral JSON-LD object.
    *
    * @param context the new context to use.
    * @param in the context-neutral JSON-LD object to add the context to.
    * @param out to be set to the JSON-LD object with the new context.
    */
   static bool addContext(
      monarch::rt::DynamicObject& context,
      monarch::rt::DynamicObject& in, monarch::rt::DynamicObject& out);

   /**
    * Changes the context of JSON-LD object "in" to "context", placing the
    * output in "out".
    *
    * @param context the new context to use.
    * @param in the input JSON-LD object.
    * @param out the output JSON-LD object.
    *
    * @return true on success, false on failure with exception set.
    */
   static bool changeContext(
      monarch::rt::DynamicObject& context,
      monarch::rt::DynamicObject& in, monarch::rt::DynamicObject& out);
};

} // end namespace json
} // end namespace data
} // end namespace monarch
#endif
