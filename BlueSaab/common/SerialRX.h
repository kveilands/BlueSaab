/*
 * C++ class for handling per line async read from mbed Serial.
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

#ifndef SERIALRX_H_
#define SERIALRX_H_

#include <mbed.h>
#include <rtos.h>

const int RX_BUF_SIZE = 80;

struct RXEntry {
	char buf[RX_BUF_SIZE];
};

class SerialRX {
	Mail<RXEntry, 4> rx_mail_box;
	RXEntry *curRXEntry;
	Serial &serial;

public:
	SerialRX(Serial &serial): curRXEntry(NULL), serial(serial) {}

	void initialize();
	// If waitForRXLine() returns a non-NULL entry, call free() when done with it to give it back.
	// timeout in milliseconds
	RXEntry* waitForRXLine(uint32_t timeout);
	void free(RXEntry *entry);
	void onSerialRX(int p);
	void clearRXMail();
};

#endif /* SERIALRX_H_ */
