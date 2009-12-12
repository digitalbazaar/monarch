/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Math_H
#define monarch_util_Math_H

#include <math.h>

namespace monarch
{
namespace util
{

/**
 * The Math class provides methods for performing basic math operations.
 *
 * @author Dave Longley
 */
class Math
{
private:
   /**
    * Creates a new Math object.
    */
   Math() {};

public:
   /**
    * The maximum unsigned integer value.
    */
   static const unsigned int MAX_UINT_VALUE = 0xffffffff;

   /**
    * The maximum signed integer value.
    */
   static const int MAX_INT_VALUE = MAX_UINT_VALUE / 2;

   /**
    * The maximum unsigned long long value.
    */
   static const unsigned long long MAX_ULONG_VALUE = 0xffffffffffffffffULL;

   /**
    * The half of the maximum signed long long value.
    */
   static const unsigned long HALF_MAX_LONG_VALUE =
      (unsigned long)(MAX_ULONG_VALUE / 2);

   /**
    * The maximum signed long long value.
    */
   static const long long MAX_LONG_VALUE = HALF_MAX_LONG_VALUE;

   // Note: Commented out because the C functions are simple enough --
   // just use them and they are faster.
//   /**
//    * Rounds a double up to the next largest whole number.
//    *
//    * @param number the double to round up.
//    */
//   static const long double& ceil(const long double& number);
//
//   /**
//    * Rounds a double down to the next smallest whole number.
//    *
//    * @param number the double to round down.
//    */
//   static const long double& floor(const long double& number);
//
//   /**
//    * Given two double longs, the larger is returned.
//    *
//    * @param first the first long.
//    * @param second the second long.
//    *
//    * @return the larger of the two longs.
//    */
//   static const long& maximum(const long& first, const long& second);
//
//   /**
//    * Given two longs, the smaller is returned.
//    *
//    * @param first the first long.
//    * @param second the second long.
//    *
//    * @return the smaller of the two longs.
//    */
//   static const long& minimum(const long& first, const long& second);
//
//   /**
//    * Rounds a double up if it is closer to the next largest whole number,
//    * otherwise rounds the double down.
//    *
//    * @param number the double to round.
//    */
//   static const long double& round(const long double& number);
};

// Note: Commented out because the C functions are simple enough --
// just use them and they are faster.
//inline const long double& Math::ceil(const long double& number)
//{
//   return ceil(number);
//}
//
//inline const long double& Math::floor(const long double& number)
//{
//   return floor(number);
//}
//
//inline const long& Math::maximum(const long& first, const long& second)
//{
//   return (second > first) ? second : first;
//}
//
//inline const long& Math::minimum(const long& first, const long& second)
//{
//   return (second < first) ? second : first;
//}
//
//inline const long double& Math::round(const long double& number)
//{
//   return round(number);
//}

} // end namespace util
} // end namespace monarch
#endif
