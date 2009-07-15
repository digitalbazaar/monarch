/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/data/CharacterSetMutator.h"

#include "db/rt/Exception.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/MutatorInputStream.h"
#include <errno.h>
#include <cstdlib>

using namespace db::data;
using namespace db::io;
using namespace db::rt;

#define INVALID_ICONV ((iconv_t)-1)

CharacterSetMutator::CharacterSetMutator() :
   mConvertDescriptor(INVALID_ICONV),
   mFinished(false),
   mNonReversibles(0)
{
}

CharacterSetMutator::~CharacterSetMutator()
{
   // ensure convert descriptor closed
   if(mConvertDescriptor != INVALID_ICONV)
   {
      iconv_close(mConvertDescriptor);
      mConvertDescriptor = INVALID_ICONV;
   }
}

bool CharacterSetMutator::setCharacterSets(const char* from, const char* to)
{
   bool rval = true;
   
   if(mConvertDescriptor != INVALID_ICONV)
   {
      // close old convert descriptor
      if(iconv_close(mConvertDescriptor) != 0)
      {
         ExceptionRef e = new Exception(
            "Could not close conversion descriptor.",
            "db.data.CharacterSetMutator.CloseError");
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
         rval = false;
      }
      else
      {
         mConvertDescriptor = INVALID_ICONV;
      }
   }
   
   if(rval)
   {
      mConvertDescriptor = iconv_open(to, from);
      if(mConvertDescriptor == INVALID_ICONV)
      {
         ExceptionRef e = new Exception(
            "Could not open conversion descriptor.",
            "db.data.CharacterSetMutator.OpenError");
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
         rval = false;
      }
   }
   
   return rval;
}

bool CharacterSetMutator::reset()
{
   bool rval = false;
   
   if(mConvertDescriptor == INVALID_ICONV)
   {
      ExceptionRef e = new Exception(
         "Could not reset CharacterSetMutator, "
         "no character sets specified yet.",
         "db.data.CharacterSetMutator.NoCharacterSets");
      Exception::set(e);
      rval = false;
   }
   else
   {
      // reset convert state
      if(iconv(mConvertDescriptor, NULL, NULL, NULL, NULL) == ((size_t)-1))
      {
         ExceptionRef e = new Exception(
            "Could not reset CharacterSetMutator.",
            "db.data.CharacterSetMutator.ResetError");
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
         rval = false;
      }
   }
   
   // reset non-reversibles
   mNonReversibles = 0;
   
   return rval;
}

MutationAlgorithm::Result CharacterSetMutator::mutateData(
   ByteBuffer* src, ByteBuffer* dst, bool finish)
{
   MutationAlgorithm::Result rval;
   
   if(!isFinished())
   {
      if(src->isEmpty() && !finish)
      {
         // more data required
         rval = MutationAlgorithm::NeedsData;
      }
      else
      {
// libiconv uses const char* for iconv input glibc uses char* which conforms 
// to POSIX
#ifndef _LIBICONV_VERSION
         char* in;
// the iconv() function on Apple PPC has a different signature than the i386
// version of the header
#elif (_LIBICONV_VERSION == 0x010B) && (__APPLE__) && !defined(__ppc__)
         char* in;
#else
         const char* in;
#endif
         // get in buffer
         in = src->data();
         size_t inBytesLeft = src->length();
         
         // get out buffer
         char* out = dst->end();
         size_t outBytesLeft = dst->freeSpace();
         
         // do character conversion
         size_t count = iconv(
            mConvertDescriptor, &in, &inBytesLeft, &out, &outBytesLeft);
         
         // clear used bytes from source buffer
         src->clear(src->length() - inBytesLeft);
         
         // add written bytes to destination buffer
         dst->extend(dst->freeSpace() - outBytesLeft);
         
         // check conversion result
         if(count == ((size_t)-1))
         {
            switch(errno)
            {
               case EILSEQ:
               {
                  ExceptionRef e = new Exception(
                     "Invalid multibyte sequence.",
                     "db.data.CharacterSetMutator.InvalidMultibyteSequence");
                  e->getDetails()["error"] = strerror(errno);
                  Exception::set(e);
                  rval = MutationAlgorithm::Error;
                  break;
               }
               case EINVAL:
               {
                  // incomplete multibyte sequence encountered, need more
                  // data in the source buffer to convert the charset
                  rval = MutationAlgorithm::NeedsData;
                  break;
               }
               case E2BIG:
               {
                  // not enough room in the output buffer for the data,
                  // but there was plenty of source data... only resize
                  // output buffer and try again if finish is true
                  if(finish)
                  {
                     dst->allocateSpace(src->length() * 2, true);
                     rval = mutateData(src, dst, finish);
                  }
                  else
                  {
                     rval = MutationAlgorithm::Stepped;
                  }
                  break;
               }
               default:
               {
                  ExceptionRef e = new Exception(
                     "Conversion error.",
                     "db.data.CharacterSetMutator.Error");
                  e->getDetails()["error"] = strerror(errno);
                  Exception::set(e);
                  rval = MutationAlgorithm::Error;
                  break;
               }
            }
         }
         else
         {
            // count == number of non-reversible conversions performed
            mNonReversibles += count;
            
            if(finish)
            {
               // now finished
               mFinished = true;
               rval = MutationAlgorithm::CompleteTruncate;
            }
            else
            {
               // did some converting
               rval = MutationAlgorithm::Stepped;
            }
         }
      }
   }
   else
   {
      // algorithm completed
      rval = MutationAlgorithm::CompleteTruncate;
   }
   
   return rval;
}

bool CharacterSetMutator::isFinished()
{
   return mFinished;
}

uint32_t CharacterSetMutator::getNonReversibleConversions()
{
   return mNonReversibles;
}

bool CharacterSetMutator::convert(
   const std::string& in, const char* inCharSet,
   std::string& out, const char* outCharSet)
{
   bool rval = false;
   
   CharacterSetMutator csm;
   if(csm.setCharacterSets(inCharSet, outCharSet))
   {
      out.clear();
      ByteArrayInputStream bais(in.c_str(), in.length());
      MutatorInputStream mis(&bais, false, &csm, false);
      
      int numBytes;
      char* buf = (char*)malloc(1024);
      while((numBytes = mis.read(buf, 1024)) > 0)
      {
         out.append(buf, numBytes);
      }
      free(buf);
      mis.close();
      
      rval = (numBytes != -1);
   }
   
   return rval;
}
