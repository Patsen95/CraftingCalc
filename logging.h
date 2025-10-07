#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <cstdarg>


#ifdef _MSVC_LANG
#pragma warning(disable:4996) // MSVC is whining about "unsafe" use of ctime header functions, so we need to shut him up
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

	public:
		/****************************************************************************/
		static void init(bool toConsole = true);
		static void enable();
		static void disable();

		static void includeTimestamp(bool state);
		static void useRelativeTimestamps(bool state);
		static void loggingToConsole(bool state);
		static void useTags(bool state);
		static void setGlobalTag(const char* tag);
		static void useFiltering(bool state);
		static void setGlobalLogLevel(LogLevel level);
		static void setMinLogLevel(LogLevel level);

		static void log(const char* fmt, ...);
		static void log(const char* tag, const char* fmt, ...);
		static void log(LogLevel lvl, const char* fmt, ...);
		static void log(LogLevel lvl, const char* tag, const char* fmt, ...);


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

		static std::string m_globalTag;
		static std::time_t m_startTime;
		//static std::vector<Record> m_logBuffer;

	private:

		//static void dumpToFile();
		static void _log(LogLevel lvl, const char* tag, const char* fmt, va_list args);
		
		static std::string timeToStr(const std::tm& time);
		static const char* levelToColor(LogLevel lvl);
		static const char* levelToStr(LogLevel lvl);
	};


/****************************************************************************/
#define LOG_INFO(...)    (Logger::log(Logger::LogLevel::INFO, "", __VA_ARGS__))
#define LOG_WARNING(...) (Logger::log(Logger::LogLevel::WARNING, "", __VA_ARGS__))
#define LOG_ERROR(...)   (Logger::log(Logger::LogLevel::ERR, "", __VA_ARGS__))
#define LOG_DEBUG(...)   (Logger::log(Logger::LogLevel::DEBUG, "", __VA_ARGS__))
#define LOG_TRACE(...)   (Logger::log(Logger::LogLevel::VERBOSE, "", __VA_ARGS__))


#define _LOG_TAGGED_(lvl, tag, ...) (Logger::log(lvl, tag, __VA_ARGS__))

#define LOG_INFO_T(...)         _LOG_TAGGED_(Logger::LogLevel::INFO, __VA_ARGS__)
#define LOG_WARNING_T(tag, ...) _LOG_TAGGED_(Logger::LogLevel::WARNING, tag, __VA_ARGS__)
#define LOG_ERROR_T(tag, ...)   _LOG_TAGGED_(Logger::LogLevel::ERR, tag, __VA_ARGS__)
#define LOG_DEBUG_T(tag, ...)   _LOG_TAGGED_(Logger::LogLevel::DEBUG, tag, __VA_ARGS__)
#define LOG_TRACE_T(tag, ...)   _LOG_TAGGED_(Logger::LogLevel::VERBOSE, tag, __VA_ARGS__)
}