/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_json_JsonLd_H
#define monarch_data_json_JsonLd_H

#include "monarch/rt/DynamicObject.h"

#include <string>

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
      monarch::rt::DynamicObject in, monarch::rt::DynamicObject& out);

   /**
    * Removes the context from a JSON-LD object, expanding it to full-form.
    *
    * @param in the JSON-LD object to remove the context from.
    * @param out to be set to the context-neutral JSON-LD object.
    *
    * @return true on success, false on failure with exception set.
    */
   static bool expand(
      monarch::rt::DynamicObject in, monarch::rt::DynamicObject& out);

   /**
    * Expands the given input and then adds the given context to compact it.
    *
    * @param context the new context to use.
    * @param in the JSON-LD object to add the context to.
    * @param out to be set to the JSON-LD object with the new context.
    */
   static bool compact(
      monarch::rt::DynamicObject context,
      monarch::rt::DynamicObject in, monarch::rt::DynamicObject& out);

   /**
    * Merges one context with another.
    *
    * @param ctx1 the context to overwrite/append to.
    * @param ctx2 the new context to merge onto ctx1.
    *
    * @return the merged context or NULL on failure with exception set.
    */
   static monarch::rt::DynamicObject mergeContexts(
      monarch::rt::DynamicObject ctx1,
      monarch::rt::DynamicObject ctx2);

   /**
    * Expands a term into an absolute IRI. The term may be a regular term, a
    * CURIE, a relative IRI, or an absolute IRI. In any case, the associated
    * absolute IRI will be returned.
    *
    * @param ctx the context to use.
    * @param term the term to expand.
    *
    * @return the expanded term as an absolute IRI.
    */
   static std::string expandTerm(
      monarch::rt::DynamicObject ctx, const char* term);

   /**
    * Compacts an IRI into a term or CURIE it can be. IRIs will not be
    * compacted to relative IRIs if they match the given context's default
    * vocabulary.
    *
    * @param ctx the context to use.
    * @param iri the IRI to compact.
    *
    * @return the compacted IRI as a term or CURIE or the original IRI.
    */
   static std::string compactIri(
      monarch::rt::DynamicObject ctx, const char* iri);

   /**
    * Frames JSON-LD input.
    *
    * @param in the JSON-LD input.
    * @param frame the frame to use.
    * @param out the output framed JSON-LD object.
    * @param options framing options to use, NULL for defaults.
    *
    * @return true on success, false on failure with exception set.
    */
   static bool frame(
      monarch::rt::DynamicObject in, monarch::rt::DynamicObject frame,
      monarch::rt::DynamicObject& out,
      monarch::rt::DynamicObject* options = NULL);

   /**
    * Check if a JSON-LD object has any values for a property. This method will
    * handle existance of the property as well as properties with single
    * values, an array of values, or an empty array with no values.
    *
    * @param jsonld the JSON-LD.
    * @param property the property to check.
    *
    * @return true if property exists and has values, false if not.
    */
   static bool hasProperty(
      monarch::rt::DynamicObject& jsonld,
      const char* property);

   /**
    * Check if a JSON-LD property has a value. This method will handle
    * existance of the property as well as properties with single or an array
    * of values.
    *
    * @param jsonld the JSON-LD.
    * @param property the property to check.
    * @param value the value to check for.
    *
    * @return true if value exists, false if not.
    */
   static bool hasValue(
      monarch::rt::DynamicObject& jsonld,
      const char* property,
      monarch::rt::DynamicObject& value);

   /**
    * Check if a JSON-LD property has a value by converting value to an object
    * and calling hasValue().
    *
    * @param jsonld the JSON-LD.
    * @param property the property to check.
    * @param value the value to check for.
    *
    * @return true if value exists, false if not.
    */
   static bool hasValue(
      monarch::rt::DynamicObject& jsonld,
      const char* property,
      const char* value);

   /**
    * Add a value to a property in JSON-LD. This method will handle existance
    * of the property as well as properties with single or an array of values.
    * If propertyIsList is false, the value will be added to the property only
    * if hasValue() is false. If propertyIsList is true, the value will always
    * be added.
    *
    * @param jsonld the JSON-LD.
    * @param property the property to add to.
    * @param value the value to add.
    * @param propertyIsList true if property is a list, false if not.
    */
   static void addValue(
      monarch::rt::DynamicObject& jsonld,
      const char* property,
      monarch::rt::DynamicObject& value,
      bool propertyIsList = false);

   /**
    * Add a value to a property in JSON-LD by converting value to an object and
    * calling addValue().
    *
    * @param jsonld the JSON-LD.
    * @param property the property to add to.
    * @param value the value to add.
    * @param propertyIsList true if property is a list, false if not.
    */
   static void addValue(
      monarch::rt::DynamicObject& jsonld,
      const char* property,
      const char* value,
      bool propertyIsList = false);
};

} // end namespace json
} // end namespace data
} // end namespace monarch
#endif
