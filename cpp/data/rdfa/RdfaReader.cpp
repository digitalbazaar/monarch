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

static void _finishGraph(DynamicObject& context, RdfaReader::Graph* g)
{
   // write context as JSON-LD context in target
   DynamicObjectIterator i = context.getIterator();
   while(i->hasNext())
   {
      DynamicObject& entry = i->next();
      g->target["#"][i->getName()] = entry["uri"].clone();
   }

   // create a mapping of subject to JSON-LD DynamicObject
   DynamicObject subjects;
   subjects->setType(Map);
   for(RdfaReader::TripleList::iterator ti = g->triples.begin();
       ti != g->triples.end(); ++ti)
   {
      rdftriple* t = *ti;

      // get the abbreviated subject and predicate
      char* subject = _applyContext(context, t->subject);
      char* predicate = _applyContext(context, t->predicate);

      // add the property to the subject object
      DynamicObject& s = subjects[t->subject];
      if(!s->hasMember("@"))
      {
         s["@"] = subject;
      }
      if(s->hasMember(predicate))
      {
         if(s[predicate]->getType() != Array)
         {
            DynamicObject tmp = s[predicate].clone();
            s[predicate]->append(tmp);
         }
         s[predicate]->append(t->object);
      }
      else
      {
         s[predicate] = t->object;
      }
      free(subject);
      free(predicate);
   }

   // remove "a" from context, only used during processing, its a builtin
   // token replacement for rdf type
   g->target["#"]->removeMember("a");

   // build final JSON-LD object, perform nesting and shorten object names
   DynamicObjectIterator si = subjects.getIterator();
   while(si->hasNext())
   {
      DynamicObject& subject = si->next();

      // iterate over properties (predicate=object)
      DynamicObjectIterator pi = subject.getIterator();
      while(pi->hasNext())
      {
         DynamicObject next = pi->next();
         const char* predicate = pi->getName();
         if(strcmp(predicate, "@") != 0)
         {
            // next is either an object or an array of objects (predicates
            // can point at N objects), so iterate, which will work either way
            DynamicObjectIterator oi = next.getIterator();
            for(int idx = 0; oi->hasNext(); ++idx)
            {
               DynamicObject& object = oi->next();
               // if the object is a subject in the graph that is referenced
               // exactly once then embed it
               // (clone it to prevent circular references)
               RdfaReader::SubjectCountMap::iterator ci =
                  g->subjectCounts.find(object);
               if(ci != g->subjectCounts.end() && ci->second == 1 &&
                  subjects->hasMember(object))
               {
                  DynamicObject obj = subjects[object->getString()].clone();
                  if(subject[predicate]->getType() == Array)
                  {
                     subject[predicate][idx] = obj;
                  }
                  else
                  {
                     subject[predicate] = obj;
                  }
               }
               // object cannot/should not be embedded, just abbreviate its name
               else
               {
                  const char* abbr = _applyContext(context, object);
                  if(subject[predicate]->getType() == Array)
                  {
                     subject[predicate][idx] = abbr;
                  }
                  else
                  {
                     subject[predicate] = abbr;
                  }
                  free((char*)abbr);
               }
            }
         }
      }

      // if a subject is NOT referenced or it is referenced more than once,
      // then it will not have been embedded anywhere ... add at the top-level
      RdfaReader::SubjectCountMap::iterator ci =
         g->subjectCounts.find(si->getName());
      if(ci == g->subjectCounts.end() || ci->second != 1)
      {
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

   // clear triples
   _freeTriples(g->triples);
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
   RdfaReader::SubjectCountMap::iterator i =
      g->subjectCounts.find(triple->object);
   if(i == g->subjectCounts.end())
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
