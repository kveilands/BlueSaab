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
 */

#include "RN52impl.h"
#include "RN52strings.h"
#include <string.h>

#include "SerialLog.h"
#include "Scroller.h"

void RN52impl::toUART(const char* c){serial.puts(c);};

void RN52impl::onError(int location, Error error){};

void RN52impl::onGPIO2() {queueCommand(RN52_CMD_QUERY);}

void RN52impl::onProfileChange(BtProfile profile, bool connected) {
	switch (profile) {
	case A2DP:
		bt_a2dp = connected;
		if (connected && playing) {sendAVCRP(RN52::RN52driver::PLAYPAUSE);}
		break;
	case SPP:
		bt_spp = connected;
		break;
	case IAP:
		bt_iap = connected;
		break;
	case HFP:
		bt_hfp = connected;
		break;
	}
}

void RN52impl::initialize() {
	curRXEntry = rx_mail_box.alloc();
	serial.read((uint8_t*) curRXEntry->buf, sizeof(curRXEntry->buf), callback(this, &RN52impl::onSerialRX), SERIAL_EVENT_RX_ALL, '\n');

	thread.start(callback(this, &RN52impl::run));
//	getLog()->registerThread("RN52impl::run", &thread);

	bt_pwren_pin = 1;
	bt_cmd_pin = 1;

	getLog()->log("Configuring RN52...\r\n");
	setDiscoveryMask();
	setConnectionMask();
	setCod();
	setDeviceName();
	setExtendedFeatures();
	setMaxVolume();
	setPairTimeout();
	reboot();
	Thread::wait(5000);
	getLog()->log("RN52 configuration completed!\r\n");

	bt_event_pin.fall(callback(this, &RN52impl::onGPIO2));
}

RXEntry* RN52impl::waitForRXLine(uint32_t timeout) {

	osEvent evt = rx_mail_box.get(timeout);
	if (evt.status == osEventMail) {
		RXEntry *e = (RXEntry*) evt.value.p;
		char *curBuf = e->buf;
		curBuf[RX_BUF_SIZE - 1] = 0; // Zero terminate the whole buffer to be safe
		char *pos = strchr(curBuf, '\n');
		if (pos) {
			*(pos + 1) = 0; // Zero terminate received data
		}
		return e;
	}
	return NULL;
}

void RN52impl::onSerialRX(int p) {
	if (p & (SERIAL_EVENT_RX_CHARACTER_MATCH | SERIAL_EVENT_RX_COMPLETE)) {
		RXEntry *newEntry = rx_mail_box.alloc();
		if (newEntry == NULL) {
//			getLog()->log("NO MAIL TO ALLOC, drop and reuse\r\n");
		} else {
			rx_mail_box.put(curRXEntry);
			curRXEntry = newEntry;
		}
	}

	serial.read((uint8_t*) curRXEntry->buf, sizeof(curRXEntry->buf), callback(this, &RN52impl::onSerialRX), SERIAL_EVENT_RX_ALL, '\n');
}

void RN52impl::clearRXMail() {
	int n = 0;
	for (;;) {
		osEvent evt = rx_mail_box.get(0);
		if (evt.status == osEventMail) {
			RXEntry *e = (RXEntry*) evt.value.p;
			rx_mail_box.free(e);
			n++;
		} else {
			break;
		}
	}
//	if (n > 0)
//		getLog()->log("%d mails cleared\r\n", n);
}

static bool isCmd(const char *buffer, const char *cmd) {
	return strncmp(buffer, cmd, strlen(cmd)) == 0;
}

char title[72];
char artist[72];

void copy_text(char *to, const char *from, int max_len) {
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

void RN52impl::run() {
//	getLog()->log("RN52impl::run() started\r\n");

	osEvent evt;
	evt.status = osOK;
	for (;;) {
		if (evt.status != osEventMessage) { // osEventMessage would mean there is a command we haven't sent yet
			evt = rtosQueue.get();
		}
//		getLog()->log("Entering cmd mode\r\n");
		clearRXMail();
		bt_cmd_pin.write(0);
		RXEntry* gotBuf = waitForRXLine(500); // get command
		if (gotBuf) {
//			getLog()->logShortString(gotBuf->buf);
			bool gotCMD = isCmd(gotBuf->buf, RN52_CMD_BEGIN);
			rx_mail_box.free(gotBuf);

			if (gotCMD) {
				for (;;) {
					if (evt.status == osEventMessage) {
						const char *cmd = (const char*) evt.value.p;
//						getLog()->log("send: %s", (int) cmd);
						toUART(cmd);
						evt.status = osOK;

						if (isCmd(cmd, RN52_CMD_GET_TRACK_DATA)) { // Gather track info until timeout
							int lines = 0;
							title[0] = 0;
							artist[0] = 0;
							while (true) {
								gotBuf = waitForRXLine(100);
								if (gotBuf) {
									lines++;
									// The strings after Title= and Artist= seem to be limited to 60 chars,
									// plus \r\r\n. Which makes it max 70 chars total.
									if (isCmd(gotBuf->buf, "Title=")) {
										copy_text(title, (gotBuf->buf) + 6,
												sizeof(title));
//										getLog()->log("t=%s\r\n", (int) title);
									} else if (isCmd(gotBuf->buf, "Artist=")) {
										copy_text(artist, (gotBuf->buf) + 7,
												sizeof(artist));
//										getLog()->log("a=%s\r\n", (int) artist);
									} else {
//										getLog()->logShortString(gotBuf->buf);
									}
									rx_mail_box.free(gotBuf);
								} else {
//									getLog()->log("track info response %d lines\r\n", lines);
									break;
								}
							}
							scroller.set_info(artist, title);
						} else if (isCmd(cmd, RN52_CMD_DETAILS)) { // Gather details until timeout
							int lines = 0;
							while (true) {
								gotBuf = waitForRXLine(100);
								if (gotBuf) {
									lines++;
									if (isCmd(gotBuf->buf, "BTA=")) {
										strncpy(title, gotBuf->buf,
												sizeof(title)); // May not zero terminate
										title[sizeof(title) - 1] = 0;
//										getLog()->log(title);
									}
									rx_mail_box.free(gotBuf);
								} else {
//									getLog()->log("details response %d lines\r\n", lines);
									break;
								}
							}
						} else if (isCmd(cmd, RN52_CMD_QUERY)) {
							gotBuf = waitForRXLine(500);
							if (gotBuf) {
//								getLog()->logShortString(gotBuf->buf);
								if (strlen((char *) gotBuf->buf) != 6
										|| !parseQResponse(
												(char *) gotBuf->buf)) {
									queueCommand(RN52_CMD_QUERY);
								}
								rx_mail_box.free(gotBuf);
							}
						} else {
							gotBuf = waitForRXLine(500);
							if (gotBuf) {
//								getLog()->logShortString(gotBuf->buf);
								rx_mail_box.free(gotBuf);
							}
						}
					}
					evt = rtosQueue.get(500);
					if (evt.status == osEventTimeout) {
						break;
					}
				}
			}
		}
//		getLog()->log("Exiting cmd mode\r\n");
		bt_cmd_pin.write(1);
		gotBuf = waitForRXLine(500); // Get END or timeout
		if (gotBuf) {
//			getLog()->logShortString(gotBuf->buf);
			rx_mail_box.free(gotBuf);
		}
	}

}
