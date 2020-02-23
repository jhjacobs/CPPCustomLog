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
#include <map>

using namespace std;
enum DbgLevel { Verbose, Debug, Info, Error, Custom };

class CustomLogger
{
	private:
	DbgLevel logLevel;
	string logFileLocation;
	uint32_t rotateFiles;
	uint32_t rotateFileSize;
	bool directoryCreated;
	uint32_t curRotateNum;

	map<string, DbgLevel> stringToDebugLevel;

	static void Rotate(string location, uint32_t rotateNum);

	public:
	CustomLogger(DbgLevel lvl = Error,
			string location = "./FirstLog/Log.txt",
			uint32_t rFiles = 100,
			uint32_t rSize = 10000,
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
	void GetSettingsFromXml(void);

};

#endif /* CUSTOMLOGGER_H_ */
