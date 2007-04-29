#ifndef Math_H
#define Math_H

#include <math.h>

namespace db
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
public:
   /**
    * The maximum unsigned integer value.
    */
   static const unsigned int MAX_UINT_VALUE = 0xFFFFFFFF;
   
   /**
    * The maximum signed integer value.
    */
   static const int MAX_INT_VALUE = MAX_UINT_VALUE / 2;
   
   /**
    * The maximum unsigned long value.
    */
   static const unsigned long MAX_ULONG_VALUE = 0xFFFFFFFFFFFFFFFFLL;
   
   /**
    * The maximum signed long value.
    */
   static const long MAX_LONG_VALUE = MAX_ULONG_VALUE / 2;
   
   /**
    * Rounds a double up to the next largest whole number.
    * 
    * @param number the double to round up.
    */
   static long double ceil(long double number);
   
   /**
    * Rounds a double down to the next smallest whole number.
    * 
    * @param number the double to round down.
    */
   static long double floor(long double number);
   
   /**
    * Given two double longs, the larger is returned.
    * 
    * @param first the first long.
    * @param second the second long.
    * 
    * @return the larger of the two longs.
    */
   static long max(long first, long second);
   
   /**
    * Given two longs, the smaller is returned.
    * 
    * @param first the first long.
    * @param second the second long.
    * 
    * @return the smaller of the two longs.
    */
   static long min(long first, long second);
   
   /**
    * Rounds a double up if it is closer to the next largest whole number,
    * otherwise rounds the double down.
    * 
    * @param number the double to round. 
    */
   static long double round(long double number);
};

inline long double Math::ceil(long double number)
{
   return ceil(number);
}

inline long double Math::floor(long double number)
{
   return floor(number);
}

inline long Math::max(long first, long second)
{
   return (second > first) ? second : first;
}

inline long Math::min(long first, long second)
{
   return (second < first) ? second : first;
}

inline long double Math::round(long double number)
{
   return round(number);
}

} // end namespace util
} // end namespace db
#endif
