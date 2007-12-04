/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/engine.h>

#include "db/test/Test.h"
#include "db/test/TestRunner.h"
#include "db/util/Base64Codec.h"
#include "db/rt/Object.h"
#include "db/rt/Runnable.h"
#include "db/rt/Thread.h"
#include "db/rt/Semaphore.h"
#include "db/rt/System.h"
#include "db/rt/JobDispatcher.h"
#include "db/util/Crc16.h"
#include "db/util/StringTools.h"
#include "db/util/DynamicObject.h"
#include "db/util/DynamicObjectIterator.h"
#include "db/net/TcpSocket.h"
#include "db/net/UdpSocket.h"
#include "db/net/DatagramSocket.h"
#include "db/net/Internet6Address.h"
#include "db/net/SslSocket.h"
#include "db/crypto/MessageDigest.h"
#include "db/crypto/AsymmetricKeyFactory.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/FileList.h"
#include "db/crypto/DigitalEnvelope.h"
#include "db/crypto/DigitalSignatureInputStream.h"
#include "db/crypto/DigitalSignatureOutputStream.h"
#include "db/crypto/DefaultBlockCipher.h"
#include "db/util/Convert.h"
#include "db/net/Url.h"
//#include "db/util/regex/Pattern.h"
#include "db/util/Date.h"
#include "db/net/http/HttpHeader.h"
#include "db/net/http/HttpRequest.h"
#include "db/net/http/HttpResponse.h"
#include "db/net/http/HttpConnectionServicer.h"
#include "db/net/http/HttpRequestServicer.h"
#include "db/net/http/HttpClient.h"
#include "db/modest/Kernel.h"
#include "db/net/Server.h"
#include "db/net/NullSocketDataPresenter.h"
#include "db/net/SslSocketDataPresenter.h"
#include "db/net/SocketDataPresenterList.h"
#include "db/util/StringTokenizer.h"
#include "db/io/FilterOutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/data/xml/XmlReader.h"
#include "db/data/xml/XmlWriter.h"
#include "db/data/xml/XmlBindingInputStream.h"
#include "db/data/xml/XmlBindingOutputStream.h"
#include "db/data/DataMappingFunctor.h"
#include "db/data/DynamicObjectWriter.h"
#include "db/data/DynamicObjectReader.h"
#include "db/data/DynamicObjectBinding.h"
#include "db/io/OStreamOutputStream.h"
#include "db/crypto/BigDecimal.h"
#include "db/io/ByteArrayOutputStream.h"
#include "db/io/ByteBuffer.h"
#include "db/io/BufferedOutputStream.h"
#include "db/sql/Row.h"
#include "db/sql/RowObject.h"
#include "db/sql/sqlite3/Sqlite3Connection.h"
#include "db/sql/sqlite3/Sqlite3ConnectionPool.h"
#include "db/sql/mysql/MySqlConnection.h"
#include "db/sql/mysql/MySqlConnectionPool.h"
#include "db/sql/util/DatabaseClient.h"
#include "db/event/Observable.h"
#include "db/event/ObserverDelegate.h"
#include "db/event/EventController.h"
//#include "db/logging/Logger.h"
//#include "db/logging/OutputStreamLogger.h"
//#include "db/logging/FileLogger.h"
#include "db/util/UniqueList.h"
#include "db/data/json/JsonWriter.h"
#include "db/data/json/JsonReader.h"
#include "db/mail/SmtpClient.h"
#include "db/mail/MailTemplateParser.h"
#include "db/config/ConfigManager.h"

using namespace std;
using namespace db::test;
using namespace db::config;
using namespace db::crypto;
using namespace db::event;
using namespace db::io;
using namespace db::modest;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;
using namespace db::util::regex;
using namespace db::data;
using namespace db::data::xml;
using namespace db::data::json;
using namespace db::sql::sqlite3;
using namespace db::sql::mysql;
using namespace db::sql::util;
//using namespace db::logging;

// WTF? this is required to get static library building for unknown reason
#include "db/io/PeekInputStream.h"
PeekInputStream g_junk1(NULL, false);
OperationList g_junk2;
NullSocketDataPresenter g_junk3;
StringTokenizer g_junk4;
FilterOutputStream g_junk5(NULL, false);
ByteArrayInputStream g_junk6(NULL, 0);
IgnoreOutputStream g_junk7(NULL);
BufferedOutputStream g_junk8(NULL, NULL, false);

void runTimeTest()
{
   cout << "Running Time Test" << endl << endl;
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   cout << "Time start=" << start << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   cout << "Time end=" << end << endl;
   
   cout << endl << "Time Test complete." << endl;
}

class TestRunnable : public virtual Object, public Runnable
{
public:
   bool mustWait;
   
   TestRunnable()
   {
      mustWait = true;
   }
   
   virtual ~TestRunnable()
   {
   }
   
   virtual void run()
   {
      Thread* t = Thread::currentThread();
      string name = t->getName();
      //cout << name << ": This is a TestRunnable thread,addr=" << t << endl;
      
      if(name == "Thread 1")
      {
         //cout << "Thread 1 Waiting for interruption..." << endl;
         InterruptedException* e = NULL;
         
         lock();
         {
            lock();
            lock();
            lock();
            e = wait();
            // thread 1 should be interrupted
            assert(e != NULL);
            unlock();
            unlock();
            unlock();
         }
         unlock();
         
//         if(Thread::interrupted())
//         {
//            cout << "Thread 1 Interrupted. Exception message="
//                 << e->getMessage() << endl;
//         }
//         else
//         {
//            cout << "Thread 1 Finished." << endl;
//         }
      }
      else if(name == "Thread 2")
      {
         //cout << "Thread 2 Finished." << endl;
      }
      else if(name == "Thread 3")
      {
         //cout << "Thread 3 Waiting for Thread 5..." << endl;
         
         lock();
         lock();
         lock();
         {
            //cout << "Thread 3 starting wait..." << endl;
            while(mustWait)
            {
               // thread 3 should be notified, not interrupted
               assert(wait(5000) == NULL);
            }
            //cout << "Thread 3 Awake!" << endl;
         }
         unlock();
         unlock();
         unlock();
         
//         if(Thread::interrupted())
//         {
//            cout << "Thread 3 Interrupted." << endl;
//         }
//         else
//         {
//            cout << "Thread 3 Finished." << endl;
//         }         
      }
      else if(name == "Thread 4")
      {
         //cout << "Thread 4 Finished." << endl;
      }
      else if(name == "Thread 5")
      {
         //cout << "Thread 5 waking up a thread..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            // wait for a moment
            Thread::sleep(100);
            mustWait = false;
            //cout << "Thread 5 notifying a thread..." << endl;
            notifyAll();
            //cout << "Thread 5 notified another thread." << endl;
         }
         unlock();
         unlock();
         unlock();
         unlock();
         
         //cout << "Thread 5 Finished." << endl;
      }
   }
};

void runThreadTest(TestRunner& tr)
{
   tr.test("Thread");
   
   //cout << "Running Thread Test" << endl << endl;
   
   TestRunnable r1;
   Thread t1(&r1, "Thread 1");
   Thread t2(&r1, "Thread 2");
   Thread t3(&r1, "Thread 3");
   Thread t4(&r1, "Thread 4");
   Thread t5(&r1, "Thread 5");
   
   //cout << "Threads starting..." << endl;
   
   t1.start();
   t2.start();
   t3.start();
   t4.start();
   t5.start();
   
   t1.interrupt();
   
   t2.join();
   t3.join();
   t1.join();
   t4.join();
   t5.join();
   
   tr.pass();
   
   //cout << endl << "Thread Test complete." << endl;
}

class TestJob : public Runnable
{
public:
   string mName;
   
   TestJob(const string& name)
   {
      mName = name;
   }
   
   virtual ~TestJob()
   {
   }
   
   virtual void run()
   {
      //cout << endl << "TestJob: Running a job,name=" << mName << endl;
      
      if(mName == "1")
      {
         Thread::sleep(375);
      }
      else if(mName == "2")
      {
         Thread::sleep(125);
      }
      else
      {
         Thread::sleep(125);
      }
      
      //cout << endl << "TestJob: Finished a job,name=" << mName << endl;
   }
};

void runJobThreadPoolTest(TestRunner& tr)
{
   tr.test("JobThreadPool");
   
   Exception::clearLast();
   
   //cout << "Running JobThreadPool Test" << endl << endl;
   
   // create a job thread pool
   JobThreadPool pool(3);
   
   // create jobs
   TestJob job1("1");
   TestJob job2("2");
   TestJob job3("3");
   TestJob job4("4");
   TestJob job5("5");
   
   // run jobs
   pool.runJob(&job1);
   pool.runJob(&job2);
   pool.runJob(&job3);
   pool.runJob(&job4);
   pool.runJob(&job5);
   
   // wait
   //cout << "Waiting for jobs to complete..." << endl;
   Thread::sleep(1250);
   //cout << "Finished waiting for jobs to complete." << endl;
   
   // terminate all jobs
   pool.terminateAllThreads();
   
   tr.passIfNoException();
   
   //cout << endl << "JobThreadPool Test complete." << endl << endl;
}

void runJobDispatcherTest(TestRunner& tr)
{
   tr.test("JobDispatcher");
   
   Exception::clearLast();
   
   //cout << "Running JobDispatcher Test" << endl << endl;
   
   // create a job dispatcher
   //JobDispatcher jd;
   JobThreadPool pool(3);
   JobDispatcher jd(&pool, false);
   
   // create jobs
   TestJob job1("1");
   TestJob job2("2");
   TestJob job3("3");
   TestJob job4("4");
   TestJob job5("5");
   TestJob job6("6");
   
   // queue jobs
   jd.queueJob(&job1);
   jd.queueJob(&job2);
   jd.queueJob(&job3);
   jd.queueJob(&job4);
   jd.queueJob(&job5);
   jd.queueJob(&job6);
   
   // start dispatching
   jd.startDispatching();
   
   // wait
   //cout << "Waiting 10 seconds for jobs to complete..." << endl;
   Thread::sleep(1250);
   //cout << "Finished waiting for jobs to complete." << endl;
   
   // stop dispatching
   jd.stopDispatching();      
   
   tr.passIfNoException();
   
   //cout << endl << "JobDispatcher Test complete." << endl << endl;
}

class TestGuard : public OperationGuard
{
public:
   virtual bool canExecuteOperation(ImmutableState* s)
   {
      bool rval = false;
      
      int ops = 0;
      s->getInteger("number.of.ops", ops);
      
      bool loggingOut = false;
      s->getBoolean("logging.out", loggingOut);
      
      rval = !loggingOut && ops < 3;
      if(!rval)
      {
         //cout << "Operation must wait or cancel." << endl;
      }
      else
      {
         //cout << "Operation can run." << endl;
      }
      
      return rval;
   }
   
   virtual bool mustCancelOperation(ImmutableState* s)
   {
      bool loggedOut = false;
      s->getBoolean("logged.out", loggedOut);
      
      if(loggedOut)
      {
         //cout << "Operation must cancel, user logged out." << endl;
      }
      else
      {
         //cout << "Operation can wait, user is not logged out yet." << endl;
      }
      
      return loggedOut;
   }
};

class TestStateMutator : public StateMutator
{
protected:
   bool mLogout;
public:
   TestStateMutator(bool logout)
   {
      mLogout = logout;
   }
   
   virtual void mutatePreExecutionState(State* s, Operation& op)
   {
      int ops = 0;
      s->getInteger("number.of.ops", ops);
      s->setInteger("number.of.ops", ++ops);
      
      if(mLogout)
      {
         s->setBoolean("logging.out", true);
         //cout << "Logging out..." << endl;
      }
   }
   
   virtual void mutatePostExecutionState(State* s, Operation& op)
   {
      int ops = 0;
      s->getInteger("number.of.ops", ops);
      s->setInteger("number.of.ops", --ops);
      
      if(mLogout)
      {
         s->setBoolean("logged.out", true);
         //cout << "Logged out." << endl;
      }
   }
};

class RunOp : public virtual Object, public Runnable
{
protected:
   string mName;
   unsigned long mTime;
   
public:
   RunOp(string name, unsigned long time)
   {
      mName = name;
      mTime = time;
   }
   
   virtual void run()
   {
      //cout << "Operation running: " << mName << endl;
      
      lock();
      {
         wait(mTime);
      }
      unlock();
      
      //cout << "Operation finished: " << mName << endl;
   }
   
   virtual string& toString(string& str)
   {
      str = mName;
      return mName;
   }
};

void runModestTest(TestRunner& tr)
{
   tr.test("Modest Engine");
   
   //cout << "Starting Modest test." << endl << endl;
   Exception::clearLast();
   
   Kernel k;
   
   //cout << "Modest engine started." << endl;
   k.getEngine()->start();
   
   RunOp r1("Number 1", 500);
   RunOp r2("Number 2", 500);
   RunOp r3("Number 3", 500);
   RunOp r4("Number 4", 500);
   RunOp r5("Number 5", 500);
   RunOp rLogout("Logout", 250);
   
   TestStateMutator m(false);
   TestStateMutator mLogout(true);
   TestGuard g;
   
   Operation op1(r1);
   Operation op2(r2);
   Operation op3(r3);
   Operation op4(r4);
   Operation op5(r5);
   Operation opLogout(rLogout);
   
   op1->addGuard(&g);
   op2->addGuard(&g);
   op3->addGuard(&g);
   op4->addGuard(&g);
   op5->addGuard(&g);
   opLogout->addGuard(&g);
   
   op1->addStateMutator(&m);
   op2->addStateMutator(&m);
   op3->addStateMutator(&m);
   op4->addStateMutator(&m);
   op5->addStateMutator(&m);
   opLogout->addStateMutator(&mLogout);
   
   k.getEngine()->queue(op1);
   k.getEngine()->queue(op2);
   k.getEngine()->queue(op3);
   k.getEngine()->queue(op4);
   k.getEngine()->queue(opLogout);
   k.getEngine()->queue(op5);
   
   op1->waitFor();
   op2->waitFor();
   op3->waitFor();
   op4->waitFor();
   op5->waitFor();
   opLogout->waitFor();
   
   //cout << "Operations complete." << endl;
   
   k.getEngine()->stop();
   //cout << "Modest engine stopped." << endl;
   
   tr.passIfNoException();
   
   //cout << endl << "Modest test complete." << endl;
}

void runBase64Test(TestRunner& tr)
{
   const char* expected = "YmNkZQ==";

   tr.test("Base64");
   
   char data[] = {'a', 'b', 'c', 'd', 'e'};
   string encoded = Base64Codec::encode(data + 1, 4);
   assert(encoded == expected);
   
   char* decoded;
   unsigned int length;
   Base64Codec::decode(encoded, &decoded, length);
   
   assert(length == 4);
   for(unsigned int i = 0; i < length; i++)
   {
      assert(decoded[i] == data[i + 1]);
   }
   
   string encoded2 = Base64Codec::encode(decoded, 4);
   assert(strcmp(encoded2.c_str(), expected) == 0);
   
   if(decoded != NULL)
   {
      delete [] decoded;
   }
   
   tr.pass();
}

void runCrcTest(TestRunner& tr)
{
   tr.group("CRC");
   
   unsigned int correctValue = 6013;
   
   tr.test("single value update");   
   Crc16 crc16s;
   crc16s.update(10);
   crc16s.update(20);
   crc16s.update(30);
   crc16s.update(40);
   crc16s.update(50);
   crc16s.update(60);
   crc16s.update(70);
   crc16s.update(80);
   assert(crc16s.getChecksum() == correctValue);
   tr.pass();
   
   tr.test("array update");   
   Crc16 crc16a;
   char b[] = {10, 20, 30, 40, 50, 60, 70, 80};
   crc16a.update(b, 8);
   //cout << "CRC-16=" << crc16.getChecksum() << endl;
   assert(crc16a.getChecksum() == correctValue);
   tr.pass();

   tr.ungroup();
}

void runConvertTest()
{
   cout << "Starting Convert test." << endl << endl;
   
   // convert to hex
   char data[] = "abcdefghiABCDEFGZXYW0123987{;}*%6,./.12`~";
   string original(data, strlen(data));
   
   cout << "test data=" << original << endl;
   
   string lowerHex = Convert::bytesToHex(data, strlen(data));
   string upperHex = Convert::bytesToHex(data, strlen(data));
   
   cout << "lower-case hex=" << lowerHex << endl;
   cout << "lower-case hex length=" << lowerHex.length() << endl;
   cout << "upper-case hex=" << upperHex << endl;
   cout << "upper-case hex length=" << upperHex.length() << endl;
   
   char decoded1[lowerHex.length() / 2];
   char decoded2[upperHex.length() / 2];
   
   unsigned int length1;
   unsigned int length2;
   Convert::hexToBytes(lowerHex.c_str(), lowerHex.length(), decoded1, length1);
   Convert::hexToBytes(upperHex.c_str(), upperHex.length(), decoded2, length2);
   
   string ascii1(decoded1, length1);
   
   string ascii2(decoded2, length2);
   
   cout << "lower-case hex to ascii=" << ascii1 << endl;
   cout << "lower-case hex length=" << length1 << endl;
   cout << "upper-case hex to ascii=" << ascii2 << endl;
   cout << "upper-case hex length=" << length2 << endl;
   
   if(ascii1 == ascii2 && ascii1 == original)
   {
      cout << "Test successful!" << endl;
   }
   else
   {
      cout << "Test FAILED! Strings do not match!" << endl;
   }
   
   cout << "10 to lower-case hex=" << Convert::intToHex(10) << endl;
   cout << "33 to lower-case hex=" << Convert::intToHex(33) << endl;
   cout << "100 to lower-case hex=" << Convert::intToHex(100) << endl;
   cout << "10 to upper-case hex=" << Convert::intToUpperHex(10) << endl;
   cout << "33 to upper-case hex=" << Convert::intToUpperHex(33) << endl;
   cout << "100 to upper-case hex=" << Convert::intToUpperHex(100) << endl;
   cout << "8975 to lower-case hex=" << Convert::intToHex(8975) << endl;
   cout << "8975 to upper-case hex=" << Convert::intToUpperHex(8975) << endl;
   cout << "65537 to lower-case hex=" << Convert::intToHex(65537) << endl;
   cout << "65537 to upper-case hex=" << Convert::intToUpperHex(65537) << endl;
   
   string hex = "230f";
   cout << "0x230f to integer=" <<
      Convert::hexToInt(hex.c_str(), hex.length()) << endl;
   hex = "230F";
   cout << "0x230F to integer=" <<
      Convert::hexToInt(hex.c_str(), hex.length()) << endl;
   hex = "230FABCD";
   cout << "0x230FABCD to integer=" <<
      Convert::hexToInt(hex.c_str(), hex.length()) << endl;
   hex = "0";
   cout << "0x0 to integer=" <<
      Convert::hexToInt(hex.c_str(), hex.length()) << endl;
   
   cout << endl << "Convert test complete." << endl;
}

void runRegexTest()
{
   cout << "Starting Regex test." << endl << endl;
   
//   string regex = "[a-z]{3}";
//   string str = "abc";
//   
//   if(Pattern::match(regex.c_str(), str.c_str()))
//   {
//      cout << "Simple pattern matches!" << endl;
//   }
//   else
//   {
//      cout << "Simple pattern DOES NOT MATCH!" << endl;
//   }
//   
//   cout << endl << "Doing sub-match test..." << endl << endl;
//   
//   string submatches = "Look for green globs of green matter in green goo.";
//   Pattern* p = Pattern::compile("green");
//   
//   unsigned int start, end;
//   unsigned int index = 0;
//   while(p->match(submatches.c_str(), index, start, end))
//   {
//      cout << "Found match at (" << start << ", " << end << ")" << endl;
//      cout << "Match=" << submatches.substr(start, end - start) << endl;
//      index = end;
//   }
//   
//   delete p;
//   
//   cout << endl << "Doing replace all test..." << endl << endl;
//   
//   cout << "change 'green' to 'blue'" << endl;
//   cout << submatches << endl;
//   StringTools::regexReplaceAll(submatches, "green", "blue");
//   cout << submatches << endl;
   
   cout << endl << "Regex test complete." << endl;
}

void runDateTest()
{
   cout << "Starting Date test." << endl << endl;
   
   TimeZone gmt = TimeZone::getTimeZone("GMT");
   TimeZone local = TimeZone::getTimeZone();
   
   Date d;
   string str;
   //d.format(str);
   //d.format(str, "E EEEE d dd M MMMM MM yy w ww yyyy a", "java");
   //d.format(str, "EEEE, MMMM dd yyyy hh:mm:ss a", "java");
   //d.format(str, "EEE, MMMM dd yyyy hh:mm:ss a", "java", &local);
   //d.format(str, "EEE, d MMM yyyy HH:mm:ss", "java", &gmt);
   //d.format(str, "%a, %d %b %Y %H:%M:%S");
   d.format(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d.format(str, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Current Date: " << str << endl;
   
   // parse date
   Date d2;
   d2.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d2.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &local);
   string str2;
   d2.format(str2, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d2.format(str2, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Parsed Date 1: " << str2 << endl;
   
//   // FIXME: parser may have a problem with AM/PM
   // parse date again
   Date d3;
   str = "Thu, 02 Aug 2007 10:30:00";
   d3.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   string str3;
   //d3.format(str3, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   d3.format(str3, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Parsed Date 2: " << str3 << endl;
   
   cout << endl << "Date test complete." << endl;
}

void runStringTokenizerTest()
{
   cout << "Starting StringTokenizer test." << endl << endl;
   
   const char* str = "This is a test of the StringTokenizer class.";
   
   StringTokenizer st(str, ' ');
   while(st.hasNextToken())
   {
      cout << "token='" << st.nextToken() << "'" << endl;
   }
   
   cout << endl << "StringTokenizer test complete." << endl;
}

void runStringEqualityTest()
{
   cout << "Starting string equality test." << endl << endl;
   
   // Note: string length doesn't appear to matter
   string str = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str == "");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str.length() == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "String.length() == 0 time: " << (end - start) << " ms" << endl;
   
   // Note: test demonstrates that comparing to length is about 6 times faster
   
   cout << endl << "String equality test complete." << endl;
}

void runStringAppendCharTest()
{
   cout << "Starting string append char test." << endl << endl;
   
   // Note: string length doesn't appear to matter
   string str = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str.length() == 1 && str[0] == '/');
   }
   end = System::getCurrentMilliseconds();
   cout << "String.length() == 1 && str[0] == '/' time: " <<
      (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str == "/");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"/\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(str.c_str(), "/") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "strcmp(String.c_str(), \"/\") == 0 time: " <<
      (end - start) << " ms" << endl;
   
   string version = "HTTP/1.0";
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(version == "HTTP/1.0");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"HTTP/1.0\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(version.c_str(), "HTTP/1.0") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "strcmp(String.c_str(), \"HTTP/1.0\") == 0 time: " <<
      (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append(1, '/');
   }
   end = System::getCurrentMilliseconds();
   cout << "String.append(1, '/') time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("/");
   }
   end = System::getCurrentMilliseconds();
   cout << "String.append(\"/\") time: " << (end - start) << " ms" << endl;
   
   string space = " ";
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("this" + space + "is a sentence");
   }
   end = System::getCurrentMilliseconds();
   cout << "String inline append time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("this");
      str.append(space);
      str.append("is a sentence");
   }
   end = System::getCurrentMilliseconds();
   cout << "String multiline append time: " << (end - start) << " ms" << endl;
   
   cout << endl << "String append char test complete." << endl;
}

void runStringCompareTest()
{
   cout << "Starting string compare test." << endl << endl;
   
   string str1 = "blah";
   char str2[] = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str1 == "blah");
   }
   end = System::getCurrentMilliseconds();
   cout << "std::string compare time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(str2, "blah") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "char* compare time: " << (end - start) << " ms" << endl;
   
   cout << endl << "String compare test complete." << endl;
}

void runDynamicObjectTest(TestRunner& tr)
{
   tr.test("DynamicObject");
   
   DynamicObject dyno1;
   dyno1["id"] = 2;
   dyno1["username"] = "testuser1000";
   dyno1["somearray"][0] = "item1";
   dyno1["somearray"][1] = "item2";
   dyno1["somearray"][2] = "item3";
   
   DynamicObject dyno2;
   dyno2["street"] = "1700 Kraft Dr.";
   dyno2["zip"] = "24060";
   
   dyno1["address"] = dyno2;
   
   assert(dyno1["id"]->getInt32() == 2);
   assert(strcmp(dyno1["username"]->getString(), "testuser1000") == 0);
   
   assert(strcmp(dyno1["somearray"][0]->getString(), "item1") == 0);
   assert(strcmp(dyno1["somearray"][1]->getString(), "item2") == 0);
   assert(strcmp(dyno1["somearray"][2]->getString(), "item3") == 0);
   
   DynamicObject dyno3 = dyno1["address"];
   assert(strcmp(dyno3["street"]->getString(), "1700 Kraft Dr.") == 0);
   assert(strcmp(dyno3["zip"]->getString(), "24060") == 0);
   
   DynamicObject dyno4;
   dyno4["whatever"] = "test";
   dyno4["someboolean"] = true;
   assert(dyno4["someboolean"]->getBoolean());
   dyno1["somearray"][3] = dyno4;
   
   dyno1["something"]["strange"] = "tinypayload";
   assert(
      strcmp(dyno1["something"]["strange"]->getString(), "tinypayload") == 0);
   
   DynamicObject dyno5;
   dyno5[0] = "mustard";
   dyno5[1] = "ketchup";
   dyno5[2] = "pickles";
   
   int count = 0;
   DynamicObjectIterator i = dyno5.getIterator();
   while(i->hasNext())
   {
      DynamicObject next = i->next();
      
      if(count == 0)
      {
         assert(strcmp(next->getString(), "mustard") == 0);
      }
      else if(count == 1)
      {
         assert(strcmp(next->getString(), "ketchup") == 0);
      }
      else if(count == 2)
      {
         assert(strcmp(next->getString(), "pickles") == 0);
      }
      
      count++;
   }
   
   DynamicObject dyno6;
   dyno6["eggs"] = "bacon";
   dyno6["milk"] = "yum";
   assert(strcmp(dyno6->removeMember("milk")->getString(), "yum") == 0);
   count = 0;
   i = dyno6.getIterator();
   while(i->hasNext())
   {
      DynamicObject next = i->next();
      assert(strcmp(i->getName(), "eggs") == 0);
      assert(strcmp(next->getString(), "bacon") == 0);
      count++;
   }
   
   assert(count == 1);
   
   // test clone
   dyno1["dyno5"] = dyno5;
   dyno1["dyno6"] = dyno6;
   dyno1["clone"] = dyno1.clone();
   
   DynamicObject clone = dyno1.clone();
   assert(dyno1 == clone);
   
   // test subset
   clone["mrmessy"] = "weirdguy";
   assert(dyno1.isSubset(clone));
   
   // test print out code
   //cout << endl;
   //dumpDynamicObject(dyno1);
   
   tr.pass();
}

void runDynoClearTest(TestRunner& tr)
{
   tr.test("DynamicObject clear");
   
   DynamicObject d;
   
   d = "x";
   assert(d->getType() == String);
   d->clear();
   assert(d->getType() == String);
   assert(strcmp(d->getString(), "") == 0);
   
   d = (int)1;
   assert(d->getType() == Int32);
   d->clear();
   assert(d->getType() == Int32);
   assert(d->getInt32() == 0);
   
   d = (unsigned int)1;
   assert(d->getType() == UInt32);
   d->clear();
   assert(d->getType() == UInt32);
   assert(d->getBoolean() == false);
   
   d = (long long)1;
   assert(d->getType() == Int64);
   d->clear();
   assert(d->getType() == Int64);
   assert(d->getInt64() == 0);
   
   d = (unsigned long long)1;
   d->clear();
   assert(d->getType() == UInt64);
   assert(d->getUInt64() == 0);
   
   d = (double)1.0;
   d->clear();
   assert(d->getType() == Double);
   assert(d->getDouble() == 0.0);
   
   d["x"] = 0;
   d->clear();
   assert(d->getType() == Map);
   assert(d->length() == 0);
   
   d[0] = 0;
   d->clear();
   assert(d->getType() == Array);
   assert(d->length() == 0);
   
   tr.passIfNoException();
}

void runDynoConversionTest(TestRunner& tr)
{
   tr.test("DynamicObject conversion");
   
   DynamicObject d;
   d["int"] = 2;
   d["-int"] = -2;
   d["str"] = "hello";
   d["true"] = "true";
   d["false"] = "false";
   
   string s;
   s.clear();
   d["int"]->toString(s);
   assert(strcmp(s.c_str(), "2") == 0);

   s.clear();
   d["-int"]->toString(s);
   assert(strcmp(s.c_str(), "-2") == 0);

   s.clear();
   d["str"]->toString(s);
   assert(strcmp(s.c_str(), "hello") == 0);

   s.clear();
   d["true"]->toString(s);
   assert(strcmp(s.c_str(), "true") == 0);

   s.clear();
   d["false"]->toString(s);
   assert(strcmp(s.c_str(), "false") == 0);
   
   tr.pass();
}

void runJsonValidTest(TestRunner& tr)
{
   tr.group("JSON (Valid)");
   
   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);
   
   const char* tests[] = {
      "{}",
      "[]",
      " []",
      "[] ",
      " [] ",
      " [ ] ",
      "[true]",
      "[false]",
      "[null]",
      "[ true]",
      "[true ]",
      "[ true ]",
      "[true, true]",
      "[true , true]",
      "[ true , true ]",
      "[0]",
      "[-0]",
      "[0.0]",
      "[-0.0]",
      "[0.0e0]",
      "[0.0e+0]",
      "[0.0e-0]",
      "[1.0]",
      "[-1.0]",
      "[1.1]",
      "[-1.1]",
      "[0,true]",
      "[[]]",
      "[[{}]]",
      "[[],[]]",
      "[[0]]",
      "[\"\"]",
      "[\"s\"]",
      "{\"k\":\"v\"}",
      "{\"k1\":1, \"k2\":2}",
      "{\"k\":[]}",
      "{\"k\":{}}",
      "[\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t\"]",
      "{\"k\":true}",
      "{\"k\":0}",
      "{\"k\":10}",
      "{\"k\":-10}",
      "{\"k\":0.0e+0}",
      "{\"k\":\"v\",\"k2\":true,\"k3\":1000,\"k4\":\"v\"}",
      "[\"\\u0020\"]",
      "[\"\xc3\x84 \xc3\xa4 \xc3\x96 \xc3\xb6 \xc3\x9c \xc3\xbc \xc3\x9f\"]",
      NULL
   };

   // FIXME add: unicode escapes, raw unicode
   tr.warning("Add JSON tests for: unicode escapes, raw unicode");

   for(int i = 0; tests[i] != NULL; i++)
   {
      char msg[50];
      snprintf(msg, 50, "Parse #%d", i);
      tr.test(msg);
      
      DynamicObject d;
      const char* s = tests[i];
      //cout << s << endl;
      ByteArrayInputStream is(s, strlen(s));
      jr.start(d);
      assertNoException();
      jr.read(&is);
      assertNoException();
      jr.finish();
      assertNoException();
      //cout << s << endl;
      //dumpDynamicObject(d);
      
      tr.passIfNoException();
   }
   
   tr.ungroup();
}

void runJsonInvalidTest(TestRunner& tr)
{
   tr.group("JSON (Invalid)");
   
   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);
   
   const char* tests[] = {
      "",
      " ",
      "{",
      "}",
      "[",
      "]",
      "{}{",
      "[][",
      "[tru]",
      "[junk]",
      "[true,]",
      "[true, ]",
      "[,true]",
      "[ ,true]",
      "[0.]",
      "[0.0e]",
      "[0.0e+]",
      "[0.0e-]",
      "[\"\0\"]",
      "[\"\\z\"]",
      "[\"\0\"]",
      "{\"k\":}",
      "{:\"v\"}",
      "{\"k\":1,}",
      "{,\"k\":1}",
      "{null:0}",
      "[\"\n\"]",
      "[\"\t\"]",
      NULL
   };

   for(int i = 0; tests[i] != NULL; i++)
   {
      char msg[50];
      snprintf(msg, 50, "Parse #%d", i);
      tr.test(msg);

      DynamicObject d;
      const char* s = tests[i];
      //cout << s << endl;
      ByteArrayInputStream is(s, strlen(s));
      jr.start(d);
      assertNoException();
      jr.read(&is);
      jr.finish();
      assertException();
      Exception::clearLast();
      //jw.write(d, &os);
      //cout << endl;
      
      tr.passIfNoException();
   }
   
   tr.ungroup();
}

void runJsonDJDTest(TestRunner& tr)
{
   tr.group("JSON (Dyno->JSON->Dyno)");
   
   DynamicObject dyno0;
   dyno0["email"] = "example@example.com";
   dyno0["AIM"] = "example";

   DynamicObject dyno1;
   dyno1["id"] = 2;
   dyno1["-id"] = -2;
   dyno1["floats"][0] = 0.0;
   dyno1["floats"][1] = -0.0;
   dyno1["floats"][2] = 1.0;
   dyno1["floats"][3] = -1.0;
   dyno1["floats"][4] = 1.23456789;
   dyno1["floats"][5] = -1.23456789;
   dyno1["username"] = "testuser1000";
   dyno1["l33t"] = true;
   dyno1["luser"] = false;
   dyno1["somearray"][0] = "item1";
   dyno1["somearray"][1] = "item2";
   dyno1["somearray"][2] = "item3";
   dyno1["somearray"][3] = dyno0;
   dyno1["contact"] = dyno0;
   
   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);
   
   DynamicObject* dynos[] = {
      &dyno0,
      &dyno1,
      NULL
   };
   
   for(int i = 0; dynos[i] != NULL; i++)
   {
      char msg[50];
      snprintf(msg, 50, "Verify #%d", i);
      tr.test(msg);

      DynamicObject d = *dynos[i];

      ByteBuffer b;
      ByteArrayOutputStream bbos(&b);
      
      jw.setCompact(true);
      //jw.write(dyno1, &os);
      jw.write(d, &bbos);
      assertNoException();
      b.clear();
      assertNoException();
      
      jw.setCompact(false);
      jw.setIndentation(0, 3);
      //jw.write(d, &os);
      jw.write(d, &bbos);
      ByteArrayInputStream is(b.data(), b.length());
      DynamicObject dr;
      jr.start(dr);
      assertNoException();
      jr.read(&is);
      assertNoException();
      jr.finish();
      assertNoException();
      //jw.write(dr, &os);
      assertNoException();
      b.clear();

      tr.passIfNoException();
   }
   
   tr.ungroup();
}

void runJsonVerifyDJDTest(TestRunner& tr)
{
   tr.group("JSON (Verify Dyno->JSON->Dyno)");
   
   JsonWriter jw;
   JsonReader jr;
   OStreamOutputStream os(&cout);
   
   int tdcount = 0;
   DynamicObject td;
   td[tdcount  ]["dyno"]->setType(Map);
   td[tdcount++]["JSON"] = "{}";
   td[tdcount  ]["dyno"]->setType(Array);
   td[tdcount++]["JSON"] = "[]";
   td[tdcount  ]["dyno"][0] = true;
   td[tdcount++]["JSON"] = "[true]";
   td[tdcount  ]["dyno"]["k"] = "v";
   td[tdcount++]["JSON"] = "{\"k\":\"v\"}";
   td[tdcount  ]["dyno"][0] = 0;
   td[tdcount++]["JSON"] = "[0]";
   td[tdcount  ]["dyno"][0] = "\n";
   td[tdcount++]["JSON"] = "[\"\\n\"]";
   td[tdcount  ]["dyno"][0] = td[0]["dyno"];
   td[tdcount++]["JSON"] = "[{}]";
   td[tdcount  ]["dyno"][0] = -1;
   td[tdcount++]["JSON"] = "[-1]";
   td[tdcount  ]["dyno"][0] = DynamicObject(NULL);
   td[tdcount++]["JSON"] = "[null]";
   td[tdcount  ]["dyno"]["k"] = 0;
   td[tdcount++]["JSON"] = "{\"k\":0}";
   td[tdcount  ]["dyno"]["k"] = 10;
   td[tdcount++]["JSON"] = "{\"k\":10}";
   td[tdcount  ]["dyno"]["k"] = -10;
   td[tdcount++]["JSON"] = "{\"k\":-10}";
   td[tdcount  ]["dyno"][0] = "\x01";
   td[tdcount++]["JSON"] = "[\"\\u0001\"]";
   // test if UTF-16 C escapes translate into a UTF-8 JSON string
   td[tdcount  ]["dyno"][0] =
      "\u040e \u045e \u0404 \u0454 \u0490 \u0491";
   td[tdcount++]["JSON"] =
      "[\"\xd0\x8e \xd1\x9e \xd0\x84 \xd1\x94 \xd2\x90 \xd2\x91\"]";
   
   for(int i = 0; i < tdcount; i++)
   {
      char msg[50];
      snprintf(msg, 50, "Verify #%d", i);
      tr.test(msg);
      
      DynamicObject d = td[i]["dyno"];
      const char* s = td[i]["JSON"]->getString();

      ByteBuffer b;
      ByteArrayOutputStream bbos(&b);
      
      jw.setCompact(true);
      //jw.write(d, &os);
      assertNoException();
      jw.write(d, &bbos);
      assertNoException();
      
      // Verify written string
      assert(strlen(s) == (unsigned int)b.length());
      assert(strncmp(s, b.data(), b.length()) == 0);
      
      ByteArrayInputStream is(b.data(), b.length());
      DynamicObject dr;
      jr.start(dr);
      assertNoException();
      jr.read(&is);
      assertNoException();
      jr.finish();
      assertNoException();
      //jw.write(dr, &os);
      assertNoException();
      b.clear();
      
      assert(d == dr);
      
      tr.passIfNoException();
   }
   
   tr.ungroup();
}

void runJsonIOStreamTest(TestRunner& tr)
{
   tr.group("JSON I/O");
   
   tr.test("Input");
   /*
   DynamicObject di;
   di["title"] = "My Stuff";
   di["public"] = true;
   di["stuff"][0] = "item 1";
   di["stuff"][1] = "item 2";
   JsonInputStream jis(di);
   jis.setCompact(true);
   ostreamstring oss;
   OStreamOutputStream os(&oss);
   jis.read(oss);
   */
   tr.passIfNoException();
   
   tr.test("Output");
   tr.passIfNoException();
   
   tr.ungroup();
}

void runByteArrayInputStreamTest()
{
   cout << "Starting ByteArrayInputStream test." << endl << endl;
   
   char html[] = "<html>505 HTTP Version Not Supported</html>";
   ByteArrayInputStream is(html, 43);
   
   char b[10];
   int numBytes;
   string str;
   while((numBytes = is.read(b, 9)) > 0)
   {
      memset(b + numBytes, 0, 1);
      str.append(b);
   }
   
   cout << "read data='" << str << "'" << endl;
   
   cout << endl << "ByteArrayInputStream test complete." << endl;
}

void runByteBufferTest()
{
   cout << "Starting ByteBuffer test." << endl << endl;
   
   ByteBuffer b;
   
   const char* chicken = "chicken";
   const char* t = "T ";
   const char* hate = "hate ";
   b.free();
   b.put(t, strlen(t), true);
   b.put(hate, strlen(hate), true);
   b.put(chicken, strlen(chicken), true);
   b.put("", 1, true);
   
   // FIXME: this test should be more comprehensive
   
   cout << "Data=" << b.data() << endl;
   
   // this should result in printing out "T hate chicken" still
   b.allocateSpace(10, true);
   sprintf(b.data() + b.length(), " always");
   char temp[100];
   strncpy(temp, b.data(), b.length());
   memset(temp + b.length(), 0, 1);
   cout << "Data2=" << temp << endl;
   
   // this should now result in printing out "T hate chicken always"
   sprintf(b.data() + b.length() - 1, " always");
   b.extend(7);
   strncpy(temp, b.data(), b.length());
   memset(temp + b.length(), 0, 1);
   cout << "Data3=" << temp << endl;
   
   cout << endl << "ByteBuffer test complete." << endl;
}

void runByteArrayOutputStreamTest()
{
   cout << "Starting ByteArrayOutputStream test." << endl << endl;
   
   ByteBuffer b;
   
   ByteArrayOutputStream baos1(&b);
   const char* sentence = "This is a sentence.";
   baos1.write(sentence, strlen(sentence) + 1);
   
   cout << "Data1=" << b.data() << endl;
   
   const char* chicken = "chicken";
   const char* t = "T ";
   const char* hate = "hate ";
   b.clear();
   b.put(t, strlen(t), true);
   b.put(hate, strlen(hate), true);
   b.put(chicken, strlen(chicken), true);
   b.put("", 1, true);
   
   cout << "Prior Data2=" << b.data() << endl;
   
   // trim null-terminator
   b.trim(1);
   
   // false = turn off resizing buffer
   int length = strlen(sentence) + 1;
   ByteArrayOutputStream baos2(&b, false);
   if(!baos2.write(sentence, length))
   {
      IOException* e = (IOException*)Exception::getLast();
      cout << "Exception Caught=" << e->getMessage() << endl;
      cout << "Written bytes=" << e->getUsedBytes() << endl;
      cout << "Unwritten bytes=" << e->getUnusedBytes() << endl;
      cout << "Turning on resize and finishing write..." << endl;
      
      // turn on resize
      baos2.setResize(true);
      
      // write remaining bytes
      baos2.write(sentence + e->getUsedBytes(), e->getUnusedBytes());
      
      // clear exception
      Exception::clearLast();
   }
   
   cout << "Data2=" << b.data() << endl;
   
   cout << endl << "ByteArrayOutputStream test complete." << endl;
}

void runMessageDigestTest(TestRunner& tr)
{
   tr.test("MessageDigest");
   
   // correct values
   string correctMd5 = "78eebfd9d42958e3f31244f116ab7bbe";
   string correctSha1 = "5f24f4d6499fd2d44df6c6e94be8b14a796c071d";   
   
   MessageDigest testMd5("MD5");
   testMd5.update("THIS ");
   testMd5.update("IS A");
   testMd5.update(" MESSAGE");
   string digestMd5 = testMd5.getDigest();
   
   //cout << "MD5 Digest=" << digestMd5 << endl;
   assert(digestMd5 == correctMd5);
   
   MessageDigest testSha1("SHA1");
   testSha1.update("THIS IS A MESSAGE");
   string digestSha1 = testSha1.getDigest();
   
   //cout << "SHA-1 Digest=" << digestSha1 << endl;
   assert(digestSha1 == correctSha1);
   
   tr.pass();
}

void runAsymmetricKeyLoadingTest()
{
   cout << "Running Asymmetric Key Loading Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // read in PEM private key
   File file1("/work/src/dbcpp/dbcore/trunk/Debug/private.pem");
   FileInputStream fis1(&file1);
   
   string privatePem = "";
   
   char b[2048];
   int numBytes;
   while((numBytes = fis1.read(b, 2048)) > 0)
   {
      privatePem.append(b, numBytes);
   }
   
   // close stream
   fis1.close();
   
   cout << "Private Key PEM=" << endl << privatePem << endl;
   
   // read in PEM public key
   File file2("/work/src/dbcpp/dbcore/trunk/Debug/public.pem");
   FileInputStream fis2(&file2);
   
   string publicPem = "";
   
   while((numBytes = fis2.read(b, 2048)) > 0)
   {
      publicPem.append(b, numBytes);
   }
   
   // close stream
   fis2.close();
   
   cout << "Public Key PEM=" << endl << publicPem << endl;
        
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // load the private key
   PrivateKey* privateKey = factory.loadPrivateKeyFromPem(
      privatePem, "password");
   
   cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
   
   // load the public key
   PublicKey* publicKey = factory.loadPublicKeyFromPem(publicPem);
   
   cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
   
   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature* ds1 = privateKey->createSignature();
   ds1->update(data, 8);
   
   // get the signature
   char sig[ds1->getValueLength()];
   unsigned int length;
   ds1->getValue(sig, length);
   delete ds1;
   
   // verify the signature
   DigitalSignature* ds2 = publicKey->createSignature();
   ds2->update(data, 8);
   bool verified = ds2->verify(sig, length);
   delete ds2;
   
   if(verified)
   {
      cout << "Digital Signature Verified!" << endl;
   }
   else
   {
      cout << "Digital Signature NOT VERIFIED!" << endl;
   }
   
   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);
   
   cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
   cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   
   // delete the private key
   delete privateKey;
   
   // delete the public key
   delete publicKey;
   
   cout << endl << "Asymmetric Key Loading test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runDsaAsymmetricKeyCreationTest()
{
   cout << "Running DSA Asymmetric Key Creation Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKey* privateKey;
   PublicKey* publicKey;
   factory.createKeyPair("DSA", &privateKey, &publicKey);
   
   if(privateKey != NULL)
   {
      cout << "DSA Private Key created!" << endl;
   }
   else
   {
      cout << "DSA Private Key creation FAILED!" << endl;
   }
   
   if(publicKey != NULL)
   {
      cout << "DSA Public Key created!" << endl;
   }
   else
   {
      cout << "DSA Public Key creation FAILED!" << endl;
   }
   
   assert(privateKey != NULL && publicKey != NULL);
   
   // test copy constructors
   PrivateKey prvKey(*privateKey);
   PublicKey pubKey(*publicKey);
   
   // cleanup private key
   if(privateKey != NULL)
   {
      delete privateKey;
   }
   
   // cleanup public key
   if(publicKey != NULL)
   {
      delete publicKey;
   }
   
   privateKey = &prvKey;
   publicKey = &pubKey;
   
   cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
   cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
   
   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature* ds1 = privateKey->createSignature();
   ds1->update(data, 8);
   
   // get the signature
   char sig[ds1->getValueLength()];
   unsigned int length;
   ds1->getValue(sig, length);
   delete ds1;
   
   // verify the signature
   DigitalSignature* ds2 = publicKey->createSignature();
   ds2->update(data, 8);
   bool verified = ds2->verify(sig, length);
   delete ds2;
   
   if(verified)
   {
      cout << "Digital Signature Verified!" << endl;
   }
   else
   {
      cout << "Digital Signature NOT VERIFIED!" << endl;
   }
   
   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);
   
   cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
   cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   
   cout << endl << "DSA Asymmetric Key Creation test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runRsaAsymmetricKeyCreationTest()
{
   cout << "Running RSA Asymmetric Key Creation Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKey* privateKey;
   PublicKey* publicKey;
   factory.createKeyPair("RSA", &privateKey, &publicKey);
   
   if(privateKey != NULL)
   {
      cout << "RSA Private Key created!" << endl;
   }
   else
   {
      cout << "RSA Private Key creation FAILED!" << endl;
   }
   
   if(publicKey != NULL)
   {
      cout << "RSA Public Key created!" << endl;
   }
   else
   {
      cout << "RSA Public Key creation FAILED!" << endl;
   }
   
   assert(privateKey != NULL && publicKey != NULL);
   
   // test copy constructors
   PrivateKey prvKey(*privateKey);
   PublicKey pubKey(*publicKey);
   
   // cleanup private key
   if(privateKey != NULL)
   {
      delete privateKey;
   }
   
   // cleanup public key
   if(publicKey != NULL)
   {
      delete publicKey;
   }
   
   privateKey = &prvKey;
   publicKey = &pubKey;
   
   cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
   cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
   
   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature* ds1 = privateKey->createSignature();
   ds1->update(data, 8);
   
   // get the signature
   char sig[ds1->getValueLength()];
   unsigned int length;
   ds1->getValue(sig, length);
   delete ds1;
   
   // verify the signature
   DigitalSignature* ds2 = publicKey->createSignature();
   ds2->update(data, 8);
   bool verified = ds2->verify(sig, length);
   delete ds2;
   
   if(verified)
   {
      cout << "Digital Signature Verified!" << endl;
   }
   else
   {
      cout << "Digital Signature NOT VERIFIED!" << endl;
   }
   
   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);
   
   cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
   cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   
   cout << endl << "RSA Asymmetric Key Creation test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runDigitalSignatureInputStreamTest()
{
   cout << "Running DigitalSignatureInputStream Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKey* privateKey;
   PublicKey* publicKey;
   factory.createKeyPair("RSA", &privateKey, &publicKey);
   
   if(privateKey != NULL)
   {
      cout << "RSA Private Key created!" << endl;
   }
   else
   {
      cout << "RSA Private Key creation FAILED!" << endl;
   }
   
   if(publicKey != NULL)
   {
      cout << "RSA Public Key created!" << endl;
   }
   else
   {
      cout << "RSA Public Key creation FAILED!" << endl;
   }
   
   if(privateKey != NULL && publicKey != NULL)
   {
      cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
      cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
      
      // sign some data
      char data[] = {1,2,3,4,5,6,7,8};
      DigitalSignature* ds1 = privateKey->createSignature();
      
      char dummy[8];
      ByteArrayInputStream bais(data, 8);
      DigitalSignatureInputStream dsos1(ds1, &bais, false);
      dsos1.read(dummy, 8);
      
      // get the signature
      char sig[ds1->getValueLength()];
      unsigned int length;
      ds1->getValue(sig, length);
      delete ds1;
      
      // verify the signature
      DigitalSignature* ds2 = publicKey->createSignature();
      bais.setByteArray(data, 8);
      DigitalSignatureInputStream dsos2(ds2, &bais, false);
      dsos2.read(dummy, 8);
      bool verified = ds2->verify(sig, length);
      delete ds2;
      
      if(verified)
      {
         cout << "Digital Signature Verified!" << endl;
      }
      else
      {
         cout << "Digital Signature NOT VERIFIED!" << endl;
      }
      
      string outPrivatePem =
         factory.writePrivateKeyToPem(privateKey, "password");
      string outPublicPem =
         factory.writePublicKeyToPem(publicKey);
      
      cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
      cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   }
   
   // cleanup private key
   if(privateKey != NULL)
   {
      delete privateKey;
   }
   
   // cleanup public key
   if(publicKey != NULL)
   {
      delete publicKey;
   }
   
   cout << endl << "DigitalSignatureInputStream test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runDigitalSignatureOutputStreamTest()
{
   cout << "Running DigitalSignatureOutputStream Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKey* privateKey;
   PublicKey* publicKey;
   factory.createKeyPair("RSA", &privateKey, &publicKey);
   
   if(privateKey != NULL)
   {
      cout << "RSA Private Key created!" << endl;
   }
   else
   {
      cout << "RSA Private Key creation FAILED!" << endl;
   }
   
   if(publicKey != NULL)
   {
      cout << "RSA Public Key created!" << endl;
   }
   else
   {
      cout << "RSA Public Key creation FAILED!" << endl;
   }
   
   if(privateKey != NULL && publicKey != NULL)
   {
      cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
      cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
      
      // sign some data
      char data[] = {1,2,3,4,5,6,7,8};
      DigitalSignature* ds1 = privateKey->createSignature();
      
      ostringstream oss;
      OStreamOutputStream osos(&oss);
      DigitalSignatureOutputStream dsos1(ds1, &osos, false);
      dsos1.write(data, 8);
      
      // get the signature
      char sig[ds1->getValueLength()];
      unsigned int length;
      ds1->getValue(sig, length);
      delete ds1;
      
      // verify the signature
      DigitalSignature* ds2 = publicKey->createSignature();
      DigitalSignatureOutputStream dsos2(ds2, &osos, false);
      dsos2.write(data, 8);
      bool verified = ds2->verify(sig, length);
      delete ds2;
      
      if(verified)
      {
         cout << "Digital Signature Verified!" << endl;
      }
      else
      {
         cout << "Digital Signature NOT VERIFIED!" << endl;
      }
      
      string outPrivatePem =
         factory.writePrivateKeyToPem(privateKey, "password");
      string outPublicPem =
         factory.writePublicKeyToPem(publicKey);
      
      cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
      cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   }
   
   // cleanup private key
   if(privateKey != NULL)
   {
      delete privateKey;
   }
   
   // cleanup public key
   if(publicKey != NULL)
   {
      delete publicKey;
   }
   
   cout << endl << "DigitalSignatureOutputStream test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runEnvelopeTest(const std::string& algorithm)
{
   cout << "Running " << algorithm << " Envelope Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKey* privateKey;
   PublicKey* publicKey;
   factory.createKeyPair("RSA", &privateKey, &publicKey);
   
   if(privateKey != NULL && publicKey != NULL)
   {
      // create a secret message
      char message[] =
         "This is a confidential message. For British Eyes Only.";
      int length = strlen(message);
      
      string display1 = "";
      display1.append(message, length);
      cout << "Sending message '" << display1 << "'" << endl;
      cout << "Message Length=" << length << endl;
      
      // create an outgoing envelope
      SymmetricKey* secretKey;
      DigitalEnvelope* outEnv = publicKey->createEnvelope(
         "AES256", &secretKey);
      cout << "Created outgoing envelope..." << endl;
      
      // update the envelope
      char output[2048];
      int outLength;
      int totalOut = 0;
      outEnv->update(message, length, output, outLength);
      cout << "Updated outgoing envelope..." << endl;
      totalOut += outLength;
      
      // finish the envelope
      cout << "Output Length=" << outLength << endl;
      outEnv->finish(output + outLength, outLength);
      cout << "Finished sealing outgoing envelope..." << endl;
      totalOut += outLength;
      
      cout << "Total Output Length=" << totalOut << endl;
      
      // create an incoming envelope
      DigitalEnvelope* inEnv = privateKey->createEnvelope(secretKey);
      cout << "Created incoming envelope..." << endl;
      
      // update the envelope
      char input[2048];
      int inLength;
      int totalIn = 0;
      inEnv->update(output, totalOut, input, inLength);
      cout << "Updated incoming envelope..." << endl;
      totalIn += inLength;
      
      // finish the envelope
      cout << "Input Length=" << inLength << endl;
      inEnv->finish(input + inLength, inLength);
      cout << "Finished opening incoming envelope..." << endl;
      totalIn += inLength;
      
      cout << "Total Input Length=" << totalIn << endl;
      
      // create a string to display the received message
      string display2 = "";
      display2.append(input, totalIn);
      
      cout << "Received message '" << display2 << "'" << endl;
      
      // delete envelopes and key
      delete secretKey;
      delete outEnv;
      delete inEnv;
   }
   
   // cleanup private key
   if(privateKey != NULL)
   {
      delete privateKey;
   }
   
   // cleanup public key
   if(publicKey != NULL)
   {
      delete publicKey;
   }
   
   cout << endl << algorithm << " Envelope test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runCipherTest(const string& algorithm)
{
   cout << "Running " << algorithm << " Cipher Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // create a secret message
   char message[] = "I'll never teelllll!";
   int length = strlen(message);
   
   string display1 = "";
   display1.append(message, length);
   cout << "Encrypting message '" << display1 << "'" << endl;
   cout << "Message Length=" << length << endl;
   
   // get a default block cipher
   DefaultBlockCipher cipher;
   
   cout << "Starting encryption..." << endl;
   
   // generate a new key for the encryption
   SymmetricKey* key = NULL;
   cipher.startEncrypting(algorithm.c_str(), &key);
   
   if(key != NULL)
   {
      // update encryption
      char output[2048];
      int outLength;
      int totalOut = 0;
      cipher.update(message, length, output, outLength);
      cout << "Updated encryption..." << endl;
      totalOut += outLength;
      
      // finish the envelope
      cout << "Output Length=" << outLength << endl;
      cipher.finish(output + outLength, outLength);
      cout << "Finished encryption..." << endl;
      totalOut += outLength;
      
      cout << "Total Output Length=" << totalOut << endl;
      
      cout << "Starting decryption..." << endl;
      cipher.startDecrypting(key);
      
      // update the decryption
      char input[2048];
      int inLength;
      int totalIn = 0;
      cipher.update(output, totalOut, input, inLength);
      cout << "Updated decryption..." << endl;
      totalIn += inLength;
      
      // finish the decryption
      cout << "Input Length=" << inLength << endl;
      cipher.finish(input + inLength, inLength);
      cout << "Finished decrypting..." << endl;
      totalIn += inLength;
      
      cout << "Total Input Length=" << totalIn << endl;
      
      // create a string to display the received message
      string display2 = "";
      display2.append(input, totalIn);
      
      cout << "Decrypted message '" << display2 << "'" << endl;
   }
   
   // cleanup key
   if(key != NULL)
   {
      delete key;
   }
   
   cout << endl << algorithm << " Cipher test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runAddressResolveTest(TestRunner& tr)
{
   tr.test("Address Resolution");
   
   //cout << "Running Address Resolve Test" << endl << endl;
   
   Exception::clearLast();
   
   // create IPv4 address
   InternetAddress ip4;
   
   //cout << "Testing IPv4..." << endl << endl;
   
   ip4.setHost("www.bitmunk.com");
   ip4.getAddress();
   assertNoException();
   //cout << "www.bitmunk.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.google.com");
   ip4.getAddress();
   assertNoException();
   //cout << "www.google.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.yahoo.com");
   ip4.getAddress();
   assertNoException();
   //cout << "www.yahoo.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.microsoft.com");
   ip4.getAddress();
   assertNoException();
   //cout << "www.microsoft.com = " << ip4.getAddress() << endl;
   
   //cout << endl;
   
   ip4.setAddress("192.168.0.1");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
   ip4.setAddress("192.168.0.8");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
   ip4.setAddress("216.239.51.99");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
   // create IPv6 address
   //Internet6Address ip6;
   
   //cout << endl << "Testing IPv6..." << endl << endl;
   
   //ip6.setHost("ip6-localhost");
   //cout << "ip6-localhost = " << ip6.getAddress() << endl;
   
   //ip6.setHost("yuna.digitalbazaar.com");
   //cout << "yuna.digitalbazaar.com = " << ip6.getAddress() << endl;
   
   /*
   ip6.setHost("www.google.com");
   cout << "www.google.com = " << ip6.getAddress() << endl;
   
   ip6.setHost("www.yahoo.com");
   cout << "www.yahoo.com = " << ip6.getAddress() << endl;
   
   ip6.setHost("www.microsoft.com");
   cout << "www.microsoft.com = " << ip6.getAddress() << endl;
   */
   
   //cout << endl;
   
   //ip6.setAddress("fc00:840:db:bb:d::8");
   //cout << ip6.getAddress() << " = " << ip6.getHost() << endl;
   
   tr.passIfNoException();
   
   //cout << endl << "Address Resolve Test complete." << endl << endl;
}

void runSocketTest(TestRunner& tr)
{
   tr.test("Socket");
   
   //cout << "Running Socket Test" << endl << endl;
   Exception::clearLast();
   
   // create address
   //InternetAddress address("127.0.0.1", 80);
   InternetAddress address("www.google.com", 80);
   
   // ensure host was known
   assertNoException();
   
   address.getAddress();
   assertNoException();
   //cout << "Connecting to: " << address.getAddress() << endl;
   
   // create tcp socket
   TcpSocket socket;
   
   // connect
   socket.connect(&address);
   assertNoException();
   
   char request[] =
      "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
   socket.send(request, sizeof(request));
   assertNoException();
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   socket.setReceiveTimeout(10000);
   assertNoException();
   
   char response[2048];
   int numBytes = 0;
   string str;
   
   //cout << endl << "DOING A PEEK!" << endl;
   
   string peek;
   numBytes = socket.getInputStream()->peek(response, 2048);
   if(numBytes > 0)
   {
      //cout << "Peeked " << numBytes << " bytes." << endl;
      peek.append(response, numBytes);
      //cout << "Peek bytes=" << peek << endl;
   }
   assertNoException();
   
   //cout << endl << "DOING ACTUAL READ NOW!" << endl;
   int peekBytes = numBytes;
   while((numBytes = socket.getInputStream()->read(response, 2048)) > 0)
   {
      //cout << "numBytes received: " << numBytes << endl;
      str.append(response, numBytes);
   }
   
   // confirm peek bytes check out
   assert(strncmp(peek.c_str(), str.c_str(), peekBytes) == 0);
   
//   char response[2048];
//   int numBytes = 0;
//   string str = "";
//   while((numBytes = socket.receive(response, 0, 2048)) > 0)
//   {
//      cout << "numBytes received: " << numBytes << endl;
//      str.append(response, numBytes);
//   }
   
   //cout << "Response:" << endl << str << endl;
   
   // close
   socket.close();
   
   tr.passIfNoException();
   
   //cout << "Socket connection closed." << endl;
   
   //cout << endl << "Socket test complete." << endl;
}

void runSslSocketTest()
{
   cout << "Running SSL Socket Test" << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
   
   // FIXME:
   // seed PRNG
   
   // create address
   InternetAddress address("127.0.0.1", 443);
   //InternetAddress address("127.0.0.1", 19020);
   //InternetAddress address("www.google.com", 80);
   cout << address.getAddress() << endl;
   
   // ensure host was known
   if(!Exception::hasLast())
   {
      // create tcp socket
      TcpSocket socket;
      
      // connect
      socket.connect(&address);
      
      // create an SSL context
      SslContext context;
      
      // create an SSL socket
      SslSocket sslSocket(&context, &socket, true, false);
      
      // set receive timeout (10 seconds = 10000 milliseconds)
      sslSocket.setReceiveTimeout(10000);
      
      // perform handshake (automatically happens, this call isn't necessary)
      //sslSocket.performHandshake();
      
      char request[] =
         "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
      sslSocket.send(request, sizeof(request));
      
      char response[2048];
      int numBytes = 0;
      string str = "";
      
      cout << endl << "DOING A PEEK!" << endl;
      
      numBytes = sslSocket.getInputStream()->peek(response, 2048);
      if(numBytes > 0)
      {
         cout << "Peeked " << numBytes << " bytes." << endl;
         string peek = "";
         peek.append(response, numBytes);
         cout << "Peek bytes=" << peek << endl;
      }
      
      cout << endl << "DOING ACTUAL READ NOW!" << endl;
      
      while((numBytes = sslSocket.getInputStream()->read(response, 2048)) > 0)
      {
         cout << "numBytes received: " << numBytes << endl;
         str.append(response, numBytes);
      }
      
      cout << "Response:" << endl << str << endl;
      
      // close
      sslSocket.close();
      
      cout << "SSL Socket connection closed." << endl;
   }
   
   cout << endl << "SSL Socket test complete." << endl;
   
   // clean up SSL
   EVP_cleanup();
}

void runServerSocketTest()
{
   //cout << "Running Server Socket Test" << endl << endl;
   
   Exception::clearLast();
   
   // bind and listen
   InternetAddress address("127.0.0.1", 19100);
   
   // ensure host was known
   if(!Exception::hasLast())
   {
      // create tcp socket
      TcpSocket socket;
      
      if(socket.bind(&address))
      {
         cout << "Server socket bound..." << endl;
      }
      else
      {
         cout << "Could not bind server socket!" << endl;
      }
      
      if(socket.listen())
      {
         cout << "Listening for a connection..." << endl;
      }
      else
      {
         cout << "Could not listen with server socket!" << endl;
      }
      
      string str = "HTTP/1.0 200 OK\r\nContent-Length: 0\r\n\r\n";
      while(!Thread::interrupted(false))
      {
         // accept a connection
         Socket* worker = socket.accept(1);
         if(worker != NULL)
         {
            char request[100];
            int numBytes = 0;
            
            numBytes = worker->getInputStream()->peek(request, 100);
            worker->getOutputStream()->write(str.c_str(), str.length());
            
            // close worker socket
            worker->close();
            delete worker;
         }
      }
      
      // close server socket
      socket.close();
      
      cout << "Server Socket connection closed." << endl;
   }
   
   cout << endl << "Server Socket test complete." << endl;
}

void runSslServerSocketTest()
{
   cout << "Running SSL Server Socket Test" << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();   
   
   // bind and listen
   InternetAddress address("127.0.0.1", 1024);
   
   // ensure host was known
   if(!Exception::hasLast())
   {
      // create tcp socket
      TcpSocket socket;
      
      if(socket.bind(&address))
      {
         cout << "Server socket bound..." << endl;
      }
      else
      {
         cout << "Could not bind server socket!" << endl;
      }
      
      if(socket.listen())
      {
         cout << "Listening for a connection..." << endl;
      }
      else
      {
         cout << "Could not listen with server socket!" << endl;
      }
      
      // accept a connection
      TcpSocket* worker = (TcpSocket*)socket.accept(10);
      if(worker != NULL)
      {
         cout << "Accepted a connection!" << endl;
         
         // create an SSL context
         SslContext context;
         
         // create an SSL socket
         SslSocket sslSocket(&context, worker, false, false);
         
         // set receive timeout (10 seconds = 10000 milliseconds)
         sslSocket.setReceiveTimeout(10000);
         
         char request[2048];
         int numBytes = 0;
         string str = "";
         
         cout << endl << "DOING A PEEK!" << endl;
         
         numBytes = worker->getInputStream()->peek(request, 2048);
         if(numBytes > 0)
         {
            cout << "Peeked " << numBytes << " bytes." << endl;
            string peek = "";
            peek.append(request, numBytes);
            cout << "Peek bytes=" << peek << endl;
         }
         
         cout << endl << "DOING ACTUAL READ NOW!" << endl;
         
         while((numBytes = sslSocket.getInputStream()->read(request, 2048)) > 0)
         {
            cout << "numBytes received: " << numBytes << endl;
            str.append(request, numBytes);
         }
         
         cout << "Request:" << endl << str << endl;
         
         // close ssl socket socket
         sslSocket.close();
         delete worker;
      }
      else
      {
         cout << "Could not accept a connection!" << endl;
      }
      
      // close server socket
      socket.close();
      
      cout << "SSL Server Socket connection closed." << endl;
   }
   
   cout << endl << "SSL Server Socket test complete." << endl;
   
   // clean up SSL
   EVP_cleanup();
}

void runTcpClientServerTest()
{
   cout << "Running TCP Client/Server Test" << endl << endl;
   
   InternetAddress* address;
   InternetAddress ia("127.0.0.1", 9999);
   //Internet6Address ia("::0", 9999);
   address = &ia;
   
   // ensure host was known
   if(!Exception::hasLast())
   {
      // create tcp server and client sockets
      TcpSocket server;
      TcpSocket client;
      
      // set receive timeouts to 10 seconds
      server.setReceiveTimeout(10000);
      client.setReceiveTimeout(10000);
      
      // bind and listen with server
      server.bind(address);
      server.listen();
      
      cout << "Server listening at host: " << address->getHost() << endl;
      cout << "Server listening at address: " << address->getAddress() << endl;
      cout << "Server listening on port: " << address->getPort() << endl;
      
      // connect with client
      client.connect(address);
      
      cout << "Client connected." << endl;
      
      // accept a connection
      TcpSocket* worker = (TcpSocket*)server.accept(10);
      
      cout << "Client connection accepted by Server." << endl;
      
      // send some data with client
      string clientData = "Hello there, Server.";
      client.getOutputStream()->write(clientData.c_str(), clientData.length());
      
      cout << "Client sent: " << clientData << endl;
      
      // receive the client data
      char read[2048];
      int numBytes = worker->getInputStream()->read(read, 2048);
      string serverReceived(read, numBytes);
      
      cout << "Server received: " << serverReceived << endl;
      
      // send some data with server
      string serverData = "G'day, Client.";
      worker->getOutputStream()->write(serverData.c_str(), serverData.length());
      
      cout << "Server sent: " << serverData << endl;
      
      // receive the server data
      numBytes = client.getInputStream()->read(read, 2048);
      string clientReceived(read, numBytes);
      
      cout << "Client received: " << clientReceived << endl;
      
      // close sockets
      client.close();
      server.close();
      
      // delete worker
      if(worker != NULL)
      {
         worker->close();
         delete worker;
      }
      
      cout << "Sockets closed." << endl;
   }
   
   cout << endl << "TCP Client/Server test complete." << endl;
}

void runUdpClientServerTest()
{
   cout << "Running UDP Client/Server Test" << endl << endl;
   
   InternetAddress* sa;
   InternetAddress* ca;
   InternetAddress serverAddress("127.0.0.1", 9999);
   InternetAddress clientAddress("127.0.0.1", 0);
   //Internet6Address serverAddress("::1", 9999);
   //Internet6Address clientAddress("::1", 0);
   sa = &serverAddress;
   ca = &clientAddress;
   
   // ensure host was known
   if(!Exception::hasLast())
   {
      // create udp server and client sockets
      UdpSocket server;
      UdpSocket client;
      
      // set receive timeouts to 10 seconds
      server.setReceiveTimeout(10000);
      client.setReceiveTimeout(10000);
      
      // bind with server
      server.bind(sa);
      
      cout << "Server bound at host: " << sa->getHost() << endl;
      cout << "Server bound at address: " << sa->getAddress() << endl;
      cout << "Server bound on port: " << sa->getPort() << endl;
      
      // bind with client
      client.bind(ca);
      client.getLocalAddress(ca);
      
      cout << "Client bound at host: " << ca->getHost() << endl;
      cout << "Client bound at address: " << ca->getAddress() << endl;
      cout << "Client bound on port: " << ca->getPort() << endl;
      
      // send some data with client
      string clientData = "Hello there, Server.";
      client.sendDatagram(clientData.c_str(), clientData.length(), sa);
      
      cout << "Client sent: " << clientData << endl;
      
      // receive the client data
      char read[2048];
      int numBytes = server.receiveDatagram(read, 2048, ca);
      string serverReceived(read, numBytes);
      
      cout << "Server received: " << serverReceived << endl;
      cout << "Data from: " << ca->getAddress();
      cout << ":" << ca->getPort() << endl;
      
      // send some data with server
      string serverData = "G'day, Client.";
      server.sendDatagram(serverData.c_str(), serverData.length(), ca);
      
      cout << "Server sent: " << serverData << endl;
      
      // receive the server data
      numBytes = client.receiveDatagram(read, 2048, sa);
      string clientReceived(read, numBytes);
      
      cout << "Client received: " << clientReceived << endl;
      cout << "Data from: " << sa->getAddress();
      cout << ":" << sa->getPort() << endl;
      
      // close sockets
      client.close();
      server.close();
      
      cout << "Sockets closed." << endl;
   }
   
   cout << endl << "UDP Client/Server test complete." << endl;
}

void runDatagramTest()
{
   cout << "Running Datagram Test" << endl << endl;
   
   InternetAddress* sa;
   InternetAddress* ca;
   InternetAddress serverAddress("127.0.0.1", 9999);
   InternetAddress clientAddress("127.0.0.1", 0);
   //Internet6Address serverAddress("::1", 9999);
   //Internet6Address clientAddress("::1", 0);
   sa = &serverAddress;
   ca = &clientAddress;
   
   // ensure host was known
   if(!Exception::hasLast())
   {
      // create datagram server and client sockets
      DatagramSocket server;
      DatagramSocket client;
      
      // set receive timeouts to 10 seconds
      server.setReceiveTimeout(10000);
      client.setReceiveTimeout(10000);
      
      // bind with server
      server.bind(sa);
      
      cout << "Server bound at host: " << sa->getHost() << endl;
      cout << "Server bound at address: " << sa->getAddress() << endl;
      cout << "Server bound on port: " << sa->getPort() << endl;
      
      // bind with client
      client.bind(ca);
      client.getLocalAddress(ca);
      
      cout << "Client bound at host: " << ca->getHost() << endl;
      cout << "Client bound at address: " << ca->getAddress() << endl;
      cout << "Client bound on port: " << ca->getPort() << endl;
      
      // create a datagram
      Datagram d1(sa);
      d1.assignString("Hello there, Server.");
      
      // send the datagram with the client
      client.send(&d1);
      
      cout << "Client sent: " << d1.getString() << endl;
      
      // create a datagram
      char externalData[2048];
      Datagram d2(ca);
      d2.setData(externalData, 2048, false);
      
      // receive a datagram
      server.receive(&d2);
      
      cout << "Server received: " << d2.getString() << endl;
      cout << "Data from: " << d2.getAddress()->getAddress();
      cout << ":" << d2.getAddress()->getPort() << endl;
      
      // send a datagram with the server
      d2.assignString("G'day, Client.");
      server.send(&d2);
      
      cout << "Server sent: " << d2.getString() << endl;
      
      // receive the server datagram
      Datagram d3(sa, 2048);
      client.receive(&d3);
      
      cout << "Client received: " << d3.getString() << endl;
      cout << "Data from: " << d3.getAddress()->getAddress();
      cout << ":" << d3.getAddress()->getPort() << endl;
      
      // close sockets
      client.close();
      server.close();
      
      cout << "Sockets closed." << endl;
   }
   
   cout << endl << "Datagram test complete." << endl;
}

void runUrlEncodeTest(TestRunner& tr)
{
   tr.test("Url Encode/Decode");
   
   string str = "billy bob & \"jane\" +^%2{13.";
   
   string encoded = Url::encode(str.c_str(), str.length());
   string decoded = Url::decode(encoded.c_str(), encoded.length());
   
   //cout << "test data=" << str << endl;
   
   //cout << "url encoded=" << encoded << endl;
   //cout << "url decoded=" << decoded << endl;
   
   assert(decoded == str);
   
   tr.pass();
}

void dumpUrl(Url url)
{
   if(Exception::hasLast())
   {
      cout << "url=[exception]" << endl;
   }
   else
   {
      string str;
      url.toString(str);
      
      cout << "url=" << str << endl;
      cout << " scheme=" << url.getScheme() << endl;
      cout << " scheme specific part=" << url.getSchemeSpecificPart() << endl;
      cout << " authority=" << url.getAuthority() << endl;
      cout << " userinfo=" << url.getUserInfo() << endl;
      cout << " user=" << url.getUser() << endl;
      cout << " password=" << url.getPassword() << endl;
      cout << " host=" << url.getHost() << endl;
      cout << " port=" << url.getPort() << endl;
      cout << " path=" << url.getPath() << endl;
      cout << " query=" << url.getQuery() << endl;
   }
}

void runUrlTest(TestRunner& tr)
{
   tr.test("Url");

   {
      Url url("http:");
      
      //dumpUrl(url);
      assert(url.getScheme() == "http");
      assert(url.getSchemeSpecificPart() == "");
   }
   
   {
      Url url("http://");
      
      //dumpUrl(url);
      assert(url.getScheme() == "http");
      assert(url.getSchemeSpecificPart() == "//");
   }
   
   {
      Url url("http://www.bitmunk.com");
      
      //dumpUrl(url);
      assert(url.getScheme() == "http");
      assert(url.getSchemeSpecificPart() == "//www.bitmunk.com");
      assert(url.getHost() == "www.bitmunk.com");
      assert(url.getPath() == "/");
   }
   
   {
      Url url("http://www.bitmunk.com/mypath?variable1=test");
      
      //dumpUrl(url);
      assert(url.getScheme() == "http");
      assert(url.getUserInfo() == "");
      assert(url.getUser() == "");
      assert(url.getPassword() == "");
      assert(url.getHost() == "www.bitmunk.com");
      assert(url.getPort() == 80);
      assert(url.getPath() == "/mypath");
      assert(url.getQuery() == "variable1=test");
   }
   
   {
      Url url("mysql://username:password@host:3306/mydatabase");
      
      //dumpUrl(url);
      assert(url.getScheme() == "mysql");
      assert(url.getUser() == "username");
      assert(url.getPassword() == "password");
      assert(url.getHost() == "host");
      assert(url.getPort() == 3306);
      assert(url.getPath() == "/mydatabase");
   }
   
   {
      Url url("http://example.com:8080/path");

      //dumpUrl(url);
      assert(!Exception::hasLast());
      assert(url.getScheme() == "http");
      assert(url.getUserInfo() == "");
      assert(url.getUser() == "");
      assert(url.getPassword() == "");
      assert(url.getHost() == "example.com");
      assert(url.getPort() == 8080);
      assert(url.getPath() == "/path");
      assert(url.getQuery() == "");
   }
   
   {   
      Url url("scheme:schemespecific");

      //dumpUrl(url);
      assert(!Exception::hasLast());
      assert(url.getScheme() == "scheme");
      assert(url.getSchemeSpecificPart() == "schemespecific");
   }
   
   {
      Url url("scheme://user:password@host:1234/path?key1=value1&key2=value2");

      //dumpUrl(url);
      assert(!Exception::hasLast());
      assert(url.getScheme() == "scheme");
      assert(url.getUserInfo() == "user:password");
      assert(url.getUser() == "user");
      assert(url.getPassword() == "password");
      assert(url.getHost() == "host");
      assert(url.getPort() == 1234);
      assert(url.getPath() == "/path");
      assert(url.getQuery() == "key1=value1&key2=value2");
      // FIXME add query part checks
   }
   
   tr.pass();
}

class InterruptServerSocketTest : public virtual Object, public Runnable
{
public:
   /**
    * Runs the unit tests.
    */
   virtual void run()
   {
      runServerSocketTest();
      
      if(Exception::hasLast())
      {
         Exception* e = Exception::getLast();
         cout << "Exception occurred!" << endl;
         cout << "message: " << e->getMessage() << endl;
         cout << "code: " << e->getCode() << endl;
      }      
   }
};

void runInterruptServerSocketTest(TestRunner& tr)
{
   tr.test("Thread Interrupt");
   
   InterruptServerSocketTest runnable;
   Thread t(&runnable);
   t.start();
   
   //cout << "Waiting for thread..." << endl;
   Thread::sleep(2000);
   //cout << "Finished waiting for thread." << endl;
   
   //cout << "Interrupting thread..." << endl;
   t.interrupt();
   
   //cout << "Joining thread..." << endl;
   t.join();
   //cout << "Thread joined." << endl;
   
   tr.pass();
}

class TestConnectionServicer1 : public ConnectionServicer
{
public:
   unsigned int serviced;
   string reply;
   
   TestConnectionServicer1()
   {
      serviced = 0;
      reply = "HTTP/1.0 200 OK\r\nContent-Length: 0\r\n\r\n";
      //reply = "HTTP/1.0 404 Not Found\r\n";
   }
   
   virtual ~TestConnectionServicer1() {}
   
   void serviceConnection(Connection* c)
   {
      //cout << "1: Servicing connection!" << endl;
      
      char b[100];
      int numBytes = 0;
      
      //cout << endl << "Reading HTTP..." << endl;
      
      InputStream* is = c->getInputStream();
      numBytes = is->peek(b, 100);
      if(numBytes > 0)
      {
//         cout << "Read " << numBytes << " bytes." << endl;
//         string str = "";
//         str.append(b, numBytes);
//         cout << "HTTP=" << endl << str << endl;
      }
      
      OutputStream* os = c->getOutputStream();
      os->write(reply.c_str(), reply.length());
      
      //cout << "1: Finished servicing connection." << endl;
      
      serviced++;
      //cout << "Connections serviced=" << serviced << endl;
   }
};

class TestConnectionServicer2 : public ConnectionServicer
{
   void serviceConnection(Connection* c)
   {
      cout << "2: Servicing connection!" << endl;
      cout << "2: Finished servicing connection." << endl;
   }
};

class TestConnectionServicer3 : public ConnectionServicer
{
   void serviceConnection(Connection* c)
   {
      cout << "3: Servicing connection!" << endl;
      cout << "3: Finished servicing connection." << endl;
   }
};

void runServerConnectionTest()
{
   cout << "Starting Server Connection test." << endl << endl;
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("0.0.0.0", 19100);
   
   // create generic service
   TestConnectionServicer1 tcs1;
   server.addConnectionService(&address, &tcs1);
   
//   // create generic service (stomp on other service)
//   TestConnectionServicer2 tcs2;
//   server.addConnectionService(&address, &tcs2);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
//   // create generic service (stomp on second service, dynamically stop/start)
//   TestConnectionServicer3 tcs3;
//   if(!server.addConnectionService(&address, &tcs3))
//   {
//      cout << "Could not start service 3!, exception=" <<
//         Exception::getLast()->getMessage() << endl;
//   }
//   
//   Thread::sleep(5000);
//   
//   // create generic service (stomp on third service, dynamically stop/start)
//   if(!server.addConnectionService(&address, &tcs2))
//   {
//      cout << "Could not start service 2!, exception=" <<
//         Exception::getLast()->getMessage() << endl;
//   }
   
   Object lock;
   lock.lock();
   {
      lock.wait();//lock.wait(120000);
      //lock.wait(30000);
   }
   lock.unlock();
   //Thread::sleep(60000);
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   cout << endl << "Server Connection test complete." << endl;
}
unsigned int gConnections = 0;

class BlastConnections : public Runnable
{
public:
   InternetAddress* address;
   
   BlastConnections(InternetAddress* a)
   {
      address = a;
   }
   
   virtual ~BlastConnections()
   {
   }
   
   void run()
   {
      //Thread::sleep(20000);
      
      TcpSocket socket;
      socket.setReceiveTimeout(1000);
      
      //InternetAddress address2("mojo.bitmunk.com", 9120);
      
      // blast connections
      int connections = 50;
      char b[1024];
      string request =
         "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
      for(int i = 0; i < connections; i++)
      {
         // connect
         if(socket.connect(address))
         {
            //cout << "connected" << endl;
            
            // send request
            if(socket.send(request.c_str(), request.length()))
            {
               // receive response
               socket.receive(b, 1024);
            }
            else
            {
               cout << "Exception=" <<
                  Exception::getLast()->getMessage() << endl;
            }
         }
         else
         {
            cout << "Exception=" <<
               Exception::getLast()->getMessage() << endl;
         }
         
         // close socket
         socket.close();
         
         gConnections++;
      }      
   }
};

void runServerSslConnectionTest()
{
   cout << "Starting Server SSL Connection test." << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
//   // create SSL-only service
//   TestConnectionServicer1 tcs1;
//   SslContext context;
//   SslSocketDataPresenter presenter(&context);
//   server.addConnectionService(&address, &tcs1, &presenter);
   
   // create SSL/generic service
   TestConnectionServicer1 tcs1;
   SslContext context;
   SslSocketDataPresenter presenter1(&context);
   NullSocketDataPresenter presenter2;
   SocketDataPresenterList list(false);
   list.add(&presenter1);
   list.add(&presenter2);
   server.addConnectionService(&address, &tcs1, &list);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
   BlastConnections bc(&address);
   Thread t1(&bc);
   Thread t2(&bc);
   Thread t3(&bc);
   Thread t4(&bc);
   Thread t5(&bc);
   Thread t6(&bc);
   Thread t7(&bc);
   Thread t8(&bc);
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   t1.start();
   t2.start();
//   t3.start();
//   t4.start();
//   t5.start();
//   t6.start();
//   t7.start();
//   t8.start();
   
   t1.join();
   t2.join();
//   t3.join();
//   t4.join();
//   t5.join();
//   t6.join();
//   t7.join();
//   t8.join();
   cout << "all client threads joined." << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   long double time = end - start;
   long double secs = time / 1000.0;
   unsigned int connections = gConnections;//tcs1.serviced
   double rate = (double)connections / secs;
   
   cout << "Connections=" << tcs1.serviced << endl;
   cout << "Time=" << time << " ms = " << secs << " secs" << endl;
   cout << "Connections/second=" << rate << endl;
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   // clean up SSL
   EVP_cleanup();
   
   cout << endl << "Server SSL Connection test complete." << endl;
}

class TestDatagramServicer : public DatagramServicer
{
   void serviceDatagrams(DatagramSocket* s)
   {
      cout << "Servicing datagrams!" << endl;
      cout << "Finished servicing datagrams." << endl;
   }
};

void runServerDatagramTest()
{
   cout << "Starting Server Datagram test." << endl << endl;
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 10080);
   
   // create datagram service
   TestDatagramServicer tds;
   server.addDatagramService(&address, &tds);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
   Thread::sleep(10000);
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   cout << endl << "Server Datagram test complete." << endl;
}

void runHttpHeaderTest()
{
   cout << "Starting HttpHeader test." << endl << endl;
   
   // test bicapitalization of http headers
   char test[] = "ThIs-a-BICaPitAlized-hEADer";
   HttpHeader::biCapitalize(test);
   
   cout << "BiCapitalized Header=" << test << endl;
   
//   string t = "   d  f  ";
//   StringTools::trim(t);
//   cout << "t='" << t << "'" << endl;
   
   cout << endl << "Request Header:" << endl;
   
   HttpRequestHeader reqHeader;
   reqHeader.setDate();
   reqHeader.setMethod("GET");
   reqHeader.setPath("/");
   reqHeader.setVersion("HTTP/1.1");
   reqHeader.setField("host", "localhost:80");
   reqHeader.setField("Content-Type", "text/html");
   reqHeader.setField("Connection", "close");
   
   string str;
   reqHeader.toString(str);
   cout << str;
   
   cout << "End of Request Header." << endl;
   
   cout << endl << "Parsed Request Header:" << endl;
   
   HttpRequestHeader reqHeader2;
   reqHeader2.parse(str);
   
   string str2;
   reqHeader2.toString(str2);
   cout << str2;
   
   cout << "End of Parsed Request Header." << endl;
   
   cout << endl << "Response Header:" << endl;
   
   HttpResponseHeader resHeader;
   resHeader.setDate();
   resHeader.setVersion("HTTP/1.1");
   resHeader.setStatus(404, "Not Found");
   resHeader.setField("host", "localhost:80");
   resHeader.setField("Content-Type", "text/html");
   resHeader.setField("Connection", "close");
   
   resHeader.toString(str);
   cout << str;
   
   cout << "End of Response Header." << endl;
   
   cout << endl << "Parsed Response Header:" << endl;
   
   HttpResponseHeader resHeader2;
   resHeader2.parse(str);
   
   resHeader2.toString(str2);
   cout << str2;
   
   cout << "End of Parsed Response Header." << endl;
   
   cout << endl << "HttpHeader test complete." << endl;
}

class TestHttpRequestServicer : public HttpRequestServicer
{
public:
   const char* content;
   
   TestHttpRequestServicer(const char* path) : HttpRequestServicer(path)
   {
      content = "Bob Loblaw's Law Blog";
   }
   
   virtual ~TestHttpRequestServicer()
   {
   }
   
   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      // send 200 OK
      response->getHeader()->setStatus(200, "OK");
      //response->getHeader()->setField("Content-Length", 0);
      response->getHeader()->setField("Transfer-Encoding", "chunked");
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      
      HttpTrailer trailer;
      ByteArrayInputStream bais(content, strlen(content));
      response->sendBody(&bais, &trailer);
   }
};

void runHttpServerTest()
{
   cout << "Starting Http Server test." << endl << endl;
   
//   // openssl initialization code
//   SSL_library_init();
//   SSL_load_error_strings();
//   OpenSSL_add_all_algorithms();
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
   // create SSL/generic http connection servicer
   HttpConnectionServicer hcs;
//   SslContext context;
//   SslSocketDataPresenter presenter1(&context);
//   NullSocketDataPresenter presenter2;
//   SocketDataPresenterList list(false);
//   list.add(&presenter1);
//   list.add(&presenter2);
   server.addConnectionService(&address, &hcs);//, &list);
   
   // create test http request servicer
   TestHttpRequestServicer test1("/test");
   hcs.addRequestServicer(&test1, false);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
   // sleep
   Thread::sleep(30000);
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
//   // clean up SSL
//   ERR_remove_state(0);
//   ENGINE_cleanup();
//   ERR_free_strings();
//   EVP_cleanup();
//   CRYPTO_cleanup_all_ex_data();
   
   cout << endl << "Http Server test complete." << endl;
}

void runHttpClientGetTest()
{
   cout << "Starting Http Client GET test." << endl << endl;
   
   // create client
   HttpClient client;
   
   // connect
   Url url("http://www.bitmunk.com");
   if(client.connect(&url))
   {
      string str;
      cout << "Connected to: " << url.toString(str) << endl;
      InternetAddress address(url.getHost(), url.getPort());
      cout << address.toString(str) << endl;
      
      // do get
      const char* headers[] = {"Test-Header: bacon", NULL};
      HttpResponse* response = client.get(&url, headers);
      if(response != NULL)
      {
         cout << "Response=" << endl <<
            response->getHeader()->toString(str) << endl;
         if(response->getHeader()->getStatusCode() == 200)
         {
            // receive content
            HttpTrailer trailer;
            File file("/tmp/index.html");
            FileOutputStream fos(&file);
            IOException* e = client.receiveContent(&fos, &trailer);
            if(e == NULL)
            {
               cout << "Content downloaded to '" <<
                  file.getName() << "'" << endl;
               
               cout << "HTTP trailers=\n" << trailer.toString(str) << endl;
            }
            else
            {
               cout << "IOException!,message=" << e->getMessage() << endl;
            }
         }
      }
      else
      {
         cout << "There was no response!" << endl;
      }
      
      cout << "Disconnecting..." << endl;
      client.disconnect();
      cout << "Disconnected." << endl;
   }
   
   cout << endl << "Http Client GET test complete." << endl;
}

void runHttpClientPostTest()
{
   cout << "Starting Http Client POST test." << endl << endl;
   
   // create client
   HttpClient client;
   
   // connect
   Url url("http://www.bitmunk.com");
   if(client.connect(&url))
   {
      string str;
      cout << "Connected to: " << url.toString(str) << endl;
      InternetAddress address(url.getHost(), url.getPort());
      cout << address.toString(str) << endl;
      
      char someData[] = "Just some post data.";
      ByteArrayInputStream baos(someData, strlen(someData));
      
      // do post
      const char* headers[] = {
         "Content-Type: text/plain",
         "Transfer-Encoding: chunked",
         NULL};
      
      HttpTrailer trailer;
      HttpResponse* response = client.post(&url, headers, &baos, &trailer);
      if(response != NULL)
      {
         cout << "Response=" << endl <<
            response->getHeader()->toString(str) << endl;
         if(response->getHeader()->getStatusCode() == 200)
         {
            // receive content
            trailer.clearFields();
            File file("/tmp/postresponse.txt");
            FileOutputStream fos(&file);
            IOException* e = client.receiveContent(&fos, &trailer);
            if(e == NULL)
            {
               cout << "Content downloaded to '" <<
                  file.getName() << "'" << endl;
               
               cout << "HTTP trailers=\n" << trailer.toString(str) << endl;
            }
            else
            {
               cout << "IOException!,message=" << e->getMessage() << endl;
            }
         }
      }
      else
      {
         cout << "There was no response!" << endl;
      }
      
      cout << "Disconnecting..." << endl;
      client.disconnect();
      cout << "Disconnected." << endl;
   }
   
   cout << endl << "Http Client POST test complete." << endl;
}

class PingHttpRequestServicer : public HttpRequestServicer
{
public:
   const char* content;
   
   PingHttpRequestServicer(const char* path) : HttpRequestServicer(path)
   {
      content = "Bob Loblaw's Law Blog";
   }
   
   virtual ~PingHttpRequestServicer()
   {
   }
   
   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      // send 200 OK
      response->getHeader()->setStatus(200, "OK");
      //response->getHeader()->setField("Content-Length", 0);
      response->getHeader()->setField("Transfer-Encoding", "chunked");
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      
      HttpTrailer trailer;
      ByteArrayInputStream bais(content, strlen(content));
      response->sendBody(&bais, &trailer);
   }
};

class PingConnectionServicer : public ConnectionServicer
{
public:
   unsigned long long start;
   unsigned long long end;
   unsigned int serviced;
   
   PingConnectionServicer()
   {
      serviced = 0;
      start = 0;
      end = 0;
   }
   
   virtual ~PingConnectionServicer() {}
   
   void serviceConnection(Connection* c)
   {
      if(start == 0)
      {
         start = System::getCurrentMilliseconds();
      }
      
      serviced++;
      end = System::getCurrentMilliseconds();
   }
};

void runPingTest()
{
   cout << "Starting Ping test." << endl << endl;
   
//   // openssl initialization code
//   SSL_library_init();
//   SSL_load_error_strings();
//   OpenSSL_add_all_algorithms();
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
//   // create SSL/generic ping connection servicer
//   PingConnectionServicer pcs;
////   SslContext context;
////   SslSocketDataPresenter presenter1(&context);
////   NullSocketDataPresenter presenter2;
////   SocketDataPresenterList list(false);
////   list.add(&presenter1);
////   list.add(&presenter2);
//   server.addConnectionService(&address, &pcs);//, &list);

   // create SSL/generic http connection servicer
   HttpConnectionServicer hcs;
//   SslContext context;
//   SslSocketDataPresenter presenter1(&context);
//   NullSocketDataPresenter presenter2;
//   SocketDataPresenterList list(false);
//   list.add(&presenter1);
//   list.add(&presenter2);
   server.addConnectionService(&address, &hcs);//, &list);
   
   // create test http request servicer
   PingHttpRequestServicer test1("/test");
   hcs.addRequestServicer(&test1, false);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
   // connect
   Url url("http://localhost:19100");
   HttpTrailer trailer;
   File file("/tmp/index.html");
   FileOutputStream fos(&file);
   HttpClient client;
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   client.connect(&url);
   client.get(&url, NULL);
   client.receiveContent(&fos, &trailer);
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   client.disconnect();
   
   // sleep
   //Thread::sleep(10000);
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   unsigned long long millis = end - start;
   cout << "Connection Time: " << millis << endl;
   
//   unsigned long long millis = test1.end - test1.start;
//   long double cps = ((long double)pcs.serviced) / millis * 1000.0;
//   cout << "Connections serviced: " << pcs.serviced << endl;
//   cout << "Time: " << millis << endl;
//   cout << "Connections/Second: " << cps << endl;
   
//   // clean up SSL
//   ERR_remove_state(0);
//   ENGINE_cleanup();
//   ERR_free_strings();
//   EVP_cleanup();
//   CRYPTO_cleanup_all_ex_data();
   
   cout << endl << "Ping test complete." << endl;
}

void runDelegateTest()
{
   cout << "Starting Delegate test." << endl << endl;
   
   // FIXME:
   
   cout << endl << "Delegate test complete." << endl;
}

class TestContent
{
protected:
   char* mContent;
   
public:
   TestContent()
   {
      mContent = new char[1];
      memset(mContent, 0, 1);
   }
   
   virtual ~TestContent()
   {
      delete [] mContent;
   }
   
   virtual void setContent(const char* str)
   {
      delete mContent;
      mContent = new char[strlen(str) + 1];
      strcpy(mContent, str);
   }
   
   virtual const char* getContent()
   {
      return mContent;
   }
};

class TestChild : public TestContent
{
protected:
   int mId;
   
public:
   TestChild()
   {
      mId = 0;
   }
   
   virtual ~TestChild()
   {
   }
   
   virtual void setId(int id)
   {
      mId = id;
   }
   
   virtual int getId()
   {
      return mId;
   }
};

class TestParent : public TestContent
{
protected:
   TestChild* mChild;
   
public:
   TestParent()
   {
      mChild = NULL;
   }
   
   virtual ~TestParent()
   {
      if(mChild != NULL)
      {
         delete mChild;
      }
   }
   
   virtual TestChild* createChild()
   {
      return new TestChild();
   }
   
   virtual void addChild(TestChild* child)
   {
      if(mChild != NULL)
      {
         delete mChild;
      }
      
      mChild = child;
   }
   
   virtual TestChild* getChild()
   {
      return mChild;
   }
};

class TestChildDataBinding : public DataBinding
{
protected:
   DataMappingFunctor<TestChild> mChildContent;
   DataMappingFunctor<TestChild> mChildId;
   
public:
   TestChildDataBinding(TestChild* c = NULL) :
      DataBinding(c),
      mChildContent(&TestChild::setContent, &TestChild::getContent),
      mChildId(&TestChild::setId, &TestChild::getId)
   {
      // set root data name
      setDataName(NULL, "TestChild");
      
      // add mappings
      addDataMapping(NULL, "id", false, true, &mChildId);
      addDataMapping(NULL, "TestContent", true, false, &mChildContent);
   }
   
   virtual ~TestChildDataBinding()
   {
   }
};

class TestParentDataBinding : public DataBinding
{
protected:
   TestParent* mTestParent;
   TestChildDataBinding mChildBinding;
   DataMappingFunctor<TestParent> mTestContent;
   DataMappingFunctor<TestParent, TestChild> mCreateChild;
   
public:
   TestParentDataBinding(TestParent* p) :
      DataBinding(p),
      mTestContent(&TestParent::setContent, &TestParent::getContent),
      mCreateChild(&TestParent::createChild, &TestParent::addChild)
   {
      mTestParent = p;
      
      // set root data name
      setDataName(NULL, "TestContent");
      
      // add mappings
      addDataMapping(NULL, "TestContent", true, false, &mTestContent);
      addDataMapping(NULL, "TestChild", true, true, &mCreateChild);
      
      // add bindings
      addDataBinding(NULL, "TestChild", &mChildBinding);
   }
   
   virtual ~TestParentDataBinding()
   {
   }
   
   virtual void getChildren(DataName* dn, list<void*>& children)
   {
      if(mTestParent->getChild() != NULL)
      {
         children.push_back(mTestParent->getChild());
      }
   }
};

void runXmlReaderTest()
{
   cout << "Starting XmlReader test." << endl << endl;
   
   XmlReader reader;
   
//   string xml;
//   xml.append("<Book><Chapter number=\"1\"><Title>Test Chapter 1</Title>");
//   xml.append("<Content>This is the first chapter of the book.</Content>");
//   xml.append("</Chapter><Chapter number=\"2\"/></Book>");
   
   string xml;
   //xml = "<TestContent>client request<TestChild id=\"1\"/></TestContent>";
   xml.append("<TestContent>This is my content.");
   xml.append("<TestChild id=\"12\">Blah</TestChild></TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&db);
   reader.read(&bais);
   reader.finish();
   
   cout << "TestContent data='" << p.getContent() << "'" << endl;
   if(p.getChild() != NULL)
   {
      cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
      cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   }
   else
   {
      cout << "TestChild does not exist!" << endl;
   }
   
   cout << endl << "XmlReader test complete." << endl;
}

void runXmlWriterTest()
{
   cout << "Starting XmlWriter test." << endl << endl;
   
   // main object to write out
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   XmlWriter writer;
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   // write out xml
   writer.write(&db, &os);
   cout << "XML empty=\n" << oss.str() << endl;
   
   // clear string stream, reset writer
   oss.str("");
   writer.reset();
   
   // set some content
   p.setContent("Moooooooo");
   
   // add child to TestContent
   TestChild* c = new TestChild();
   c->setId(514);
   p.addChild(c);
   
   writer.write(&db, &os);
   cout << "XML full=\n" << oss.str() << endl;
   
   cout << endl << "XmlWriter test complete." << endl;
}

void runXmlReadWriteTest()
{
   cout << "Starting XmlReadWrite test." << endl << endl;
   
   XmlReader reader;
   
   string xml;
   xml.append("<TestContent>This is my content.");
   xml.append("<TestChild id=\"12\">Blah</TestChild></TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&db);
   reader.read(&bais);
   reader.finish();
   
   cout << "*****DOING XML READ*****" << endl;
   
   cout << "TestContent data='" << p.getContent() << "'" << endl;
   if(p.getChild() != NULL)
   {
      cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
      cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   }
   else
   {
      cout << "TestChild does not exist!" << endl;
   }
   
   cout << endl << "*****DOING XML WRITE*****" << endl;
   
   XmlWriter writer;
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   // write out xml
   writer.write(&db, &os);
   
   cout << "XML=\n" << oss.str() << endl;
   
   cout << endl << "XmlReadWrite test complete." << endl;
}

void runXmlBindingInputStreamTest()
{
   cout << "Starting XmlBindingInputStream test." << endl << endl;
   
   // main object to read xml from
   TestParent p;
   
   // set some content
   p.setContent("This is a sufficiently long section of element data.");
   
   // add child to TestContent
   TestChild* c = new TestChild();
   c->setId(514);
   p.addChild(c);
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create input stream
   XmlBindingInputStream xbis(&db, 20);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   char b[10];
   int numBytes;
   while((numBytes = xbis.read(b, 10)) > 0)
   {
      os.write(b, numBytes);
   }
   
   cout << "XML=\n" << oss.str() << endl;
   
   cout << endl << "XmlBindingInputStream test complete." << endl;
}

void runXmlBindingOutputStreamTest()
{
   cout << "Starting XmlBindingOutputStream test." << endl << endl;
   
   string xml1;
   string xml2;
   xml1.append("<TestContent>This is the first.");
   xml2.append("<TestChild id=\"64\">Blah</TestChild> Second.</TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create output stream for writing to binding
   XmlBindingOutputStream xbos(&db);
   
   // write xml to output stream
   xbos.write(xml1.c_str(), xml1.length());
   xbos.write(xml2.c_str(), xml2.length());
   //xbos.write((xml1 + xml2).c_str(), xml1.length() + xml2.length());
   
   cout << "TestContent data='" << p.getContent() << "'" << endl;
   if(p.getChild() != NULL)
   {
      cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
      cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   }
   else
   {
      cout << "TestChild does not exist!" << endl;
   }
   
   cout << endl << "XmlBindingOutputStream test complete." << endl;
}

class XmlHttpRequestServicer : public HttpRequestServicer
{
public:
   XmlHttpRequestServicer(const char* path) : HttpRequestServicer(path)
   {
   }
   
   virtual ~XmlHttpRequestServicer()
   {
   }
   
   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      // receive body
      ostringstream oss;
      OStreamOutputStream os(&oss);
      request->receiveBody(&os);
      string xml = oss.str();
      
      // xml object to populate
      TestParent p2;
      TestParentDataBinding db2(&p2);
      
      // read object from xml
      ByteArrayInputStream bais(xml.c_str(), xml.length());
      XmlReader reader;
      reader.start(&db2);
      reader.read(&bais);
      reader.finish();
      
      assert(strcmp(p2.getContent(), "client request") == 0);
      assert(p2.getChild()->getId() == 1);
      
      // send 200 OK
      response->getHeader()->setStatus(200, "OK");
      response->getHeader()->setField("Content-Type", "text/xml");
      response->getHeader()->setField("Transfer-Encoding", "chunked");
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      
      OutputStream* bos = response->getBodyOutputStream();
      
      // create xml object to write out
      TestParent p;
      p.setContent("server response");
      TestChild* c = new TestChild();
      c->setId(2);
      p.addChild(c);
      
      // data binding for object
      TestParentDataBinding db(&p);
      
      // write out xml
      XmlWriter writer;
      writer.write(&db, bos);
      
      // close and clean up output stream
      bos->close();
      delete bos;
   }
};

void runXmlHttpServerTest(TestRunner& tr)
{
   tr.test("XmlHttpServer");
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
   // create SSL/generic http connection servicer
   HttpConnectionServicer hcs;
   server.addConnectionService(&address, &hcs);
   
   // create xml http request servicer
   XmlHttpRequestServicer test1("/test");
   hcs.addRequestServicer(&test1, false);
   
   server.start();
   assertNoException();
   
   // connect
   Url url("http://localhost:19100");
   HttpConnection* hc = HttpClient::createConnection(&url);
   assert(hc != NULL);
   
   // send request header
   HttpRequest* request = (HttpRequest*)hc->createRequest();
   request->getHeader()->setMethod("POST");
   request->getHeader()->setPath("/test");
   request->getHeader()->setVersion("HTTP/1.1");
   request->getHeader()->setField("Host", "localhost:19100");
   request->getHeader()->setField("Content-Type", "text/xml");
   request->getHeader()->setField("Transfer-Encoding", "chunked");
   request->sendHeader();
   assertNoException();
   
   // send request body
   OutputStream* bos = request->getBodyOutputStream();
   
   // create xml object to write out
   TestParent p;
   p.setContent("client request");
   TestChild* c = new TestChild();
   c->setId(1);
   p.addChild(c);
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // write out xml
   XmlWriter writer;
   writer.write(&db, bos);
   
   // close and clean up output stream
   bos->close();
   delete bos;
   
   // receive response header
   HttpResponse* response = (HttpResponse*)request->createResponse();
   response->receiveHeader();
   assertNoException();
   
   // receive response body
   ostringstream oss;
   OStreamOutputStream os(&oss);
   response->receiveBody(&os);
   string xml = oss.str();
   
   // xml object to populate
   TestParent p2;
   TestParentDataBinding db2(&p2);
   
   // read object from xml
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   XmlReader reader;
   reader.start(&db2);
   reader.read(&bais);
   reader.finish();
   
   assert(strcmp(p2.getContent(), "server response") == 0);
   assert(p2.getChild()->getId() == 2);
   
   // clean up request and response
   delete request;
   delete response;
   
   // close and clean up connection
   hc->close();
   delete hc;
   
   // stop server
   server.stop();
   
   // stop kernel engine
   k.getEngine()->stop();
   
   tr.pass();
}

void runDynamicObjectWriterTest(TestRunner& tr)
{
   tr.test("DynamicObjectWriter");
   
   // main object to write to DynamicObject
   TestParent p;
   
   // set some content
   p.setContent("This is test content.");
   
   // add child to TestContent
   TestChild* c = new TestChild();
   c->setId(514);
   c->setContent("This is child content.");
   p.addChild(c);
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create DynamicObjectWriter
   DynamicObjectWriter writer;
   
   // write out to dynamic object
   DynamicObject dyno = writer.write(&db);
   
   assert(
      strcmp(dyno["TestContent"]->getString(), "This is test content.") == 0);
   assert(
      strcmp(dyno["TestChild"]["TestContent"]->getString(),
      "This is child content.") == 0);
   assert(dyno["TestChild"]["id"]->getInt32() == 514);
   assert(dyno->length() == 2);
   
   // test print out code
   //cout << endl;
   //dumpDynamicObject(dyno);
   
   tr.pass();
}

void runDynamicObjectReaderTest(TestRunner& tr)
{
   tr.test("DynamicObjectReader");
   
   // dynamic object to read from
   DynamicObject dyno;
   dyno["TestContent"] = "This is test content.";
   dyno["TestChild"]["id"] = 514;
   dyno["TestChild"]["TestContent"] = "This is child content.";
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create DynamicObjectReader
   DynamicObjectReader reader;
   
   // read in from dynamic object
   reader.read(dyno, &db);
   
   assert(strcmp(p.getContent(), "This is test content.") == 0);
   assert(strcmp(p.getChild()->getContent(), "This is child content.") == 0);
   assert(p.getChild()->getId() == 514);
   
   tr.pass();
}

void runDynamicObjectBasicBindingTest(TestRunner& tr)
{
   tr.test("DynamicObjectBasicBinding");
   
   // create xml writer
   XmlWriter writer;
   writer.setIndentation(0, 1);
   
   // dynamic object to read from
   DynamicObject dyno1;
   DynamicObject dyno2;
   DynamicObject dyno3;
   DynamicObject dyno4;
   dyno1 = "This is test content.";
   dyno2 = true;
   dyno3 = 1234;
   dyno4 = 123.456789;
   
   DynamicObjectBasicBinding db1(&dyno1);
   DynamicObjectBasicBinding db2(&dyno2);
   DynamicObjectBasicBinding db3(&dyno3);
   DynamicObjectBasicBinding db4(&dyno4);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   string xml[4];
   writer.write(&db1, &os);
   xml[0] = oss.str();
   oss.str("");
   
   writer.write(&db2, &os);
   xml[1] = oss.str();
   oss.str("");
   
   writer.write(&db3, &os);
   xml[2] = oss.str();
   oss.str("");
   
   writer.write(&db4, &os);
   xml[3] = oss.str();
   oss.str("");
   
//   for(int i = 0; i < 4; i++)
//   {
//      cout << "XML " << i << "=" << endl << xml[i] << endl;
//   }
   
   string outxml[4];
   for(int i = 0; i < 4; i++)
   {
      // now try to read dynamic object back in
      DynamicObject dyno5;
      DynamicObjectBasicBinding inBinding(&dyno5);
      XmlReader reader;
      ByteArrayInputStream bais(xml[i].c_str(), xml[i].length());
      reader.start(&inBinding);
      reader.read(&bais);
      reader.finish();
      
      // now send dynamic object back out
      DynamicObjectBasicBinding outBinding(&dyno5);
      writer.write(&outBinding, &os);
      outxml[i] = oss.str();
      oss.str("");
      assert(strcmp(xml[i].c_str(), outxml[i].c_str()) == 0);
      //cout << "OUT XML " << i << "=" << endl << outxml[i] << endl;
   }
   
   tr.pass();
}

void runDynamicObjectArrayBindingTest(TestRunner& tr)
{
   tr.test("DynamicObjectArrayBinding");
   
   // create xml writer
   XmlWriter writer;
   writer.setIndentation(0, 1);
   
   // dynamic object to read from
   DynamicObject dyno;
   dyno[0] = "This is test content.";
   dyno[1] = true;
   dyno[2] = 1234;
   dyno[3] = 123.456789;
   
   DynamicObject dyno2;
   dyno2[0] = "Another string.";
   dyno2[1] = false;
   dyno2[2] = 4321;
   dyno2[3] = 987.654321;
   
   dyno[4] = dyno2;
   
   DynamicObjectArrayBinding db(&dyno);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   writer.write(&db, &os);
   string xml = oss.str();
   oss.str("");
   
   // now try to read dynamic object back in
   DynamicObject dyno5;
   DynamicObjectArrayBinding inBinding(&dyno5);
   XmlReader reader;
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&inBinding);
   reader.read(&bais);
   reader.finish();
   
   // now send dynamic object back out
   DynamicObjectArrayBinding outBinding(&dyno5);
   writer.write(&outBinding, &os);
   string outxml = oss.str();
   
   //cout << "XML=" << endl << xml << endl;
   //cout << "OUT XML=" << endl << outxml << endl;
   assert(strcmp(xml.c_str(), outxml.c_str()) == 0);
   
   tr.pass();
}

void runDynamicObjectMapBindingTest(TestRunner& tr)
{
   tr.test("DynamicObjectMapBinding");
   
   // create xml writer
   XmlWriter writer;
   writer.setIndentation(0, 1);
   
   // dynamic object to read from
   DynamicObject dyno;
   dyno["astring"] = "This is test content.";
   dyno["aboolean"] = true;
   dyno["aninteger"] = 1234;
   dyno["afloat"] = 123.456789;
   
   DynamicObject dyno2;
   dyno2["astring"] = "Another string.";
   dyno2["aboolean"] = false;
   dyno2["aninteger"] = 4321;
   dyno2["afloat"] = 987.654321;
   
   dyno["anobject"] = dyno2;
   
   DynamicObjectMapBinding db(&dyno);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   writer.write(&db, &os);
   string xml = oss.str();
   oss.str("");
   
   // now try to read dynamic object back in
   DynamicObject dyno5;
   DynamicObjectMapBinding inBinding(&dyno5);
   XmlReader reader;
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&inBinding);
   reader.read(&bais);
   reader.finish();
   
   // now send dynamic object back out
   DynamicObjectMapBinding outBinding(&dyno5);
   writer.write(&outBinding, &os);
   string outxml = oss.str();
   
   //cout << "XML=" << endl << xml << endl;
   //cout << "OUT XML=" << endl << outxml << endl;
   assert(strcmp(xml.c_str(), outxml.c_str()) == 0);
   
   tr.pass();
}

void runDynamicObjectBindingTest(TestRunner& tr)
{
   tr.test("DynamicObjectBinding");
   
   // create xml writer
   XmlWriter writer;
   writer.setIndentation(0, 1);
   
   // dynamic object to read from
   DynamicObject dyno;
   dyno["astring"] = "This is test content.";
   dyno["aboolean"] = true;
   dyno["aninteger"] = 1234;
   dyno["afloat"] = 123.456789;
   
   DynamicObject dyno2;
   dyno2["astring"] = "Another string.";
   dyno2["aboolean"] = false;
   dyno2["aninteger"] = 4321;
   dyno2["afloat"] = 987.654321;
   
   dyno["anobject"] = dyno2;
   
   DynamicObject dyno3;
   dyno3[0] = "This is test content.";
   dyno3[1] = true;
   dyno3[2] = 1234;
   dyno3[3] = 123.456789;
   
   DynamicObject dyno4;
   dyno4[0] = "Another string.";
   dyno4[1] = false;
   dyno4[2] = 4321;
   dyno4[3] = 987.654321;
   
   dyno3[4] = dyno4;
   
   dyno["dyno3"] = dyno3;
   
   DynamicObjectBinding db(&dyno);
   
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   writer.write(&db, &os);
   string xml = oss.str();
   oss.str("");
   
   // now try to read dynamic object back in
   DynamicObject dyno5;
   DynamicObjectBinding inBinding(&dyno5);
   XmlReader reader;
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&inBinding);
   reader.read(&bais);
   reader.finish();
   
   // now send dynamic object back out
   DynamicObjectBinding outBinding(&dyno5);
   writer.write(&outBinding, &os);
   string outxml = oss.str();
   
   //cout << "XML=" << endl << xml << endl;
   //cout << "OUT XML=" << endl << outxml << endl;
   assert(strcmp(xml.c_str(), outxml.c_str()) == 0);
   
   tr.pass();
}

void runBigIntegerTest()
{
   cout << "Starting BigInteger test." << endl << endl;
   
   BigInteger number1 = 2;
   BigInteger number2 = 123456789;
   
   cout << "number1=" << number1 << endl;
   cout << "number2=" << number2 << endl;
   cout << "number1 + number2=" << (number1 + number2) << endl;
   cout << "number1 - number2=" << (number1 - number2) << endl;
   cout << "number1 * number2=" << (number1 * number2) << endl;
   cout << "number2 / number1=" << (number2 / number1) << endl;
   cout << "number2 % number1=" << (number2 % number1) << endl;
   cout << "number2 ^ number1=" << (number2.pow(number1)) << endl;
   
   cout << endl << "BigInteger test complete." << endl;
}

void runBigDecimalTest()
{
   cout << "Starting BigDecimal test." << endl << endl;
   
   BigDecimal number1 = 3.0;
   //BigDecimal number2 = 123456789.5;
   BigDecimal number2 = "123456789.53";
   //BigDecimal number2 = 1.234;
   //BigDecimal number2 = "1.23e-04";
   //BigDecimal number2 = "1234";
      
   cout << "number1=" << number1 << endl;
   cout << "number2=" << number2 << endl;
   cout << "number1 + number2=" << (number1 + number2) << endl;
   cout << "number1 - number2=" << (number1 - number2) << endl;
   cout << "number1 * number2=" << (number1 * number2) << endl;
   cout << "number2 / number1=" << (number2 / number1) << endl;
   cout << "number2 % number1=" << (number2 % number1) << endl;
   
   BigDecimal number3 = "129.54678";
   cout << endl << "number3=" << number3 << endl;
   
   BigDecimal bd;
   
   cout << endl;
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, Up);
      bd.round();
      cout << "round " << i << " places, up=" << bd << endl;
   }
   
   cout << endl;
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, HalfUp);
      bd.round();
      cout << "round " << i << " places, half up=" << bd << endl;
   }
   
   cout << endl;
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, Down);
      bd.round();
      cout << "round " << i << " places, down=" << bd << endl;
   }
   
   cout << endl << "BigDecimal test complete." << endl;
}

void runSqlite3ConnectionTest(TestRunner &tr)
{
   tr.test("Sqlite3 Connection");
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   assertNoException();
   
   tr.pass();
}

void runSqlite3StatementTest(TestRunner &tr)
{
   tr.group("Sqlite3 Statement");
   
   // clear any exceptions
   Exception::clearLast();
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   
   db::sql::Statement* s;
   
   tr.test("drop table");
   s = c.prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   tr.passIfNoException();
   
   tr.test("create table");
   s = c.prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   delete s;
   tr.passIfNoException();
   
   tr.test("insert test 1");
   s = c.prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   assert(s->getLastInsertRowId() == 1);
   delete s;
   tr.passIfNoException();
   
   tr.test("insert test 2");
   s = c.prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   assert(s->getLastInsertRowId() == 2);
   delete s;
   tr.passIfNoException();
   
   tr.test("insert positional parameters");
   s = c.prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setInt32(2, 2222);
   s->execute();
   assert(s->getLastInsertRowId() == 3);
   delete s;
   tr.passIfNoException();
   
   // insert named parameters test
   tr.test("insert named parameters");
   s = c.prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   assert(s->getLastInsertRowId() == 4);
   delete s;
   tr.passIfNoException();
   
   // select test
   s = c.prepare("SELECT * FROM test");
   s->execute();
   
   // fetch rows
   tr.test("fetch rows");
   db::sql::Row* row;
   string t;
   int i;

   row = s->fetch();
   assert(row != NULL);
   row->getText("t", t);
   assertNoException();
   row->getInt32("i", i);
   assertNoException();
   assert(t == "test!");
   assert(i == 1234);

   row = s->fetch();
   assert(row != NULL);
   row->getText("t", t);
   assertNoException();
   row->getInt32("i", i);
   assertNoException();
   assert(t == "!tset");
   assert(i == 4321);

   row = s->fetch();
   assert(row != NULL);
   row->getText("t", t);
   assertNoException();
   row->getInt32("i", i);
   assertNoException();
   assert(t == "boundpositional");
   assert(i == 2222);

   row = s->fetch();
   assert(row != NULL);
   row->getText("t", t);
   assertNoException();
   row->getInt32("i", i);
   assertNoException();
   assert(t == "boundnamed");
   assert(i == 2223);

   // done so next should be NULL
   row = s->fetch();
   assert(row == NULL);
   
   delete s;

   tr.pass();
   
   tr.test("connection close");
   c.close();
   tr.passIfNoException();
   
   tr.ungroup();
}

class TestRowObject
{
protected:
   char* mText;
   bool mBoolean;
   int mInt32;
   unsigned int mUInt32;
   
public:
   TestRowObject()
   {
      mText = strdup("");
      mBoolean = false;
      mInt32 = 1;
      mUInt32 = 2;
   }
   
   virtual ~TestRowObject()
   {
      delete [] mText;
   }
   
   virtual void setText(const char* t)
   {
      delete [] mText;
      mText = strdup(t);
   }
   
   virtual const char* getText()
   {
      return mText;
   }
   
   virtual void setBoolean(bool b)
   {
      mBoolean = b;
   }
   
   virtual bool getBoolean()
   {
      return mBoolean;
   }
   
   virtual void setInt32(int i)
   {
      mInt32 = i;
   }
   
   virtual int getInt32()
   {
      return mInt32;
   }
   
   virtual void setUInt32(unsigned int i)
   {
      mUInt32 = i;
   }
   
   virtual unsigned int getUInt32() const
   {
      return mUInt32;
   }
};

class TestRowObjectBinding : public DataBinding
{
protected:
   DataMappingFunctor<TestRowObject> mTextMapping;
   DataMappingFunctor<TestRowObject> mBooleanMapping;
   DataMappingFunctor<TestRowObject> mInt32Mapping;
   DataMappingFunctor<TestRowObject> mUInt32Mapping;
   
public:
   TestRowObjectBinding(TestRowObject* ro) :
      mTextMapping(&TestRowObject::setText, &TestRowObject::getText),
      mBooleanMapping(&TestRowObject::setBoolean, &TestRowObject::getBoolean),
      mInt32Mapping(&TestRowObject::setInt32, &TestRowObject::getInt32),
      mUInt32Mapping(&TestRowObject::setUInt32, &TestRowObject::getUInt32)
   {
      setObject(ro);
      
      addDataMapping(NULL, "t", true, true, &mTextMapping);
      addDataMapping(NULL, "b", true, true, &mBooleanMapping);
      addDataMapping(NULL, "i32", true, true, &mInt32Mapping);
      addDataMapping(NULL, "ui32", true, true, &mUInt32Mapping);
   }
   
   virtual ~TestRowObjectBinding()
   {
   }
};

void runSqlite3RowObjectTest(TestRunner& tr)
{
   tr.group("Sqlite3 RowObject");
   
   // clear any exceptions
   Exception::clearLast();
   
   Sqlite3Connection c;
   c.connect("sqlite3::memory:");
   
   db::sql::Statement* s;
   
   // drop existing table
   tr.test("drop table");
   s = c.prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   tr.passIfNoException();
   
   // create table
   tr.test("create table");
   s = c.prepare(
      "CREATE TABLE IF NOT EXISTS test "
      "(t TEXT, b INT, i32 INT, ui32 INT)");
   s->execute();
   delete s;
   tr.passIfNoException();
   
   // insert a row object
   TestRowObject tro1;
   tro1.setText("This is some text.");
   tro1.setBoolean(false);
   tro1.setInt32(5);
   tro1.setUInt32(14);
   TestRowObjectBinding binding1(&tro1);
   db::sql::RowObject ro1(&binding1);
   
   tr.test("insert row object 1");
   ro1.insert(&c, "test");
   tr.passIfNoException();
   
   // insert another row object
   TestRowObject tro2;
   tro2.setText("The second row object.");
   tro2.setBoolean(false);
   tro2.setInt32(-1);
   tro2.setUInt32(17);
   TestRowObjectBinding binding2(&tro2);
   db::sql::RowObject ro2(&binding2);
   
   tr.test("insert row object 2");
   ro2.insert(&c, "test");
   tr.passIfNoException();
   
   // update row object 1
   TestRowObject tro3;
   tro3.setText("The first row object.");
   tro3.setBoolean(true);
   tro3.setInt32(5);
   tro3.setUInt32(14);
   TestRowObjectBinding binding3(&tro3);
   db::sql::RowObject ro3(&binding3);
   
   tr.test("update row object 1");
   ro3.update(&c, "test", "i32");
   tr.passIfNoException();
   
   // select row object 1 using binding 2
   db::sql::RowObject ro4(&binding2);
   tr.test("select row object 1");
   tro2.setBoolean(true);
   ro4.fetch(&c, "test", "b");
   
   // assert contents
   assert(strcmp(tro2.getText(), "The first row object.") == 0);
   assert(tro2.getBoolean());
   assert(tro2.getInt32() == 5);
   assert(tro2.getUInt32() == 14);
   tr.passIfNoException();
   
   // select row object 2 using binding 3
   db::sql::RowObject ro5(&binding3);
   tro3.setBoolean(false);
   tr.test("select row object 2");
   ro5.fetch(&c, "test", "b");
   
   // assert contents
   assert(strcmp(tro3.getText(), "The second row object.") == 0);
   assert(!tro3.getBoolean());
   assert(tro3.getInt32() == -1);
   assert(tro3.getUInt32() == 17);
   tr.passIfNoException();
   
   tr.test("connection close");
   c.close();
   tr.passIfNoException();
   
   tr.ungroup();
}

void runMySqlConnectionTest()
{
   cout << "Starting MySqlConnection test." << endl << endl;
   
   MySqlConnection c;
   c.connect("mysql://dbreadclient:k288m2s8f6gk39a@mojo/test");
   assertNoException();
   
   // clean up mysql
   mysql_library_end();
   
   cout << endl << "MySqlConnection test complete." << endl;
}

void runMySqlStatementTest()
{
   cout << "Starting MySql test." << endl << endl;
   
   // clear any exceptions
   Exception::clearLast();
   
   MySqlConnection c;
   c.connect("mysql://dbwriteclient:k288m2s8f6gk39a@mojo/test");
   
   db::sql::Statement* s;
   
   // drop table test
   s = c.prepare("DROP TABLE IF EXISTS dbmysqltest");
   assert(s != NULL);
   s->execute();
   delete s;
   assertNoException();
   cout << "drop table test passed!" << endl;
   
   // create table test
   s = c.prepare(
      "CREATE TABLE IF NOT EXISTS dbmysqltest "
      "(id BIGINT AUTO_INCREMENT, t TEXT, i BIGINT, "
      "PRIMARY KEY (id))");
   s->execute();
   delete s;
   assertNoException();
   cout << "create table test passed!" << endl;
   
   // insert test 1
   s = c.prepare("INSERT INTO dbmysqltest (t, i) VALUES ('test!', 1234)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "insert test 1 passed!" << endl;
   
   // insert test 2
   s = c.prepare("INSERT INTO dbmysqltest (t, i) VALUES ('!tset', 4321)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c.prepare("INSERT INTO dbmysqltest (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setInt32(2, 2222);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "insert positional parameters test passed!" << endl;
   
//   // insert named parameters test
//   s = c.prepare("INSERT INTO dbmysqltest (t, i) VALUES (:first, :second)");
//   s->setText(":first", "boundnamed");
//   s->setInt32(":second", 2223);
//   s->execute();
//   cout << "Row #: " << s->getLastInsertRowId() << endl;
//   delete s;
//   assertNoException();
//   cout << "insert named parameters test passed!" << endl;
   
   // select test
   s = c.prepare("SELECT t, i FROM dbmysqltest");
   s->execute();
   
   // fetch rows
   db::sql::Row* row;
   string t;
   int i;
   while((row = s->fetch()) != NULL)
   {
      cout << endl << "Row result:" << endl;
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      cout << "t=" << t << endl;
      cout << "i=" << i << endl;
   }
   
   cout << endl << "Result Rows complete." << endl;
   delete s;
   cout << "select test passed!" << endl;
   
   c.close();
   assertNoException();
   
   // clean up mysql
   mysql_library_end();
   
   cout << endl << "MySql test complete." << endl;
}

void runMysqlRowObjectTest(TestRunner& tr)
{
   tr.group("MySql RowObject");
   
   // clear any exceptions
   Exception::clearLast();
   
   MySqlConnection c;
   c.connect("mysql://dbwriteclient:k288m2s8f6gk39a@mojo/test");
   
   db::sql::Statement* s;
   
   // drop existing table
   tr.test("drop table");
   s = c.prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   tr.passIfNoException();
   
   // create table
   tr.test("create table");
   s = c.prepare(
      "CREATE TABLE IF NOT EXISTS test "
      "(t TEXT, b INT, i32 INT, ui32 INT)");
   s->execute();
   delete s;
   tr.passIfNoException();
   
   // insert a row object
   TestRowObject tro1;
   tro1.setText("This is some text.");
   tro1.setBoolean(false);
   tro1.setInt32(5);
   tro1.setUInt32(14);
   TestRowObjectBinding binding1(&tro1);
   db::sql::RowObject ro1(&binding1);
   
   tr.test("insert row object 1");
   ro1.insert(&c, "test");
   tr.passIfNoException();
   
   // insert another row object
   TestRowObject tro2;
   tro2.setText("The second row object.");
   tro2.setBoolean(false);
   tro2.setInt32(-1);
   tro2.setUInt32(17);
   TestRowObjectBinding binding2(&tro2);
   db::sql::RowObject ro2(&binding2);
   
   tr.test("insert row object 2");
   ro2.insert(&c, "test");
   tr.passIfNoException();
   
   // update row object 1
   TestRowObject tro3;
   tro3.setText("The first row object.");
   tro3.setBoolean(true);
   tro3.setInt32(5);
   tro3.setUInt32(14);
   TestRowObjectBinding binding3(&tro3);
   db::sql::RowObject ro3(&binding3);
   
   tr.test("update row object 1");
   ro3.update(&c, "test", "i32");
   tr.passIfNoException();
   
   // select row object 1 using binding 2
   db::sql::RowObject ro4(&binding2);
   tr.test("select row object 1");
   tro2.setBoolean(true);
   ro4.fetch(&c, "test", "b");
   
   // assert contents
   assert(strcmp(tro2.getText(), "The first row object.") == 0);
   assert(tro2.getBoolean());
   assert(tro2.getInt32() == 5);
   assert(tro2.getUInt32() == 14);
   tr.passIfNoException();
   
   // select row object 2 using binding 3
   db::sql::RowObject ro5(&binding3);
   tro3.setBoolean(false);
   tr.test("select row object 2");
   ro5.fetch(&c, "test", "b");
   
   // assert contents
   assert(strcmp(tro3.getText(), "The second row object.") == 0);
   assert(!tro3.getBoolean());
   assert(tro3.getInt32() == -1);
   assert(tro3.getUInt32() == 17);
   tr.passIfNoException();
   
   tr.test("connection close");
   c.close();
   tr.passIfNoException();
   
   // clean up mysql
   mysql_library_end();
   
   tr.ungroup();
}

void executeStatements(db::sql::Connection* c)
{
   db::sql::Statement* s;
   
   // drop table test
   s = c->prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   assertNoException();
   //cout << "drop table test passed!" << endl;
   
   // create table test
   s = c->prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   delete s;
   assertNoException();
   //cout << "create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   //cout << "insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   //cout << "insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setInt32(2, 2222);
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   //cout << "insert positional parameters test passed!" << endl;
   
   // insert named parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   //cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   //cout << "insert named parameters test passed!" << endl;
   
   // select test
   s = c->prepare("SELECT * FROM test");
   s->execute();
   
   // fetch rows
   db::sql::Row* row;
   string t;
   int i;
   while((row = s->fetch()) != NULL)
   {
      //cout << endl << "Row result:" << endl;
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      //cout << "t=" << t << endl;
      //cout << "i=" << i << endl;
   }
   
   //cout << endl << "Result Rows complete." << endl;
   delete s;
   //cout << "select test passed!" << endl;
   
   Thread::sleep(100);
   
   c->close();
   
   //cout << "Statements finished executing." << endl;
}

class SqlConnectionTest : public Runnable
{
public:
   Sqlite3ConnectionPool* pool;
   
   virtual void run()
   {
      db::sql::Connection* c = pool->getConnection();
      executeStatements(c);
   }
};

void runConnectionPoolTest()
{
   cout << "Starting ConnectionPool test." << endl << endl;
   
   int size = 300;
   
   // create sqlite3 connection pool
   Sqlite3ConnectionPool cp("sqlite3::memory:", 100);
   assertNoException();
   
   // create connection test threads
   SqlConnectionTest tests[size];
   Thread* threads[size];
   
   // create threads, set pool for tests
   for(int i = 0; i < size; i++)
   {
      tests[i].pool = &cp;
      threads[i] = new Thread(&tests[i]);
   }
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   // run connection threads
   int count = 1;
   for(int i = 0; i < size; i++, count++)
   {
      //cout << "RUNNING CONNECTION #" << count << endl;
      while(!threads[i]->start())
      {
         threads[i - 1]->join();
      }
   }
   
   // join threads
   for(int i = 0; i < size; i++)
   {
      threads[i]->join();
   }
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   // clean up threads
   for(int i = 0; i < size; i++)
   {
      delete threads[i];
   }
   
   cout << endl;
   cout << "Number of independent connection uses: " << size << endl;
   cout << "Number of pooled connections created: " << cp.getConnectionCount()
      << endl;
   
   cout << "Total time: " << (end - start) << "ms" << endl;
   
   cout << endl << "ConnectionPool test complete." << endl;
}

void runDatabaseClientTest()
{
   cout << "Starting DatabaseClient test." << endl << endl;
   
   // clear any exceptions
   Exception::clearLast();
   
   // get a sqlite3 database client
   DatabaseClient* dc = DatabaseClient::create("sqlite3::memory:");
   assertNoException();
   
   // get a connection
   db::sql::Connection* c = dc->getConnection();
   assertNoException();
   
   // drop table test
   db::sql::Statement* s = c->prepare("DROP TABLE IF EXISTS test");
   assert(s != NULL);
   s->execute();
   delete s;
   assertNoException();
   cout << "sqlite3 drop table test passed!" << endl;
   
   // create table test
   s = c->prepare("CREATE TABLE IF NOT EXISTS test (t TEXT, i INT)");
   s->execute();
   delete s;
   assertNoException();
   cout << "sqlite3 create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO test (t, i) VALUES ('test!', 1234)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "sqlite3 insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO test (t, i) VALUES ('!tset', 4321)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "sqlite3 insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setUInt32(2, 2222);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "sqlite3 insert positional parameters test passed!" << endl;
   
   // insert named parameters test
   s = c->prepare("INSERT INTO test (t, i) VALUES (:first, :second)");
   s->setText(":first", "boundnamed");
   s->setInt32(":second", 2223);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "sqlite3 insert named parameters test passed!" << endl;
   
   // select test
   s = c->prepare("SELECT * FROM test");
   s->execute();
   
   // fetch rows
   db::sql::Row* row;
   string t;
   int i;
   while((row = s->fetch()) != NULL)
   {
      cout << endl << "Row result:" << endl;
      row->getText((unsigned int)0, t);
      assertNoException();
      row->getInt32(1, i);
      assertNoException();
      
      cout << "t=" << t << endl;
      cout << "i=" << i << endl;
   }
   
   cout << endl << "Result Rows complete." << endl;
   delete s;
   cout << "sqlite3 select test passed!" << endl;
   
   c->close();
   assertNoException();
   
   // clean up database client
   delete dc;
   
   // get a mysql database client
   dc = DatabaseClient::create(
      "mysql://dbwriteclient:k288m2s8f6gk39a@mojo/test");
   assertNoException();
   
   // get a connection
   c = dc->getConnection();
   assertNoException();
   
   // drop table test
   s = c->prepare("DROP TABLE IF EXISTS dbmysqltest");
   assert(s != NULL);
   s->execute();
   delete s;
   assertNoException();
   cout << "mysql drop table test passed!" << endl;
   
   // create table test
   string sql;
   sql.append("CREATE TABLE IF NOT EXISTS dbmysqltest ");
   sql.append("(id BIGINT AUTO_INCREMENT, t TEXT, i BIGINT, ");
   sql.append("PRIMARY KEY (id))");
   s = c->prepare(sql.c_str());
   s->execute();
   delete s;
   assertNoException();
   cout << "mysql create table test passed!" << endl;
   
   // insert test 1
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES ('test!', 1234)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "mysql insert test 1 passed!" << endl;
   
   // insert test 2
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES ('!tset', 4321)");
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "mysql insert test 2 passed!" << endl;
   
   // insert positional parameters test
   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES (?, ?)");
   s->setText(1, "boundpositional");
   s->setUInt32(2, 2222);
   s->execute();
   cout << "Row #: " << s->getLastInsertRowId() << endl;
   delete s;
   assertNoException();
   cout << "mysql insert positional parameters test passed!" << endl;
   
//   // insert named parameters test
//   s = c->prepare("INSERT INTO dbmysqltest (t, i) VALUES (:first, :second)");
//   s->setText(":first", "boundnamed");
//   s->setInt32(":second", 2223);
//   s->execute();
//   cout << "Row #: " << s->getLastInsertRowId() << endl;
//   delete s;
//   assertNoException();
//   cout << "mysql insert named parameters test passed!" << endl;
   
   // select test
   s = c->prepare("SELECT * FROM dbmysqltest");
   s->execute();
   
   // fetch rows
   while((row = s->fetch()) != NULL)
   {
      cout << endl << "Row result:" << endl;
      row->getText("t", t);
      assertNoException();
      row->getInt32("i", i);
      assertNoException();
      
      cout << "t=" << t << endl;
      cout << "i=" << i << endl;
   }
   
   cout << endl << "Result Rows complete." << endl;
   delete s;
   cout << "mysql select test passed!" << endl;
   
   c->close();
   assertNoException();
   
   // clean up database client
   delete dc;
   
   // clean up mysql
   mysql_library_end();
   
   cout << endl << "DatabaseClient test complete." << endl;
}

class TestObserver : public Observer
{
public:
   int events;
   int event1;
   int event2;
   int event3;
   int event4;
   
   ObserverDelegate<TestObserver> delegate1;
   ObserverDelegate<TestObserver> delegate2;
   ObserverDelegate<TestObserver> delegate3;
   ObserverDelegate<TestObserver> delegate4;
   
   TestObserver() :
      delegate1(this, &TestObserver::handleEvent1),
      delegate2(this, &TestObserver::handleEvent2),
      delegate3(this, &TestObserver::handleEvent3),
      delegate4(this, &TestObserver::handleEvent4)
   {
      events = 0;
      event1 = 0;
      event2 = 0;
      event3 = 0;
      event4 = 0;
   }
   
   virtual ~TestObserver()
   {
   }
   
   virtual void eventOccurred(Event e)
   {
      events++;
   }
   
   virtual void handleEvent1(Event e)
   {
      event1++;
   }
   
   virtual void handleEvent2(Event e)
   {
      event2++;
   }
   
   virtual void handleEvent3(Event e)
   {
      event3++;
   }
   
   virtual void handleEvent4(Event e)
   {
      if(e["id"]->getUInt64() == 3)
      {
         event3++;
      }
      else if(e["id"]->getUInt64() == 4)
      {
         event4++;
      }
   }
};

void runEventTest(TestRunner& tr)
{
   tr.test("Event");
   
   // create kernel and start engine
   Kernel k;
   k.getEngine()->start();
   
   // create observable and observer
   Observable observable;
   TestObserver observer;
   
   // register observer and start observable
   observable.registerObserver(&observer, 1);
   observable.start(&k);
   
   // create and schedule events
   Event e1;
   Event e2;
   Event e3;
   e1["name"] = "Event1";
   e2["name"] = "Event2";
   e3["name"] = "Event3";
   observable.schedule(e1, 1);
   observable.schedule(e2, 1);
   observable.schedule(e3, 1);
   
   // wait for a second
   Thread::sleep(1000);
   
   assert(observer.events == 3);
   
   // stop observable
   observable.stop();
   
   // stop kernel engine
   k.getEngine()->stop();
   
   tr.pass();
}

void runObserverDelegateTest(TestRunner& tr)
{
   tr.test("ObserverDelegate");
   
   // create kernel and start engine
   Kernel k;
   k.getEngine()->start();
   
   // create observable and observers
   Observable observable;
   TestObserver observer;
   
   // register observers and start observable
   observable.registerObserver(&observer.delegate1, 1);
   observable.registerObserver(&observer.delegate2, 2);
   observable.registerObserver(&observer.delegate3, 3);
   observable.registerObserver(&observer.delegate4, 4);
   observable.addTap(3, 4);
   observable.start(&k);
   
   // create and schedule events
   Event e1;
   Event e2;
   Event e3;
   Event e4;
   e1["name"] = "Event1";
   e2["name"] = "Event2";
   e3["name"] = "Event3";
   e4["name"] = "Event4";
   observable.schedule(e1, 1);
   observable.schedule(e2, 2);
   observable.schedule(e3, 3);
   observable.schedule(e4, 4);
   
   // wait for a second
   Thread::sleep(1000);
   
   assert(observer.event1 == 1);
   assert(observer.event2 == 1);
   assert(observer.event3 == 2);
   assert(observer.event4 == 1);
   
   // stop observable
   observable.stop();
   
   // stop kernel engine
   k.getEngine()->stop();
   
   tr.pass();
}

void runEventControllerTest(TestRunner& tr)
{
   tr.test("EventController");
   
   // create kernel and start engine
   Kernel k;
   k.getEngine()->start();
   
   // create event controller
   EventController ec;
   
   // create observers
   TestObserver observer;
   
   DynamicObject types;
   types[0] = "event1";
   ec.registerObserver(&observer.delegate1, types);
   types[0] = "event2";
   ec.registerObserver(&observer.delegate2, types);
   types[0] = "event3";
   ec.registerObserver(&observer.delegate3, types);
   types[0] = "event4";
   ec.registerObserver(&observer.delegate4, types);
   
   types[0] = "event1";
   ec.registerObserver(&observer, types);
   
   // add parent events
   ec.addParent("event2", "event1");
   ec.addParent("event3", "event1");
   ec.addParent("event4", "event3");
   
   // start event controller
   ec.start(&k);
   
   // create and schedule events
   Event e1;
   Event e2;
   Event e3;
   Event e4;
   e1["type"] = "event1";
   e2["type"] = "event2";
   e3["type"] = "event3";
   e4["type"] = "event4";
   ec.schedule(e1);
   ec.schedule(e2);
   ec.schedule(e3);
   ec.schedule(e4);
   
   // wait for a second
   Thread::sleep(1000);
   
   // check messages
   assert(observer.events == 4);
   assert(observer.event1 == 4);
   assert(observer.event2 == 1);
   assert(observer.event3 == 2);
   assert(observer.event4 == 1);
   
   // stop event controller
   ec.stop();
   
   // stop kernel engine
   k.getEngine()->stop();
   
   tr.pass();
}

void runLoggerTest()
{
   cout << "Starting Logger test." << endl << endl;
   
//   db::logging::OutputStreamLogger clog(
//      "stdout", Logger::Max, OStreamOutputStream::getStdoutStream());
//   Logger::addLogger(&clog);
//   Logger::addLogger(&clog, "[C1]");
//   
//   db::logging::FileLogger flog(
//      "flog", Logger::Max, new File("test.log"), true);
//   Logger::addLogger(&flog);
//
//   DB_ERROR("[M1] error test");
//   DB_WARNING("[M1] warning test");
//   DB_INFO("[M1] info test");
//   DB_DEBUG("[M1] debug test");
//   DB_CAT_ERROR("[C1]", "[M2] cat 1 error test");
//   DB_CAT_OBJECT_ERROR("[C1]", &clog, "[M3] cat 1 obj error test");
   
   cout << endl << "Logger test complete." << endl;
}

void runUniqueListTest()
{
   cout << "Starting UniqueList test." << endl << endl;
   
   UniqueList<int> list;
   
   list.add(5);
   list.add(6);
   list.add(7);
   list.add(5);
   
   Iterator<int>* i = list.getIterator();
   while(i->hasNext())
   {
      cout << "element=" << i->next() << endl;
   }
   delete i;
   
   cout << "Removing '5'..." << endl;   
   list.remove(5);
   
   i = list.getIterator();
   while(i->hasNext())
   {
      cout << "element=" << i->next() << endl;
   }
   delete i;
   
   list.clear();
   
   cout << endl << "UniqueList test complete." << endl;
}

void runFileTest()
{
   cout << "Starting File test." << endl << endl;
   
   const char* name = "/work";
   
   File dir(name);
   FileList files(true);
   dir.listFiles(&files);
   
   cout << "Files in " << dir.getName() << ":" << endl;
   
   Iterator<File*>* i = files.getIterator();
   while(i->hasNext())
   {
      File* file = i->next();
      const char* type;
      switch(file->getType())
      {
         case File::RegularFile:
            type = "Regular File";
            break;
         case File::Directory:
            type = "Directory";
            break;
         case File::SymbolicLink:
            type = "Symbolic Link";
            break;
         default:
            type = "Unknown";
            break;
      }
      
      cout << "Name: '" << file->getName() << "', Type: " << type << endl;
   }
   delete i;
   
   cout << endl << "File test complete." << endl;
}

void runSmtpClientTest(TestRunner& tr)
{
   tr.test("SmtpClient");
   
   // set url of mail server
   Url url("smtp://localhost:25");
   
   // set mail
   db::mail::Mail mail;
   mail.setSender("testuser@bitmunk.com");
   mail.addTo("support@bitmunk.com");
   mail.addCc("support@bitmunk.com");
   mail.setSubject("This is an autogenerated unit test email");
   mail.setBody("This is the test body");
   
   // send mail
   db::mail::SmtpClient c;
   c.sendMail(&url, &mail);
   
   tr.passIfNoException();
}

void runMailTemplateParser(TestRunner& tr)
{
   tr.test("MailTemplateParser");
   
   // create mail template
   const char* tpl =
      "From: testuser@bitmunk.com\r\n"
      "To: support@bitmunk.com\r\n"
      "Cc: support@bitmunk.com\r\n"
      "Bcc: $bccAddress1\r\n"
      "Subject: This is an autogenerated unit test email\r\n"
      "This is the test body. I want \\$10.00.\n"
      "I used a variable: \\$bccAddress1 with the value of "
      "'$bccAddress1'.\n"
      "Slash before variable \\\\$bccAddress1.\n"
      "2 slashes before variable \\\\\\\\$bccAddress1.\n"
      "Slash before escaped variable \\\\\\$bccAddress1.\n"
      "2 slashes before escaped variable \\\\\\\\\\$bccAddress1.\n"
      "$eggs$bacon$ham$sausage.";
   
   // create template parser
   db::mail::MailTemplateParser parser;
   
   // create input stream
   ByteArrayInputStream bais(tpl, strlen(tpl));
   
   // create variables
   DynamicObject vars;
   vars["bccAddress1"] = "support@bitmunk.com";
   vars["eggs"] = "This is a ";
   //vars["bacon"] -- no bacon
   vars["ham"] = "number ";
   vars["sausage"] = 5;
   
   // parse mail
   db::mail::Mail mail;
   parser.parse(&mail, vars, &bais);
   
   const char* expect =
      "This is the test body. I want $10.00.\r\n"
      "I used a variable: $bccAddress1 with the value of "
      "'support@bitmunk.com'.\r\n"
      "Slash before variable \\support@bitmunk.com.\r\n"
      "2 slashes before variable \\\\support@bitmunk.com.\r\n"
      "Slash before escaped variable \\$bccAddress1.\r\n"
      "2 slashes before escaped variable \\\\$bccAddress1.\r\n"
      "This is a number 5.\r\n";
   
   // get mail message
   db::mail::Message msg = mail.getMessage();
   
   // assert body parsed properly
   const char* body = msg["body"]->getString();
   assert(strcmp(body, expect) == 0);
   
//   // print out mail message
//   cout << "\nHeaders=\n";
//   DynamicObjectIterator i = msg["headers"].getIterator();
//   while(i->hasNext())
//   {
//      DynamicObject header = i->next();
//      DynamicObjectIterator doi = header.getIterator();
//      while(doi->hasNext())
//      {
//         cout << i->getName() << ": " << doi->next()->getString() << endl;
//      }
//   }
//   
//   cout << "Expect=\n" << expect << endl;
//   cout << "Body=\n" << msg["body"]->getString() << endl;
   
//   // set url of mail server
//   Url url("smtp://localhost:25");
//   
//   // send mail
//   db::mail::SmtpClient c;
//   c.sendMail(&url, &mail);
   
   tr.passIfNoException();
}

void runConfigManagerTest(TestRunner& tr)
{
   tr.group("ConfigManager");
   
   tr.test("init");
   {
      DynamicObject expect;
      expect->setType(Map);
      ConfigManager cm;
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();
   
   tr.test("init & clear");
   {
      DynamicObject expect;
      expect->setType(Map);
      ConfigManager cm;
      cm.clear();
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();
   
   tr.test("1 config");
   {
      DynamicObject expect;
      expect->setType(Map);
      expect["a"] = 0;
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      cm.addConfig(a);
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();
   
   tr.test("clear & 1 config");
   {
      DynamicObject expect;
      expect->setType(Map);
      expect["a"] = 0;
      ConfigManager cm;
      cm.clear();
      DynamicObject a;
      a["a"] = 0;
      cm.addConfig(a);
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();
   
   tr.test("config change");
   {
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      cm.addConfig(a);
      assert(cm.getConfig() == a);
      cm.getConfig()["a"] = 1;
      DynamicObject expect;
      expect["a"] = 1;
      assert(cm.getConfig() != a);
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();

   tr.test("add");
   {
      DynamicObject expect;
      expect["a"] = 0;
      expect["b"] = 1;
      expect["c"] = 2;
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      DynamicObject b;
      b["b"] = 1;
      DynamicObject c;
      c["c"] = 2;
      cm.addConfig(a);
      cm.addConfig(b);
      cm.addConfig(c);
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();

   tr.test("bad remove");
   {
      ConfigManager cm;
      assert(!cm.removeConfig(0));
      assertException();
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("remove");
   {
      DynamicObject expect;
      expect["a"] = 0;
      expect["b"] = 1;
      expect["c"] = 2;
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      DynamicObject b;
      b["b"] = 1;
      DynamicObject c;
      c["c"] = 2;
      ConfigManager::ConfigId id;
      cm.addConfig(a);
      cm.addConfig(b, ConfigManager::Default, &id);
      cm.addConfig(c);
      assert(cm.getConfig() == expect);
      DynamicObject expect2;
      expect2["a"] = 0;
      expect2["c"] = 2;
      assert(cm.removeConfig(id));
      assert(cm.getConfig() == expect2);
   }
   tr.passIfNoException();

   tr.test("update");
   {
      ConfigManager cm;
      DynamicObject expect;
      expect["a"] = 0;
      DynamicObject a;
      a["a"] = 0;
      cm.addConfig(a);
      assert(cm.getConfig() == expect);
      DynamicObject expect2;
      expect2["a"] = 1;
      a["a"] = 1;
      assert(cm.getConfig() != expect2);
      cm.update();
      assert(cm.getConfig() == expect2);
   }
   tr.passIfNoException();

   tr.test("set");
   {
      ConfigManager cm;
      DynamicObject expect;
      expect["a"] = 0;
      DynamicObject a;
      a["a"] = 0;
      ConfigManager::ConfigId id;
      cm.addConfig(a, ConfigManager::Default, &id);
      assert(cm.getConfig() == expect);
      DynamicObject expect2;
      expect2["b"] = 0;
      DynamicObject b;
      b["b"] = 0;
      cm.setConfig(id, b);
      assert(cm.getConfig() == expect2);
   }
   tr.passIfNoException();

   tr.test("get");
   {
      ConfigManager cm;
      DynamicObject expect;
      expect["a"] = 0;
      DynamicObject a;
      a["a"] = 0;
      ConfigManager::ConfigId id;
      cm.addConfig(a, ConfigManager::Default, &id);
      assert(cm.getConfig() == expect);
      DynamicObject b;
      assert(cm.getConfig(id, b));
      assert(b == expect);
   }
   tr.passIfNoException();

   tr.test("map changes");
   {
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      a["b"] = 0;
      cm.addConfig(a);
      cm.getConfig()["a"] = 1;
      DynamicObject expect;
      expect["a"] = 1;
      DynamicObject changes;
      cm.getChanges(changes);
      assert(changes == expect);
   }
   tr.passIfNoException();

   tr.test("deep map changes");
   {
      ConfigManager cm;
      DynamicObject a;
      a["a"]["b"] = 0;
      a["a"]["c"] = 0;
      cm.addConfig(a);
      cm.getConfig()["a"]["c"] = 1;
      cm.getConfig()["d"] = 0;
      DynamicObject expect;
      expect["a"]["c"] = 1;
      expect["d"] = 0;
      DynamicObject changes;
      cm.getChanges(changes);
      assert(changes == expect);
   }
   tr.passIfNoException();

   tr.test("array changes");
   {
      ConfigManager cm;
      DynamicObject a;
      a[0] = 10;
      a[1] = 11;
      a[2] = 12;
      cm.addConfig(a);
      cm.getConfig()[1] = 21;
      DynamicObject expect;
      expect[0] = "__default__";
      expect[1] = 21;
      expect[2] = "__default__";
      DynamicObject changes;
      cm.getChanges(changes);
      assert(changes == expect);
   }
   tr.passIfNoException();

   tr.test("bigger array changes");
   {
      ConfigManager cm;
      DynamicObject a;
      a[0] = 10;
      a[1] = 11;
      cm.addConfig(a);
      cm.getConfig()[2] = 22;
      DynamicObject expect;
      expect[0] = "__default__";
      expect[1] = "__default__";
      expect[2] = 22;
      DynamicObject changes;
      cm.getChanges(changes);
      assert(changes == expect);
   }
   tr.passIfNoException();

   tr.test("system vs user changes");
   {
      ConfigManager cm;

      // system
      DynamicObject a;
      a[0] = 10;
      a[1] = 11;
      cm.addConfig(a, ConfigManager::Default);

      // user
      DynamicObject b;
      b[0] = 20;
      b[1] = 21;
      cm.addConfig(b, ConfigManager::User);
      
      // custom
      cm.getConfig()[1] = 31;

      {
         // Changes from system configs
         DynamicObject expect;
         expect[0] = 20;
         expect[1] = 31;
         DynamicObject changes;
         cm.getChanges(changes);
         assert(changes == expect);
      }
      
      {
         // Changes from system+user configs
         DynamicObject expect;
         expect[0] = "__default__";
         expect[1] = 31;
         DynamicObject changes;
         cm.getChanges(changes, ConfigManager::All);
         assert(changes == expect);
      }
   }
   tr.passIfNoException();

   tr.test("default value");
   {
      ConfigManager cm;
      DynamicObject a;
      a = 1;
      cm.addConfig(a);
      DynamicObject b;
      b = "__default__";
      cm.addConfig(b);
      DynamicObject expect;
      expect = 1;
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();

   tr.test("default values");
   {
      ConfigManager cm;
      DynamicObject a;
      a[0] = 10;
      a[1] = 11;
      a[2]["0"] = 120;
      a[2]["1"] = 121;
      cm.addConfig(a);
      DynamicObject b;
      b[0] = "__default__";
      b[1] = 21;
      b[2]["0"] = "__default__";
      b[2]["1"] = 221;
      cm.addConfig(b);
      DynamicObject expect;
      expect[0] = 10;
      expect[1] = 21;
      expect[2]["0"] = 120;
      expect[2]["1"] = 221;
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();

   tr.test("schema check");
   {
      DynamicObject schema;
      DynamicObject config;
      assert(ConfigManager::isValidConfig(config, schema));
      schema->setType(Map);
      config->setType(Map);
      assert(ConfigManager::isValidConfig(config, schema));
      schema["s"] = "";
      schema["i"] = 0;
      config["s"] = "string";
      config["i"] = 1;
      assert(ConfigManager::isValidConfig(config, schema));
      schema["m"]["s"] = "";
      schema["m"]["s2"] = "";
      schema["a"][0] = 0;
      schema["a"][1] = 1;
      config["m"]["s"] = "s";
      config["m"]["s2"] = "s2";
      config["a"][0] = 0;
      config["a"][1] = 1;
   }
   tr.passIfNoException();

   tr.test("schema check bad");
   {
      DynamicObject schema;
      DynamicObject config;
      assert(ConfigManager::isValidConfig(config, schema));
      schema->setType(Map);
      config->setType(Array);
      assert(!ConfigManager::isValidConfig(config, schema));
      config->setType(Map);
      schema["s"] = "";
      schema["i"] = 0;
      config["s"] = 1;
      config["i"] = "string";
      assert(!ConfigManager::isValidConfig(config, schema));
   }
   tr.passIfNoException();

   tr.test("user preferences");
   {
      ConfigManager cm;

      // node
      // built in or loaded defaults
      DynamicObject nodec;
      nodec["node"]["host"] = "localhost";
      nodec["node"]["port"] = 19100;
      nodec["node"]["modulePath"] = "/usr/lib/bitmunk/modules";
      nodec["node"]["userModulePath"] = "~/.bitmunk/modules";
      cm.addConfig(nodec);

      // user
      // loaded defaults
      DynamicObject userc;
      userc["node"]["port"] = 19100;
      userc["node"]["comment"] = "My precious...";
      cm.addConfig(userc, ConfigManager::User);
      
      // user makes changes during runtime
      DynamicObject c = cm.getConfig();
      c["node"]["port"] = 19200;
      c["node"]["userModulePath"] = "~/.bitmunk/modules:~/.bitmunk/modules-dev";

      // get the changes from defaults to current config
      // serialize this to disk as needed
      DynamicObject changes;
      cm.getChanges(changes);

      // check it's correct
      DynamicObject expect;
      expect["node"]["port"] = 19200;
      expect["node"]["comment"] = "My precious...";
      expect["node"]["userModulePath"] = "~/.bitmunk/modules:~/.bitmunk/modules-dev";
      assert(changes == expect);
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

class RunTests : public virtual Object, public Runnable
{
public:
   /**
    * Run automatic unit tests.
    */
   virtual void runAutomaticUnitTests(TestRunner& tr)
   {
      // db::rt tests
      runThreadTest(tr);
      runJobThreadPoolTest(tr);
      runJobDispatcherTest(tr);
      
      // db::config tests
      runConfigManagerTest(tr);
      
      // db::modest tests
      runModestTest(tr);
      
      // db::util tests
      runBase64Test(tr);
      runCrcTest(tr);
      runDynamicObjectTest(tr);
      runDynoClearTest(tr);
      runDynoConversionTest(tr);
      
      // db::data tests
      runJsonValidTest(tr);
      runJsonInvalidTest(tr);
      runJsonDJDTest(tr);
      runJsonVerifyDJDTest(tr);
      runJsonIOStreamTest(tr);
      
      // db::crypto tests
      runMessageDigestTest(tr);
      
      // db::net tests
      runAddressResolveTest(tr);
      runSocketTest(tr);
      runUrlEncodeTest(tr);
      runUrlTest(tr);
      //runInterruptServerSocketTest(tr);
      
      // db::data tests
      runXmlHttpServerTest(tr);
      runDynamicObjectWriterTest(tr);
      runDynamicObjectReaderTest(tr);
      runDynamicObjectBasicBindingTest(tr);
      runDynamicObjectArrayBindingTest(tr);
      runDynamicObjectMapBindingTest(tr);
      runDynamicObjectBindingTest(tr);
      
      // db::sql tests
      runSqlite3ConnectionTest(tr);
      runSqlite3StatementTest(tr);
      runSqlite3RowObjectTest(tr);
      runMysqlRowObjectTest(tr);
      
      // db::event tests
      runEventTest(tr);
      runObserverDelegateTest(tr);
      runEventControllerTest(tr);
      
      // db::mail tests
      //runSmtpClientTest(tr);
      runMailTemplateParser(tr);
      
      assertNoException();
   }

   /**
    * Runs interactive unit tests.
    */
   virtual void runInteractiveUnitTests(TestRunner& tr)
   {
//      runTimeTest();
//      runConvertTest();
//      runRegexTest();
//      runDateTest();
//      runStringTokenizerTest();
//      runStringEqualityTest();
//      runStringAppendCharTest();
//      runStringCompareTest();
//      runDynamicObjectTest(tr);
//      runByteBufferTest();
//      runByteArrayInputStreamTest();
//      runByteArrayOutputStreamTest();
//      runAsymmetricKeyLoadingTest();
//      runDsaAsymmetricKeyCreationTest();
//      runRsaAsymmetricKeyCreationTest();
//      runDigitalSignatureInputStreamTest();
//      runDigitalSignatureOutputStreamTest();
//      runEnvelopeTest("DSA");
//      runEnvelopeTest("RSA");
//      runCipherTest("AES256");
//      runBigIntegerTest();
//      runBigDecimalTest();
//      runSslSocketTest();
//      runServerSocketTest();
//      runSslServerSocketTest();
//      runTcpClientServerTest();
//      runUdpClientServerTest();
//      runDatagramTest();
//      runServerConnectionTest();
//      runServerSslConnectionTest();
//      runServerDatagramTest();
//      runHttpHeaderTest();
//      runHttpServerTest();
//      runHttpClientGetTest();
//      runHttpClientPostTest();
//      runPingTest();
//      runXmlReaderTest();
//      runXmlWriterTest();
//      runXmlReadWriteTest();
//      runXmlBindingInputStreamTest();
//      runXmlBindingOutputStreamTest();
//      runXmlHttpServerTest(tr);
//      runDynamicObjectTest(tr);
//      runDynamicObjectWriterTest(tr);
//      runDynamicObjectReaderTest(tr);
//      runDynamicObjectBasicBindingTest(tr);
//      runDynamicObjectArrayBindingTest(tr);
//      runDynamicObjectMapBindingTest(tr);
//      runDynamicObjectBindingTest(tr);
//      runMySqlConnectionTest();
//      runMySqlStatementTest();
//      runConnectionPoolTest();
//      runDatabaseClientTest();
//      runEventTest(tr);
//      runObserverDelegateTest(tr);
//      runEventControllerTest(tr);
//      runLoggerTest();
//      runUniqueListTest();
//      runFileTest();
//      runSmtpClientTest(tr);
//      runMailTemplateParser(tr);
      
      assertNoException();
   }

   /**
    * Runs the unit tests.
    */
   virtual void run()
   {
      TestRunner tr(true, TestRunner::Names);
      
      tr.group(""); // root group
      runInteractiveUnitTests(tr);
      runAutomaticUnitTests(tr);
      tr.ungroup();
      
      assertNoException();
      tr.done();
   }
};

int main()
{
   // initialize winsock
   #ifdef WIN32
      WSADATA wsaData;
      if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
      {
         cout << "ERROR! Could not initialize winsock!" << endl;
      }
   #endif
   
   RunTests runnable;
   Thread t(&runnable);
   t.start();
   t.join();
   
   // cleanup winsock
   #ifdef WIN32
      WSACleanup();
   #endif
   
   Exception::setLast(new Exception("Main thread exception leak test"));

   #ifndef WIN32
   // FIXME: calling Thread::exit() on windows causes a busy loop of
   // some sort (perhaps a deadlock spin lock)
   Thread::exit();
   #endif
   
   return 0;
}
