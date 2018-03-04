#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "RN52impl.h"

class Bluetooth {
	RN52impl impl;

public:
	Bluetooth();

	void initialize();

	void play();
	void prev();
	void next();
	void vassistant();
	void volup();
	void voldown();
	void visible();
	void invisible();
	void reconnect();
	void disconnect();
	void set_maxvol();
	void reboot();
	void get_details();

	void handleDebugChar(char c);
};

extern Bluetooth bluetooth;

#endif /* BLUETOOTH_H_ */
