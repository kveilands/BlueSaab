#ifndef MESSAGESENDER_H_
#define MESSAGESENDER_H_

#include <rtos.h>

class MessageSender {
	int frameId;
	unsigned char (*frames)[8];
	int frameCount;
	uint32_t interval;
	Thread thread;

	void run();
public:
	MessageSender(int frameId, unsigned char frames[][8], int frameCount, uint32_t interval)
		:frameId(frameId), frames(frames), frameCount(frameCount), interval(interval)
	{}
	void send();
};

#endif /* MESSAGESENDER_H_ */
