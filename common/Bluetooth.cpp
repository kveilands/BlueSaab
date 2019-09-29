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

#include "Bluetooth.h"
#include "SerialLog.h"

Bluetooth bluetooth;

void Bluetooth::initialize() {
	rn52.initialize();
	getLog()->attachRXCallback(callback(this, &Bluetooth::handleDebugChar));
}

void Bluetooth::play() {
	rn52.sendAVCRP(RN52::PLAYPAUSE);
}

void Bluetooth::prev() {
	rn52.sendAVCRP(RN52::PREV);
}

void Bluetooth::next() {
	rn52.sendAVCRP(RN52::NEXT);
}

void Bluetooth::vassistant() {
	rn52.sendAVCRP(RN52::VASSISTANT);
}

void Bluetooth::discoverable() {
	rn52.discoverable(true);
}

void Bluetooth::connectable() {
	rn52.discoverable(false);
}

void Bluetooth::reconnect() {
	rn52.reconnectLast();
}

void Bluetooth::disconnect() {
	rn52.disconnect();
}

void Bluetooth::resetPdl() {
	rn52.resetPdl();
}

void Bluetooth::reboot() {
	rn52.reboot();
}

void Bluetooth::getDetails() {
	rn52.getDetails();
}

void Bluetooth::handleDebugChar(char c) {
	switch (c) {
		case 'V':
			discoverable();
			getLog()->log("Switching to discoverable mode\r\n");
			break;
		case 'I':
			connectable();
			getLog()->log("Switching to connectable/non-discoverable mode\r\n");
			break;
		case 'C':
			reconnect();
			getLog()->log("Re-connecting to the last known device\r\n");
			break;
		case 'D':
			disconnect();
			getLog()->log("Disconnecting from the current device\r\n");
			break;
		case 'P':
			play();
			getLog()->log("\"Play/Pause\" current track\r\n");
			break;
		case 'N':
			next();
			getLog()->log("Skip to \"Next\" track\r\n");
			break;
		case 'R':
			prev();
			getLog()->log("Go back to \"Previous\" track\r\n");
			break;
		case 'A':
			vassistant();
			getLog()->log("Invoking voice assistant\r\n");
			break;
		case 'B':
			reboot();
			getLog()->log("Rebooting RN52\r\n");
			break;
		case 'd':
			getDetails();
			getLog()->log("Getting details\r\n");
			break;
		case 'u':
			resetPdl();
			getLog()->log("Resetting PDL\r\n");
			break;
		case 'H':
			getLog()->log("Commands:\r\n"
				"V - Switch to discoverable mode\r\n"
				"I - Switch to non-discoverable/connectable mode\r\n"
				"C - Reconnect to last known device\r\n"
				"D - Disconnect from current device\r\n"
				"P - Play/Pause current track\r\n"
				"N - Skip to next track\r\n"
				"R - Previous track/beginning of track\r\n"
				"A - Invoke voice assistant\r\n"
				"B - Reboot the RN52 module\r\n"
				"H - Show this list of commands\r\n"
				"d - Get RN52 details\r\n"
				"u - Reset PDL (Paired Devices List)\r\n");
			break;
		default:
			break;

	}
}
