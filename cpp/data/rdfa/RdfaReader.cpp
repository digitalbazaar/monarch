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
   _freeTriples(mTriples);
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
         _freeTriples(mTriples);
         if(mRdfaCtx != NULL)
         {
            rdfa_parse_end(mRdfaCtx);
            rdfa_free_context(mRdfaCtx);
         }
      }

      // clear auto-context, set target to output dyno
      mSubjectCounts.clear();
      mAutoContext->clear();
      mTarget = dyno;

      // "a" is automatically shorthand for rdf type
      mAutoContext["a"] = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";

      // create and setup rdfa context
      mRdfaCtx = rdfa_create_context(mBaseUri);
      mRdfaCtx->callback_data = this;
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
               rval = rdfa_parse_buffer(mRdfaCtx, bytes) == RDFA_PARSE_SUCCESS;
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
      }
   }

   // no prefixes found to shorten name
   if(rval == NULL)
   {
      rval = strdup(name);
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
   // so it doesn't need to be remeasured for each possible triple match, also
   // output the JSON-LD context at the same time
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

      // update JSON-LD context
      mTarget["#"][i->getName()] = uri;
   }

   // create a mapping of subject to JSON-LD DynamicObject
   DynamicObject subjects;
   subjects->setType(Map);
   for(TripleList::iterator ti = mTriples.begin(); ti != mTriples.end(); ++ti)
   {
      rdftriple* t = *ti;

      // get the abbreviated subject and predicate
      char* subject = _applyContext(mContext, t->subject);
      char* predicate = _applyContext(mContext, t->predicate);

      // add the property to the subject object
      DynamicObject& s = subjects[t->subject];
      if(!s->hasMember("@"))
      {
         s["@"] = subject;
      }
      s[predicate] = t->object;
      free(subject);
      free(predicate);
   }

   // remove "a" from context, only used during processing, its a builtin
   // token replacement for rdf type
   mTarget["#"]->removeMember("a");

   // build final JSON-LD object, perform nesting and shorten object names
   DynamicObjectIterator si = subjects.getIterator();
   while(si->hasNext())
   {
      DynamicObject& subject = si->next();

      // iterate over properties (predicate=object)
      DynamicObjectIterator oi = subject.getIterator();
      while(oi->hasNext())
      {
         const char* object = oi->next()->getString();
         const char* predicate = oi->getName();
         if(strcmp(predicate, "@") != 0)
         {
            // if the object is a subject in the graph, embed it
            // (clone it to prevent circular references)
            if(subjects->hasMember(object))
            {
               subject[predicate] = subjects[object].clone();
            }
            // object is not a subject in the graph, just abbreviate its name
            else
            {
               object = _applyContext(mContext, object);
               subject[predicate] = object;
               free((char*)object);
            }
         }
      }

      // if a subject is NOT referenced, it will not have been embedded
      // anywhere ... add it as an independent top-level subgraph
      if(mSubjectCounts.find(si->getName()) == mSubjectCounts.end())
      {
         // first subgraph to add, so just merge into target
         if(!mTarget->hasMember("@"))
         {
            mTarget.merge(subject, false);
         }
         // not the first subgraph...
         else
         {
            // change top-level subject into an array
            if(mTarget["@"]->getType() != Array)
            {
               DynamicObject tmp = mTarget.clone();
               mTarget->clear();
               mTarget["#"] = tmp["#"];
               tmp->removeMember("#");
               mTarget["@"]->append(tmp);
            }
            // add next top-level subgraph
            mTarget["@"]->append(subject);
         }
      }
   }

   // clear user-set context, triples, parser
   mContext.setNull();
   _freeTriples(mTriples);
   rdfa_free_context(mRdfaCtx);
   mRdfaCtx = NULL;

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

void RdfaReader::processTriple(rdftriple* triple)
{
   /* If subject is "@prefix" then add it to the existing graph context if
      it won't overwrite anything, (one might expect the predicate to be
      "@prefix" but its the subject ... this isn't a real triple from the
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
      // update map with the number of references to a particular subject
      // using the object of this triple
      SubjectCountMap::iterator i = mSubjectCounts.find(triple->object);
      if(i == mSubjectCounts.end())
      {
         mSubjectCounts[triple->object] = 1;
      }
      else
      {
         ++mSubjectCounts[triple->object];
      }

      // store triple
      mTriples.push_back(triple);
   }
}

void RdfaReader::callbackProcessTriple(rdftriple* triple, void* reader)
{
   return static_cast<RdfaReader*>(reader)->processTriple(triple);
}
