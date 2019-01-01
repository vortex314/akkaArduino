
#include <Akka.h>
#include <Echo.h>
#include <Mqtt.h>
#include <Bridge.h>
#include <System.h>
#include <Publisher.h>
#include <ESP8266WiFi.h>

Log logger(1024);
ActorMsgBus eb;
WiFiClient espClient;
PubSubClient client(espClient);
const char* ssid = "Merckx3";
const char* mqtt_server = "limero.ddns.net";

Mailbox defaultMailbox("default", 100); // nbr of messages in queue max
MessageDispatcher defaultDispatcher;
ActorSystem actorSystem("ESP-ARDUINO", defaultDispatcher, defaultMailbox);
ActorRef sender = actorSystem.actorOf<Echo>("echo");
ActorRef mqtt = actorSystem.actorOf<Mqtt>("mqtt", "tcp://limero.ddns.net:1883",&client);
ActorRef bridge = actorSystem.actorOf<Bridge>("bridge",mqtt);
ActorRef sys = actorSystem.actorOf<System>("system",mqtt);
ActorRef publisher = actorSystem.actorOf<Publisher>("publisher",mqtt);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Sys::hostname("ESP-ARDUINO");
  INFO("Connecting to %s as %s ",ssid,Sys::hostname());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

   defaultDispatcher.attach(defaultMailbox);
  defaultDispatcher.unhandled(bridge.cell());

  
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    INFO("Attempting MQTT connection... %s",Sys::hostname());
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      INFO("connected");
      eb.publish(Msg(Mqtt::Connected).src(mqtt));
    } else {
      eb.publish(Msg(Mqtt::Disconnected).src(mqtt));
      INFO("failed, rc=%d",client.state());
      INFO(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  defaultDispatcher.execute(false);

}
