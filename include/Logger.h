#pragma once

#include <Export.h>

#include <string>

namespace Log
{

class Log_API MLogger
{
public:
	MLogger( const char* aCategory );
	~MLogger();

	void  logInfo( const char* aMsg, const char* aFile, const char* aFunction, int aLine );
	void  logWarning( const char* aMsg, const char* aFile, const char* aFunction, int aLine );
	void  logError( const char* aMsg, const char* aFile, const char* aFunction, int aLine );

	enum LogLevel
	{
		Trace,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};

private:
	void* mLoggerPtr;

	void log( const std::string aMsg, const char* aFile, const char* aFunction, int aLine, LogLevel aLevel );
};

}