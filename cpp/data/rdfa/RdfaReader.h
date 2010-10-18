/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_rdfa_RdfaReader_H
#define monarch_data_rdfa_RdfaReader_H

#include "monarch/data/DynamicObjectReader.h"
#include "monarch/util/StringTools.h"

#include <rdfa.h>
#include <map>
#include <vector>

namespace monarch
{
namespace data
{
namespace rdfa
{

/**
 * An RdfaReader provides an interface for deserializing objects from
 * RDFa and into a JSON-LD-formatted DynamicObject.
 *
 * @author Dave Longley
 */
class RdfaReader : public DynamicObjectReader
{
protected:
   /**
    * True if this parser has started, false if not.
    */
   bool mStarted;

   /**
    * The base URI to be used.
    */
   char* mBaseUri;

   /**
    * The current rdfacontext.
    */
   rdfacontext* mRdfaCtx;

   /**
    * The user-set JSON-LD graph context.
    */
   monarch::rt::DynamicObject mContext;

   /**
    * The auto-generated (from parsed rdfa) JSON-LD graph context.
    */
   monarch::rt::DynamicObject mAutoContext;

   /**
    * A list of parsed triples.
    */
   typedef std::vector<rdftriple*> TripleList;
   TripleList mTriples;

   /**
    * A map of subject name to graph-reference count.
    */
   typedef std::map<const char*, int, monarch::util::StringComparator>
      SubjectCountMap;
   SubjectCountMap mSubjectCounts;

   /**
    * The final target DynamicObject set from start().
    */
   monarch::rt::DynamicObject mTarget;

public:
   /**
    * Creates a new RdfaReader.
    */
   RdfaReader();

   /**
    * Destructs this RdfaReader.
    */
   virtual ~RdfaReader();

   /**
    * Sets the base URI to use. Relative uris that are encountered will be
    * appended to this uri.
    *
    * @param uri the baseUri to use.
    */
   virtual void setBaseUri(const char* uri);

   /**
    * Sets the context to use in the JSON-LD object output.
    *
    * @param context the context to use.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool setContext(monarch::rt::DynamicObject& context);

   /**
    * Starts deserializing an object from RDFa. This RdfaReader can be re-used
    * by calling start() with the same or a new object. Calling start() before
    * a previous deserialization has finished will abort the previous state.
    *
    * Using a non-empty target object can be used to merge in new values. This
    * is only defined for similar object types (ie, merging an array into a map
    * will overwrite the map).
    *
    * @param dyno the DynamicObject for the object to deserialize.
    *
    * @return true on success, false on failure.
    */
   virtual bool start(monarch::rt::DynamicObject& dyno);

   /**
    * This method reads RDFa from the passed InputStream until the end of
    * the stream, blocking if necessary.
    *
    * The start() method must be called at least once before calling read(). As
    * the RDFa is read, the DynamicObject provided in start() is used to
    * deserialize an object.
    *
    * This method may be called multiple times if the input stream needs to
    * be populated in between calls or if multiple input streams are used.
    *
    * The object is built incrementally and on error will be partially built.
    *
    * finish() should be called after the read is complete in order to check
    * that a top level object is complete.
    *
    * @param is the InputStream to read the RDFa from.
    *
    * @return true if the read succeeded, false if an Exception occurred.
    */
   virtual bool read(monarch::io::InputStream* is);

   /**
    * Finishes deserializing an object from RDFa. This method should be called
    * to complete deserialization and verify valid RDFa was found.
    *
    * @return true if the finish succeeded, false if an Exception occurred.
    */
   virtual bool finish();

   /**
    * Reads a DynamicObject as RDFa from a string.
    *
    * @param dyno the DynamicObject to fill.
    * @param s the string to read from.
    * @param slen the length of s.
    * @param uri the baseUri to use.
    *
    * @return true on success, false and exception set on failure.
    */
   static bool readFromString(
      monarch::rt::DynamicObject& dyno, const char* s, size_t slen,
      const char* baseUri);

protected:
   /**
    * Processes a triple found by the rdfa parser.
    *
    * @param triple the triple to handle.
    */
   virtual void processTriple(rdftriple* triple);

   /**
    * Called by the rdfa parser when a triple is encountered.
    *
    * @param triple the triple to handle.
    * @param reader the RdfaReader instance.
    */
   static void callbackProcessTriple(rdftriple* triple, void* reader);
};

} // end namespace rdfa
} // end namespace data
} // end namespace monarch
#endif
