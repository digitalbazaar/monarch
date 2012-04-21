/*
 * Copyright (c) 2010-2012 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/rdfa/RdfaReader.h"

#include "monarch/data/json/JsonLd.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/Convert.h"

#include <cstdio>
#include <rdfa_utils.h>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::data::rdfa;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

#define RDFA_READER      "monarch.data.rdfa.RdfaReader"
#define RDF_TYPE         "http://www.w3.org/1999/02/22-rdf-syntax-ns#type"

RdfaReader::RdfaReader() :
   mStarted(false),
   mBaseUri(NULL),
   mRdfaCtx(NULL),
   mContext(NULL),
   mAutoContext(Map),
   mUseAutoContext(true)
{
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
   free(mBaseUri);
}

void RdfaReader::setBaseUri(const char* uri)
{
   free(mBaseUri);
   mBaseUri = strdup(uri);
}

bool RdfaReader::setContext(DynamicObject& context, bool useAutoContext)
{
   bool rval = true;

   // FIXME: validate context, etc.

   mContext = context;
   mUseAutoContext = useAutoContext;

   return rval;
}

bool RdfaReader::setFrame(DynamicObject& frame, DynamicObject* options)
{
   bool rval = true;

   // FIXME: validate frame, etc.

   mDefaultGraph.frame = frame;
   if(options == NULL)
   {
      mDefaultGraph.frameOptions.setNull();
   }
   else
   {
      mDefaultGraph.frameOptions = *options;
   }

   return rval;
}

static void _setPredicate(
   DynamicObject& s, const char* predicate, DynamicObject& object)
{
   if(s->hasMember(predicate))
   {
      s[predicate].push(object);
   }
   else
   {
      s[predicate] = object;
   }
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

static bool _finishGraph(DynamicObject& ctx, RdfaReader::Graph* g)
{
   bool rval = true;

   // sort triples
   std::sort(g->triples.begin(), g->triples.end(), &_sortTriples);

   // create a mapping of subject to JSON-LD DynamicObject
   DynamicObject subjects(Map);
   for(RdfaReader::TripleList::iterator ti = g->triples.begin();
       ti != g->triples.end(); ++ti)
   {
      rdftriple* t = *ti;

      // get predicate
      const char* predicate = t->predicate;

      // JSON-LD encode object
      DynamicObject object(NULL);
      if(t->object_type == RDF_TYPE_IRI)
      {
         object = DynamicObject();

         // JSON-LD encode type
         if(strcmp(t->predicate, RDF_TYPE) == 0)
         {
            object = t->object;
            predicate = "@type";
         }
         else
         {
            object["@id"] = t->object;
         }
      }
      else if(t->object_type == RDF_TYPE_TYPED_LITERAL)
      {
         object = DynamicObject(Map);
         object["@value"] = t->object;
         object["@type"] = t->datatype;
         if(t->language != NULL && strlen(t->language) > 0)
         {
            object["@language"] = t->language;
         }
      }
      else
      {
         if(t->language != NULL && strlen(t->language) > 0)
         {
            object = DynamicObject(Map);
            object["@value"] = t->object;
            object["@language"] = t->language;
         }
         else
         {
            object = DynamicObject(String);
            object = t->object;
         }
      }

      // create/get the subject dyno
      DynamicObject& s = subjects[t->subject];
      if(!s->hasMember("@id"))
      {
         // JSON-LD encode subject
         s["@id"] = t->subject;
      }

      // add the predicate and object to the subject dyno
      _setPredicate(s, predicate, object);
   }

   // clear triples
   _freeTriples(g->triples);

   /* Note: At this point "subjects" holds a reference to every subject in
      the graph and each of those subjects has all of its predicates. Embedding
      specific objects in the target according to a frame is next, followed
      by adding the specific context. */
   if(rval)
   {
      DynamicObject out;
      if(!g->frame.isNull())
      {
         rval = JsonLd::frame(
            subjects.values(), g->frame, g->frameOptions, out);
         if(rval && out.isNull())
         {
            out = DynamicObject(g->frame->getType());
         }
      }
      else
      {
         rval = JsonLd::compact(
            subjects.values(), ctx, DynamicObject(Map), out);
      }

      if(rval)
      {
         // set target to output, preserving original dyno reference
         *(g->target) = *out;
      }
   }

   return rval;
}

static DynamicObject _getExceptionGraph(
   DynamicObject& context, DynamicObject& autoContext, RdfaReader::Graph* g)
{
   DynamicObject rval(NULL);

   // clone auto context
   DynamicObject ctx = autoContext.clone();

   // use user-set context
   if(!context.isNull())
   {
      ctx = context.clone();
   }

   // save the old processor target and frame
   DynamicObject target = g->target;
   DynamicObject frame = g->frame;

   // use frame to embed error context in exception
   g->frame = DynamicObject();
   //g->frame["@context"] = JsonLd::createDefaultContext();
   g->frame["@type"] =
      "http://www.w3.org/ns/rdfa_processing_graph#Error";
   g->frame["http://www.w3.org/ns/rdfa_processing_graph#context"]->setType(Map);

   // finish processor graph
   g->target = DynamicObject();
   _finishGraph(ctx, g);
   rval = g->target;

   // reset old target and frame
   g->target = target;
   g->frame = frame;

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

      // reset state
      mAutoContext->clear();
      mDefaultGraph.target = dyno;
      mProcessorGraph.target = DynamicObject();

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

   // create context to finish graph
   // use auto-context if requested
   DynamicObject ctx(NULL);
   if(mUseAutoContext)
   {
      ctx = mAutoContext.clone();
   }
   else if(!mContext.isNull())
   {
      ctx = mContext.clone();
   }

   if(rval)
   {
      // finish graphs
      _finishGraph(ctx, &mDefaultGraph);
      _finishGraph(ctx, &mProcessorGraph);

      // clear parser
      rdfa_free_context(mRdfaCtx);
      mRdfaCtx = NULL;
   }

   return rval;
}

bool RdfaReader::readFromStream(
   DynamicObject& dyno, InputStream& is,
   const char* baseUri, DynamicObject* context,
   DynamicObject* frame, DynamicObject* options)
{
   RdfaReader rr;
   rr.setBaseUri(baseUri);
   if(context != NULL)
   {
      rr.setContext(*context);
   }
   return
      rr.start(dyno) &&
      ((frame != NULL) ? rr.setFrame(*frame, options) : true) &&
      rr.read(&is) &&
      rr.finish();
}

bool RdfaReader::readFromString(
   monarch::rt::DynamicObject& dyno, const char* s, size_t slen,
   const char* baseUri, DynamicObject* context,
   DynamicObject* frame, DynamicObject* options)
{
   ByteArrayInputStream is(s, slen);
   return readFromStream(dyno, is, baseUri, context, frame, options);
}

void RdfaReader::processDefaultTriple(rdftriple* triple)
{
   // store triple
   mDefaultGraph.triples.push_back(triple);
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
      // add to auto-context if requested
      if(mUseAutoContext && !mAutoContext->hasMember(triple->predicate))
      {
         mAutoContext[triple->predicate] = triple->object;
      }
      rdfa_free_triple(triple);
   }
   else
   {
      // store triple
      mProcessorGraph.triples.push_back(triple);
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
