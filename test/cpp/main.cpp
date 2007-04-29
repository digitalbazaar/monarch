/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>

#include "Base64Coder.h"
#include "Object.h"
#include "Runnable.h"
#include "Thread.h"
#include "System.h"

using namespace std;
using namespace db::rt;
using namespace db::util;

void runBase64Test()
{
	cout << "Running Base64 Test" << endl << endl;
	
	char data[] = {'a', 'b', 'c', 'd'};
	string encoded = Base64Coder::encode(data, 0, 4);
	cout << "encoded=" << encoded << endl;
	
	char* decoded = Base64Coder::decode(encoded);
   int length = sizeof(decoded);
	
	cout << "decoded bytes=" << length << endl;
   for(int i = 0; i < length; i++)
   {
      cout << "decoded[" << i << "]=" << decoded[i] << endl;
   }
	
	string encoded2 = Base64Coder::encode(decoded, 0, 4);
	cout << "encoded again=" << encoded2 << endl;
	
   if(decoded != NULL)
   {
	   delete [] decoded;
   }
}

void runTimeTest()
{
   cout << "Running Time Test" << endl << endl;
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   cout << "Time start=" << start << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   cout << "Time end=" << end << endl;
}

class TestRunnable : public virtual Object, public virtual Runnable
{
   virtual void run()
   {
      string name = Thread::currentThread()->getName();
      cout << name << ": This is a TestRunnable thread." << endl;
      
      if(name == "Thread 1")
      {
         cout << "Thread 1 Waiting for Thread 5..." << endl;
         
         lock();
         {
            lock();
            lock();
            lock();
            wait();
         }
         unlock();
         
         cout << "Thread 1 Finished." << endl;
      }
      else if(name == "Thread 3")
      {
         cout << "Thread 3 Waiting for Thread 5..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            wait();
         }
         unlock();
         
         cout << "Thread 3 Finished." << endl;
      }
      else if(name == "Thread 5")
      {
         cout << "Thread 5 waking up threads..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            notifyAll();
         }
         unlock();
      }
   }
};

void runThreadTest()
{
   cout << "Running Thread Test" << endl << endl;
   
   TestRunnable r1;
   Thread t1(&r1, "Thread 1");
   
   //TestRunnable r2;
   Thread t2(&r1, "Thread 2");
   
   //TestRunnable r3;
   Thread t3(&r1, "Thread 3");
   
   //TestRunnable r4;
   Thread t4(&r1, "Thread 4");
   
   //TestRunnable r5;
   Thread t5(&r1, "Thread 5");
   
   t1.start();
   t2.start();
   t3.start();
   t4.start();
   t5.start();
   
   t1.join();
   t2.join();
   t3.join();
   t4.join();
   t5.join();
}

int main()
{
   cout << "Tests starting..." << endl << endl;
   
   //runBase64Test();
   runTimeTest();
   //runThreadTest();
   
   cout << endl << "Tests finished." << endl;
}
