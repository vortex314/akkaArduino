#include <Echo.h>

Echo::Echo(va_list args)  {}
Echo::~Echo() {}

Receive& Echo::createReceive() {
	return receiveBuilder()
	       .match(PING,
	[this](Envelope& msg) {
		uint32_t counter;
		assert(msg.get("counter", counter)==0);
		sender().tell(Msg(PONG)("counter",counter+1),self());
	})
	.build();
}
