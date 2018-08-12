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
 *
 * https://github.com/timotto/RN52lib by Tim Otto used as the starting point.
 */

#ifndef RN52_H
#define RN52_H

#include <mbed.h>
#include <rtos.h>
#include "SerialRX.h"

class RN52 {
	Timeout timeout;
	Thread thread;
	Queue<const char, 20> rtosQueue;

	Serial serial;
	SerialRX serialRX;

	bool a2dpConnected;
	DigitalOut bt_cmd_pin;
	DigitalOut bt_pwren_pin;
	InterruptIn bt_event_pin;

	int queueCommand(const char *cmd);
	void onA2DPProfileChange(bool connected);
	void onGPIO2();
	void run();
	void processCommand(const char *cmd);
	bool parseQResponse(const char data[4]);

public:
	enum AVCRP {PLAYPAUSE, NEXT, PREV, VASSISTANT, VOLUP, VOLDOWN};

	RN52()
		: thread(osPriorityNormal, 512)
		, serial(PA_9, PA_10, 115200) // UART1 Tx/Rx
		, serialRX(serial)
		, a2dpConnected(false)
		, bt_cmd_pin(PA_7)
		, bt_pwren_pin(PC_8)
		, bt_event_pin(PB_0)
	{}
	void initialize();
	void sendAVCRP(AVCRP cmd);

	void reconnectLast();
	void disconnect();
	void getDetails();
	void getTrackData();
	void visible(bool visible);
	void reboot();
};

#endif
