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

#ifndef RN52impl_H
#define RN52impl_H

#include "mbed.h"
#include "RN52driver.h"

const int RX_BUF_SIZE = 80;

struct RXEntry {
	char buf[RX_BUF_SIZE];
};

class RN52impl: public RN52::RN52driver {
	Mail<RXEntry, 4> rx_mail_box;
	RXEntry *curRXEntry;
	Timeout timeout;

	void onProfileChange(BtProfile profile, bool connected);

	Serial serial;

	bool playing;
	bool bt_iap;
	bool bt_spp;
	bool bt_a2dp;
	bool bt_hfp;

	DigitalOut bt_cmd_pin;
	DigitalOut bt_pwren_pin;
	InterruptIn bt_event_pin;

	Thread thread;

	public:

		RN52impl() :
			curRXEntry(NULL), serial(PA_9, PA_10, 115200) // UART1 Tx/Rx
			, bt_cmd_pin(PA_7)
			, bt_pwren_pin(PC_8)
			, bt_event_pin(PB_0)
			, thread(osPriorityNormal, 512) {
			playing = true;
			bt_iap = false;
			bt_spp = false;
			bt_a2dp = false;
			bt_hfp = false;
		}

		void toUART(const char* c);
		void onError(int location, Error error);
		void onGPIO2();
		void initialize();
		void run();

	private:
		RXEntry* waitForRXLine(uint32_t timeout);
		void onSerialRX(int p);
		void clearRXMail();
};

#endif
