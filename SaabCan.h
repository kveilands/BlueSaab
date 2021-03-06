/* C++ class for handling/emulating node communications on SAAB I-bus
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

#ifndef SAABCAN_H_
#define SAABCAN_H_

#include "rtos.h"
#include "SerialLog.h"

/**
 * TX frames:
 */

#define GENERAL_STATUS_CDC			0x3C8
#define NODE_STATUS_TX_CDC			0x6A2
#define SOUND_REQUEST				0x430

/**
 * RX frames:
 */

#define CDC_CONTROL					0x3C0
#define DISPLAY_RESOURCE_GRANT		0x368
#define NODE_STATUS_RX_IHU			0x6A1
#define STEERING_WHEEL_BUTTONS		0x290
#define IHU_DISPLAY_RESOURCE_REQ	0x348

/**
 * Time interval definitions:
 */

#define NODE_STATUS_TX_INTERVAL		140 			// Replies to '6A1' request need to be sent with no more than 140ms interval; tolerances +/- 10%
#define CDC_STATUS_TX_BASETIME		950 			// The CDC status frame must be sent periodically within this time-frame; tolerances +/- 10%
#define NODE_UPDATE_BASETIME		1000

/**
 * Other useful stuff
 */

#define MODULE_NAME					"BlueSaab v6"
#define LAST_EVENT_IN_TIMEOUT		3000 		    // Milliseconds
#define NODE_STATUS_TX_MSG_SIZE		4 			    // Decimal; defines how many frames do we need to reply with to '6A1'

struct FrameCallback {
	unsigned int id;
	Callback<void(CANMessage&)> callBack;
	FrameCallback(): id(0) {}
};

class SaabCan {
	enum {CAN_MAX_CALLBACKS = 10};
	Mail <CANMessage, 16> canFrameQueue;
	FrameCallback callBacks[CAN_MAX_CALLBACKS];
	Thread send_thread;

	void sendFunc();
public:
	SaabCan(): send_thread(osPriorityNormal, 256) {}
	void initialize(int hz);
	void sendCanFrame(int canId, const unsigned char *data);
	void sendCanMessage(CANMessage &msg);
	void sendCanMessage(CANFormat format, unsigned int id, unsigned char len, const char *data);
	void onRx();
	void attach(unsigned int canId, Callback<void(CANMessage&)>);
};

extern SaabCan saabCan;

#endif /* SAABCAN_H_ */
