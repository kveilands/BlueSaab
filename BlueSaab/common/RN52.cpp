/* C++ class for handling communications with Microchip RN52 Bluetooth module
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
 *
 * https://github.com/timotto/RN52lib by Tim Otto used as the starting point.
 */

#include "RN52.h"
#include "RN52strings.h"
#include <ctype.h>

#include "SerialLog.h"
#include "Scroller.h"

static int getVal(char c) {
	if (c >= '0' && c <= '9')
		return (c - '0');
	else
		return (c - 'A' + 10);
}

void RN52::onGPIO2() {
	queueCommand(RN52_CMD_QUERY);
}

void RN52::onA2DPProfileChange(bool connected) {
	if (connected) {
		sendAVCRP(RN52::PLAYPAUSE);
		// Ask for track data a bit later, to give the phone some time to start playing
		timeout.attach(callback(this, &RN52::getTrackData), 3.0);
	}
}

void RN52::initialize() {
	serialRX.initialize();

	thread.start(callback(this, &RN52::run));
//	getLog()->registerThread("RN52::run", &thread);

	bt_pwren_pin = 1;
	bt_cmd_pin = 1;

	getLog()->log("Configuring RN52...\r\n");
	queueCommand(RN52_SET_DISCOVERY_MASK);
	queueCommand(RN52_SET_CONNECTION_MASK);
	queueCommand(RN52_SET_COD);
	queueCommand(RN52_SET_DEVICE_NAME);
	queueCommand(RN52_SET_EXTENDED_FEATURES);
	queueCommand(RN52_SET_MAXVOL);
	queueCommand(RN52_SET_IDLE_TIMEOUT);
	reboot();
	Thread::wait(5000);
	getLog()->log("RN52 configuration completed!\r\n");

	bt_event_pin.fall(callback(this, &RN52::onGPIO2));
}


int RN52::queueCommand(const char *cmd) {
//	getLog()->log("queue: %s\r\n", (int) cmd);
	rtosQueue.put(cmd);
	return 0;
}

static bool isCmd(const char *buffer, const char *cmd) {
	return strncmp(buffer, cmd, strlen(cmd)) == 0;
}

char title[72];
char artist[72];

static void copy_text(char *to, const char *from, int max_len) {
	to[max_len - 1] = 0;
	for (int i = 0; i < max_len - 1; ++i) {
		if (*from == 0 || *from == '\r' || *from == '\n') {
			*to = 0;
			return;
		}
		*to = *from;
		++from;
		++to;
	}
}

void RN52::processCommand(const char *cmd) {
	//getLog()->log("send: %s", (int) cmd);
	serial.puts(cmd);

	if (isCmd(cmd, RN52_CMD_GET_TRACK_DATA)) { // Gather track info until timeout
		int lines = 0;
		title[0] = 0;
		artist[0] = 0;
		while (true) {
			RXEntry* gotBuf = serialRX.waitForRXLine(100);
			if (gotBuf) {
				lines++;
				// The strings after Title= and Artist= seem to be limited to 60 chars,
				// plus \r\r\n. Which makes it max 70 chars total.
				if (isCmd(gotBuf->buf, "Title=")) {
					copy_text(title, (gotBuf->buf) + 6, sizeof(title));
					//getLog()->log("t=%s\r\n", (int) title);
				} else if (isCmd(gotBuf->buf, "Artist=")) {
					copy_text(artist, (gotBuf->buf) + 7, sizeof(artist));
					//getLog()->log("a=%s\r\n", (int) artist);
				} else {
					//getLog()->logShortString(gotBuf->buf);
				}
				serialRX.free(gotBuf);
			} else {
				//getLog()->log("track info response %d lines\r\n", lines);
				break;
			}
		}
		scroller.set_info(artist, title);
	} else if (isCmd(cmd, RN52_CMD_DETAILS)) { // Gather details until timeout
		int lines = 0;
		while (true) {
			RXEntry* gotBuf = serialRX.waitForRXLine(100);
			if (gotBuf) {
				lines++;
				if (isCmd(gotBuf->buf, "BTA=")) {
					strncpy(title, gotBuf->buf, sizeof(title)); // May not zero terminate
					title[sizeof(title) - 1] = 0;
					getLog()->log(title);
				}
				serialRX.free(gotBuf);
			} else {
				//getLog()->log("details response %d lines\r\n", lines);
				break;
			}
		}
	} else if (isCmd(cmd, RN52_CMD_QUERY)) {
		RXEntry* gotBuf = serialRX.waitForRXLine(500);
		if (gotBuf) {
			//getLog()->logShortString(gotBuf->buf);
			if (strlen((char *) gotBuf->buf) != 6 || !parseQResponse((char *) gotBuf->buf)) {
				// If the response is not in the format we expected, then ask again.
				queueCommand(RN52_CMD_QUERY);
			}
			serialRX.free(gotBuf);
		}
	} else {
		RXEntry* gotBuf = serialRX.waitForRXLine(500);
		if (gotBuf) {
			//getLog()->logShortString(gotBuf->buf);
			serialRX.free(gotBuf);
		}
	}
}

void RN52::run() {
//	getLog()->log("RN52::run() started\r\n");

	osEvent evt;
	evt.status = osOK;
	for (;;) {
		if (evt.status != osEventMessage) { // osEventMessage would mean there is a command we haven't sent yet
			evt = rtosQueue.get();
		}
//		getLog()->log("Entering cmd mode\r\n");
		serialRX.clearRXMail();
		bt_cmd_pin.write(0);
		RXEntry* gotBuf = serialRX.waitForRXLine(500); // get command
		if (gotBuf) {
//			getLog()->logShortString(gotBuf->buf);
			bool gotCMD = isCmd(gotBuf->buf, RN52_CMD_BEGIN);
			serialRX.free(gotBuf);

			if (gotCMD) {
				// Got "CMD" from RN52, so it is now in command mode. Let's send commands.
				for (;;) {
					if (evt.status == osEventMessage) {
						evt.status = osOK;
						const char *cmd = (const char*) evt.value.p;
						processCommand(cmd);
					}
					evt = rtosQueue.get(500);
					if (evt.status == osEventTimeout) {
						// No more commands in the queue for 500ms, let's leave the command mode.
						break;
					}
				}
			}
		}
//		getLog()->log("Exiting cmd mode\r\n");
		bt_cmd_pin.write(1);
		gotBuf = serialRX.waitForRXLine(500); // Get END or timeout
		if (gotBuf) {
//			getLog()->logShortString(gotBuf->buf);
			serialRX.free(gotBuf);
		}
	}
}

bool RN52::parseQResponse(const char data[4]) {
	for (int i = 0; i < 4; i++) {
		if (!isxdigit(data[i]))
			return false;
	}

	int profile = (getVal(data[0]) << 4 | getVal(data[1])) & 0x0f;
	//int state = (getVal(data[2]) << 4 | getVal(data[3])) & 0x0f;

	bool lastA2dpConnected = a2dpConnected;
	a2dpConnected = profile & 0x04;

	bool trackChanged = getVal(data[0]) & 0x02;

	if (lastA2dpConnected != a2dpConnected) onA2DPProfileChange(a2dpConnected);
	if (trackChanged) getTrackData();
	return true;
}


void RN52::sendAVCRP(AVCRP cmd) {
	if (!a2dpConnected) {
		return;
	}

	switch (cmd) {
	case PLAYPAUSE:
		queueCommand(RN52_CMD_AVCRP_PLAYPAUSE);
		break;
	case PREV:
		queueCommand(RN52_CMD_AVCRP_PREV);
		break;
	case NEXT:
		queueCommand(RN52_CMD_AVCRP_NEXT);
		break;
	case VASSISTANT:
		queueCommand(RN52_CMD_AVCRP_VASSISTANT);
		break;
	case VOLUP:
		queueCommand(RN52_CMD_VOLUP);
		break;
	case VOLDOWN:
		queueCommand(RN52_CMD_VOLDOWN);
		break;
	}
}

void RN52::reconnectLast() {
	queueCommand(RN52_CMD_RECONNECTLAST);
}

void RN52::disconnect() {
	queueCommand(RN52_CMD_DISCONNECT);
}

void RN52::resetPdl() {
	queueCommand(RN52_CMD_RESET_PDL);
}

void RN52::getDetails() {
	queueCommand(RN52_CMD_DETAILS);
}

void RN52::getTrackData() {
	queueCommand(RN52_CMD_GET_TRACK_DATA);
}

void RN52::visible(bool visible) {
	if (visible) {
		queueCommand(RN52_CMD_DISCOVERY_ON);
	} else {
		queueCommand(RN52_CMD_DISCOVERY_OFF);
	}
}

void RN52::reboot() {
	queueCommand(RN52_CMD_REBOOT);
}

