/*
 * LOg.cpp
 *
 *  Created on: Jul 3, 2016
 *      Author: lieven
 */
#include <Log.h>
#include <stdio.h>
#include <stdlib.h>

char Log::_logLevel[7] = {'T', 'D', 'I', 'W', 'E', 'F', 'N'};

#ifdef ARDUINO
#include <Arduino.h>
#include <WString.h>
#endif
#ifdef OPENCM3
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#endif
#ifdef ESP32_IDF
#endif
#ifdef ESP_OPEN_RTOS
#endif

std::string& string_format(std::string& str, const char* fmt, ...) {
	int size = strlen(fmt) * 2 + 50; // Use a rubric appropriate for your code
	va_list ap;
	while (1) { // Maximum two passes on a POSIX system...
		ASSERT(size < 1024);
		str.resize(size);
		va_start(ap, fmt);
		int n = vsnprintf((char*)str.data(), size, fmt, ap);
		va_end(ap);
		if (n > -1 && n < size) { // Everything worked
			str.resize(n);
			return str;
		}
		if (n > -1)       // Needed size returned
			size = n + 1; // For null char
		else
			size *= 2; // Guess at a larger size (OS specific)
	}
	return str;
}

void Log::serialLog(char* start, uint32_t length) {
#ifdef ARDUINO
	Serial.write((const uint8_t*)start, length);
	Serial.write("\r\n");
#endif
#ifdef OPENCM3
	*(start + length) = '\0';
	while (*s) {
		usart_send_blocking(USART1, *(s++));
	}
#endif
#if defined(__linux__) || defined(ESP_OPEN_RTOS) || defined(ESP32_IDF)
	*(start + length) = '\0';
	fprintf(stdout, "%s\n", start);
	fflush(stdout);
#endif
}

Log::Log(uint32_t size)
	: _enabled(true), _logFunction(serialLog), _level(LOG_INFO),
	  _sema(Semaphore::create()) {
	if (_line == 0) {
		_line = new std::string;
		_line->reserve(size);
	}
	_application[0] = 0;
	_hostname[0] = 0;
}

Log::~Log() {}

void Log::setLogLevel(char c) {
	::printf("%s:%d was here\n",__FILE__,__LINE__);
	for (uint32_t i = 0; i < sizeof(_logLevel); i++)
		if (_logLevel[i] == c) {
			_level = (Log::LogLevel)i;
			break;
		}
}

bool Log::enabled(LogLevel level) {
	if (level >= _level) {
		return true;
	}
	return false;
}

void Log::disable() { _enabled = false; }

void Log::enable() { _enabled = true; }

void Log::defaultOutput() { _logFunction = serialLog; }

void Log::setOutput(LogFunction function) { _logFunction = function; }

#ifdef ESP8266
extern "C" {
#include <ets_sys.h>
};
#endif

void Log::log(char level, const char* file, uint32_t lineNbr,
              const char* function, const char* fmt, ...) {
	_sema.wait();
	if (_line == 0) {
		::printf("%s:%d %s:%d\n", __FILE__, __LINE__, file, lineNbr);
		return;
	}

	va_list args;
	va_start(args, fmt);
	static char logLine[256];
	vsnprintf(logLine, sizeof(logLine) - 1, fmt, args);
	va_end(args);
	_application[0] = 0;
#ifdef __linux__
	::snprintf(_application,sizeof(_application),"%X",(uint32_t)pthread_self());
#endif
#if defined(ESP32_IDF) || defined(ESP_OPEN_RTOS)
	extern void* pxCurrentTCB;
	::snprintf(_application, sizeof(_application), "%X",
	           (uint32_t)pxCurrentTCB);
#endif
	string_format(*_line, "%s %c | %8s | %s | %+10s:%-4d | %s", _application,
	              level, time(), Sys::hostname(), file, lineNbr, logLine);
	logger.flush();
	_sema.release();
}

void Log::flush() {
	if (_logFunction)
		_logFunction((char*)_line->c_str(), _line->size());
	*_line = "";
}

void Log::level(LogLevel l) { _level = l; }

Log::LogLevel Log::level() { return _level; }
//---------------------------------------------------------------------------------------------

//_________________________________________ LINUX
//___________________________________________
//
#ifdef __linux__
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
//---------------------------------------------------------------------------------------------
const char* Log::time() {
	struct timeval tv;
	struct timezone tz;
	struct tm* tm;
	static char buffer[100];
	gettimeofday(&tv, &tz);
	tm = ::localtime(&tv.tv_sec);
	sprintf(buffer, "%02d:%02d:%02d.%03ld ", tm->tm_hour, tm->tm_min,
	        tm->tm_sec, tv.tv_usec / 1000);
	return buffer;
}

extern const char* __progname;

#endif
//_________________________________________ EMBEDDED

#if defined(ESP32_IDF) || defined(ARDUINO)
const char* Log::time() {
	static char szTime[20];
	snprintf(szTime, sizeof(szTime), "%llu", Sys::millis());
	return szTime;
}
#endif

#if defined(ESP_OPEN_RTOS) // doesn't support 64 bit printf
const char* Log::time() {
	static char szTime[20];
	snprintf(szTime, sizeof(szTime), "%d", (uint32_t)Sys::millis());
	return szTime;
}
#endif

#ifdef ARDUINO

#endif
