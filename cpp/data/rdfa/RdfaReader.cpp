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
   bool rval = false;

   // FIXME: validate context, etc.

   mContext = context;

   return rval;
}

bool RdfaReader::setFrame(DynamicObject& frame, bool explicitOnly)
{
   bool rval = false;

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
// FIXME: remove me
#include "monarch/data/json/JsonWriter.h"
using namespace monarch::data::json;
static void _pruneCycles(
   DynamicObject& subjects, DynamicObject& embeds, bool explicitOnly)
{
   DynamicObject tmp;
   tmp->setType(Map);
   DynamicObjectIterator i = embeds.getIterator();
   while(i->hasNext())
   {
      DynamicObject& embed = i->next();
      printf("EXAMINING EMBED\n");
      JsonWriter::writeToStdOut(embed);
      const char* subject = embed["s"]["@"];
      const char* object = i->getName();

      // only handle embed if not broken and not (explicit and not manual)
      if(!embed["broken"]->getBoolean() &&
         !(explicitOnly && !embed["manual"]->getBoolean()))
      {
         printf("TRYING TO HANDLE EMBED\n");

         // see if object cyclically references subject
         bool manual;
         if(_isCycle(embeds, subject, object, &manual))
         {
            // cycle found
            DynamicObject& cycle = embeds[subject];

            printf("CYCLE FOUND\n");
            JsonWriter::writeToStdOut(cycle);

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
            printf("NO CYCLE, EMBED VALID\n");
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
   DynamicObject& targets, int limit = -1)
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
               i->remove();
               break;
            }
         }
      }
   }
}

static void _findTargetObjects(
   DynamicObject& frameParent, DynamicObject& frame,
   DynamicObject& subjects, const char* subject, const char* predicate,
   DynamicObject& targets, bool explicitOnly, DynamicObject& removals)
{
   // build a list of subjects to look for the target objects in based
   // on subject->predicate (if subject is NULL, use all subjects)
   DynamicObject subjectSet;
   subjectSet->setType(Map);
   DynamicObjectIterator i =
      (subject == NULL) ? subjects.getIterator() :
      subjects[subject][predicate].getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      const char* s = (subject == NULL) ? i->getName() : next->getString();
      // FIXME: going to need to check datatype here
      // to ensure we aren't creating a relationship using
      // a string that isn't a URL
      if(subjects->hasMember(s))
      {
         subjectSet[s] = subjects[s];
      }
   }
   printf("LOOKING IN SUBJECT SET FOR TARGET OBJECTS: subject=%s\n", subject);
   JsonWriter::writeToStdOut(subjectSet);

   // look for the target objects in the subjectSet using the frame
   // use "@" first, if not present use "a"
   if(frame->hasMember("@"))
   {
      const char* s = frame["@"];
      if(subjectSet->hasMember(s))
      {
         targets->append(subjects[s]);
         subjectSet->removeMember(s);
      }
   }
   else if(frame->hasMember("a"))
   {
      // find all types (limit to the first found if frame parent is a map)
      const char* type = frame["a"];
      printf("LOOKING FOR TYPE: %s\n", type);
      _findTypes(
         subjectSet, type, targets, (frameParent->getType() == Map) ? 1 : -1);
   }

   printf("FOUND TARGET OBJECTS:\n");
   JsonWriter::writeToStdOut(targets);

   // if top-level subject or explicit only is on, remove any subjects not
   // marked as targets (targets are removed from the subjectSet)
   if(subject == NULL || explicitOnly)
   {
      printf("TOP-LEVEL OR EXPLICIT ON, ADDING REMOVALS\n");
      JsonWriter::writeToStdOut(subjectSet);

      DynamicObjectIterator si = subjectSet.getIterator();
      while(si->hasNext())
      {
         DynamicObject& next = si->next();
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

static void _processFrameObject(
   DynamicObject& frameParent, DynamicObject& frame,
   DynamicObject& subjects, const char* subject, const char* predicate,
   DynamicObject& embeds, bool explicitOnly, DynamicObject& removals)
{
   printf("PROCESSING FRAME:\n");
   JsonWriter::writeToStdOut(frame);

   // find target objects that match the frame requirements
   DynamicObject targets;
   targets->setType(Array);
   _findTargetObjects(
      frameParent, frame, subjects,
      subject, predicate, targets, explicitOnly, removals);

   // iterate over objects to place in the target
   DynamicObjectIterator i = targets.getIterator();
   while(i->hasNext())
   {
      DynamicObject& target = i->next();

      // if this is the first call (subject == NULL), remove target from embeds
      if(subject == NULL)
      {
         printf("REMOVING EMBED: %s\n", target["@"]->getString());
         embeds->removeMember(target["@"]);
      }
      // add manual embed
      else
      {
         DynamicObject embed;
         embed["s"] = subjects[subject];
         embed["p"] = predicate;
         embed["manual"] = true;
         embeds[target["@"]->getString()] = embed;

         printf("ADDED EMBED: %s\n", target["@"]->getString());
         JsonWriter::writeToStdOut(embed);
      }

      printf("ITERATING OVER TARGET SUBJECT\n");
      JsonWriter::writeToStdOut(target);

      // iterate over predicates and objects in subject
      DynamicObjectIterator oi = target.getIterator();
      while(oi->hasNext())
      {
         DynamicObject& object = oi->next();
         const char* p = oi->getName();

         // skip "@" and "a" predicates
         if(strcmp(p, "@") != 0 && strcmp(p, "a") != 0)
         {
            printf("NEXT PREDICATE: %s\n", p);

            // frame mentions predicate
            if(frame->hasMember(p))
            {
               printf("FRAME MENTIONS PREDICATE: %s\n", p);

               DynamicObject& fobject = frame[p];

               // if frame wants a single value, pick the first one
               if(object->getType() == Array && fobject->getType() != Array)
               {
                  printf("LIMITING TO ONE OBJECT: %s\n", p);
                  object = object[0];
               }
               // recursion required
               else if(fobject->getType() == Map || fobject->getType() == Array)
               {
                  printf("RECURSING AT PREDICATE: %s\n", p);

                  // recurse into frame object
                  if(fobject->getType() == Map)
                  {
                     _processFrameObject(
                        frame, fobject, subjects,
                        target["@"], p, embeds, explicitOnly, removals);
                  }
                  else
                  {
                     // convert object to an array if necessary
                     if(object->getType() != Array)
                     {
                        DynamicObject tmp = object.clone();
                        object->setType(Array);
                        object->append(tmp);

                        printf("CONVERTED TO ARRAY\n");
                        JsonWriter::writeToStdOut(object);
                     }

                     // recurse into frame array
                     DynamicObjectIterator foi = fobject.getIterator();
                     while(foi->hasNext())
                     {
                        DynamicObject nextFrame = foi->next();
                        _processFrameObject(
                           fobject, nextFrame, subjects,
                           target["@"], p, embeds, explicitOnly, removals);
                     }

                     /*
                     // add embed for every object if in non-explicit mode
                     if(fobject->length() == 0 && !explicitOnly)
                     {
                        DynamicObjectIterator ii = object.getIterator();
                        while(ii->hasNext())
                        {
                           const char* oSubject = ii->next();
                           if(!embeds->hasMember(oSubject) ||
                              !embeds[oSubject]["manual"]->getBoolean())
                           {
                              DynamicObject embed;
                              embed["s"] = target;
                              embed["p"] = p;
                              embed["manual"] = true;
                              embeds[oSubject] = embed;

                              printf("ADDED EMBED: %s\n", oSubject);
                              JsonWriter::writeToStdOut(embed);
                           }
                        }
                     }*/
                  }
               }
            }
            // frame does not mention predicate, if in explicit mode, remove it
            else if(explicitOnly)
            {
               printf("EXPLICIT, REMOVED: %s\n", p);
               DynamicObject& remove = removals->append();
               remove["s"] = target["@"];
               remove["p"] = p;
            }
         }
      }
   }
}

static void _processRemovals(
   DynamicObject& subjects, DynamicObject& embeds, DynamicObject& removals)
{
   printf("PROCESSING REMOVALS\n");
   JsonWriter::writeToStdOut(removals);

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

   printf("REMOVALS DONE, RESULTING SUBJECTS\n");
   JsonWriter::writeToStdOut(subjects);
}

static void _processEmbeds(
   DynamicObject& subjects, DynamicObject& embeds)
{
   printf("FINAL EMBEDS:\n");
   JsonWriter::writeToStdOut(embeds);

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
   // handle frame if one was provided
   if(!frame.isNull())
   {
      // process frame, store removals (removals are stored to avoid removing
      // graph nodes during recursion)
      DynamicObject removals;
      removals->setType(Array);

      // frame is a map
      if(frame->getType() == Map)
      {
         _processFrameObject(
            frame, frame, subjects, NULL, NULL, embeds, explicitOnly, removals);
      }
      // frame is an array
      else
      {
         // FIXME: problem ... going to remove all subjects that aren't
         // in the frame here..., need to handle frame arrays recursively
         DynamicObjectIterator i = frame.getIterator();
         while(i->hasNext())
         {
            DynamicObject& nextFrame = i->next();
            _processFrameObject(
               frame, nextFrame, subjects, NULL, NULL, embeds,
               explicitOnly, removals);
         }
      }

      // clean up removals
      _processRemovals(subjects, embeds, removals);
   }

   /* Now that all possible embeds have been marked, we can prune cycles. */
   _pruneCycles(subjects, embeds, explicitOnly);

   // handle all embeds
   _processEmbeds(subjects, embeds);

   printf("FINISHED TOP-LEVEL SUBJECTS\n");
   JsonWriter::writeToStdOut(subjects);

   // remove "a" from context, only used during processing, its a builtin
   // token replacement for rdf type
   g->target["#"]->removeMember("a");

   // build final JSON-LD object by adding all remaining top-level objects
   DynamicObjectIterator i = subjects.getIterator();
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

      // FIXME: don't count subjects that aren't urls

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

bool RdfaReader::readFromString(
   monarch::rt::DynamicObject& dyno, const char* s, size_t slen,
   const char* baseUri)
{
   ByteArrayInputStream is(s, slen);
   RdfaReader rr;
   rr.setBaseUri(baseUri);
   return rr.start(dyno) && rr.read(&is) && rr.finish();
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
