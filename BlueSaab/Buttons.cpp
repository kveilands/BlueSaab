#include "Buttons.h"
#include "Bluetooth.h"

Buttons buttons;

Buttons::Buttons() {
}

void Buttons::initialize() {
	saabCan.attach(CDC_CONTROL, callback(this, &Buttons::onFrame));
}

Buttons::Button decode(unsigned char data1, unsigned char data2) {
	switch (data1) {
//	case 0x24:
//		return Buttons::CDC_MODE_ON;
//	case 0x14:
//		return Buttons::CDC_MODE_ON;
	case 0x59: // NXT
		return Buttons::NXT;
	case 0x84: // SEEK button (middle) long press on IHU
		return Buttons::SEEK_LONG;
	case 0x88: // > 2 second long press of SEEK button (middle) on IHU
		return Buttons::SEEK_EXTRA_LONG;
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

	Buttons::Button button = decode(frame.data[1], frame.data[2]);
	if (button != Buttons::NONE) {
//		callBack.call(button);
		getLog()->log("Buttons::onFrame button %d", button);
		if (button == Buttons::NXT) {
			bluetooth.play();
		}
		if (button == Buttons::TRACK_PLUS) {
			bluetooth.next();
		}
		if (button == Buttons::TRACK_MINUS) {
			bluetooth.prev();
		}
	} else {
		getLog()->log("Buttons::onFrame button unknown");
	}
}
