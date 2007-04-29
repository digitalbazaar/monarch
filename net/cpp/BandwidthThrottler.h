#ifndef BandwidthThrottler_H
#define BandwidthThrottler_H

namespace db
{
namespace net
{

/**
 * A BandwidthThrottler is a class that is used to throttle bandwidth.
 * 
 * @author Dave Longley
 */
class BandwidthThrottler
{
protected:
   /**
    * The rate limit for this BandwidthThrottler.
    */
   unsigned long long mRateLimit;
   
   /**
    * The time (in milliseconds) at which a window began for requesting data.
    */
   unsigned long long mWindowTime;
   
   /**
    * The number of bytes that have been granted in the current window.
    */
   unsigned long long mBytesGranted;
   
   /**
    * The last time (in milliseconds) a request was made for bytes.
    */
   unsigned long long mLastRequestTime;
   
   /**
    * The amount of time (in milliseconds) that must pass before a byte
    * is available. This number is never more than 1000 and never less
    * than 1.
    */
   unsigned long long mAvailableByteTime;
   
   /**
    * The number of available bytes.
    */
   unsigned long long mAvailableBytes;
   
   /**
    * Resets the window time.
    */
   void resetWindowTime();
   
   /**
    * Updates the time at which a window for requesting data began --
    * if the number of granted bytes in the current window is high enough.
    */
   void updateWindowTime();
   
   /**
    * Gets the time at which the current window for requesting
    * data began. 
    * 
    * @return the time at which the current window for requesting data began.
    */
   unsigned long long getWindowTime();
   
   /**
    * Updates the amount of time (in milliseconds) that must pass before
    * a byte is available.
    */
   void updateAvailableByteTime();
   
   /**
    * Gets the amount of time (in milliseconds) that must pass before
    * a byte is available. This number is never more than 1000 and never
    * less than 1.
    * 
    * @return the amount of time (in milliseconds) that must pass before
    *         a byte is available.
    */
   unsigned long long getAvailableByteTime();
   
   /**
    * Updates the number of bytes that are currently available.
    */
   void updateAvailableBytes();
   
   /**
    * Gets the number of bytes that are currently available.
    * 
    * @return the number of bytes that are currently available.
    */
   unsigned long long getAvailableBytes();
   
   /**
    * This method blocks until at least one byte is available without
    * violating the rate limit or until the current thread has been
    * interrupted.
    * 
    * @exception InterruptedException thrown if the thread this throttler
    *            is waiting on gets interrupted.
    */
   void limitBandwidth();// throw InterruptedException
   
public:
   /**
    * Creates a new BandwidthThrottler.
    * 
    * @param rateLimit the bytes/second rate limit to use. A value of 0
    *                  indicates no rate limit.
    */
   BandwidthThrottler(unsigned long long rateLimit);
   
   /**
    * Requests the passed number of bytes from this throttler. This method
    * will block until at least one byte can be sent without violating
    * the rate limit or if the current thread has been interrupted.
    * 
    * @param count the number of bytes requested.
    * 
    * @return the number of bytes permitted to send.
    * 
    * @exception InterruptedException thrown if the thread this throttler
    *            is waiting on gets interrupted.
    */
   // FIXME: this method was synchronized in java
   unsigned int requestBytes(unsigned int count);// throw InterruptedException
   
   /**
    * Sets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    * 
    * @param rateLimit the bytes/second rate limit to use.
    */
   // FIXME: this method was synchronized in java
   void setRateLimit(unsigned long long rateLimit);
   
   /**
    * Gets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    * 
    * @return the rate limit in bytes/second.
    */
   unsigned long long getRateLimit();
};

} // end namespace net
} // end namespace db
#endif
