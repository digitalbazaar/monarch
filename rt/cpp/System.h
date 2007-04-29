#ifndef System_H
#define System_H

namespace db
{
namespace rt
{

/**
 * The System class has methods for obtaining information or interacting
 * with the system a program is running on.
 * 
 * @author Dave Longley
 */
class System
{
public:
   /**
    * Gets the current time in milliseconds.
    * 
    * @return the current time in milliseconds.
    */
   static unsigned long long getCurrentMilliseconds();
};

} // end namespace rt
} // end namespace db
#endif
