/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_mail_MailTemplateParser_H
#define db_mail_MailTemplateParser_H

#include "db/mail/Mail.h"
#include "db/io/InputStream.h"

namespace db
{
namespace mail
{

/**
 * A MailTemplateParser is used to parse templates that contain email messages
 * with specially marked variables.
 * 
 * A template is a string of text with variable names that are delimited by
 * starting with a '$' and ending with a non-alpha-numeric character (or the
 * end of the text). If the starting '$' is preceeded by a '\' then it will
 * be interpreted as a regular '$' not the beginning of a variable name. If
 * a '\' is to appear in the message, it must be escaped like so: '\\'.
 * 
 * The variable values are stored in a DynamicObject. If the variable value
 * is a basic value like a string, boolean, or number, then the variable name
 * will simply be replaced with the value from the DynamicObject. If the
 * value is a map, then the variable name will be replaced by a comma-delimited
 * list of key-value pairs. If the value is an array, then the variable name
 * will be replaced with a comma-delimited list of values.
 * 
 * In addition to parsing the above variables, this parser will parse message
 * headers in the template. The parser always starts assuming that the message
 * begins with headers, with the last header being the 'Subject' header. It is
 * case-sensitive.
 * 
 * Headers must not contain whitespace and must start at the beginning of a
 * line and end with a colon and a space. They can be variables. The value of
 * the header can contain variables and must end with CR, LF, or CRLF.
 * 
 * Once the 'Subject' header has been read, the rest of the message will be
 * interpreted as the email's body. Again, the 'Subject' header must be
 * present, even if its value is empty, as it delimits the headers section.
 * 
 * Note: The current implementation assumes an ASCII character encoding. The
 * implementation, however, may not need to change if the text is in UTF-8.
 * 
 * @author Dave Longley
 */
class MailTemplateParser
{
protected:
   /**
    * Parses a single line from the template and adds its contents to the
    * passed Mail either as a header or as a line of the message body.
    * Template variables are replaced according to the passed "vars"
    * DynamicObject and the headers flag is cleared once the "Subject" header
    * has been parsed.
    * 
    * @param mail the Mail to populate.
    * @param vars the key-value variables in the template.
    * @param line the line to parse.
    * @param headers the flag to clear once the headers have been parsed.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parseLine(
      Mail* mail, db::rt::DynamicObject& vars,
      const char* line, bool& headers);
   
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
    * @param is the InputStream with template content.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool parse(
      Mail* mail, db::rt::DynamicObject& vars, db::io::InputStream* is);
};

} // end namespace mail
} // end namespace db
#endif
