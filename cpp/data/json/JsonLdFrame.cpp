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

#define RDF_TYPE   "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>"

struct SubjectRef
{
   DynamicObject subject;
   const char* predicate;
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
 * used to sort triples alphabetically, first by subject, then predicate,
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
      // subjects equal, compare predicates
      c = strcmp(t1[1], t2[1]);
      if(c < 0)
      {
         // t1 predicate < t2 predicate
         rval = true;
      }
      // predicates equal, compare objects
      else if(c == 0)
      {
         rval = (strcmp(t1[2], t2[2]) < 0);
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

      // iterate over predicates, doing reverse lookups to find subjects
      DynamicObjectIterator pi = subject.getIterator();
      while(pi->hasNext())
      {
         DynamicObject& objects = pi->next();
         const char* predicate = pi->getName();

         // skip self-subject identifier and type identifiers
         if(strcmp(predicate, "@") != 0 && strcmp(predicate, RDF_TYPE) != 0)
         {
            // iterate over objects (may be more than one value per predicate)
            DynamicObjectIterator oi = objects.getIterator();
            while(oi->hasNext())
            {
               const char* object = oi->next();
               if(subjects->hasMember(object))
               {
                  // subject reference found, update refs
                  if(refs.find(subject) == refs.end())
                  {
                     refs[object].subject = subject;
                     refs[object].predicate = predicate;
                     refs[object].count = 1;
                  }
                  else
                  {
                     ++refs[object].count;
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
         embed["p"] = i->second.predicate;
         embed["manual"] = false;
         embed["broken"] = false;
      }
   }
}

static bool _hasSubject(DynamicObject& object, const char* subject)
{
   return object->hasMember("@") && strcmp(object["@"], subject) == 0;
}

static void _addUniqueObject(
   DynamicObject& s, const char* predicate, DynamicObject& object)
{
   DynamicObject& p = s[predicate];
   DynamicObjectType type = p->getType();
   if(type != Array)
   {
      // set predicate to object if it references the same one by subject
      if(p == object ||
         (type == String && _hasSubject(object, p)) ||
         (type == Map && _hasSubject(p, object)))
      {
         p = object;
      }
      // convert predicate to array and append object
      else
      {
         DynamicObject tmp = p;
         p = DynamicObject();
         p->append(tmp);
         p->append(object);
      }
   }
   else
   {
      int index = p->indexOf(object);
      if(index != -1)
      {
         // object exists, replace it
         p[index] = object;
      }
      // the object might still exist as a subject within an embed
      else if(object->getType() == String)
      {
         // look for an existing object with the object as a subject
         bool found = false;
         DynamicObjectIterator i = p.getIterator();
         while(!found && i->hasNext())
         {
            if(_hasSubject(i->next(), object))
            {
               // replace object
               p[i->getIndex()] = object;
               found = true;
            }
         }

         // object not found, append it
         if(!found)
         {
            p->append(object);
         }
      }
      // ensure object's subject doesn't exist as a string in the array
      else
      {
         int index = p->indexOf(object["@"]);
         if(index != -1)
         {
            // replace string with object
            p[index] = object;
         }
         else
         {
            p->append(object);
         }
      }
   }
}

static void _setPredicate(
   DynamicObject& s, const char* predicate, DynamicObject& object)
{
   // set the subject's predicate to the embedded object
   if(s->hasMember(predicate))
   {
      // ensure object is only added uniquely
      _addUniqueObject(s, predicate, object);
   }
   else
   {
      s[predicate] = object;
   }
}

static bool _isCycle(
   DynamicObject& embeds, const char* subject, const char* object,
   bool* manual)
{
   bool rval = false;

   // no manual embed found yet
   *manual = false;

   /* Here we are checking to see if the given subject and object reference
      each other cyclically. How to detect a cycle:

      In order for a subject to be in a cycle, it must be possible to walk
      the embeds map, following references, and find the subject.
    */
   const char* tmp = subject;
   while(!rval && embeds->hasMember(tmp))
   {
      DynamicObject& embed = embeds[tmp];
      tmp = embed["s"]["@"];
      if(tmp == subject)
      {
         // cycle found
         rval = true;
      }
      else if(embed["manual"]->getBoolean())
      {
         // manual embed found in cycle
         *manual = true;
      }
   }

   return rval;
}

static void _pruneCycles(
   DynamicObject& subjects, DynamicObject& embeds, bool explicitOnly)
{
   DynamicObject tmp;
   tmp->setType(Map);
   DynamicObjectIterator i = embeds.getIterator();
   while(i->hasNext())
   {
      DynamicObject& embed = i->next();
      const char* subject = embed["s"]["@"];
      const char* object = i->getName();

      // only handle embed if not broken and not (explicit and not manual)
      if(!embed["broken"]->getBoolean() &&
         !(explicitOnly && !embed["manual"]->getBoolean()))
      {
         // see if object cyclically references subject
         bool manual;
         if(_isCycle(embeds, subject, object, &manual))
         {
            // cycle found
            DynamicObject& cycle = embeds[subject];

            // keep current embed if it is manual or the cycle is automated
            if(embed["manual"]->getBoolean() || !manual)
            {
               cycle["broken"] = true;
               tmp[object] = embed;
            }
            // keep cycle embed
            else
            {
               embed["broken"] = true;
            }
         }
         // no cycle, add as valid embed
         else
         {
            tmp[object] = embed;
         }
      }
   }

   embeds = tmp;
}

static void _findTypes(
   DynamicObject& subjects, const char* type,
   DynamicObject& targets, DynamicObject& targetMap, int limit = -1)
{
   DynamicObjectIterator i = subjects.getIterator();
   while((limit == -1 || targets->length() < limit) && i->hasNext())
   {
      DynamicObject& next = i->next();
      if(next->hasMember(RDF_TYPE))
      {
         // RDF_TYPE is either an array of strings or a string
         DynamicObjectIterator ti = next[RDF_TYPE].getIterator();
         while(ti->hasNext())
         {
            DynamicObject& t = ti->next();
            if((t->getType() == String && strcmp(t, type) == 0) ||
               (t->getType() == Array && t->indexOf(type) != -1))
            {
               // add target, remove from subject set
               targets->append(next);
               targetMap[i->getName()] = next;
               i->remove();
               break;
            }
         }
      }
   }
}

static void _buildSubjectSet(
   DynamicObject& subjects, const char* subject, const char* predicate,
   DynamicObject& subjectSet)
{
   // build a list of subjects to look for the target objects in based
   // on subject->predicate (if subject is NULL, use all subjects)
   subjectSet->setType(Map);
   DynamicObjectIterator i =
      (subject == NULL) ? subjects.getIterator() :
      subjects[subject][predicate].getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      const char* s = (subject == NULL) ? i->getName() : next->getString();
      if(subjects->hasMember(s))
      {
         subjectSet[s] = subjects[s];
      }
   }
}

static void _findTargetObjects(
   DynamicObject& parent, DynamicObject& frame, DynamicObject& subjects,
   const char* subject, const char* predicate,
   DynamicObject& targets, DynamicObject& targetMap,
   bool explicitOnly, DynamicObject& removals)
{
   // if the frame is empty and explicit is off, include all targets
   if(!explicitOnly && frame->length() == 0)
   {
      _buildSubjectSet(subjects, subject, predicate, targetMap);
      DynamicObjectIterator i = targetMap.getIterator();
      while(i->hasNext())
      {
         targets->append(i->next());
      }
   }
   else
   {
      // build a subject set to look in
      DynamicObject subjectSet;
      _buildSubjectSet(subjects, subject, predicate, subjectSet);

      // look for the target objects in the subjectSet using the frame
      // use "@" first, if not present use RDF_TYPE
      if(frame->hasMember("@"))
      {
         DynamicObjectIterator i = frame["@"].getIterator();
         while(i->hasNext())
         {
            const char* s = i->next();
            if(subjectSet->hasMember(s))
            {
               targets->append(subjects[s]);
               targetMap[s] = subjectSet[s];
               subjectSet->removeMember(s);
            }
         }
      }
      else if(frame->hasMember(RDF_TYPE))
      {
         // find all types (limit to the first found if frame parent is a map)
         DynamicObjectIterator i = frame[RDF_TYPE].getIterator();
         while(i->hasNext())
         {
            const char* type = i->next();
            _findTypes(
               subjectSet, type, targets, targetMap,
               (parent->getType() == Map) ? 1 : -1);
         }
      }
   }
}

/**
 * Process frame is a recursive function that traverses the frame tree. As
 * it traverses, it finds "target objects" in the graph to embed according
 * to the structure specified by the frame. This method will not actually
 * build the final JSON-LD object, but rather it will make suggestions about
 * what to embed and what to remove from the graph. The final object will
 * be built using a map of subjects keeps track of every subject mentioned in
 * the graph. This map will be later modified using the embeds and removals
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
 * @param predicate the current parent predicate in the traversal.
 * @param embeds a map of suggested objects to embed (according to the frame).
 * @param explicitOnly true to only include subjects mentioned in the frame.
 * @param removals a list of subjects to potentially remove.
 */
static void _processFrame(
   DynamicObject& frame, DynamicObject targetMap,
   DynamicObject& subjects, const char* subject, const char* predicate,
   DynamicObject& embeds, bool explicitOnly, DynamicObject& removals)
{
   // create a map to keep track of subjects that are targets so we can
   // add removals for unused subjects
   targetMap->setType(Map);

   /* Note: The frame is either an array of maps or a map. It cannot be
      an array of arrays. Therefore, we build a frame container to iterate
      over the frame in the same way regardless of its type. */

   // build frame container to iterate over
   DynamicObject top(NULL);
   bool isArray = (frame->getType() == Array);
   if(isArray)
   {
      top = frame.clone();
      // if explicit off and array is empty, use an empty frame to include all
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
   DynamicObjectIterator fi = top.getIterator();
   while(fi->hasNext())
   {
      DynamicObject& f = fi->next();

      // find target objects that match the frame requirements
      DynamicObject targets;
      targets->setType(Array);
      _findTargetObjects(
         frame, f, subjects, subject, predicate,
         targets, targetMap, explicitOnly, removals);

      // iterate over objects to place in the target
      DynamicObjectIterator i = targets.getIterator();
      while(i->hasNext())
      {
         DynamicObject& target = i->next();

         // if first call (subject == NULL), remove target from embeds
         if(subject == NULL)
         {
            embeds->removeMember(target["@"]);
         }
         // remove auto-embeds if type is empty string
         else if(f->getType() == String && f->length() == 0)
         {
            const char* s = target["@"]->getString();
            if(embeds->hasMember(s) && embeds[s]["s"] == subjects[subject])
            {
               embeds->removeMember(s);
            }
         }
         // add manual embed
         else
         {
            DynamicObject embed;
            embed["s"] = subjects[subject];
            embed["p"] = predicate;
            embed["manual"] = true;
            embed["broken"] = false;
            embeds[target["@"]->getString()] = embed;
         }

         // iterate over predicates and objects in subject
         DynamicObjectIterator oi = target.getIterator();
         while(oi->hasNext())
         {
            DynamicObject& obj = oi->next();
            const char* p = oi->getName();

            // skip "@" and RDF_TYPE predicates
            if(strcmp(p, "@") != 0 && strcmp(p, RDF_TYPE) != 0)
            {
               // frame mentions predicate
               if(f->hasMember(p))
               {
                  // get next frame
                  DynamicObject& nf = f[p];

                  // if the frame wants a string, remove any related embeds
                  if(f[p]->getType() == String)
                  {
                     // iterate over object subjects
                     DynamicObjectIterator ooi = obj.getIterator();
                     while(ooi->hasNext())
                     {
                        const char* os = ooi->next();
                        if(embeds->hasMember(os))
                        {
                           if(embeds[os]["s"] ==
                              subjects[target["@"]->getString()])
                           {
                              embeds->removeMember(os);
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
                        nf, DynamicObject(), subjects, target["@"], p, embeds,
                        explicitOnly, removals);
                  }
               }
               // frame does not mention predicate, if in explicit mode,
               // mark it for potential removal
               else if(explicitOnly)
               {
                  DynamicObject& remove = removals->append();
                  remove["s"] = target["@"];
                  remove["p"] = p;
               }
            }
         }
      }
   }

   // if explicit only is on, remove any subjects not marked as targets
   // (targets are removed from the subjectSet)
   if(explicitOnly)
   {
      // build a clean subject set to compare against
      DynamicObject subjectSet;
      _buildSubjectSet(subjects, subject, predicate, subjectSet);
      DynamicObjectIterator si = subjectSet.getIterator();
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
               // remove subject->predicate link
               remove["s"] = subject;
               remove["p"] = predicate;
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

      // if no predicate, remove subject entirely
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
            // the current predicate)
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

         // if no object was specified, remove the entire predicate
         if(!remove->hasMember("o"))
         {
            subject->removeMember(p);
         }
         // remove the object from subject->predicate
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
               // if array is empty, remove predicate
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
      const char* predicate = embed["p"]->getString();
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

      // set the subject's predicate to the embedded object
      _setPredicate(s, predicate, obj);
   }
}

bool JsonLdFrame::setFrame(DynamicObject& frame, bool explicitOnly)
{
   bool rval = true;

   // FIXME: validation of frame?

   mFrame = frame;
   mExplicit = explicitOnly;

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
      _setPredicate(s, triple[1], triple[2]);
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
      the graph and each of those subjects has all of its predicates. There
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

   // save context
   DynamicObject ctx(NULL);
   if(jsonld->hasMember("#"))
   {
      ctx = jsonld["#"];
   }

   // normalize jsonld
   DynamicObject normalized;
   rval = JsonLd::normalize(jsonld, normalized);
   if(rval)
   {
      // prepare output
      out->clear();

      // create array of subjects to simplify code path
      DynamicObject array;
      array->setType(Array);
      if(normalized["@"]->getType() == Array)
      {
         array.merge(normalized["@"], true);
      }
      else
      {
         array->append(normalized);
      }

      // build map of subjects from normalized input
      DynamicObject subjects;
      subjects->setType(Map);
      DynamicObjectIterator i = array.getIterator();
      while(i->hasNext())
      {
         DynamicObject& subject = i->next();
         subjects[subject["@"]->getString()] = subject;
      }

      if(ctx.isNull())
      {
         // frame subjects
         rval = frameSubjects(subjects, out);
      }
      else
      {
         // frame subjects and re-add context
         DynamicObject framed;
         rval =
            frameSubjects(subjects, framed) &&
            JsonLd::addContext(ctx, framed, out);
      }
   }

   return rval;
}
