#include "Publisher.h"

#include <sstream>

Publisher::Publisher(va_list args) :_mqtt ( ActorRef::NoSender()) {
	_mqtt = va_arg(args,ActorRef) ;
}

Publisher::~Publisher() {
}

void Publisher::preStart() {
	timers().startPeriodicTimer("START_TEST", TimerExpired(), 1000);
	_it = context().system().actorRefs().begin();
	eb.subscribe(self(), MessageClassifier(_mqtt,Mqtt::Disconnected));
	eb.subscribe(self(), MessageClassifier(_mqtt,Mqtt::Connected));
}

ActorRef* Publisher::nextRef() {
	if ( _it == context().system().actorRefs().end() ) {
		_it = context().system().actorRefs().begin();
	}
	ActorRef* pa = *_it;
	++_it;
//	INFO(" next ref : %s ",pa->path());
	return pa;
}

Receive& Publisher::createReceive() {
	return receiveBuilder()
	       .match(PropertiesReply(),
	[this](Envelope& msg) {
		msg.rewind();
		while(msg.hasData() ) {
			std::stringstream topic;

			Tag tag(msg.peek());
			Msg m(Mqtt::Publish);
			Uid tag_uid(tag.uid);
			if ( tag_uid == UID_DST  || tag_uid==UID_SRC || tag_uid==UID_CLS || tag_uid==UID_ID  ) {
				msg.skip();
			} else {
				topic  << "src/" <<  sender().path() << "/" << tag_uid.label();
				std::string message="---";
				if ( tag.type == Xdr::BYTES ) {
					msg.get(tag.uid,message);
				} else if ( tag.type == Xdr::UINT ) {
					uint64_t ui64;
					msg.get(tag.uid,ui64);
					string_format(message,"%lu",ui64);
				} else if ( tag.type == Xdr::INT ) {
					int64_t i64;
					msg.get(tag.uid,i64);
					string_format(message,"%ld",i64);
				} else if ( tag.type == Xdr::FLOAT ) {
					double d;
					msg.get(tag.uid,d);
					string_format(message,"%f",d);
				} else if ( tag.type == Xdr::BOOL ) {
					bool b;
					msg.get(tag.uid,b);
					b ? message="true" : message="false";
				} else {
					msg.skip();
				}
				_mqtt.tell(Msg(Mqtt::Publish)("topic",topic.str())("message",message),self());
			}

		}


		Msg m(Mqtt::Publish);
		m.add("system/alive","true");
		_mqtt.tell(m,self());
	})
	.match(TimerExpired(),
	[this](Envelope& msg) {
		if ( _mqttConnected == true ) {
			ActorRef* ref=nextRef();
			ref->tell(Msg(Properties()).src(self()),self());
		}
	})

	.match(Mqtt::Connected,
	[this](Envelope& msg) { _mqttConnected=true; })

	.match(Mqtt::Disconnected,
	[this](Envelope& msg) { _mqttConnected=false;  })
	.build();
}
