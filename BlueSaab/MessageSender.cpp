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

#include "MessageSender.h"
#include "SaabCan.h"

void MessageSender::send() {
	thread.signal_set(signal);
}

void MessageSender::run() {
	while(1) {
		Thread::signal_wait(signal);
//		uint32_t t0 = us_ticker_read();
		for(int i=0; i<frameCount; i++) {
			if (i > 0)
				Thread::wait(interval);
//			int32_t dt = us_ticker_read() - t0;
//			getLog()->log("MessageSender::run send, dt1 = %d\r\n", dt/1000);
			saabCan.sendCanFrame(frameId, frames[i]);
		}
	}
}
