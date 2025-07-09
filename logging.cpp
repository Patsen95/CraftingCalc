#include "logging.h"

#include <cstdarg>



namespace p95
{
	bool Logger::m_enabled = true;
	bool Logger::m_includeTimestamp = true;
	bool Logger::m_relativeTimestamps = false;
	bool Logger::m_toConsole = true;
	bool Logger::m_useTags = true;
	bool Logger::m_useFiltering = true;
	Logger::LogLevel Logger::m_globalLvl = Logger::LogLevel::INFO;
	Logger::LogLevel Logger::m_minLogLevel = Logger::LogLevel::ERR;

	std::string Logger::m_currentTag = "";
	std::time_t Logger::m_startTime = NULL;
	//std::vector<Record> m_logBuffer;

	/****************************************************************************/
	void Logger::init()
	{
		enable();
		m_startTime = time(NULL);
	}

	void Logger::enable()
	{
		m_enabled = true;
		log(LogLevel::WARNING, "Logger enabled");
	}

	void Logger::disable()
	{
		log(LogLevel::WARNING, "Logger disabled");
		m_enabled = false;
	}

	void Logger::includeTimestamp(bool state)
	{
		m_includeTimestamp = state;
	}

	void Logger::useRelativeTimestamps(bool state)
	{
		m_relativeTimestamps = state;
	}

	void Logger::loggingToConsole(bool state)
	{
		m_toConsole = state;
	}

	void Logger::useTag(bool state)
	{
		m_useTags = state;
	}

	void Logger::setTag(const char* tag)
	{
		m_currentTag = tag;
	}

	void Logger::useFiltering(bool state)
	{
		m_useFiltering = state;
	}

	void Logger::setGlobalLogLevel(LogLevel level)
	{
		m_globalLvl = level;
	}

	void Logger::setMinLogLevel(LogLevel level)
	{
		m_minLogLevel = level;
	}

	void Logger::log(const char* fmt, ...)
	{
		if(!m_enabled) return;
		if(m_useFiltering && m_globalLvl < m_minLogLevel) return;

		if(fmt == "\n")
		{
			std::printf("\033[0m\n");
			return;
		}

		if(m_toConsole)
		{
			std::printf("%s", levelToColor(m_globalLvl));
			if(m_includeTimestamp)
			{
				std::time_t _currentTime = std::time(NULL);
				std::tm _dt = { };

				if(m_relativeTimestamps)
				{
					std::time_t _timeDiff = std::difftime(m_startTime, _currentTime) * -1; // difftime returns negative value
					_dt = *std::localtime(&_timeDiff);
					_dt.tm_hour = 0;
				}
				else
					_dt = *std::localtime(&_currentTime);

				if(m_useTags && !m_currentTag.empty())
					std::printf("[%s] [%s] [%s] ", timeToStr(_dt).c_str(), levelToStr(m_globalLvl), m_currentTag.c_str());
				else
					std::printf("[%s] [%s] ", timeToStr(_dt).c_str(), levelToStr(m_globalLvl));

			}
			else
			{
				if(m_useTags && !m_currentTag.empty())
					std::printf("[%s] [%s] ", levelToStr(m_globalLvl), m_currentTag.c_str());
				else
					std::printf("[%s] ", levelToStr(m_globalLvl));
			}

			va_list args;
			va_start(args, fmt);
			std::vprintf(fmt, args);
			va_end(args);
			std::printf("\033[0m\n");
		}
	}

	void Logger::log(LogLevel lvl, const char* fmt, ...)
	{
		if(!m_enabled) return;
		if(m_useFiltering && lvl < m_minLogLevel) return;

		if(fmt == "\n")
		{
			std::printf("\033[0m\n");
			return;
		}

		if(m_toConsole)
		{
			std::printf("%s", levelToColor(lvl));
			if(m_includeTimestamp)
			{
				std::time_t _currentTime = std::time(NULL);
				std::tm _dt = { };

				if(m_relativeTimestamps)
				{
					std::time_t _timeDiff = std::difftime(m_startTime, _currentTime) * -1;
					_dt = *std::localtime(&_timeDiff);
					_dt.tm_hour = 0;
				}
				else
					_dt = *std::localtime(&_currentTime);

				if(m_useTags && !m_currentTag.empty())
					std::printf("[%s] [%s] [%s] ", timeToStr(_dt).c_str(), levelToStr(lvl), m_currentTag.c_str());
				else
					std::printf("[%s] [%s] ", timeToStr(_dt).c_str(), levelToStr(lvl));
			}
			else
			{
				if(m_useTags && !m_currentTag.empty())
					std::printf("[%s] [%s] ", levelToStr(lvl), m_currentTag.c_str());
				else
					std::printf("[%s] ", levelToStr(lvl));
			}

			va_list args;
			va_start(args, fmt);
			std::vprintf(fmt, args);
			va_end(args);
			std::printf("\033[0m\n");
		}
	}

	bool Logger::enabled()
	{
		return m_enabled;
	}

	/****************************************************************************/
	// NOTE: returns ONLY time value in 24-hour format
	std::string Logger::timeToStr(const std::tm& time)
	{
		char _buff[9];
		std::strftime(_buff, sizeof(_buff), "%T", &time);
		return _buff;
	}

	const char* Logger::levelToColor(LogLevel lvl)
	{
		switch(lvl) {
			case LogLevel::INFO:    return "\033[92m"; // green
			case LogLevel::WARNING: return "\033[33m"; // yellow
			case LogLevel::ERR:		return "\033[31m"; // red
			case LogLevel::DEBUG:   return "\033[36m"; // cyan
			case LogLevel::VERBOSE:   return "\033[95m"; // magenta
			default: return "\033[0m";  // white
		}
	}

	const char* Logger::levelToStr(LogLevel lvl)
	{
		switch(lvl)
		{
			default:
			case LogLevel::INFO: return "INFO";
			case LogLevel::WARNING: return "WARNING";
			case LogLevel::ERR: return "ERROR";
			case LogLevel::DEBUG: return "DEBUG";
			case LogLevel::VERBOSE: return "VERBOSE";
		}
	}
}
