#include "MessageSender.h"
#include "SaabCan.h"

void MessageSender::send() {
	thread.signal_set(0x1);
}

void MessageSender::run() {
	while(1) {
		Thread::signal_wait(signal);
		uint32_t t0 = us_ticker_read();
		for(int i=0; i<frameCount; i++) {
			if (i > 0)
				Thread::wait(interval);
			int32_t dt = us_ticker_read() - t0;
			getLog()->log("MessageSender::run send, dt1 = %d\r\n", dt/1000);
			saabCan.sendCanFrame(frameId, frames[i]);
		}
	}
}
