#include <iostream>

#include "Base64Coder.h"
#include "System.h"

using namespace std;
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

int main()
{
   cout << "Tests starting..." << endl << endl;
   
   //runBase64Test();
   runTimeTest();
   
   cout << endl << "Tests finished." << endl;
}
