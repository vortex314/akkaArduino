#include <Mqtt.h>

static Mqtt* _mqtt;



MsgClass Mqtt::PublishRcvd("PublishRcvd");
MsgClass Mqtt::Connected("Connected");
MsgClass Mqtt::Disconnected("Disconnected");
MsgClass Mqtt::Publish("Publish");
MsgClass Mqtt::Subscribe("Subscribe");




Mqtt::Mqtt(va_list args) {
	_address = va_arg(args, const char*);
	_client= va_arg(args,PubSubClient*);
	_client->setServer("limero.ddns.net", 1883);
	_client->setCallback(callback);
	_mqtt=this;

};
Mqtt::~Mqtt() {}


void Mqtt::callback(char* topic, byte* payload, unsigned int length) {

	std::string message((char*)payload,length);
	eb.publish(Msg(Mqtt::PublishRcvd)("topic",topic)("message",message).src(_mqtt->self().id()));

}

void Mqtt::preStart() {

	timers().startPeriodicTimer("PUB_TIMER", TimerExpired(), 5000);
}

Receive& Mqtt::createReceive() {
	return receiveBuilder()
	       .match(TimerExpired(),
	[this](Envelope& msg) {
		string topic = "src/";
		topic += context().system().label();
		topic += "/system/alive";
		if (_connected) {
			mqttPublish(topic.c_str(), "true");
		}
	}).match(Mqtt::Publish,
	[this](Envelope& msg) {
		std::string topic;
		std::string message;
		if ( msg.get("topic",topic)==0 && msg.get("message",message)==0 ) {
			mqttPublish(topic.c_str(),message.c_str());
		}
	})
	.build();
}

void Mqtt::mqttPublish(const char* topic, const char* message) {
	_client->publish(topic, message);
}

void Mqtt::mqttSubscribe(const char* topic) {
	_client->subscribe(topic);
}