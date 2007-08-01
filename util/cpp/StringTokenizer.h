/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_StringTokenizer_H
#define db_util_StringTokenizer_H

#include <string>
#include <vector>

namespace db
{
namespace util
{

/**
 * The StringTokenizer tokenizes a string according to some delimiter.
 * 
 * @author Dave Longley
 */
class StringTokenizer
{
protected:
   /**
    * The list of tokens.
    */
   std::vector<std::string> mTokens;
   
   /**
    * The current token index.
    */
   unsigned int mTokenIndex;
   
public:
   /**
    * Creates a new empty StringTokenizer.
    */
   StringTokenizer();
   
   /**
    * Creates a new StringTokenizer that tokenizes the passed string.
    * 
    * @param str the string to tokenize.
    * @param delimiter the delimiter character.
    */
   StringTokenizer(const std::string& str, char delimiter = '\n');
   
   /**
    * Destructs this StringTokenizer.
    */
   virtual ~StringTokenizer();
   
   /**
    * Tokenizes the passed string.
    * 
    * @param str the string to tokenize.
    * @param delimiter the delimiter character.
    */
   virtual void tokenize(const std::string& str, char delimiter = '\n');
   
   /**
    * Restarts the internal current token index.
    */
   virtual void restartTokens();
   
   /**
    * Returns true if there are more tokens available.
    * 
    * @return true if there are more tokens available, false if not.
    */
   virtual bool hasNextToken();
   
   /**
    * Gets the next token.
    * 
    * @return the next token.
    */
   virtual const std::string& nextToken();
   
   /**
    * Gets the token at the given index. A negative index will retrieve
    * from the back of the list of tokens, with -1 retrieving the last token.
    * 
    * @param i the index of the token to retrieve.
    * 
    * @return the token at the given index.
    */
   virtual const std::string& getToken(int i);
   
   /**
    * Gets the number of tokens.
    * 
    * @return the number of tokens.
    */
   virtual unsigned int getTokenCount();
};

} // end namespace util
} // end namespace db
#endif
