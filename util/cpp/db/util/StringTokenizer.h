/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_StringTokenizer_H
#define db_util_StringTokenizer_H

namespace db
{
namespace util
{

/**
 * A StringToken consists of the string data for the token and a
 * pointer to the next StringToken.
 * 
 * @author Dave Longley
 */
struct StringToken
{
   /**
    * The string data for this token.
    */
   char* data;
   
   /**
    * A pointer to the next StringToken.
    */
   StringToken* next;
};

/**
 * The StringTokenizer tokenizes a string according to some delimiter.
 * 
 * The current implementation maintains a list of StringToken objects that
 * contain the actual string token and a pointer to the next StringToken.
 * 
 * When StringTokens are discarded they are put in a free list that can
 * contain up to 100 free tokens for later quick use.
 * 
 * @author Dave Longley
 */
class StringTokenizer
{
protected:
   /**
    * A pointer to the first token.
    */
   StringToken* mFirstToken;
   
   /**
    * A pointer to the last token.
    */
   StringToken* mLastToken;
   
   /**
    * The current token.
    */
   StringToken* mCurrentToken;
   
   /**
    * The number of tokens.
    */
   unsigned int mTokenCount;
   
   /**
    * A pointer to the first free-list token.
    */
   StringToken* mFirstFreeToken;
   
   /**
    * The number of free-list tokens.
    */
   unsigned int mFreeTokenCount;
   
   /**
    * Cleans up the passed list of StringTokens.
    * 
    * @param first the pointer to the first StringToken in a list.
    */
   virtual void cleanupStringTokens(StringToken* first);
   
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
   StringTokenizer(const char* str, char delimiter = '\n');
   
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
   virtual void tokenize(const char* str, char delimiter = '\n');
   
   /**
    * Restarts the internal current token pointer.
    */
   virtual void restartTokens();
   
   /**
    * Returns true if there are more tokens available.
    * 
    * @return true if there are more tokens available, false if not.
    */
   virtual bool hasNextToken();
   
   /**
    * Gets the next token. If there isn't a next token, good luck because
    * it's going to do nasty NULL things.
    * 
    * @return the next token.
    */
   virtual const char* nextToken();
   
   /**
    * Gets the token at the given index. A negative index will retrieve
    * from the back of the list of tokens, with -1 retrieving the last token.
    * 
    * Since the internal implementation for this StringTokenizer uses a
    * linked-list, this method is not preferred for speed.
    * 
    * @param i the index of the token to retrieve.
    * 
    * @return the token at the given index or NULL if the index is invalid.
    */
   virtual const char* getToken(int i);
   
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
