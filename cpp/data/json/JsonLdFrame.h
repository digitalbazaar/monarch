/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_json_JsonLdFrame_H
#define monarch_data_json_JsonLdFrame_H

#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace data
{
namespace json
{

/**
 * The JsonLdFrame class is used to construct JSON-LD objects from a set
 * of triples using a particular data structure (a frame).
 *
 * @author Dave Longley
 */
class JsonLdFrame
{
protected:
   /**
    * The root frame.
    */
   monarch::rt::DynamicObject mFrame;

   /**
    * Set to true if only those predicates explicitly mentioned in the
    * frame should be included in the result.
    */
   bool mExplicit;

public:
   /**
    * Creates a new JsonLdFrame.
    */
   JsonLdFrame();

   /**
    * Destructs this JsonLdFrame.
    */
   virtual ~JsonLdFrame();

   /**
    * Frames an array of triples. The triples will be automatically sorted
    * (alphabetically starting with subject, then predicate, then object)
    * unless sort is set to false.
    *
    * @param triples the array of triples, where each entry is an array of
    *           size 3 with subject, predicate, object, in that order.
    * @param out the framed output object.
    * @param sort true to sort the triples, false not to.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool frameTriples(
      monarch::rt::DynamicObject& triples,
      monarch::rt::DynamicObject& out,
      bool sort = true);

   /**
    * Frames a map of subjects. The map keys are subjects (IRIs), the map
    * values are objects with "@" set to the subject and all other predicates
    * set to strings (that may be IRIs). There are no deeply-embedded objects.
    *
    * If there are multiple objects for a predicate, they are assumed to be
    * in sorted order.
    *
    * @param subjects the map of subjects to frame.
    * @param out the framed output object.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool frameSubjects(
      monarch::rt::DynamicObject subjects, monarch::rt::DynamicObject& out);
};

} // end namespace json
} // end namespace data
} // end namespace monarch
#endif
