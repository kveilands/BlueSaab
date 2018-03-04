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

void Bluetooth::set_maxvol() {
    impl.set_max_volume();
}

void Bluetooth::reboot() {
    impl.reboot();
}

void Bluetooth::get_details() {
	impl.get_details();
}

void Bluetooth::handleDebugChar(char c) {
	switch (c) {
		case 'V':
			visible();
			getLog()->log("Going into Discoverable Mode\r\n");
			break;
		case 'I':
			invisible();
			getLog()->log("Going into non-Discoverable/Connectable Mode\r\n");
			break;
		case 'C':
			reconnect();
			getLog()->log("Re-connecting to the Last Known Device\r\n");
			break;
		case 'D':
			disconnect();
			getLog()->log("Disconnecting from the Current Device\r\n");
			break;
		case 'P':
			play();
			getLog()->log("\"Play/Pause\" Current Track\r\n");
			break;
		case 'N':
			next();
			getLog()->log("Skip to \"Next\" Track\r\n");
			break;
		case 'R':
			prev();
			getLog()->log("Go back to \"Previous\" Track\r\n");
			break;
		case 'A':
			vassistant();
			getLog()->log("Invoking Voice Assistant\r\n");
			break;
		case 'B':
			reboot();
			getLog()->log("Rebooting the RN52\r\n");
			break;
		case 'd':
			get_details();
			getLog()->log("Getting details\r\n");
			break;
		case 'H':
		case 'h':
		//default:
			getLog()->log("Commands:\r\n"
				"V - Go into Discoverable Mode\r\n"
				"I - Go into non-Discoverable but Connectable Mode\r\n"
				"C - Reconnect to Last Known Device\r\n"
				"D - Disconnect from Current Device\r\n"
				"P - Play/Pause Current Track\r\n"
				"N - Skip to Next Track\r\n"
				"R - Previous Track/Beginning of Track\r\n"
				"A - Invoke Voice Assistant\r\n"
				"B - Reboot the RN52 module\r\n"
				"H - Show this list of commands\r\n"
				"d - get RN52 details\r\n");
			break;
//		case ' ':
//		case '\t':
//		case '\r':
//		case '\n':
//			break; // just discard whitespace
	}
}
