
#include <Bridge.h>
#include <sys/types.h>
#include <unistd.h>
// volatile MQTTAsync_token deliveredtoken;

Bridge::Bridge(va_list args) : _mqtt ( ActorRef::NoSender()) {
	_mqtt = va_arg(args,ActorRef) ;
	_rxd=0;
	_txd=0;
};
Bridge::~Bridge() {}


void Bridge::preStart() {
	timers().startPeriodicTimer("PUB_TIMER", TimerExpired(), 5000);
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Connected));
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Disconnected));
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::PublishRcvd));
}


Receive& Bridge::createReceive() {
	return receiveBuilder()
	       .match(AnyClass(),
	[this](Envelope& msg) {
		if (!(*msg.receiver == self())) {
			std::string message;
			std::string topic ;
			messageToJson(topic,message, msg);
			_mqtt.tell(Msg(Mqtt::Publish)("topic",topic)("message",message),self());
			_txd++;
		}
	})
	.match(Mqtt::Connected,
	[this](Envelope& env) {
		INFO(" MQTT CONNECTED");
		_connected=true;
	})
	.match(Mqtt::Disconnected,
	[this](Envelope& env) {
		INFO(" MQTT DISCONNECTED");
		_connected=false;
	})
	.match(Mqtt::PublishRcvd,
	[this](Envelope& env) {
		std::string topic;
		std::string message;
		Msg msg;
//		INFO("envelope %s",env.toString().c_str());

		if (env.get("topic",topic)==0
		        && env.get("message",message)==0
		        && jsonToMessage(msg,topic,message)) {
			uid_type uid;
			ActorRef* dst,*src;
			if ( msg.get(UID_DST,uid)==0 && (dst=ActorRef::lookup(uid))!=0 && msg.get(UID_SRC,uid)==0 && (src=ActorRef::lookup(uid))!=0) {
				dst->mailbox().enqueue(msg);
				_rxd++;
				INFO(" processed message %s", msg.toString().c_str());
			} else {
				WARN(" incorrect message : %s", message.c_str());
			}
		} else {
			WARN(" processing failed : %s ", message.c_str());
		}
	})
	.match(Actor::TimerExpired(),
	[this](Envelope& msg) {
		string topic = "src/";
		topic += context().system().label();
		topic += "/system/alive";
		if (_connected) {
			Msg m(Mqtt::Publish);
			m("topic",topic)("data","true");
			_mqtt.tell(m,self());
		}
	})
	.match(Properties(),[this](Envelope& msg) {
		sender().tell(Msg(PropertiesReply())
		              ("txd",_txd)
		              ("rxd",_rxd)
		              ,self());
	})
	.build();
}

bool Bridge::messageToJson(std::string& topic,std::string& message, Envelope& msg) {
	topic= "dst/";
	topic += msg.receiver->path();
	topic +="/";
	uid_type uid;
	msg.get(UID_CLS,uid);
	topic += Uid(uid).label();

	Tag tag(0);
	msg.rewind();
	_jsonBuffer.clear();
	JsonObject& jsonObject = _jsonBuffer.createObject();

	std::string str;
	while (msg.hasData()) {
		tag.ui32 = msg.peek();
		Uid tagUid=Uid(tag.uid);
		switch (tag.type) {
			case Xdr::BYTES: {
					msg.getNext(tag.uid,str);
					jsonObject.set(Uid::label(tag.uid),str.c_str());
					break;
				}
			case Xdr::UINT: {
					uint64_t u64;
					msg.getNext(tag.uid,u64);
					if ( tagUid==UID_DST || tagUid==UID_SRC || tagUid==UID_CLS ) {
						if ( tagUid==UID_SRC )
							jsonObject.set(Uid::label(tag.uid),Uid::label(u64));
					} else {
						jsonObject.set(Uid::label(tag.uid),u64);
					}
					break;
				}
			case Xdr::INT: {
					int64_t i64;
					msg.getNext(tag.uid,i64);
					jsonObject.set(Uid::label(tag.uid),i64);
					break;
				}
			case Xdr::FLOAT: {
					double d;
					msg.getNext(tag.uid,d);
					jsonObject.set(Uid::label(tag.uid),d);
					break;
				}
			default:
				{ msg.skip(); }
		}
	};
	jsonObject.printTo(message);
	return true;
}
/*
 *  dst/actorSystem/actor/message_type
 *
 *
 *
 *
 */

bool Bridge::jsonToMessage(Msg& msg,std::string& topic,std::string& message) {
	_jsonBuffer.clear();
	JsonObject& jsonObject = _jsonBuffer.parseObject(message.data());
	if (jsonObject == JsonObject::invalid()) {
		WARN(" Invalid Json : %s ",message.c_str());
		return false;
	}
	if (!jsonObject.containsKey(AKKA_SRC) ) {
		WARN(" missing source in json message : %s ",message.c_str());
		return false;
	}
	uint32_t offsets[3]= {0,0,0};
	uint32_t prevOffset=0;
	for(uint32_t i=0; i<3; i++) {
		uint32_t offset = topic.find('/',prevOffset);
		if ( offset == string::npos ) break;
		offsets[i]= offset;
		prevOffset=offset+1;
	}
	if ( offsets[0]==0 || offsets[2]==0 ) {
		WARN(" invalid topic : %s",topic.c_str());
		return false;
	}

	INFO(" topic : %s %d,%d,%d",topic.c_str(),offsets[0],offsets[1],offsets[2]);
	INFO(" local actor : %s ",topic.substr(offsets[0]+1,offsets[2]-offsets[0]-1).c_str());
	INFO(" local msg class : %s ",topic.substr(offsets[2]+1).c_str());

	uid_type uidDst = Uid::add(topic.substr(offsets[0]+1,offsets[2]-offsets[0]-1).c_str());
	uid_type uidCls = Uid::add(topic.substr(offsets[2]+1).c_str());
	msg(UID_DST,uidDst);
	msg(UID_CLS,uidCls);
	uid_type uidSrc = Uid::add(jsonObject.get<const char*>(AKKA_SRC));
	msg(UID_SRC,uidSrc);

	ActorRef* dst = ActorRef::lookup(uidDst);
	if (dst == 0) {
		WARN(" local Actor : %s not found ", Uid::label(uidDst));
	}
	ActorRef* src = ActorRef::lookup(uidSrc);
	if (src == 0) {
		src = new ActorRef(uidSrc, &self().mailbox());
	}

	for (JsonObject::iterator it = jsonObject.begin(); it != jsonObject.end() ; ++it) {
		const char* key=it->key;
		if ( strcmp(key,AKKA_SRC)==0  ) {} else {
			if (it->value.is<char*>()) {
				msg(it->key,it->value.as<char*>());
			} else if (it->value.is<unsigned long>()) {
				msg(it->key,(uint64_t)it->value.as<unsigned long>());
			}  else if (it->value.is<long>()) {
				msg(it->key,(int64_t)it->value.as< long>());
			} else if (it->value.is<double>()) {
				msg(it->key,it->value.as< double>());
			}
		}
	}
//	INFO("%s = %s => %s : %s",Uid::label(uidSrc),Uid::label(uidCls),Uid::label(uidDst),message.c_str());
	return true;
}
