/* C++ class for handling CAN messages/message groups on SAAB I-bus
 * Copyright (C) 2018 Girts Linde
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MESSAGESENDER_H_
#define MESSAGESENDER_H_

#include <rtos.h>
#include "SerialLog.h"

class MessageSender {
	const int32_t signal;
	const int frameId;
	const unsigned char (*frames)[8];
	int frameCount;
	uint32_t interval;
	Thread thread;

	void run();
public:
	MessageSender(int32_t signal, int frameId, unsigned char frames[][8], int frameCount, uint32_t interval)
		:signal(signal), frameId(frameId), frames(frames), frameCount(frameCount), interval(interval),
		 thread(osPriorityNormal, 256)
	{
		thread.start(callback(this, &MessageSender::run));
//		getLog()->registerThread("MessageSender::run", &thread);
	}
	void send();
};

#endif /* MESSAGESENDER_H_ */
