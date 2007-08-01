/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "StringTokenizer.h"

#include <sstream>

using namespace std;
using namespace db::util;

StringTokenizer::StringTokenizer()
{
   mTokenIndex = 0;
}

StringTokenizer::StringTokenizer(const string& str, char delimiter)
{
   tokenize(str, delimiter);
}

StringTokenizer::~StringTokenizer()
{
}

void StringTokenizer::tokenize(const string& str, char delimiter)
{
   mTokenIndex = 0;
   mTokens.clear();
   
   istringstream iss(str);
   string token;
   while(getline(iss, token, delimiter))
   {
      mTokens.push_back(token);
   }
}

void StringTokenizer::restartTokens()
{
   mTokenIndex = 0;
}

bool StringTokenizer::hasNextToken()
{
   return (mTokenIndex < mTokens.size());
}

const string& StringTokenizer::nextToken()
{
   return mTokens[mTokenIndex++];
}

const string& StringTokenizer::getToken(int i)
{
   const string* rval = NULL;
   
   if(i >= 0)
   {
      rval = &mTokens[i];
   }
   else
   {
      rval = &mTokens[mTokens.size() + i];
   }
   
   return *rval;
}

unsigned int StringTokenizer::getTokenCount()
{
   return mTokens.size();
}
