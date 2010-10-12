/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/rdfa/RdfaReader.h"

#include "monarch/util/Convert.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/rt/Exception.h"

#include <cstdlib>
#include <cstdio>

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
}

RdfaReader::~RdfaReader()
{
   if(mRdfaCtx != NULL)
   {
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
         // free context
         rdfa_free_context(mRdfaCtx);
      }

      // clear stack
      mDynoStack.clear();

      // set object as target and push to stack
      mTarget = &dyno;
      mDynoStack.push_back(dyno);

      // create rdfa context
      mRdfaCtx = rdfa_create_context(mBaseUri);
      mRdfaCtx->callback_data = this;

      // set handlers
      rdfa_buffer_filler(mRdfaCtx, &RdfaReader::callbackFillBuffer);
      rdfa_set_triple_handler(mRdfaCtx, &RdfaReader::callbackProcessTriple);

      // try to start parser
      int rc = rdfa_parse_start(mRdfaCtx);
      if(rc != RDFA_PARSE_SUCCESS)
      {
         // reader not started
         ExceptionRef e = new Exception(
            "Could not start rdfa parser.",
            RDFA_READER ".ParseError");
         // TODO: get some error message from the parser
         Exception::set(e);
         rval = false;
      }

      // read started
      mStarted = true;
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
               rval = rdfa_parse_buffer(mRdfaCtx, bytes) != RDFA_PARSE_SUCCESS;
               if(!rval)
               {
                  ExceptionRef e = new Exception(
                     "RDFa parse error.",
                     RDFA_READER ".ParseError");
                  // TODO: add details from error triples generated in librdfa
                  // parser (once that feature is implemented in librdfa)
                  //e->getDetails()["foo"] = "bar";
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

   // no longer started
   mStarted = false;

   // FIXME: do post processing
   // s1 p1 o1
   // s2 p2 o2
   //
   // if s1 == o2, then nest as: s2 p2 {s1 p1 o1}
   // so to build this ... whenever you encounter an object, check a
   // map for a DynamicObject entry there, if its not there, enter a
   // null dyno and add yourself to a vector of dynos to be reprocessed,
   // if it is there, then just store that as your object
   //
   // what about circular references? (just clone stuff)
   // also, if a bnode is referenced more than once, it can't necessarily
   // be nested so easily lest there be confusion over which nodes are
   // the same nodes
   //
   // use "#" instead of separate context object

   return rval;
}

bool RdfaReader::readFromString(
   monarch::rt::DynamicObject& dyno, const char* s, size_t slen)
{
   ByteArrayInputStream is(s, slen);
   RdfaReader rr;
   rr.start(dyno);
   return rr.read(&is) && rr.finish();
}

static inline const char* _applyContext(DynamicObject& ctx, const char* name)
{
   const char* rval = name;

   // FIXME: see if name can be abbreviated via context
   printf("Applying context to: %s\n", name);

   return rval;
}

void RdfaReader::processTriple(rdftriple* triple)
{
   // FIXME: add the triple to the dyno

   // FIXME: keep track of the number of times each node in the graph
   // is referenced, for single references, nest the data, only need to
   // worry about this double-referenced nesting issue for bnodes?

   // FIXME: use context to short triple names, create static function to
   // do that
   printf("triple emitted: %s %s %s\n",
      triple->subject, triple->predicate, triple->object);

   rdfa_free_triple(triple);
}

void RdfaReader::callbackProcessTriple(rdftriple* triple, void* reader)
{
   return static_cast<RdfaReader*>(reader)->processTriple(triple);
}
