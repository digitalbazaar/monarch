/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_CharacterSetMutator_H
#define db_data_CharacterSetMutator_H

#include "db/io/MutationAlgorithm.h"

#include <iconv.h>
#include <string>

namespace db
{
namespace data
{

/**
 * A CharacterSetMutator is used to convert data from one character set
 * to another.
 *
 * @author Dave Longley
 */
class CharacterSetMutator : public db::io::MutationAlgorithm
{
protected:
   /**
    * The descriptor for the current convert state.
    */
   iconv_t mConvertDescriptor;

   /**
    * Set to true the conversion is finished.
    */
   bool mFinished;

   /**
    * Stores the number of non-reversible conversions performed in
    * the last finished conversion.
    */
   uint32_t mNonReversibles;

public:
   /**
    * Creates a new CharacterSetMutator.
    */
   CharacterSetMutator();

   /**
    * Destructs this CharacterSetMutator.
    */
   virtual ~CharacterSetMutator();

   /**
    * Sets the character sets to convert from and to. If the codes are not
    * supported, an error will be returned.
    *
    * Once this method has been called, this mutator can be re-used as
    * many times as desired, but reset() should be called for new data if
    * the previous data did not finish.
    *
    * @param from the character set code to convert from.
    * @param to the character set code to convert to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool setCharacterSets(const char* from, const char* to);

   /**
    * Resets this mutator so it can convert new data using the same
    * previously set to/from character sets.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool reset();

   /**
    * Gets data out of the source ByteBuffer, mutates it in some implementation
    * specific fashion, and then puts it in the destination ByteBuffer.
    *
    * The return value of this method should be:
    *
    * NeedsData: If this algorithm requires more data in the source buffer to
    * execute its next step.
    *
    * Stepped: If this algorithm had enough data to execute its next step,
    * regardless of whether or not it wrote data to the destination buffer.
    *
    * CompleteAppend: If this algorithm completed and any remaining source data
    * should be appended to the data it wrote to the destination buffer.
    *
    * CompleteTruncate: If this algorithm completed and any remaining source
    * data must be ignored (it *must not* be appended to the data written to
    * the destination buffer). The remaining source data will be untouched so
    * that it can be used for another purpose if so desired.
    *
    * Error: If an exception occurred.
    *
    * Once one a CompleteX result is returned, this method will no longer
    * be called for the same data stream.
    *
    * Note: The source and/or destination buffer may be resized by this
    * algorithm to accommodate its data needs.
    *
    * @param src the source ByteBuffer with bytes to mutate.
    * @param dst the destination ByteBuffer to write the mutated bytes to.
    * @param finish true if there will be no more source data and the mutation
    *               algorithm should finish, false if there is more data.
    *
    * @return the MutationAlgorithm::Result.
    */
   virtual db::io::MutationAlgorithm::Result mutateData(
      db::io::ByteBuffer* src, db::io::ByteBuffer* dst, bool finish);

   /**
    * Returns whether or not this mutator has finished converting.
    *
    * @return true if this mutator is finished.
    */
   virtual bool isFinished();

   /**
    * Gets the number of non-reversible conversions performed so far, since
    * the last call to reset() (or the first use of this mutator instance).
    *
    * @return the number of non-reversible conversions since reset().
    */
   virtual uint32_t getNonReversibleConversions();

   /**
    * A static helper method that converts a string from one character
    * set to another. This method is only useful for character sets that
    * do not allow null characters, as the string parameters rely on null
    * terminators.
    *
    * @param in the input string.
    * @param inCharSet the input character set code.
    * @param out the output string.
    * @param outCharSet the output character set code.
    *
    * @return true if successful, false on error.
    */
   static bool convert(
      const std::string& in, const char* inCharSet,
      std::string& out, const char* outCharSet);
};

} // end namespace io
} // end namespace db
#endif
