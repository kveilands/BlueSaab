/*
 * C++ Class for handling CD changer emulator communications on SAAB I-Bus
 * Copyright (C) 2017 Karlis Veilands
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mbed.h"
#include "SaabCan.h"

CAN iBus(PB_8, PB_9);
CANMessage canRxFrame;
CANMessage canTxFrame;
SaabCan saabCan;
Serial pcSerial(USBTX, USBRX, 115200);

unsigned long cdcStatusLastSendTime = 0;		// Timer used to ensure we send the CDC status frame in a timely manner
unsigned long lastIcomingEventTime = 0; 		// Timer used for determining if we should treat current event as, for example, a long press of a button
bool cdcActive = false; 						// True while our module, the simulated CDC, is active
bool cdcStatusResendNeeded = false; 			// True if an internal operation has triggered the need to send the CDC status frame as an event
bool cdcStatusResendDueToCdcCommand = false; 	// True if the need for sending the CDC status frame was triggered by CDC_CONTROL frame (IHU)
int incomingEventCounter = 0; 					// Counter for incoming events to determine when we will treat the event, for example, as a long press of a button
unsigned char cdcPoweronCmd[NODE_STATUS_TX_MSG_SIZE][8] = {
		{ 0x32, 0x00, 0x00, 0x03, 0x01, 0x02, 0x00, 0x00 },
		{ 0x42, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00 },
		{ 0x52, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00 },
		{ 0x62, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00 }
};
unsigned char cdcActiveCmd[NODE_STATUS_TX_MSG_SIZE][8] = {
		{ 0x32, 0x00, 0x00, 0x16, 0x01, 0x02, 0x00, 0x00 },
		{ 0x42, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00 },
		{ 0x52, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00 },
		{ 0x62, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00 }
};
unsigned char cdcPowerdownCmd[NODE_STATUS_TX_MSG_SIZE][8] = {
		{ 0x32, 0x00, 0x00, 0x19, 0x01, 0x00, 0x00, 0x00 },
		{ 0x42, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x00 },
		{ 0x52, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x00 },
		{ 0x62, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x00 }
};

void SaabCan::initialize() {
	iBus.frequency(47619);
}

void SaabCan::printCanRxFrame() {
	pcSerial.printf("%#x: ", canRxFrame.id);
	for (int i = 0; i < canRxFrame.len; i++) {
		pcSerial.printf("%02x ", canRxFrame.data[i]);
	}
	pcSerial.printf("\r\n");
}

void SaabCan::sendCanFrame(int canId, unsigned char *data) {
	canTxFrame.id = canId;
	for (int i = 0; i < canTxFrame.len; i++) {
		canTxFrame.data[i] = data[i];
	}
	iBus.write(canTxFrame);
}
