/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_mail_MailTemplateParser_H
#define monarch_mail_MailTemplateParser_H

#include "monarch/mail/Mail.h"
#include "monarch/io/InputStream.h"

namespace monarch
{
namespace mail
{

/**
 * A MailTemplateParser is used to parse templates that contain email messages
 * with specially marked variables. See monarch::data::TemplateInputStream for
 * variable rules. Variables may appear in any part of the email message.
 *
 * In addition to parsing variables, this parser will parse message headers
 * in the template. The parser always starts assuming that the message
 * begins with headers. A blank line should follow the headers to indicate
 * there are no more of them and the body is about to begin.
 *
 * Headers must not contain whitespace and must start at the beginning of a
 * line and end with a colon and a space. The value of the header must end
 * with CR, LF, or CRLF.
 *
 * Once the blank line after the headers has been read, the rest of the message
 * will be interpreted as the email's body.
 *
 * Note: The current implementation assumes an ASCII character encoding. The
 * implementation, however, may not need to change if the text is in UTF-8.
 *
 * @author Dave Longley
 */
class MailTemplateParser
{
public:
   /**
    * Creates a new MailTemplateParser.
    */
   MailTemplateParser();

   /**
    * Destructs this MailTemplateParser.
    */
   virtual ~MailTemplateParser();

   /**
    * Parses the text from the given input stream and writes it out to
    * the passed Mail. The passed "vars" DynamicObject should contain
    * key-value pairs where the key is the name of a variable in the template
    * and the value is the value to replace the template variable with in the
    * actual message.
    *
    * @param mail the Mail to populate.
    * @param vars the key-value variables in the template.
    * @param strict true to raise an exception if the passed variables do not
    *               have a variable that is found in the template, false if not.
    * @param is the InputStream with template content.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parse(
      Mail* mail, monarch::rt::DynamicObject& vars, bool strict,
      monarch::io::InputStream* is);
};

} // end namespace mail
} // end namespace monarch
#endif
