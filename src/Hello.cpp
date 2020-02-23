/*
 * Hello.cpp
 *
 *  Created on: Feb 17, 2020
 *      Author: jeff
 */
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include "CustomLogger.h"

using namespace std;


int main(int argc, char *argv[])
{
	cout << "argv[0] " << argv[0] << endl;
	int i = 0;
	CustomLogger myCLogger;
	myCLogger.GetSettingsFromXml();
	stringstream msg;

	for( i = 0; i < 200; i++ ) {
		msg << "First Try " << i;
		myCLogger.Log(Custom, msg.str());
		usleep(1500000);
		msg.str("");
	}

}




