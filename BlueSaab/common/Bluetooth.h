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
	void setMaxVol();
	void reboot();
	void getDetails();

	void handleDebugChar(char c);
};

extern Bluetooth bluetooth;

#endif /* BLUETOOTH_H_ */
