#ifndef __MQTT_H
#define __MQTT_H

#include <Akka.h>
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

class Mqtt : public Actor {

		std::string _address;
		bool _connected=false;

		PubSubClient* _client;

	public:
		static MsgClass PublishRcvd;
		static MsgClass Connected;
		static MsgClass Disconnected;
		static MsgClass Subscribe;
		static MsgClass Publish;

		Mqtt(va_list args);
		~Mqtt();
		void preStart();
		Receive& createReceive();

		void mqttPublish(const char* topic, const char* message);
		void mqttSubscribe(const char* topic);
		static void callback(char* topic, byte* payload, unsigned int length) ;

};
#endif
