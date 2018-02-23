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

SidResource::SidResource(): thread(osPriorityNormal, 256) {
	sidDriverBreakthroughNeeded = false;
//	sidRequestLastSendTime = 0;
	sidWriteAccessWanted = false;
	writeTextOnDisplayUpdateNeeded = false;
}

SidResource::~SidResource() {

}

void SidResource::initialize() {
	saabCan.attach(DISPLAY_RESOURCE_GRANT, callback(this, &SidResource::grantReceived));
	saabCan.attach(IHU_DISPLAY_RESOURCE_REQ, callback(this, &SidResource::ihuRequestReceived));
//	saabCan.attach(NODE_STATUS_RX_IHU, callback(this, &CDCStatus::onIhuStatusFrame));
//	saabCan.attach(CDC_CONTROL, callback(this, &CDCStatus::onCDCControlFrame));
	getLog()->log("SidResource::initialize()\r\n");
	thread.start(callback(this, &SidResource::run));
	getLog()->registerThread("SidResource::run", &thread);
}

void SidResource::run() {
	getLog()->log("SidResource::run()\r\n");
//	bool cdcStatusResendNeeded = false;
//	bool cdcStatusResendDueToCdcCommand = false;

	while(1) {
//		getLog()->log2("CDCStatus::run() loop\r\n");
//		getLog()->log("cdcActive %d\r\n", cdcActive);
//		sendCdcStatus(cdcStatusResendNeeded, cdcStatusResendDueToCdcCommand, cdcActive);
		sendDisplayRequest();
		sidDriverBreakthroughNeeded = false;
		Thread::wait(100);
//		cdcStatusResendNeeded = false;
//		cdcStatusResendDueToCdcCommand = false;
		osEvent result = Thread::signal_wait(0x16, NODE_UPDATE_BASETIME-100);
		if (result.status == osEventSignal) {
//			cdcStatusResendNeeded = true;
//			cdcStatusResendDueToCdcCommand = true;
		}
	}
}

//void SidResource::update() {
//    if (sidDriverBreakthroughNeeded && (millis() - sidRequestLastSendTime > 100)) {
//        sendDisplayRequest();
//        sidDriverBreakthroughNeeded = false;
//    }
//    if (millis() - sidRequestLastSendTime > NODE_UPDATE_BASETIME) {
//        sendDisplayRequest();
//    }
//}

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

//    CDC.sendCanFrame(NODE_DISPLAY_RESOURCE_REQ, displayRequestCmd);
    saabCan.sendCanFrame(NODE_DISPLAY_RESOURCE_REQ, displayRequestCmd);

    // Record the time of sending and reset status variables
//    sidRequestLastSendTime = millis();
}

unsigned char sidMessageGroup[3][8] = {
    {0x42,0x96,0x02,'B','l','u','e','S'},
    {0x01,0x96,0x02,'a','a','b',' ','v'},
    {0x00,0x96,0x02,'6',0,0,0,0}
};

MessageSender textSender(0x32, NODE_WRITE_TEXT_ON_DISPLAY, sidMessageGroup, 3, 10);

void SidResource::grantReceived(CANMessage& frame) {
    if (sidWriteAccessWanted) {
        if ((frame.data[0] == 0x02) && (frame.data[1] == NODE_SID_FUNCTION_ID)) {
            // We have been granted write access on 2nd row of SID
        //    const char *buffer = scroller.get();
        //    writeTextOnDisplay(buffer[0] ? buffer : MODULE_NAME, writeTextOnDisplayUpdateNeeded);
        	textSender.send();
        }
//        else if ((data[0] == 0x02) && (data[1] == 0x19)) {
//            requestDriverBreakthrough();
//        }
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

//void SidResource::writeTextOnDisplay(const char textIn[], bool event) {
//
//    if (!textIn) {
//        return;
//    }
//    // Copy the provided string and make sure we have a new array of the correct length
//    unsigned char textToSid[15];
//    int n = strnlen(textIn, 12); // 12 is the number of characters SID can display on each row; anything beyond 12 is going to be zeroed out
//    for (int i = 0; i < n; i++) {
//        textToSid[i] = textIn[i];
//    }
//    for (int i = n; i < 15; i++) {
//        textToSid[i] = 0;
//    }
//
//    unsigned char eventByte = event ? 0x82 : 0x02;
//    unsigned char sidMessageGroup[3][8] = {
//        {0x42,0x96,eventByte,textToSid[0],textToSid[1],textToSid[2],textToSid[3],textToSid[4]},
//        {0x01,0x96,eventByte,textToSid[5],textToSid[6],textToSid[7],textToSid[8],textToSid[9]},
//        {0x00,0x96,eventByte,textToSid[10],textToSid[11],textToSid[12],textToSid[13],textToSid[14]}
//    };
//
//    messageSender.sendCanMessage(NODE_WRITE_TEXT_ON_DISPLAY,sidMessageGroup,3,10);
//    writeTextOnDisplayUpdateNeeded = false;
//}
