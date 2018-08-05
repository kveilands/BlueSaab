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

Bluetooth::Bluetooth() {
}

void Bluetooth::initialize() {
	impl.initialize();
	getLog()->attachRXCallback(callback(this, &Bluetooth::handleDebugChar));
}

void Bluetooth::play() {
	impl.sendAVCRP(RN52::RN52driver::PLAYPAUSE);
}

void Bluetooth::prev() {
	impl.sendAVCRP(RN52::RN52driver::PREV);
}

void Bluetooth::next() {
	impl.sendAVCRP(RN52::RN52driver::NEXT);
}

void Bluetooth::vassistant() {
	impl.sendAVCRP(RN52::RN52driver::VASSISTANT);
}

void Bluetooth::volup() {
	impl.sendAVCRP(RN52::RN52driver::VOLUP);
}

void Bluetooth::voldown() {
	impl.sendAVCRP(RN52::RN52driver::VOLDOWN);
}

void Bluetooth::visible() {
	impl.visible(true);
}

void Bluetooth::invisible() {
	impl.visible(false);
}

void Bluetooth::reconnect() {
	impl.reconnectLast();
}

void Bluetooth::disconnect() {
	impl.disconnect();
}

void Bluetooth::setMaxVol() {
	impl.setMaxVolume();
}

void Bluetooth::reboot() {
	impl.reboot();
}

void Bluetooth::getDetails() {
	impl.getDetails();
}

void Bluetooth::handleDebugChar(char c) {
	switch (c) {
		case 'V':
			visible();
			getLog()->log("Switching to discoverable mode\r\n");
			break;
		case 'I':
			invisible();
			getLog()->log("Switching to non-discoverable/connectable mode\r\n");
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
				"d - Get RN52 details\r\n");
			break;
		default:
			break;

	}
}
