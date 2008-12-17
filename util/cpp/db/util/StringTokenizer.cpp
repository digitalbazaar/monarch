/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/StringTokenizer.h"

#include <string>
#include <cstdlib>
#include <cstring>

using namespace db::util;

StringTokenizer::StringTokenizer()
{
   mFirstToken = mLastToken = mNextToken = mPrevToken = NULL;
   mTokenCount = 0;
   
   mFirstFreeToken = NULL;
   mFreeTokenCount = 0;
}

StringTokenizer::StringTokenizer(const char* str, char delimiter, bool front)
{
   mFirstToken = mLastToken = mNextToken = mPrevToken = NULL;
   mTokenCount = 0;
   
   mFirstFreeToken = NULL;
   mFreeTokenCount = 0;
   
   tokenize(str, delimiter, front);
}

StringTokenizer::~StringTokenizer()
{
   // clean up all used tokens
   cleanupStringTokens(mFirstToken);
   
   // clean up all free tokens
   cleanupStringTokens(mFirstFreeToken);
}

void StringTokenizer::cleanupStringTokens(StringToken* first)
{
   // clean up all StringTokens
   StringToken* next;
   while(first != NULL)
   {
      next = first->next;
      free(first->data);
      delete first;
      first = next;
   }
}

void StringTokenizer::tokenize(const char* str, char delimiter, bool front)
{
   // move tokens into free-list
   StringToken* next;
   while(mFirstToken != NULL && mFreeTokenCount < 100)
   {
      // save first free token
      next = mFirstFreeToken;
      
      // update first free token to first used token
      mFirstFreeToken = mFirstToken;
      
      // update first used token to next used token
      mFirstToken = mFirstToken->next;
      
      // update next free token to old first free token
      mFirstFreeToken->next = next;
      
      // update free token count
      mFreeTokenCount++;
   }
   
   if(mFirstToken != NULL)
   {
      // clean up remaining first tokens
      cleanupStringTokens(mFirstToken);
   }
   
   // reset token vars
   mFirstToken = mLastToken = mNextToken = mPrevToken = NULL;
   mTokenCount = 0;
   
   // find tokens in the passed string
   StringToken* token;
   const char* start = str;
   const char* end;
   while(start != NULL)
   {
      // increase token count
      mTokenCount++;
      
      // acquire a StringToken
      if(mFirstFreeToken != NULL)
      {
         // grab one from the free-list
         token = mFirstFreeToken;
         mFirstFreeToken = mFirstFreeToken->next;
         token->next = NULL;
         token->prev = NULL;
         mFreeTokenCount--;
         
         // free data in token
         free(token->data);
      }
      else
      {
         // allocate a new one
         token = new StringToken();
         token->next = NULL;
         token->prev = NULL;
         token->data = NULL;
      }
      
      // find the end of the token
      end = strchr(start, delimiter);
      if(end != NULL)
      {
         // copy data into token
         token->data = (char*)realloc(token->data, (end - start) + 1);
         strncpy(token->data, start, end - start);
         token->data[(end - start)] = 0;
         
         // move start pointer
         start = end + 1;
      }
      else
      {
         // copy data into token
         size_t length = strlen(start);
         token->data = (char*)realloc(token->data, length + 1);
         strncpy(token->data, start, length);
         token->data[length] = 0;
         
         // move start pointer
         start = end;
      }
      
      // append token
      if(mFirstToken == NULL)
      {
         mFirstToken = mLastToken = token;
      }
      else
      {
         token->prev = mLastToken;
         mLastToken->next = token;
         mLastToken = token;
      }
   }
   
   restartTokens(front);
}

inline void StringTokenizer::restartTokens(bool front)
{
   if(front)
   {
      mNextToken = mFirstToken;
      mPrevToken = NULL;
   }
   else
   {
      mPrevToken = mLastToken;
      mNextToken = NULL;
   }
}

inline bool StringTokenizer::hasNextToken()
{
   return mNextToken != NULL;
}

inline bool StringTokenizer::hasPreviousToken()
{
   return mPrevToken != NULL;
}

const char* StringTokenizer::nextToken()
{
   const char* rval = mNextToken->data;
   mPrevToken = mNextToken;
   mNextToken = mNextToken->next;
   return rval;
}

const char* StringTokenizer::previousToken()
{
   const char* rval = mPrevToken->data;
   mNextToken = mPrevToken;
   mPrevToken = mPrevToken->prev;
   return rval;
}

const char* StringTokenizer::getToken(int i)
{
   const char* rval = NULL;
   
   StringToken* token;
   if(i < 0)
   {
      i = -i;
      token = mLastToken;
      for(int n = 1; n < i && token != NULL; n++)
      {
         token = token->prev;
      }
   }
   else
   {
      token = mFirstToken;
      for(int n = 0; n < i && token != NULL; n++)
      {
         token = token->next;
      }
   }
   
   if(token != NULL)
   {
      rval = token->data;
   }
   
   return rval;
}

inline unsigned int StringTokenizer::getTokenCount()
{
   return mTokenCount;
}
