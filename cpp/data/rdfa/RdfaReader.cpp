/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/rdfa/RdfaReader.h"

#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/Convert.h"

#include <cstdio>
#include <rdfa_utils.h>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::rdfa;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

#define RDFA_READER   "monarch.data.rdfa.RdfaReader"

RdfaReader::RdfaReader() :
   mStarted(false),
   mBaseUri(NULL),
   mRdfaCtx(NULL),
   mContext(NULL),
   mDefaultGraphFrame(NULL),
   mProcessorGraphFrame(NULL),
   mExceptionGraphFrame(NULL),
   mExplicit(false)
{
   mAutoContext->setType(Map);
}

static void _freeTriples(vector<rdftriple*>& triples)
{
   for(vector<rdftriple*>::iterator i = triples.begin();
       i != triples.end(); ++i)
   {
      rdfa_free_triple(*i);
   }
   triples.clear();
}

RdfaReader::~RdfaReader()
{
   _freeTriples(mDefaultGraph.triples);
   _freeTriples(mProcessorGraph.triples);
   if(mRdfaCtx != NULL)
   {
      rdfa_parse_end(mRdfaCtx);
      rdfa_free_context(mRdfaCtx);
   }
   if(mBaseUri != NULL)
   {
      free(mBaseUri);
   }
}

void RdfaReader::setBaseUri(const char* uri)
{
   if(mBaseUri != NULL)
   {
      free(mBaseUri);
   }
   mBaseUri = strdup(uri);
}

bool RdfaReader::setContext(DynamicObject& context)
{
   bool rval = true;

   // FIXME: validate context, etc.

   mContext = context;

   return rval;
}

bool RdfaReader::setFrame(DynamicObject& frame, bool explicitOnly)
{
   bool rval = true;

   // FIXME: validate frame, etc.

   mDefaultGraphFrame = frame;
   mExplicit = explicitOnly;

   return rval;
}

static char* _applyContext(DynamicObject& ctx, const char* name)
{
   char* rval = NULL;

   // check the whole graph context for a prefix that could shorten the name
   DynamicObjectIterator i = ctx.getIterator();
   while(rval == NULL && i->hasNext())
   {
      DynamicObject& entry = i->next();
      const char* uri = entry["uri"]->getString();
      const char* ptr = strstr(name, uri);
      if(ptr != NULL && ptr == name)
      {
         uint32_t len = entry["len"]->getUInt32();
         uint32_t namelen = strlen(name);
         if(namelen > len)
         {
            // add 2 to make room for null-terminator and colon
            int total = strlen(i->getName()) + (namelen - len) + 2;
            rval = (char*)malloc(total);
            snprintf(rval, total, "%s:%s", i->getName(), ptr + len);
         }
         // do full replacement for rdf-type
         else if(namelen == len && strcmp(i->getName(), "a") == 0)
         {
            rval = strdup("a");
         }
      }
   }

   // no prefixes found to shorten name
   if(rval == NULL)
   {
      rval = strdup(name);
   }

   return rval;
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

static void _setPredicate(
   DynamicObject& s, const char* predicate, const char* object)
{
   DynamicObject obj;
   obj = object;
   _setPredicate(s, predicate, obj);
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
            // FIXME: if both are manual, clone one to handle the cycle?
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
static bool _sortTriples(rdftriple* t1, rdftriple* t2)
{
   bool rval = false;

   // compare subjects
   int c = strcmp(t1->subject, t2->subject);
   if(c < 0)
   {
      // t1 subject < t2 subject
      rval = true;
   }
   else if(c == 0)
   {
      // subjects equal, compare predicates
      c = strcmp(t1->predicate, t2->predicate);
      if(c < 0)
      {
         // t1 predicate < t2 predicate
         rval = true;
      }
      // predicates equal, compare objects
      else if(c == 0)
      {
         rval = (strcmp(t1->object, t2->object) < 0);
      }
   }

   return rval;
}

static void _findTypes(
   DynamicObject& subjects, const char* type,
   DynamicObject& targets, DynamicObject& targetMap, int limit = -1)
{
   DynamicObjectIterator i = subjects.getIterator();
   while((limit == -1 || targets->length() < limit) && i->hasNext())
   {
      DynamicObject& next = i->next();
      if(next->hasMember("a"))
      {
         // "a" is either an array of strings or a string
         DynamicObjectIterator ti = next["a"].getIterator();
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
      // use "@" first, if not present use "a"
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
      else if(frame->hasMember("a"))
      {
         // find all types (limit to the first found if frame parent is a map)
         DynamicObjectIterator i = frame["a"].getIterator();
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

            // skip "@" and "a" predicates
            if(strcmp(p, "@") != 0 && strcmp(p, "a") != 0)
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

static void _frameTarget(
   RdfaReader::Graph* g, DynamicObject& frame,
   DynamicObject& subjects, DynamicObject& embeds,
   bool explicitOnly)
{
   DynamicObject topLevel = subjects;

   // handle frame if one was provided
   if(!frame.isNull())
   {
      // process frame, store removals (removals are stored to avoid removing
      // graph nodes during recursion)
      topLevel = DynamicObject();
      DynamicObject removals;
      removals->setType(Array);
      _processFrame(
         frame, topLevel, subjects, NULL, NULL, embeds, explicitOnly, removals);

      // clean up removals
      _processRemovals(subjects, embeds, removals);
   }

   /* Now that all possible embeds have been marked, we can prune cycles. */
   _pruneCycles(subjects, embeds, explicitOnly);

   // handle all embeds
   _processEmbeds(subjects, embeds);

   // remove "a" from context, only used during processing, its a builtin
   // token replacement for rdf type
   g->target["#"]->removeMember("a");

   // build final JSON-LD object by adding all remaining top-level objects
   DynamicObjectIterator i = topLevel.getIterator();
   while(i->hasNext())
   {
      DynamicObject& subject = i->next();

      // first subgraph to add, so just merge into target
      if(!g->target->hasMember("@"))
      {
         g->target.merge(subject, false);
      }
      // not the first subgraph...
      else
      {
         // change top-level subject into an array
         if(g->target["@"]->getType() != Array)
         {
            DynamicObject tmp = g->target.clone();
            g->target->clear();
            g->target["#"] = tmp["#"];
            tmp->removeMember("#");
            g->target["@"]->append(tmp);
         }
         // add next top-level subgraph
         g->target["@"]->append(subject);
      }
   }
}

static void _finishGraph(
   DynamicObject& context, RdfaReader::Graph* g, DynamicObject& frame,
   bool explicitOnly)
{
   // write context as JSON-LD context in target
   DynamicObjectIterator i = context.getIterator();
   while(i->hasNext())
   {
      DynamicObject& entry = i->next();
      g->target["#"][i->getName()] = entry["uri"].clone();
   }

   // sort triples
   std::sort(g->triples.begin(), g->triples.end(), &_sortTriples);

   // create a mapping of subject to JSON-LD DynamicObject and a
   // mapping of object to info for where to embed the object
   DynamicObject subjects;
   subjects->setType(Map);
   DynamicObject embeds;
   embeds->setType(Map);
   for(RdfaReader::TripleList::iterator ti = g->triples.begin();
       ti != g->triples.end(); ++ti)
   {
      rdftriple* t = *ti;

      // get the abbreviated subject, predicate, and object
      char* subject = _applyContext(context, t->subject);
      char* predicate = _applyContext(context, t->predicate);
      char* object = _applyContext(context, t->object);

      // create/get the subject dyno
      DynamicObject& s = subjects[subject];
      if(!s->hasMember("@"))
      {
         s["@"] = subject;
      }

      // if the object is referenced exactly once and it does not appear
      // in a cyclical reference, then it can be embedded, then it can be
      // embedded under the predicate, so keep track of that
      if(strcmp(predicate, "a") != 0 &&
         g->subjectCounts.find(t->object)->second == 1)
      {
         DynamicObject& embed = embeds[object];
         embed["s"] = s;
         embed["p"] = predicate;
         embed["manual"] = false;
         embed["broken"] = false;
      }

      // add the predicate and object to the subject dyno
      _setPredicate(s, predicate, object);

      free(subject);
      free(predicate);
      free(object);
   }

   // clear triples
   _freeTriples(g->triples);

   /* Note: At this point "subjects" holds a reference to every subject in
      the graph and each of those subjects has all of its predicates. There
      are no embedded objects, but "embeds" contains a list of potential
      objects to embed. Embedding specific objects in the target according to
      a frame is next. */
   _frameTarget(g, frame, subjects, embeds, explicitOnly);
}

static DynamicObject _getExceptionGraph(
   DynamicObject& context, DynamicObject& autoContext, RdfaReader::Graph* g,
   DynamicObject& frame)
{
   DynamicObject rval(NULL);

   // clone contexts
   DynamicObject ctx = context.clone();
   DynamicObject ac = autoContext.clone();

   // merge user-set context over auto-context
   if(!ctx.isNull())
   {
      ac.merge(ctx, false);
   }

   // rebuild context as a map of entries with the length of each uri stored
   // so it doesn't need to be remeasured for each possible triple match
   ctx = DynamicObject();
   ctx->setType(Map);
   DynamicObjectIterator i = ac.getIterator();
   while(i->hasNext())
   {
      DynamicObject entry;
      const char* uri = i->next()->getString();
      entry["uri"] = uri;
      entry["len"] = strlen(uri);
      ctx[i->getName()] = entry;
   }

   // save the old processor target
   DynamicObject target = g->target;

   // finish processor graph
   g->target = DynamicObject();
   g->target->setType(Map);
   _finishGraph(ctx, g, frame, false);
   rval = g->target;

   // reset old target
   g->target = target;

   return rval;
}

bool RdfaReader::start(DynamicObject& dyno)
{
   bool rval = true;

   if(mBaseUri == NULL)
   {
      // reader not started
      ExceptionRef e = new Exception(
         "Cannot start reader, no base URI set yet.",
         RDFA_READER ".InvalidBaseUri");
      Exception::set(e);
      rval = false;
   }
   else
   {
      if(mStarted)
      {
         // free rdfa triples and context
         _freeTriples(mDefaultGraph.triples);
         _freeTriples(mProcessorGraph.triples);
         if(mRdfaCtx != NULL)
         {
            rdfa_parse_end(mRdfaCtx);
            rdfa_free_context(mRdfaCtx);
         }
      }

      // reset auto context
      mAutoContext->clear();

      // reset default graph, set target to output dyno
      mDefaultGraph.subjectCounts.clear();
      mDefaultGraph.target = dyno;

      // reset processor graph
      mProcessorGraph.subjectCounts.clear();
      mProcessorGraph.target = DynamicObject();
      mProcessorGraph.target->setType(Map);

      // "a" is automatically shorthand for rdf type
      mAutoContext["a"] = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";

      // create and setup rdfa context
      mRdfaCtx = rdfa_create_context(mBaseUri);
      if(mRdfaCtx == NULL)
      {
         // reader not started
         ExceptionRef e = new Exception(
            "Failed to create RDFa context.",
            RDFA_READER ".ContextCreationFailure");
         e->getDetails()["baseUri"] = mBaseUri;
         Exception::set(e);
         rval = false;
      }
      else
      {
         mRdfaCtx->callback_data = this;
         rdfa_set_default_graph_triple_handler(
            mRdfaCtx, &RdfaReader::callbackProcessDefaultTriple);
         rdfa_set_processor_graph_triple_handler(
            mRdfaCtx, &RdfaReader::callbackProcessProcessorTriple);

         // try to start parser
         int rc = rdfa_parse_start(mRdfaCtx);
         if(rc != RDFA_PARSE_SUCCESS)
         {
            // reader not started
            ExceptionRef e = new Exception(
               "Could not start RDFa parser.",
               RDFA_READER ".ParseError");
            // TODO: get some error message from the parser
            Exception::set(e);
            rval = false;
         }

         // read started
         mStarted = true;
      }
   }

   return rval;
}

bool RdfaReader::read(InputStream* is)
{
   bool rval = true;

   if(!mStarted)
   {
      // reader not started
      ExceptionRef e = new Exception(
         "Cannot read yet, RdfaReader not started.",
         RDFA_READER ".NotStarted");
      Exception::set(e);
      rval = false;
   }
   else
   {
      char* buf;
      size_t blen;
      int bytes = 0;
      do
      {
         // get rdfa parser buffer
         buf = rdfa_get_buffer(mRdfaCtx, &blen);
         if(buf == NULL)
         {
            // set memory exception
            ExceptionRef e = new Exception(
               "Insufficient memory to parse RDFa.",
               RDFA_READER ".InsufficientMemory");
            Exception::set(e);
            rval = false;
         }
         else
         {
            // read data into buffer
            bytes = is->read(buf, blen);
            if(bytes > 0)
            {
               // parse data
               rval = rdfa_parse_buffer(mRdfaCtx, bytes) == RDFA_PARSE_SUCCESS;
               if(!rval)
               {
                  ExceptionRef e = new Exception(
                     "RDFa parse error.",
                     RDFA_READER ".ParseError");
                  e->getDetails()["graph"] = _getExceptionGraph(
                     mContext, mAutoContext, &mProcessorGraph,
                     mExceptionGraphFrame);
                  Exception::set(e);
               }
            }
            else if(bytes == -1)
            {
               // input stream read error
               rval = false;
            }
         }
      }
      while(rval && bytes > 0);
   }

   return rval;
}

bool RdfaReader::finish()
{
   bool rval = true;

   // finish parsing
   rdfa_parse_end(mRdfaCtx);

   // no longer started
   mStarted = false;

   // merge user-set context over auto-context
   if(!mContext.isNull())
   {
      mAutoContext.merge(mContext, false);
   }

   // rebuild context as a map of entries with the length of each uri stored
   // so it doesn't need to be remeasured for each possible triple match
   mContext = DynamicObject();
   mContext->setType(Map);
   DynamicObjectIterator i = mAutoContext.getIterator();
   while(i->hasNext())
   {
      DynamicObject entry;
      const char* uri = i->next()->getString();
      entry["uri"] = uri;
      entry["len"] = strlen(uri);
      mContext[i->getName()] = entry;
   }

   // finish graphs
   _finishGraph(mContext, &mDefaultGraph, mDefaultGraphFrame, mExplicit);
   _finishGraph(mContext, &mProcessorGraph, mProcessorGraphFrame, false);

   // clear user-set context and parser
   mContext.setNull();
   rdfa_free_context(mRdfaCtx);
   mRdfaCtx = NULL;

   return rval;
}

bool RdfaReader::readFromStream(
   DynamicObject& dyno, InputStream& is,
   const char* baseUri, DynamicObject* frame)
{
   RdfaReader rr;
   rr.setBaseUri(baseUri);
   return
      rr.start(dyno) &&
      ((frame != NULL) ? rr.setFrame(*frame) : true) &&
      rr.read(&is) &&
      rr.finish();
}

bool RdfaReader::readFromString(
   monarch::rt::DynamicObject& dyno, const char* s, size_t slen,
   const char* baseUri, DynamicObject* frame)
{
   ByteArrayInputStream is(s, slen);
   return readFromStream(dyno, is, baseUri, frame);
}

static void _processTriple(RdfaReader::Graph* g, rdftriple* triple)
{
   // update subject count if object is an IRI
   if(triple->object_type == RDF_TYPE_IRI)
   {
      // update map with the number of references to a particular subject
      // using the object of this triple
      if(g->subjectCounts.find(triple->object) == g->subjectCounts.end())
      {
         g->subjectCounts[triple->object] = 1;
      }
      else
      {
         ++g->subjectCounts[triple->object];
      }
   }

   // store triple
   g->triples.push_back(triple);
}

void RdfaReader::processDefaultTriple(rdftriple* triple)
{
   _processTriple(&mDefaultGraph, triple);
}

void RdfaReader::processProcessorTriple(rdftriple* triple)
{
   /* If subject is "@prefix" then add it to the existing graph context if
      it won't overwrite anything, (one might expect the predicate to be
      "@prefix" but its the subject) this isn't a real triple from the
      default graph, its a meta triple that follows the same serialization
      format turtle. It's a bit hackish. */
   if(strcmp(triple->subject, "@prefix") == 0)
   {
      if(!mAutoContext->hasMember(triple->predicate))
      {
         mAutoContext[triple->predicate] = triple->object;
      }
      rdfa_free_triple(triple);
   }
   else
   {
      _processTriple(&mProcessorGraph, triple);
   }
}

void RdfaReader::callbackProcessDefaultTriple(rdftriple* triple, void* reader)
{
   return static_cast<RdfaReader*>(reader)->processDefaultTriple(triple);
}

void RdfaReader::callbackProcessProcessorTriple(rdftriple* triple, void* reader)
{
   return static_cast<RdfaReader*>(reader)->processProcessorTriple(triple);
}
