/*
 * CustomLogger.cpp
 *
 *  Created on: Feb 17, 2020
 *      Author: jeff
 */

#include "CustomLogger.h"
#include <pystring.h>
#include <vector>
#include <sys/stat.h>
#include <ctime>
#include <sys/time.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

using namespace std;

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

template <class Container>
void Splitter(const std::string& str, Container& cont,
              const std::string delim = " ")
{
    std::vector<std::string> vec;
    pystring::split(str, vec, delim);
    std::copy(vec.begin(), vec.end(), std::back_inserter(cont));
    if(str.at(0) == '.') {
    	if(str.at(1) == '.') {
    		vec.at(0) = "..";
    	}
    	else {
    		vec.at(0) = ".";
    	}
    }
}

enum CmpType {GZIP, BZ2};
void PdoCompress(string fileName, CmpType ct)
{
	stringstream cmd;
	FILE *tarResStream;
	if(ct == GZIP) {
		// cmd << "tar -cvzf " << fileName << ".gz " << fileName << " 2<&1";
		cmd << "gzip " << "-f " << fileName << " 2<&1";
	}
	else if(ct == BZ2) {
		cmd << "tar -cvjf " << fileName << ".bz2 " << fileName << " 2<&1";
	}
	tarResStream = popen(cmd.str().c_str(), "r");
	char outch;
	do {
		outch = fgetc(tarResStream);
		cout << outch;
		if( feof(tarResStream) )
			break;
	}while ( 1 );

	fclose(tarResStream);
}

uint32_t doDef(FILE *source, FILE *dest, int32_t level)
{
	const int chunk = 65536;
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[chunk];
    unsigned char out[chunk];

    /* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = &in[0];
	//ret = deflateInit(&strm, level);
	ret = deflateInit2(&strm, level,
			16 + MAX_WBITS, 8, Z_DEFLATED,
			Z_DEFAULT_STRATEGY);
	if (ret != Z_OK)
		return ret;
	/* compress until end of file */
	do {
		strm.avail_in = fread(in, 1, chunk, source);
		if (ferror(source)) {
			(void)deflateEnd(&strm);
			return Z_ERRNO;
		}
		flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;
		/* run deflate() on input until output buffer not full, finish
		   compression if all of source has been read in */
		do {
			strm.avail_out = chunk;
			strm.next_out = out;
			ret = deflate(&strm, flush);    /* no bad return value */
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			have = chunk - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */
	} while(flush != Z_FINISH);
	assert(ret == Z_STREAM_END);
	/* clean up and return */
	(void)deflateEnd(&strm);
	return Z_OK;
}

void CustomLogger::Rotate(string location, uint32_t rotateNum)
{
	stringstream newName, zipFileName;
	// FILE *newNameFile, *zipFile;
	// rename the file
	newName << location << rotateNum;
	zipFileName << newName.str() << ".z";

	cout << "newName: " << newName.str() << endl;
	cout << "zipFileName: " << zipFileName.str() << endl;

	rename(location.c_str(), newName.str().c_str());

	// newNameFile = fopen(newName.str().c_str(), "rb");
	// zipFile = fopen(zipFileName.str().c_str(), "wb");

	// SET_BINARY_MODE(newNameFile);
	// SET_BINARY_MODE(zipFile);

	// doDef(newNameFile, zipFile, Z_DEFAULT_COMPRESSION);
	// doDef(newNameFile, zipFile, Z_BEST_COMPRESSION);
	string fName = GetFileName(newName.str());
	PdoCompress(newName.str(), GZIP);
	remove(newName.str().c_str());
	// fclose(newNameFile);
	// fclose(zipFile);

// zip the file
}

CustomLogger::CustomLogger(DbgLevel lvl,
		string location,
		uint32_t rFiles,
		uint32_t rSize,
		bool dirCreated):
		logLevel(lvl),
		logFileLocation(location),
		rotateFiles(rFiles),
		rotateFileSize(rSize),
		directoryCreated(dirCreated),
		curRotateNum(0)
{
	stringToDebugLevel["Verbose"] = Verbose;
	stringToDebugLevel["Debug"] = Debug;
	stringToDebugLevel["Info"] = Info;
	stringToDebugLevel["Error"] = Error;
	stringToDebugLevel["Custom"] = Custom;

}

mutex logMutex;
void CustomLogger::Log(DbgLevel lvl, string msg)
{
	logMutex.lock();
    if(!directoryCreated)
    {
    	string logFileDir = GetLogFileDir();

    	int res = mkdir(logFileDir.c_str(), S_IRWXU | S_IRGRP | S_IROTH);
        if( res == -1 )
        {
            perror("Error ");
            if( errno == EEXIST )
            {
                directoryCreated = true;
            }
        }
        else
        {
            directoryCreated = true;
        }
    }

    ofstream logFile;
    logFile.open(logFileLocation, ios::out | ios::app);
    bool isTheLogFileOpen = logFile.is_open();

    cout << "logFile.is_open() " << isTheLogFileOpen << " " << true;

	if(lvl >= logLevel) {
		// cout << msg << endl;
        // time_t lTime;
        struct tm lTm;
        struct timeval tv;
        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &lTm);
        char timeString[200];
        strftime(timeString, 200, "%x %X", &lTm);
        cout << timeString << "."<< tv.tv_usec << ": " << msg << /* "\n" */ endl;
        logFile << timeString << "."<< tv.tv_usec << ": " << msg << /* "\n" */ endl;
        logFile.close();
	}
	if( GetLogFileSize() > rotateFileSize )
	{

		thread rotateThread(Rotate, logFileLocation, curRotateNum);
		curRotateNum++;
		if( curRotateNum >= rotateFiles) {
			curRotateNum = 0;
		}
		rotateThread.detach();

	}
	logMutex.unlock();
}

void CustomLogger::SetLogLevel(DbgLevel lvl)
{
	logLevel = lvl;
}


DbgLevel CustomLogger::GetLogLevel(void)
{
	return logLevel;
}

string CustomLogger::GetLocation(void)
{
	return logFileLocation;
}

void CustomLogger::SetLocation(string location)
{
	logFileLocation = location;
}

bool CustomLogger::GetDirectoryCreated(void)
{
	return directoryCreated;
}

void CustomLogger::SetDirectoryCreated(bool val)
{
	directoryCreated = val;
}

string CustomLogger::GetLogFileDir(void)
{
	vector<string> pathPieces;
	Splitter(this->logFileLocation, pathPieces, "/");
	stringstream path;
	bool isFirstString = true;
	vector<string>::iterator nextToLast;
	for(vector<string>::iterator it = pathPieces.begin(); it !=pathPieces.end(); it++) {
		nextToLast = it;
		nextToLast++;
		if(nextToLast == pathPieces.end())
			break;
		// char test = it->c_str()[0];
		if( /* (test == '.') && */ isFirstString ) {
			path << it->c_str();
		}
		else {
			path << '/' << it->c_str();
		}

		isFirstString = false;

	}
	return path.str();
}

uint64_t CustomLogger::GetLogFileSize(void)
{
    struct stat stat_buf;
    int rc = stat(logFileLocation.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : 0;
}

string CustomLogger::GetFileName(string location)
{
	vector<string> pathPieces;
	Splitter(location, pathPieces, "/");
	return(pathPieces[pathPieces.size() - 1]);
}

void CustomLogger::GetSettingsFromXml(void)
{
	using boost::property_tree::ptree;

	ptree pt;
	// read_xml("/home/jeff/eclipse-workspace/FirstCpp/src/FirstPrj.xml", pt);
	read_xml("./CustomLoggerSettings.xml", pt);

	BOOST_FOREACH( ptree::value_type const& v, pt.get_child("CustomLogger") ) {
		if( v.first == "debugLevel" ) {
			string check = v.second.get_value<string>();
			if(stringToDebugLevel.find(check) == stringToDebugLevel.end())
			{
				cout << "invalid debugLevel " << check;
				return;
			}
			this->logLevel = stringToDebugLevel[check];
		}
		else if( v.first == "logFileLocation" ) {
			this->logFileLocation = v.second.get_value<string>();
		}
		else if( v.first == "rotateFiles" ) {
			this->rotateFiles = atoi(v.second.get_value<string>().c_str());
		}
		else if( v.first == "rotateFileSize") {
			this->rotateFileSize = atoi(v.second.get_value<string>().c_str());
		}
	}
}
