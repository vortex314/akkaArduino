/*
 * Config.h
 *
 *  Created on: Jul 12, 2016
 *      Author: lieven
 */

#ifndef ACTOR_CONFIG_H_
#define ACTOR_CONFIG_H_
#define ARDUINOJSON_USE_LONG_LONG 1
//#define ARDUINOJSON_USE_INT64 1
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>
#include <Log.h>

class Config {
		void initialize();
		void initMagic();
		bool checkMagic();
		char _charBuffer[1024];
		DynamicJsonBuffer _jsonBuffer;
		JsonObject* _root;

		std::string _nameSpace;
		bool _loaded;

		JsonObject& nameSpace();
		const char* clone(const char* s);

	public:
		Config();
		virtual ~Config();

		void clear();
		void load();
		void save();
		void print(std::string& str);
		void printPretty(std::string& str);

		bool hasKey(const char* key);
		void setNameSpace(const char* ns);
		const char* getNameSpace();
		void get(const char*, int32_t&, int32_t defaultValue);
		void get(const char*, uint32_t&, uint32_t defaultValue);
		void get(const char*, std::string&, const char* defaultValue);
		void get(const char*, double&, double defaultValue);
		void remove(const char* key);

		void set(const char*, uint32_t);
		void set(const char*, int32_t);
		void set(const char*, std::string&);
		void set(const char*, double);
		void set(const char* key, const char* value);
};

extern Config config;

#endif /* ACTOR_CONFIG_H_ */
