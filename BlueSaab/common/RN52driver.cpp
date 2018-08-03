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

#include <ctype.h>
#include <string.h>
#include "RN52driver.h"
#include "RN52strings.h"
#include "SerialLog.h"

namespace RN52 {

static int getVal(char c);

RN52driver::RN52driver():
		state(0), profile(0), a2dpConnected(false), sppConnected(false), streamingAudio(false) {
}

int RN52driver::queueCommand(const char *cmd) {
//	getLog()->log("queue: %s\r\n", (int) cmd);
	rtosQueue.put(cmd);
	return 0;
}

bool RN52driver::parseQResponse(const char data[4]) {
	for (int i = 0; i < 4; i++) {
		if (!isxdigit(data[i]))
			return false;
	}

	int profile = (getVal(data[0]) << 4 | getVal(data[1])) & 0x0f;
	int state = (getVal(data[2]) << 4 | getVal(data[3])) & 0x0f;

	bool lastSppConnected = sppConnected;
	bool lastA2dpConnected = a2dpConnected;

	sppConnected = profile & 0x02;
	a2dpConnected = profile & 0x04;

	bool trackChanged = getVal(data[0]) & 0x02;

	bool changed = (this->state != state) || (this->profile != profile);

	bool streamingChanged = (this->state == 13 && state != 13) || (this->state != 3 && state == 13);
	this->state = state;
	this->profile = profile;

	if (changed) onStateChange(state, profile);
	if (streamingChanged) onStreaming(state == 13);
	if (lastSppConnected != sppConnected) onProfileChange(SPP, sppConnected);
	if (lastA2dpConnected != a2dpConnected) onProfileChange(A2DP, a2dpConnected);
	if (trackChanged) getTrackData();
	return true;
}

int RN52driver::sendAVCRP(AVCRP cmd) {
	if (!a2dpConnected) {onError(6, NOTCONNECTED);
		return -2;
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
	default:
		break;
	}
	return 0;
}

void RN52driver::reconnectLast() {
	queueCommand(RN52_CMD_RECONNECTLAST);
}

void RN52driver::disconnect() {
	queueCommand(RN52_CMD_DISCONNECT);
}

void RN52driver::visible(bool visible) {
	if (visible) {
		queueCommand(RN52_CMD_DISCOVERY_ON);
	} else {
		queueCommand(RN52_CMD_DISCOVERY_OFF);
	}
}

void RN52driver::setDiscoveryMask() {
	queueCommand(RN52_SET_DISCOVERY_MASK);
}

void RN52driver::setConnectionMask() {
	queueCommand(RN52_SET_CONNECTION_MASK);
}

void RN52driver::setCod() {
	queueCommand(RN52_SET_COD);
}

void RN52driver::setDeviceName() {
	queueCommand(RN52_SET_DEVICE_NAME);
}

void RN52driver::setBaudrate() {
	queueCommand(RN52_SET_BAUDRATE_9600);
}

void RN52driver::setMaxVolume() {
	queueCommand(RN52_SET_MAXVOL);
}

void RN52driver::setExtendedFeatures() {
	queueCommand(RN52_SET_EXTENDED_FEATURES);
}

void RN52driver::setPairTimeout() {
	queueCommand(RN52_SET_PAIR_TIMEOUT);
}

void RN52driver::reboot() {
	queueCommand(RN52_CMD_REBOOT);
}

void RN52driver::getDetails() {
	queueCommand(RN52_CMD_DETAILS);
}

void RN52driver::getTrackData() {
	queueCommand(RN52_CMD_GET_TRACK_DATA);
}

void RN52driver::refreshState() {
	queueCommand(RN52_CMD_QUERY);
}

static int getVal(char c) {
	if (c >= '0' && c <= '9')
		return (c - '0');
	else
		return (c - 'A' + 10);
}

} /* namespace RN52 */
