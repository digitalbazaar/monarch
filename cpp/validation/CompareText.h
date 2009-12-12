/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_CompareText_H
#define monarch_validation_CompareText_H

#include "monarch/validation/Validator.h"

namespace monarch
{
namespace validation
{

/**
 * Validates a text string. All CRLF's will be converted to LF's and then all
 * CR's will be converted to LF's to make end of line characters uniform.
 *
 * d => "Apples\r\nAnd\rOranges\n"
 *
 *   CompareText ctt("Apples\nAnd\nOranges\n");
 *   ctt.isValid(d) => true
 *
 *   CompareText ctf("ApplesAndOranges\n");
 *   ctf.isValid(d) => false
 *
 * @author Dave Longley
 */
class CompareText : public Validator
{
protected:
   /**
    * The text to compare against.
    */
   std::string mText;

public:
   /**
    * Creates a new validator.
    *
    * @param text the text to compare against.
    * @param errorMessage a custom error message.
    */
   CompareText(const char* text, const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~CompareText();

   /**
    * Checks if an object is valid.
    *
    * @param obj the object to validate.
    * @param context context to use during validation.
    *
    * @return true if obj is valid, false and exception set otherwise.
    */
   virtual bool isValid(monarch::rt::DynamicObject& obj, ValidatorContext* context);
   using Validator::isValid;
};

} // end namespace validation
} // end namespace monarch
#endif
