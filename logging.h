#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <iostream>

#ifdef _MSVC_LANG
#pragma warning(disable:4996)
#endif


namespace p95
{
	class Logger
	{
	public:

		enum class LogLevel 
		{
			VERBOSE,
			DEBUG,
			ERR,    // "ERROR" is defined in some Windows header file and collision occurs -_-
			WARNING,
			INFO
		};

		struct Record
		{
			time_t timestamp;
			LogLevel logLevel;
			std::string message;
		};

		/****************************************************************************/
		static void init();
		static void enable();
		static void disable();
		static void includeTimestamp(bool state = true);
		static void useRelativeTimestamps(bool state = false);
		static void loggingToConsole(bool state = true);
		static void useTag(bool state = true);
		static void setTag(const char* tag);
		static void useFiltering(bool state = true);
		static void setGlobalLogLevel(LogLevel level);
		static void setMinLogLevel(LogLevel level = LogLevel::ERR);

		static void log(const char* fmt, ...);
		static void log(LogLevel lvl, const char* fmt, ...);

		static bool enabled();

	private:
		
		static bool m_enabled;
		static bool m_includeTimestamp;
		static bool m_relativeTimestamps;
		static bool m_toConsole;
		static bool m_useTags;
		static bool m_useFiltering;
		static LogLevel m_globalLvl;
		static LogLevel m_minLogLevel;
		
		static std::string m_currentTag;
		static std::time_t m_startTime;
		static std::vector<Record> m_logBuffer;

	private:

		//static void dumpToFile();

		static std::string timeToStr(const std::tm& time);
		static const char* levelToColor(LogLevel lvl);
		static const char* levelToStr(LogLevel lvl);
	};

#define LOG_INFO(...)    (Logger::log(Logger::LogLevel::INFO, __VA_ARGS__));
#define LOG_WARNING(...) (Logger::log(Logger::LogLevel::WARNING, __VA_ARGS__));
#define LOG_ERROR(...)   (Logger::log(Logger::LogLevel::ERR, __VA_ARGS__));
#define LOG_DEBUG(...)   (Logger::log(Logger::LogLevel::DEBUG, __VA_ARGS__));
#define LOG_TRACE(...)   (Logger::log(Logger::LogLevel::VERBOSE, __VA_ARGS__));
}