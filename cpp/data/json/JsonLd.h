/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
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
    * Normalizes a JSON-LD object by removing its context.
    *
    * @param in the JSON-LD object to normalize.
    * @param out to be set to the normalized JSON-LD object.
    *
    * @return true on success, false on failure with exception set.
    */
   static bool normalize(
      monarch::rt::DynamicObject& in, monarch::rt::DynamicObject& out);

   /**
    * Denormalizes the given normalized JSON-LD object according to the given
    * context.
    *
    * @param context the new context to use.
    * @param in the JSON-LD object to denormalize.
    * @param out to be set to the denormalized JSON-LD object.
    */
   static bool denormalize(
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

   /**
    * Filter the JSON-LD formatted "in" to the JSON-LD formatted "out" by
    * filtering all results for those subjects that match properties in
    * "filter". The result is denormalized using the given "context".
    *
    * Currently only uses the first level of properties in the filter.  The
    * properties can have multiple values but deeper checking is not performed.
    *
    * If the simplify flag is true, filter() will attempt to flatten the result
    * to one top level object if only one result was found. If false it will
    * always leave the results in an array similar to {"@":[R0,R1,...]}.
    *
    * @param context the new context to use.
    * @param filter the filter to use.
    * @param in the input JSON-LD object.
    * @param out the output JSON-LD object.
    * @param simplify simplify the result object if possible.
    *
    * @return true on success, false on failure with exception set.
    */
   static bool filter(
      monarch::rt::DynamicObject& context,
      monarch::rt::DynamicObject& filter,
      monarch::rt::DynamicObject& in, monarch::rt::DynamicObject& out,
      bool simplify = false);
};

} // end namespace json
} // end namespace data
} // end namespace monarch
#endif
