/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
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
   mContext(NULL)
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

static void _setPredicate(
   DynamicObject& s, const char* predicate, DynamicObject& object)
{
   // set the subject's predicate to the embedded object
   if(s->hasMember(predicate))
   {
      if(s[predicate]->getType() != Array)
      {
         s[predicate]->append(s[predicate]);
      }
      s[predicate]->append(object);
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
   DynamicObject& embeds, const char* subject, const char* object)
{
   bool rval = false;

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
   }

   return rval;
}

static void _pruneCycles(DynamicObject& subjects, DynamicObject& embeds)
{
   DynamicObject tmp;
   tmp->setType(Map);
   DynamicObjectIterator i = embeds.getIterator();
   while(i->hasNext())
   {
      DynamicObject& embed = i->next();
      const char* subject = embed["s"]["@"];
      const char* object = i->getName();

      // only handle embed if not broken
      if(!embed["broken"]->getBoolean())
      {
         // see if object cyclically references subject
         if(_isCycle(embeds, subject, object))
         {
            // cycle found, keep current embed, but break cycle at other embed
            tmp[object] = embed;
            DynamicObject& cycle = embeds[subject];
            _setPredicate(cycle["s"], cycle["p"], object);
            cycle["broken"] = true;
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

static void _finishGraph(DynamicObject& context, RdfaReader::Graph* g)
{
   // write context as JSON-LD context in target
   DynamicObjectIterator i = context.getIterator();
   while(i->hasNext())
   {
      DynamicObject& entry = i->next();
      g->target["#"][i->getName()] = entry["uri"].clone();
   }

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
      // in a cyclical reference, then it can be embedded , then it can be
      // embedded under the predicate, so keep track of that
      if(g->subjectCounts.find(t->object)->second == 1)
      {
         DynamicObject& embed = embeds[object];
         embed["s"] = s;
         embed["p"] = predicate;
      }
      // add the predicate and object to the subject dyno
      else
      {
         _setPredicate(s, predicate, object);
      }
      free(subject);
      free(predicate);
      free(object);
   }

   // clear triples
   _freeTriples(g->triples);

   /* Now that all possible embeds have been marked, we can prune cycles. */
   _pruneCycles(subjects, embeds);

   // handle all embeds
   i = embeds.getIterator();
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

   // remove "a" from context, only used during processing, its a builtin
   // token replacement for rdf type
   g->target["#"]->removeMember("a");

   // build final JSON-LD object by adding all top-level subject dynos
   i = subjects.getIterator();
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

static DynamicObject _getExceptionGraph(
   DynamicObject& context, DynamicObject& autoContext, RdfaReader::Graph* g)
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
   _finishGraph(ctx, g);
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
                     mContext, mAutoContext, &mProcessorGraph);
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
   _finishGraph(mContext, &mDefaultGraph);
   _finishGraph(mContext, &mProcessorGraph);

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
