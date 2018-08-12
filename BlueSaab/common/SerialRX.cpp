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

#include "SerialRX.h"

void SerialRX::initialize() {
	curRXEntry = rx_mail_box.alloc();
	serial.read((uint8_t*) curRXEntry->buf, sizeof(curRXEntry->buf), callback(this, &SerialRX::onSerialRX), SERIAL_EVENT_RX_ALL, '\n');
}

RXEntry* SerialRX::waitForRXLine(uint32_t timeout) {

	osEvent evt = rx_mail_box.get(timeout);
	if (evt.status == osEventMail) {
		RXEntry *e = (RXEntry*) evt.value.p;
		char *curBuf = e->buf;
		curBuf[RX_BUF_SIZE - 1] = 0; // Zero terminate the whole buffer to be safe
		char *pos = strchr(curBuf, '\n');
		if (pos) {
			*(pos + 1) = 0; // Zero terminate received data
		}
		return e;
	}
	return NULL;
}

void SerialRX::free(RXEntry *entry) {
	rx_mail_box.free(entry);
}

void SerialRX::onSerialRX(int p) {
	if (p & (SERIAL_EVENT_RX_CHARACTER_MATCH | SERIAL_EVENT_RX_COMPLETE)) {
		RXEntry *newEntry = rx_mail_box.alloc();
		if (newEntry == NULL) {
//			getLog()->log("NO MAIL TO ALLOC, drop and reuse\r\n");
		} else {
			rx_mail_box.put(curRXEntry);
			curRXEntry = newEntry;
		}
	}

	serial.read((uint8_t*) curRXEntry->buf, sizeof(curRXEntry->buf), callback(this, &SerialRX::onSerialRX), SERIAL_EVENT_RX_ALL, '\n');
}

void SerialRX::clearRXMail() {
	int n = 0;
	for (;;) {
		osEvent evt = rx_mail_box.get(0);
		if (evt.status == osEventMail) {
			RXEntry *e = (RXEntry*) evt.value.p;
			rx_mail_box.free(e);
			n++;
		} else {
			break;
		}
	}
//	if (n > 0)
//		getLog()->log("%d mails cleared\r\n", n);
}
