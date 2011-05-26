/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonLdFrame.h"

#include "monarch/data/json/JsonLd.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/util/StringTools.h"

#include <cstdio>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::rt;
using namespace monarch::util;

#define RDF_TYPE   "http://www.w3.org/1999/02/22-rdf-syntax-ns#type"

struct SubjectRef
{
   DynamicObject subject;
   const char* property;
   int count;
};
typedef std::map<const char*, SubjectRef, StringComparator> SubjectRefMap;

JsonLdFrame::JsonLdFrame() :
   mFrame(NULL),
   mExplicit(false)
{
}

JsonLdFrame::~JsonLdFrame()
{
}

/**
 * Returns true if the first triple is less than the second. This function is
 * used to sort triples alphabetically, first by subject, then property,
 * then object.
 *
 * @param t1 the first triple.
 * @param t2 the second triple.
 *
 * @return true if t1 < t2.
 */
static bool _sortTriples(DynamicObject t1, DynamicObject t2)
{
   bool rval = false;

   // compare subjects
   int c = strcmp(t1[0], t2[0]);
   if(c < 0)
   {
      // t1 subject < t2 subject
      rval = true;
   }
   else if(c == 0)
   {
      // subjects equal, compare properties
      c = strcmp(t1[1], t2[1]);
      if(c < 0)
      {
         // t1 property < t2 property
         rval = true;
      }
      // properties equal, compare objects
      else if(c == 0)
      {
         if(t1->getType() == String)
         {
            // consider String less than object, so t1 < t2
            if(t2->getType() != String)
            {
               rval = true;
            }
            else
            {
               // compare string values
               rval = t1[2] < t2[2];
            }
         }
         else if(t2->getType() == String)
         {
            // consider String less than object, so t2 < t1
            rval = false;
         }
         else
         {
            // compare IRI or literal
            const char* v1 = t1[2]->hasMember("@iri") ?
               t1[2]["@iri"]->getString() : t1[2]["@literal"]->getString();
            const char* v2 = t2[2]->hasMember("@iri") ?
               t2[2]["@iri"]->getString() : t2[2]["@literal"]->getString();
            rval = (strcmp(v1, v2) < 0);
         }
      }
   }

   return rval;
}

/**
 * Adds automatic embeds to the map of suggested embeds.
 *
 * @param subjects the map of all subjects in the graph.
 * @param embeds the map of suggested embeds.
 */
static void _addAutoEmbeds(DynamicObject& subjects, DynamicObject& embeds)
{
   // build a map of subject references, if a subject is referenced exactly
   // once then it can be embedded in another one
   SubjectRefMap refs;
   DynamicObjectIterator si = subjects.getIterator();
   while(si->hasNext())
   {
      DynamicObject& subject = si->next();

      // iterate over properties, doing reverse lookups to find subjects
      DynamicObjectIterator pi = subject.getIterator();
      while(pi->hasNext())
      {
         DynamicObject& objects = pi->next();
         const char* property = pi->getName();

         // skip self-subject identifier and type identifiers
         if(strcmp(property, "@") != 0 && strcmp(property, RDF_TYPE) != 0)
         {
            // iterate over objects (may be more than one value per property)
            DynamicObjectIterator oi = objects.getIterator();
            while(oi->hasNext())
            {
               DynamicObject& object = oi->next();
               if(object->getType() == Map && object->hasMember("@iri") &&
                  subjects->hasMember(object["@iri"]))
               {
                  const char* iri = object["@iri"];

                  // subject reference found, update refs
                  if(refs.find(subject) == refs.end())
                  {
                     refs[iri].subject = subject;
                     refs[iri].property = property;
                     refs[iri].count = 1;
                  }
                  else
                  {
                     ++refs[iri].count;
                  }
               }
            }
         }
      }
   }

   // for every subject that is referenced exactly once, add an auto-embed
   for(SubjectRefMap::iterator i = refs.begin(); i != refs.end(); ++i)
   {
      if(i->second.count == 1)
      {
         DynamicObject& embed = embeds[i->first];
         embed["s"] = i->second.subject;
         embed["p"] = i->second.property;
         embed["manual"] = false;
         embed["broken"] = false;
      }
   }
}

/**
 * Returns true if the given object is a reference to the given subject. This
 * means that o["@iri"] == s["@"]["@iri"].
 *
 * @param o the object.
 * @param s the subject.
 *
 * @return true if the given object is a reference to the given subject.
 */
static bool _isReference(DynamicObject& o, DynamicObject& s)
{
   return o->hasMember("@iri") && o["@iri"] == s["@"]["@iri"];
}

/**
 * Embeds an object (that is a subject) if it isn't already under the given
 * property. If the property only references the object by IRI, the reference
 * will be replaced with the embedded object.
 *
 * @param s the subject to update.
 * @param p the property.
 * @param o the object to embed.
 */
static void _embedObject(
   DynamicObject& s, const char* p, DynamicObject& o)
{
   DynamicObject& existing = s[p];
   if(existing->getType() != Array)
   {
      // if the existing object is a reference to the embed, replace it
      if(_isReference(existing, o))
      {
         existing = o;
      }
   }
   // if the existing object isn't already embedded
   else if(existing->indexOf(o) == -1)
   {
      // find and replace the reference to the object
      bool found = false;
      DynamicObjectIterator i = existing.getIterator();
      while(!found && i->hasNext())
      {
         if(_isReference(i->next(), o))
         {
            // replace reference
            existing[i->getIndex()] = o;
            found = true;
         }
      }
   }
}

/**
 * Determines if the given subject and object reference each other cyclically.
 *
 * @param embeds the embeds map.
 * @param subject the subject.
 * @param object the object.
 * @param manual set to whether or not a manual embed was found when looking
 *           for a cycle.
 *
 * @return true if a subject-object cycle was found, false if not.
 */
static bool _isCycle(
   DynamicObject& embeds, const char* subject, const char* object,
   bool* manual)
{
   // FIXME: change all references of "manual" to "auto" or "frame", etc.
   // no manual embed found yet
   *manual = false;

   /* Here we are checking to see if the given subject and object reference
      each other cyclically. How to detect a cycle:

      In order for a subject to be in a cycle via the given object, it must be
      possible to walk the embeds map, following references, and find the
      object and then the subject. */
   bool cycleFound = false;
   bool objectFound = false;
   const char* tmp = subject;
   while(!cycleFound && embeds->hasMember(tmp))
   {
      DynamicObject& embed = embeds[tmp];
      tmp = embed["s"]["@"]["@iri"];
      if(embed["manual"]->getBoolean())
      {
         // manual embed found during walk (in possible cycle)
         *manual = true;
      }
      if(tmp == object)
      {
         // object found
         objectFound = true;
      }
      else if(tmp == subject)
      {
         // cycle found
         cycleFound = true;
      }
   }

   return cycleFound && objectFound;
}

/**
 * Removes any cycles detected in the embeds map.
 *
 * @param subjects the map of top-level subjects.
 * @param embeds the map of embeds.
 * @param explicitOnly
 */
static void _pruneCycles(
   DynamicObject& subjects, DynamicObject& embeds, bool explicitOnly)
{
   // iterate over embeds adding those that will be kept to "keep"
   DynamicObject keep;
   keep->setType(Map);
   DynamicObjectIterator i = embeds.getIterator();
   while(i->hasNext())
   {
      DynamicObject& objectEmbed = i->next();

      // do not keep embeds that are already broken and if in explicitOnly
      // mode, do not keep any automatic (non-manual) embeds
      if(!objectEmbed["broken"]->getBoolean() &&
         (!explicitOnly || objectEmbed["manual"]->getBoolean()))
      {
         const char* subject = objectEmbed["s"]["@"]["@iri"];
         const char* object = i->getName();

         // see if object cyclically references subject
         bool cycleManual;
         if(_isCycle(embeds, subject, object, &cycleManual))
         {
            // cycle found
            DynamicObject& subjectEmbed = embeds[subject];

            // keep embed of "object" if it is manual or the cycle is automated
            // and break embed of "subject"
            if(objectEmbed["manual"]->getBoolean() || !cycleManual)
            {
               subjectEmbed["broken"] = true;
               keep[object] = objectEmbed;
            }
            // keep embed of "subject", break embed of "object"
            else
            {
               objectEmbed["broken"] = true;
            }
         }
         // no cycle, add as valid embed
         else
         {
            keep[object] = objectEmbed;
         }
      }
   }

   embeds = keep;
}

/**
 * Finds all of the subjects in the given subject map with the given type, adds
 * them to the given targets array and target map, and removes them from the
 * subject map.
 *
 * @param subjects the set of all of the subjects to search.
 * @param type the type to look for.
 * @param targets the array of targets to populate.
 * @param targetMap the map of targets to populate.
 * @param limit the maximum number of targets to find, -1 for no limit.
 */
static void _findTypes(
   DynamicObject& subjects, DynamicObject& type,
   DynamicObject& targets, DynamicObject& targetMap, int limit = -1)
{
   DynamicObjectIterator i = subjects.getIterator();
   while((limit == -1 || targets->length() < limit) && i->hasNext())
   {
      DynamicObject& next = i->next();
      if(next->hasMember(RDF_TYPE))
      {
         // RDF_TYPE is either an array of types or a single type
         DynamicObjectIterator ti = next[RDF_TYPE].getIterator();
         while(ti->hasNext())
         {
            DynamicObject& t = ti->next();
            if((t->getType() == Array && t->indexOf(type) != -1) ||
               (t->getType() != Array && t == type))
            {
               // add target, remove from subject map
               targets->append(next);
               targetMap[i->getName()] = next;
               i->remove();
               break;
            }
         }
      }
   }
}

/**
 * Builds a set of subjects to be searched.
 *
 * If the subject parameter is NULL, all subjects will be added to the set. If
 * not, then all subjects under subject[property] will be added to the set.
 *
 * @param subjects all top-level subjects.
 * @param subject the subject to add from, NULL to add all subjects.
 * @param property the property on the subject.
 * @param subjectMap the set to populate.
 */
static void _buildSubjectMap(
   DynamicObject& subjects, const char* subject, const char* property,
   DynamicObject& subjectMap)
{
   // build a list of subjects to search that is based on subject[property]
   // (if subject is NULL, use all subjects)
   subjectMap->setType(Map);
   DynamicObjectIterator i =
      (subject == NULL) ? subjects.getIterator() :
      subjects[subject][property].getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();

      if(next->getType() == Array)
      {
         DynamicObjectIterator ii = next.getIterator();
         while(ii->hasNext())
         {
            DynamicObject& n = ii->next();
            if(n->hasMember("@") && subjects->hasMember(n["@"]["@iri"]))
            {
               const char* s = n["@"]["@iri"];
               subjectMap[s] = n;
            }
         }
      }
      else if(next->hasMember("@") && subjects->hasMember(next["@"]["@iri"]))
      {
         const char* s = next["@"]["@iri"];
         subjectMap[s] = next;
      }
   }
}

/**
 * Finds all of the target objects that match the given frame.
 *
 * @param parent the parent frame.
 * @param frame the frame to find matches for.
 * @param subjects all of the top-level subjects.
 * @param subject the subject to look in, NULL to look in all subjects.
 * @param property the property of the subject to look in.
 * @param targets the array to populate with targets that match.
 * @param targetMap the map to populate with targets that match.
 * @param explicitOnly true if only information specified in the frame should
 *           be extracted from the graph, false if not.
 */
static void _findTargetObjects(
   DynamicObject& parent, DynamicObject& frame, DynamicObject& subjects,
   const char* subject, const char* property,
   DynamicObject& targets, DynamicObject& targetMap, bool explicitOnly)
{
   // if the frame is empty and explicit is off, auto-match all subjects
   if(frame->length() == 0 && !explicitOnly)
   {
      _buildSubjectMap(subjects, subject, property, targetMap);
      DynamicObjectIterator i = targetMap.getIterator();
      while(i->hasNext())
      {
         targets->append(i->next());
      }
   }
   else
   {
      // build a subject map to look in
      DynamicObject subjectMap;
      _buildSubjectMap(subjects, subject, property, subjectMap);

      // look for the target objects in the subjectMap using the frame
      // use "@" first, if not present use RDF_TYPE
      if(frame->hasMember("@"))
      {
         // frame MUST use compact '@' form, not normalized @iri form for '@'
         DynamicObjectIterator i = frame["@"].getIterator();
         while(i->hasNext())
         {
            const char* s = i->next();
            if(subjectMap->hasMember(s))
            {
               targets->append(subjects[s]);
               targetMap[s] = subjectMap[s];
               subjectMap->removeMember(s);
            }
         }
      }
      else if(frame->hasMember(RDF_TYPE))
      {
         // find all types (limit to the first found if frame parent is a map)
         DynamicObjectIterator i = frame[RDF_TYPE].getIterator();
         while(i->hasNext())
         {
            _findTypes(
               subjectMap, i->next(), targets, targetMap,
               (parent->getType() == Map) ? 1 : -1);
         }
      }
   }
}

/**
 * Handles embedding the given target or removing an embed for it if it
 * should be at the top-level according to the given frame.
 *
 * @param subjects the subjects in the graph.
 * @param parent the parent subject of the target.
 * @param property the property of the parent that relates the target to it.
 * @param embeds the embeds map.
 * @param frame the frame.
 * @param target the target.
 */
static void _embedTarget(
   DynamicObject& subjects, const char* parent, const char* property,
   DynamicObject& embeds, DynamicObject& frame, DynamicObject& target)
{
   // if first call (parent == NULL), remove target from embeds since
   // it should be at the top-level
   if(parent == NULL)
   {
      embeds->removeMember(target["@"]["@iri"]);
   }
   // remove auto-embed of the target in the parent subject if the
   // frame type is empty string -- this indicates that a reference
   // should be used, not an embed
   else if(frame->getType() == String && frame->length() == 0)
   {
      const char* iri = target["@"]["@iri"];
      if(embeds->hasMember(iri) && embeds[iri]["s"] == subjects[parent])
      {
         embeds->removeMember(iri);
      }
   }
   // add a manual embed of the current target in the parent subject
   else
   {
      DynamicObject embed;
      embed["s"] = subjects[parent];
      embed["p"] = property;
      embed["manual"] = true;
      embed["broken"] = false;
      embeds[target["@"]["@iri"]->getString()] = embed;
   }
}

/**
 * Process frame is a recursive function that traverses the frame tree. As
 * it traverses, it finds "target objects" in the graph to embed according
 * to the structure specified by the frame. This method will not actually
 * build the final JSON-LD object, but rather it will make suggestions about
 * what to embed and what to remove from the graph. The final object will
 * be built using a map of subjects that keeps track of every subject mentioned
 * in the graph. This map will be later modified using the embeds and removals
 * added by this function.
 *
 * If an object is to be embedded, it is marked as such -- but the actual
 * embedding will occur later, after all suggested embeds have been added,
 * in order to resolve cycles, etc.
 *
 * If the "explicitOnly" flag is set, then any objects in the graph that are
 * not mentioned by the frame are marked to be later removed. Objects might
 * also be marked for removal that are not actually removed -- this is because
 * they might not be mentioned explicitly in one part of the frame but will
 * be mentioned in another.
 *
 * The actual removal process happens after the entire traversal completes
 * to ensure iterators remain valid during recursion.
 *
 * @param frame the frame node (first call starts at the top of the frame tree).
 * @param targetMap a map to populate with the target subjects.
 * @param subjects a map of all subjects in the graph.
 * @param subject the current parent subject in the traversal, NULL to start.
 * @param property the current parent property in the traversal.
 * @param embeds a map of suggested objects to embed (according to the frame).
 * @param explicitOnly true to only include subjects mentioned in the frame.
 * @param removals a list of subjects to potentially remove.
 */
static void _processFrame(
   DynamicObject& frame, DynamicObject targetMap,
   DynamicObject& subjects, const char* subject, const char* property,
   DynamicObject& embeds, bool explicitOnly, DynamicObject& removals)
{
   // create a map to keep track of subjects that are targets to be extracted
   // from the graph and put in the output so that we can add removals for
   // unused subjects
   targetMap->setType(Map);

   /* Note: The frame is either an array of maps or a map. It cannot be
      an array of arrays. To simplify the code path, we build a top-level
      frame container that is an array that can be iterated over the same way
      regardless of the input frame type. */

   // build frame container to iterate over
   DynamicObject top(NULL);
   if(frame->getType() == Array)
   {
      top = frame.clone();
      // if explicit off and array is empty, use an empty frame to include all
      // subjects in the graph as targets
      if(frame->length() == 0 && !explicitOnly)
      {
         DynamicObject empty;
         empty->setType(Map);
         top->append(empty);
      }
   }
   else
   {
      top = DynamicObject();
      top->setType(Array);
      top->append(frame);
   }

   // now iterate over common frame container
   DynamicObjectIterator fi = top.getIterator();
   while(fi->hasNext())
   {
      DynamicObject& f = fi->next();

      // find target objects that match the frame requirements
      DynamicObject targets;
      targets->setType(Array);
      _findTargetObjects(
         frame, f, subjects, subject, property,
         targets, targetMap, explicitOnly);

      // FIXME: change findTargetObjects to check frame[property] and do
      // explicitOnly check there ... better recursion, less repeated code

      // iterate over target objects, handling embeds and recursing
      DynamicObjectIterator i = targets.getIterator();
      while(i->hasNext())
      {
         DynamicObject& target = i->next();

         // embed target appropriately
         _embedTarget(subjects, subject, property, embeds, f, target);

         // iterate over properties in target to recurse
         DynamicObjectIterator oi = target.getIterator();
         while(oi->hasNext())
         {
            DynamicObject& obj = oi->next();
            const char* p = oi->getName();

            // skip "@" and RDF_TYPE properties
            if(strcmp(p, "@") != 0 && strcmp(p, RDF_TYPE) != 0)
            {
               // frame mentions property
               if(f->hasMember(p))
               {
                  // get next frame
                  DynamicObject& nf = f[p];

                  // if the frame wants a string, remove any related embeds
                  if(f[p]->getType() == String)
                  {
                     // normalize property objects to array for single
                     // code path
                     DynamicObject tmpArray;
                     tmpArray->setType(Array);
                     if(obj->getType() != Array)
                     {
                        tmpArray.push(obj);
                     }
                     else
                     {
                        tmpArray.merge(obj, true);
                     }

                     // iterate over object values
                     DynamicObjectIterator vi = tmpArray.getIterator();
                     while(vi->hasNext())
                     {
                        DynamicObject& ov = vi->next();
                        if(ov->hasMember("@iri"))
                        {
                           const char* iri = ov["@iri"];
                           if(embeds->hasMember(iri) &&
                              embeds[iri]["s"] ==
                              subjects[target["@"]["@iri"]->getString()])
                           {
                              embeds->removeMember(iri);
                           }
                        }
                     }
                  }

                  // if frame wants a single value, pick the first one
                  if(obj->getType() == Array && nf->getType() != Array)
                  {
                     obj = obj[0];
                  }
                  // recursion required
                  else if(nf->getType() == Map || nf->getType() == Array)
                  {
                     // convert object to an array if necessary
                     if(nf->getType() == Array && obj->getType() != Array)
                     {
                        DynamicObject tmp = obj.clone();
                        obj->setType(Array);
                        obj->append(tmp);
                     }

                     // recurse into next frame
                     _processFrame(
                        nf, DynamicObject(), subjects, target["@"]["@iri"],
                        p, embeds, explicitOnly, removals);
                  }
               }
               // current frame does not mention property, so if in explicit
               // mode, mark it for potential removal (if it is mentioned
               // elsewhere, the removal will not apply)
               else if(explicitOnly)
               {
                  DynamicObject& remove = removals->append();
                  remove["s"] = target["@"]["@iri"];
                  remove["p"] = p;
               }
            }
         }
      }
   }

   // if explicit only is on, remove any subjects not marked as targets
   // (targets are removed from the subjectMap)
   if(explicitOnly)
   {
      // build a clean subject map to compare against
      DynamicObject subjectMap;
      _buildSubjectMap(subjects, subject, property, subjectMap);
      DynamicObjectIterator si = subjectMap.getIterator();
      while(si->hasNext())
      {
         DynamicObject& next = si->next();

         // add removal if subject is not in target map
         if(!targetMap->hasMember(si->getName()))
         {
            DynamicObject& remove = removals->append();
            if(subject == NULL)
            {
               // remove top-level subject
               remove["s"] = si->getName();
            }
            else
            {
               // remove subject->property link
               remove["s"] = subject;
               remove["p"] = property;
               remove["o"] = next["@"];
            }
         }
      }
   }
}

static void _processRemovals(
   DynamicObject& subjects, DynamicObject& embeds, DynamicObject& removals)
{
   DynamicObjectIterator i = removals.getIterator();
   while(i->hasNext())
   {
      DynamicObject& remove = i->next();
      const char* s = remove["s"]->getString();

      // if no property, remove subject entirely
      if(!remove->hasMember("p"))
      {
         // only remove subject if it isn't mentioned by a manual embed
         if(!embeds->hasMember(s) || !embeds[s]["manual"]->getBoolean())
         {
            subjects->removeMember(s);
         }
      }
      else if(subjects->hasMember(s))
      {
         DynamicObject& subject = subjects[s];
         const char* p = remove["p"];

         // remove associated embeds
         DynamicObjectIterator ii = subject[p].getIterator();
         while(ii->hasNext())
         {
            DynamicObject& next = ii->next();

            // remove embed if its name matches the only listed in remove
            // or if none is listed (which means remove all objects under
            // the current property)
            if(embeds->hasMember(next) &&
               (!remove->hasMember("o") || strcmp(remove["o"], next) == 0))
            {
               DynamicObject& embed = embeds[next->getString()];
               if(strcmp(embed["s"], s) == 0 && strcmp(embed["p"], p) == 0)
               {
                  embeds->removeMember(next);
               }
            }
         }

         // if no object was specified, remove the entire property
         if(!remove->hasMember("o"))
         {
            subject->removeMember(p);
         }
         // remove the object from subject->property
         else
         {
            DynamicObject& object = subject[p];
            if(object->getType() == Array)
            {
               // remove object from array
               DynamicObjectIterator ri = object.getIterator();
               while(ri->hasNext())
               {
                  if(strcmp(ri->next(), remove["o"]) == 0)
                  {
                     ri->remove();
                     break;
                  }
               }
               // if array is empty, remove property
               if(object->length() == 0)
               {
                  subject->removeMember(p);
               }
            }
            // object is a string
            else if(strcmp(object, remove["o"]) == 0)
            {
               subject->removeMember(p);
            }
         }
      }
   }
}

static void _processEmbeds(
   DynamicObject& subjects, DynamicObject& embeds)
{
   DynamicObjectIterator i = embeds.getIterator();
   while(i->hasNext())
   {
      // get the subject dyno that will hold the embedded object
      DynamicObject& embed = i->next();
      const char* object = i->getName();
      const char* property = embed["p"]->getString();
      DynamicObject& s = embed["s"];

      // get the referenced object
      DynamicObject obj(NULL);
      if(subjects->hasMember(object))
      {
         // get embedded object and remove it from list of top-level subjects
         obj = subjects[object];
         subjects->removeMember(object);
      }
      else
      {
         // referenced object is just a string
         obj = DynamicObject();
         obj = object;
      }

      // set the subject's property to the embedded object
      _embedObject(s, property, obj);
   }
}

bool JsonLdFrame::setFrame(DynamicObject& frame, bool explicitOnly)
{
   bool rval = true;

   // FIXME: validation of frame?

   mFrame = frame.clone();
   mExplicit = explicitOnly;

   // FIXME: expand all predicates and types in the frame

   return rval;
}

bool JsonLdFrame::frameTriples(
   DynamicObject& triples, DynamicObject& out, bool sort)
{
   // sort triples if requested
   if(sort)
   {
      triples.sort(&_sortTriples);
   }

   // build a map of subjects
   DynamicObject subjects;
   subjects->setType(Map);
   DynamicObjectIterator i = triples.getIterator();
   while(i->hasNext())
   {
      DynamicObject& triple = i->next();
      DynamicObject& s = subjects[triple[0]->getString()];
      if(!s->hasMember("@"))
      {
         s["@"] = triple[0]->getString();
      }
      const char* p = triple[1];
      DynamicObject& o = triple[2];
      if(s->hasMember(p))
      {
         // add IRIs uniquely, always add non-IRIs
         if(!o->hasMember("@iri") ||
            (s[p]->getType() == Array && s[p]->indexOf(o) == -1) ||
            (s[p]->getType() == Map && s[p] != o))
         {
            s[p].push(o);
         }
      }
      else
      {
         s[p] = o;
      }
   }

   // frame subjects
   return frameSubjects(subjects, out);
}

bool JsonLdFrame::frameSubjects(DynamicObject subjects, DynamicObject& out)
{
   bool rval = true;

   // clone to prevent changing incoming graph
   subjects = subjects.clone();

   // default top-level result to subjects
   DynamicObject topLevel = subjects;

   // create a map for embeds (any given object can only be embedded once)
   DynamicObject embeds;
   embeds->setType(Map);

   // if explicit mode is not on, then add objects that can be auto-embedded
   if(!mExplicit)
   {
      _addAutoEmbeds(subjects, embeds);
   }

   /* Note: At this point "subjects" holds a reference to every subject in
      the graph and each of those subjects has all of its properties. There
      are no embedded objects, but "embeds" contains a list of potential
      objects to embed. Embedding specific objects in the target according to
      a frame is next. */
   if(!mFrame.isNull())
   {
      // remove context from frame
      DynamicObject frame;
      rval = JsonLd::removeContext(mFrame, frame);
      if(rval)
      {
         // process frame, store removals (removals are stored to avoid
         // removing graph nodes during recursion)
         topLevel = DynamicObject();
         DynamicObject removals;
         removals->setType(Array);
         _processFrame(
            frame, topLevel, subjects, NULL, NULL, embeds, mExplicit, removals);

         // clean up removals
         _processRemovals(subjects, embeds, removals);
      }
   }

   /* Now that all possible embeds have been marked, we can prune cycles. */
   _pruneCycles(subjects, embeds, mExplicit);

   // handle all embeds
   _processEmbeds(subjects, embeds);

   // default output to a map
   out->setType(Map);
   out->clear();

   // build final JSON-LD object by adding all remaining top-level objects
   DynamicObjectIterator i = topLevel.getIterator();
   while(i->hasNext())
   {
      DynamicObject& subject = i->next();

      // first subgraph to add, so just merge into target
      if(!out->hasMember("@"))
      {
         out.merge(subject, false);
      }
      // not the first subgraph...
      else
      {
         // change top-level subject into an array
         if(out["@"]->getType() != Array)
         {
            DynamicObject tmp = out.clone();
            out->clear();
            out["@"]->append(tmp);
         }
         // add next top-level subgraph
         out["@"]->append(subject);
      }
   }

   return rval;
}

bool JsonLdFrame::reframe(DynamicObject& jsonld, DynamicObject& out)
{
   bool rval = true;

   /* Note: The context for the output will be any context from the input
      merged with any context from the frame (where the frame has preference
      over the input).
    */

   // clone any existing context from the input
   DynamicObject ctx(NULL);
   if(jsonld->hasMember("@context"))
   {
      ctx = jsonld["@context"].clone();
   }
   if(!mFrame.isNull() && mFrame->hasMember("@context"))
   {
      DynamicObject frameCtx = mFrame["@context"].clone();
      if(ctx.isNull())
      {
         ctx = frameCtx;
      }
      else
      {
         ctx = JsonLd::mergeContexts(ctx, frameCtx);
         rval = !ctx.isNull();
      }
   }

   // normalize jsonld
   DynamicObject normalized;
   rval = rval && JsonLd::normalize(jsonld, normalized);
   if(rval)
   {
      // prepare output
      out->clear();

      // build map of subjects from normalized input
      DynamicObject subjects;
      subjects->setType(Map);
      DynamicObjectIterator i = normalized.getIterator();
      while(i->hasNext())
      {
         // FIXME: this will skip top-level blank nodes, should they instead
         // be given a temporary name that is later removed?
         DynamicObject& subject = i->next();
         if(subject->hasMember("@"))
         {
            subjects[subject["@"]["@iri"]->getString()] = subject;
         }
      }

      // frame subjects
      DynamicObject framed;
      rval = frameSubjects(subjects, framed);
      if(rval)
      {
         // apply context to output
         if(!ctx.isNull())
         {
            rval = JsonLd::addContext(ctx, framed, out);
         }
         else
         {
            out = framed;
         }
      }
   }

   return rval;
}
