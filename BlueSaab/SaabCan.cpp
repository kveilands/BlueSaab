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
#include "rtos.h"
#include "SaabCan.h"

CAN iBus(PB_8, PB_9);
CANMessage canRxFrame;
Mail <CANMessage, 16> canFrameQueue;
SaabCan saabCan;
Serial pcSerial(USBTX, USBRX, 115200);

unsigned long cdcStatusLastSendTime = 0;						// Timer used to ensure we send the CDC status frame in a timely manner
unsigned long lastIcomingEventTime = 0; 						// Timer used for determining if we should treat current event as, for example, a long press of a button
bool cdcActive = false; 										// True while our module, the simulated CDC, is active
bool cdcStatusResendNeeded = false; 							// True if an internal operation has triggered the need to send the CDC status frame as an event
bool cdcStatusResendDueToCdcCommand = false; 				// True if the need for sending the CDC status frame was triggered by CDC_CONTROL frame (IHU)
int incomingEventCounter = 0; 								// Counter for incoming events to determine when we will treat the event, for example, as a long press of a button
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
unsigned char soundCmd[] = {0x80,0x04,0x00,0x00,0x00,0x00,0x00,0x00};

void SaabCan::initialize() {
	if (iBus.frequency(47619)) {
		pcSerial.printf("CAN initialized\r\n");
	}
	iBus.attach(callback(this,&SaabCan::onRx), mbed::CAN::RxIrq);
}

void SaabCan::printCanRxFrame() {
	pcSerial.printf("%#x: ", canRxFrame.id);
	for (int i = 0; i < canRxFrame.len; i++) {
		pcSerial.printf("%02x ", canRxFrame.data[i]);
	}
	pcSerial.printf("\r\n");
}

void SaabCan::sendCanFrame(int canId, unsigned char *data) {
	CANMessage *canTxFrame = canFrameQueue.alloc();
	canTxFrame->id = canId;
	for (int i = 0; i < canTxFrame->len; i++) {
		canTxFrame->data[i] = data[i];
	}
	canFrameQueue.put(canTxFrame);
}

void SaabCan::handleRxFrame() {
	if (iBus.read(canRxFrame)) {
		switch (canRxFrame.id) {
		case NODE_STATUS_RX_IHU:
			switch (canRxFrame.data[3] & 0x0F) {
			case (0x3):
				// messageSender.sendCanMessage(NODE_STATUS_TX_CDC, cdcPoweronCmd, 4, NODE_STATUS_TX_INTERVAL);
				break;
			case (0x2):
				// messageSender.sendCanMessage(NODE_STATUS_TX_CDC, cdcActiveCmd, 4, NODE_STATUS_TX_INTERVAL);
				break;
			case (0x8):
				// messageSender.sendCanMessage(NODE_STATUS_TX_CDC, cdcPowerdownCmd, 4, NODE_STATUS_TX_INTERVAL);
				break;
			}
			break;
		case CDC_CONTROL:
			handleIhuButtons();
			break;
		case STEERING_WHEEL_BUTTONS:
			handleSteeringWheelButtons();
			break;
		case DISPLAY_RESOURCE_GRANT:
			// sidResource.grantReceived(CAN_RxMsg.data);
			break;
		case IHU_DISPLAY_RESOURCE_REQ:
			// sidResource.ihuRequestReceived(CAN_RxMsg.data);
			break;
		default:
			break;

		}
	}
}

void SaabCan::handleIhuButtons() {
	bool event = (canRxFrame.data[0] == 0x80);
	if ((!event) && (cdcActive)) {
		// checkCanEvent(1);
		return;
	}
	switch (canRxFrame.data[1]) {
	case 0x24:
		cdcActive = true;
		// sidResource.activate();
		// BT.bt_reconnect();
		sendCanFrame(SOUND_REQUEST, soundCmd);
		break;
	case 0x14:
		// sidResource.deactivate();
		// BT.bt_disconnect();
		cdcActive = false;
		break;
	default:
		break;
	}
	// sidResource.requestDriverBreakthrough();
	if ((event) && (canRxFrame.data[1] != 0x00)) {
		if (cdcActive) {
			switch (canRxFrame.data[1]) {
			case 0x59: // NXT
				// BT.bt_play();
				break;
			case 0x84: // SEEK button (middle) long press on IHU
				break;
			case 0x88: // > 2 second long press of SEEK button (middle) on IHU
				break;
			case 0x76: // Random ON/OFF (Long press of CD/RDM button)
				break;
			case 0xB1: // Pause ON
				// BT.bt_play();
				break;
			case 0xB0: // Pause OFF
				// BT.bt_play();
				break;
			case 0x35: // Track +
				// BT.bt_next();
				break;
			case 0x36: // Track -
				// BT.bt_prev();
				break;
			case 0x68: // IHU buttons "1-6"
				switch (canRxFrame.data[2]) {
				case 0x01:
					// BT.bt_volup();
					break;
				case 0x02:
					// BT.bt_set_maxvol();
					break;
				case 0x03:
					// BT.bt_reconnect();
					break;
				case 0x04:
					// BT.bt_voldown();
					break;
				case 0x06:
					// BT.bt_disconnect();
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
		cdcStatusResendNeeded = true;
		cdcStatusResendDueToCdcCommand = true;
	}
}

void SaabCan::handleSteeringWheelButtons() {
    if (cdcActive) {
        // checkCanEvent(4);
        switch (canRxFrame.data[2]) {
            case 0x04: // NXT button on wheel
                //BT.bt_play();
                break;
            case 0x10: // Seek+ button on wheel
                //BT.bt_next();
                break;
            case 0x08: // Seek- button on wheel
                //BT.bt_prev();
                break;
            default:
                break;
        }
    }
}

extern DigitalOut myLED;

void SaabCan::onRx() {
	myLED = 1;
	if (iBus.read(canRxFrame)) {
	}
}
