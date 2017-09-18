#ifndef MESSAGESENDER_H_
#define MESSAGESENDER_H_

#include <rtos.h>
#include "SerialLog.h"

class MessageSender {
	const int frameId;
	const unsigned char (*frames)[8];
	int frameCount;
	uint32_t interval;
	Thread thread;

	void run();
public:
	MessageSender(int frameId, unsigned char frames[][8], int frameCount, uint32_t interval)
		:frameId(frameId), frames(frames), frameCount(frameCount), interval(interval),
		 thread(osPriorityNormal, 768)
	{
		thread.start(callback(this, &MessageSender::run));
		getLog()->registerThread("MessageSender::run", &thread);
	}
	void send();
};

#endif /* MESSAGESENDER_H_ */
