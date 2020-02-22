/*
 * CustomLogger.h
 *
 *  Created on: Feb 17, 2020
 *      Author: jeff
 */

#ifndef CUSTOMLOGGER_H_
#define CUSTOMLOGGER_H_

#include <iostream>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

using namespace std;
enum DbgLevel { Verbose, Debug, Info, Error, Custom };

class CustomLogger
{
	private:
	DbgLevel logLevel;
	string logFileLocation;
	uint32_t rotateFiles;
	uint32_t rotateSize;
	bool directoryCreated;
	uint32_t curRotateNum;

	static void Rotate(string location, uint32_t rotateNum);

	public:
	CustomLogger(DbgLevel lvl = Error,
			string location = "./FirstLog/Log.txt",
			uint32_t rFiles = 10,
			uint32_t rSize = 75,
			bool dirCreated = false);
	void Log(DbgLevel lvl, string msg);
	void SetLogLevel(DbgLevel lvl);
	DbgLevel GetLogLevel(void);
	string GetLocation(void);
	void SetLocation(string location);
	void SetDirectoryCreated(bool val);
	bool GetDirectoryCreated(void);
	string GetLogFileDir(void);
	uint64_t GetLogFileSize(void);
	static string GetFileName(string location);
};

#endif /* CUSTOMLOGGER_H_ */
