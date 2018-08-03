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
