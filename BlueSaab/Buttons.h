#ifndef BUTTONS_H_
#define BUTTONS_H_
#include "SaabCan.h"

class Buttons {
public:
	enum Button {
		NONE,
		IHU1, IHU2, IHU3, IHU4, IHU5, IHU6,
		NXT,
		SEEK_LONG,
		SEEK_EXTRA_LONG,
		RANDOM,
		PAUSE_ON,
		PAUSE_OFF,
		TRACK_PLUS,
		TRACK_MINUS,
		CDC_MODE_ON,
		CDC_MODE_OFF
	};

private:
	Callback<void(Buttons::Button)> callBack;

public:
	Buttons();
	void initialize();
	void onFrame(CANMessage& frame);
	void attach(Callback<void(Buttons::Button)> cb) {
		callBack = cb;
	}
};

extern Buttons buttons;

#endif /* BUTTONS_H_ */
