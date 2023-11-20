#include <iostream>
#include "WPA.h"
using namespace std;
int main()
{
	WPA wpa(10,2,4,6,500,1000,1000);
	wpa.run();
	return 0;
}