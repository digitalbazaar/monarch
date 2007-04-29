/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>

#include "Base64Coder.h"
#include "Runnable.h"
#include "Thread.h"
#include "System.h"

using namespace std;
using namespace db::system;
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

class TestRunnable : public Runnable
{
   virtual void run()
   {
      cout << Thread::currentThread()->getName() <<
         ": This is a TestRunnable thread." << endl;
   }
};

void runThreadTest()
{
   cout << "Running Thread Test" << endl << endl;
   
   TestRunnable r1;
   Thread t1(&r1);
   
   TestRunnable r2;
   Thread t2(&r2);
   
   TestRunnable r3;
   Thread t3(&r3);
   
   TestRunnable r4;
   Thread t4(&r4);
   
   t1.start();
   t2.start();
   t3.start();
   t4.start();
   
   t1.join();
   t2.join();
   t3.join();
   t4.join();
}

int main()
{
   cout << "Tests starting..." << endl << endl;
   
   //runBase64Test();
   //runTimeTest();
   runThreadTest();
   
   cout << endl << "Tests finished." << endl;
}
