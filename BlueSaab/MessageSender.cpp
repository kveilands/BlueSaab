#include "MessageSender.h"
#include "SaabCan.h"

void MessageSender::send() {
	thread.start(callback(this, &MessageSender::run));
}

void MessageSender::run() {
	for(int i=0; i<frameCount; i++) {
		if (i > 0)
			Thread::wait(interval);
		saabCan.sendCanFrame(frameId, frames[i]);
	}
}
