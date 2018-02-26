/*
 * C++ Class for handling SID resource requests and write access
 * Copyright (C) 2017 Girts Linde
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Created by: Girts Linde
 * Created on: April 18, 2017
 */

#ifndef SAAB_CDC_SIDRESOURCE_H_
#define SAAB_CDC_SIDRESOURCE_H_

#include <rtos.h>
#include "MessageSender.h"

class SidResource {
	unsigned char sidMessageGroup[3][8];
	MessageSender textSender;

	bool sidDriverBreakthroughNeeded;
//	unsigned long sidRequestLastSendTime;
	bool sidWriteAccessWanted;                        // True while we want to write on SID

	bool writeTextOnDisplayUpdateNeeded;

	Thread thread;

	void run();

	void sendDisplayRequest();
	void formatTextMessage(const char textIn[], bool event);
public:
	SidResource();
	~SidResource();

	void initialize();
//	void update();
	void requestDriverBreakthrough() {
		sidDriverBreakthroughNeeded = true;
		thread.signal_set(0x10);
	}
	void activate() {
		sidWriteAccessWanted = true;
		writeTextOnDisplayUpdateNeeded = true;
	}
	void deactivate() {
		sidWriteAccessWanted = false;
	}

	void grantReceived(CANMessage& frame);
	void ihuRequestReceived(CANMessage& frame);
};

extern SidResource sidResource;

#endif /* SAAB_CDC_SIDRESOURCE_H_ */
