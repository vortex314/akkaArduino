/*
 * Log.h
 *
 *  Created on: Jul 3, 2016
 *      Author: lieven
 */

#ifndef LOG_H_
#define LOG_H_

#include <Semaphore.h>
#include <Sys.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <string>

extern std::string& string_format(std::string& str, const char* fmt, ...) ;


typedef void (*LogFunction)(char* start, uint32_t length);

class Log {
	public:
		typedef enum {
			LOG_TRACE = 0,
			LOG_DEBUG,
			LOG_INFO,
			LOG_WARN,
			LOG_ERROR,
			LOG_FATAL,
			LOG_NONE
		} LogLevel;
		static char _logLevel[7];
		std::string* _line;

	private:
		bool _enabled;
		LogFunction _logFunction;
		char _hostname[20];
		char _application[20];
		LogLevel _level;
		Semaphore& _sema;

	public:
		Log(uint32_t size);
		~Log();
		bool enabled(LogLevel level);
		void setLogLevel(char l);
		void disable();
		void enable();
		void defaultOutput();
		void setOutput(LogFunction function);
		void printf(const char* fmt, ...);
		void log(char level, const char* file, uint32_t line, const char* function,
		         const char* fmt, ...);

		void vprintf(const char* fmt, va_list args);
		const char* time();
		void host(const char* hostname);
		void location(const char* module, uint32_t line);
		void application(const char* applicationName);
		void flush();
		void level(LogLevel l);
		void logLevel();
		static void serialLog(char* start, uint32_t length);
		LogLevel level();
};

extern Log logger;
#include <cstdio>

#define LOGF(fmt, ...)                                                         \
	logger.log(__FLE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

/*
#define LOGF(fmt, ...)                                                         \
    logger.log(__FLE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
*/
#undef ASSERT
#define ASSERT(xxx)                                                            \
	if (!(xxx)) {                                                              \
		WARN(" ASSERT FAILED : %s", #xxx);                                     \
		while (1) {                                                            \
			Sys::delay(1000);                                                  \
		};                                                                     \
	}

#define INFO(fmt, ...)                                                         \
	if (logger.enabled(Log::LOG_INFO))                                         \
		logger.log('I', __FLE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)                                                        \
	if (logger.enabled(Log::LOG_ERROR))                                        \
		logger.log('E', __FLE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)                                                         \
	if (logger.enabled(Log::LOG_WARN))                                         \
		logger.log('W', __FLE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...)                                                        \
	if (logger.enabled(Log::LOG_FATAL))                                        \
		logger.log('F', __FLE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...)                                                        \
	if (logger.enabled(Log::LOG_DEBUG))                                        \
		logger.log('D', __FLE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define TRACE(fmt, ...)                                                        \
	if (logger.enabled(Log::LOG_TRACE))                                        \
		logger.log('T', __FLE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

#define __FLE__                                                                \
	(__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1   \
	 : __FILE__)

#endif /* LOG_H_ */

//#define __FLE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
//#define __FLE__ __FILE__
//#define __FLE__ past_last_slash(__FILE__)
/*
 * using cstr = const char* const;

static constexpr cstr past_last_slash(cstr str, cstr last_slash) {
    return *str == '\0' ? last_slash
                        : *str == '/' ? past_last_slash(str + 1, str + 1)
                                      : past_last_slash(str + 1, last_slash);
}

static constexpr cstr past_last_slash(cstr str) {
    return past_last_slash(str, str);
}
#define __FLEE__                                                               \
    ({                                                                         \
        constexpr cstr sf__{past_last_slash(__FILE__)};                        \
        sf__;                                                                  \
    })*/
