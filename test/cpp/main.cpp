#include <iostream>

#include "Base64Coder.h"

using namespace std;
using namespace db::util;

void runBase64Test()
{
	cout << "Running Base64 Test" << endl;
	
	char data[] = {'a', 'b', 'c', 'd'};
	string encoded = Base64Coder::encode(data);
	cout << encoded << endl;
	
	char* decoded = Base64Coder::decode(encoded);
	
	cout << sizeof(decoded) << endl;
	
	string encoded2 = Base64Coder::encode(decoded);
	cout << encoded2 << endl;
	cout << decoded << endl;
	
   if(decoded != NULL)
   {
	   delete [] decoded;
   }
}

int main()
{
   cout << "Tests starting..." << endl << endl;
   
   runBase64Test();
   
   cout << endl << "Tests finished." << endl;
}
