/* C++ class for handling SID (SAAB Information Display) resource requests and write access
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

#include "SidResource.h"
//#include "CDC.h"
#include "MessageSender.h"
#include "Scroller.h"
#include "SaabCan.h"

/**
 * Various constants used for SID text control
 */

#define NODE_APL_ADR                    0x1F
#define NODE_SID_FUNCTION_ID            0x12
#define NODE_DISPLAY_RESOURCE_REQ       0x357
#define NODE_WRITE_TEXT_ON_DISPLAY      0x337

SidResource sidResource;

SidResource::SidResource():
		textSender(0x20, NODE_WRITE_TEXT_ON_DISPLAY, sidMessageGroup, 3, 10),
		thread(osPriorityNormal, 256)
{
	sidDriverBreakthroughNeeded = false;
	sidWriteAccessWanted = false;
	writeTextOnDisplayUpdateNeeded = false;

	// Fill in some default values
	memcpy(sidMessageGroup[0],"\x42\x96\x02" "BlueS",sizeof(sidMessageGroup[0]));
	memcpy(sidMessageGroup[1],"\x01\x96\x02" "aab v",sizeof(sidMessageGroup[1]));
	memcpy(sidMessageGroup[2],"\x00\x96\x02" "6\0\0\0\0",sizeof(sidMessageGroup[2]));
}

SidResource::~SidResource() {

}

void SidResource::initialize() {
	saabCan.attach(DISPLAY_RESOURCE_GRANT, callback(this, &SidResource::grantReceived));
	saabCan.attach(IHU_DISPLAY_RESOURCE_REQ, callback(this, &SidResource::ihuRequestReceived));
//	getLog()->log("SidResource::initialize()\r\n");
	thread.start(callback(this, &SidResource::run));
//	getLog()->registerThread("SidResource::run", &thread);
}

void SidResource::run() {
//	getLog()->log("SidResource::run()\r\n");

	while(1) {
		sendDisplayRequest();
		sidDriverBreakthroughNeeded = false;
		Thread::wait(100);
		osEvent result = Thread::signal_wait(0x10, NODE_UPDATE_BASETIME-100);
		if (result.status == osEventSignal) {
		}
	}
}

/**
 * Sends a request for using the SID, row 2. We may NOT start writing until we've received a grant frame with the correct function ID!
 */

void SidResource::sendDisplayRequest() {

	/* Format of NODE_DISPLAY_RESOURCE_REQ frame:
	 ID: Node ID requesting to write on SID
	 [0]: Request source
	 [1]: SID object to write on; 0 = entire SID; 1 = 1st row; 2 = 2nd row
	 [2]: Request type: 1 = Engineering test; 2 = Emergency; 3 = Driver action; 4 = ECU action; 5 = Static text; 0xFF = We don't want to write on SID
	 [3]: Request source function ID
	 [4-7]: Zeroed out; not in use
	 */

	unsigned char displayRequestCmd[8];
	displayRequestCmd[0] = NODE_APL_ADR;
	displayRequestCmd[1] = 0x02;
	displayRequestCmd[2] = (sidWriteAccessWanted ? (sidDriverBreakthroughNeeded ? 0x01 : 0x05) : 0xFF);
	displayRequestCmd[3] = NODE_SID_FUNCTION_ID;
	displayRequestCmd[4] = 0x00;
	displayRequestCmd[5] = 0x00;
	displayRequestCmd[6] = 0x00;
	displayRequestCmd[7] = 0x00;

	saabCan.sendCanFrame(NODE_DISPLAY_RESOURCE_REQ, displayRequestCmd);
}

void SidResource::grantReceived(CANMessage& frame) {
	if (sidWriteAccessWanted) {
		if ((frame.data[0] == 0x02) && (frame.data[1] == NODE_SID_FUNCTION_ID)) {
			// We have been granted write access on 2nd row of SID
			const char *buffer = scroller.get();
			formatTextMessage(buffer[0] ? buffer : MODULE_NAME, writeTextOnDisplayUpdateNeeded);
			textSender.send();
		}
	}
}

void SidResource::ihuRequestReceived(CANMessage& frame) {
	if (sidWriteAccessWanted) {
		if ((frame.data[2] == 0x03) || (frame.data[2] == 0x05)) { // IHU requested DriverBreakthrough
			requestDriverBreakthrough();
		}
	}
}

/**
 * Formats provided text for writing on the SID. This function assumes that we have been granted write access. Do not call it if we haven't!
 * Note: the character set used by the SID is slightly nonstandard. "Normal" characters should work fine.
 */

void SidResource::formatTextMessage(const char textIn[], bool event) {
	// Copy the provided string and make sure we have a new array of the correct length
	unsigned char textToSid[15];
	int n = strnlen(textIn, 12); // 12 is the number of characters SID can display on each row; anything beyond 12 is going to be zeroed out
	for (int i = 0; i < n; i++) {
		textToSid[i] = textIn[i];
	}
	for (int i = n; i < 15; i++) {
		textToSid[i] = 0;
	}

	unsigned char eventByte = event ? 0x82 : 0x02;
	sidMessageGroup[0][2] = eventByte;
	sidMessageGroup[1][2] = eventByte;
	sidMessageGroup[2][2] = eventByte;
	memcpy(&sidMessageGroup[0][3], textToSid, 5);
	memcpy(&sidMessageGroup[1][3], textToSid + 5, 5);
	memcpy(&sidMessageGroup[2][3], textToSid + 10, 5);
}
