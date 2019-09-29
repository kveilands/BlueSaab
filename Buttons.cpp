/* C++ class for SAAB infotainment head unit and steering wheel button event handling
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

#include "Buttons.h"
#include "Bluetooth.h"
#include "SidResource.h"

Buttons buttons;

Buttons::Buttons() {
}

void Buttons::initialize() {
	saabCan.attach(CDC_CONTROL, callback(this, &Buttons::onFrame));
}

Buttons::Button decode(unsigned char data1, unsigned char data2) {
	switch (data1) {
	case 0x59: // NXT
		return Buttons::NXT;
	case 0x45: // SEEK+ button long press on IHU
		return Buttons::SEEK_PLUS_LONG;
	case 0x46: // SEEK- button long press on IHU
		return Buttons::SEEK_MINUS_LONG;
	case 0x84: // SEEK button (middle) long press on IHU
		return Buttons::SEEK_MIDDLE_LONG;
	case 0x88: // > 2 second long press of SEEK button (middle) on IHU
		return Buttons::SEEK_MIDDLE_EXTRA_LONG;
	case 0x76: // Random ON/OFF (Long press of CD/RDM button)
		return Buttons::RANDOM;
	case 0xB1: // Pause ON
		return Buttons::PAUSE_ON;
	case 0xB0: // Pause OFF
		return Buttons::PAUSE_OFF;
	case 0x35: // Track +
		return Buttons::TRACK_PLUS;
	case 0x36: // Track -
		return Buttons::TRACK_MINUS;
	case 0x68: // IHU buttons "1-6"
		switch (data2) {
		case 0x01:
			return Buttons::IHU1;
		case 0x02:
			return Buttons::IHU2;
		case 0x03:
			return Buttons::IHU3;
		case 0x04:
			return Buttons::IHU4;
		case 0x05:
			return Buttons::IHU5;
		case 0x06:
			return Buttons::IHU6;
		}
	}
	return Buttons::NONE;
}

void Buttons::onFrame(CANMessage& frame) {
	if (frame.data[0] != 0x80)
		return;

#if SID_TEXT_CONTROL_ENABLED
	sidResource.requestDriverBreakthrough();
#endif

	Buttons::Button button = decode(frame.data[1], frame.data[2]);
	if (button != Buttons::NONE) {
//		getLog()->log("Buttons::onFrame button %d", button);
		switch (button) {
		case Buttons::NXT:
			bluetooth.play();
			break;
		case Buttons::TRACK_PLUS:
			bluetooth.next();
			break;
		case Buttons::TRACK_MINUS:
			bluetooth.prev();
			break;
		case Buttons::IHU1:
			bluetooth.discoverable();
			break;
		case Buttons::IHU3:
			bluetooth.reconnect();
			break;
		case Buttons::IHU6:
			bluetooth.disconnect();
			break;
		default:
			break;
		}
	} else {
//		getLog()->log("Buttons::onFrame button unknown");
	}
}
