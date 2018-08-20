/* C++ class for scrolling retrieved track metadata on SAAB SID (SAAB Information Display)
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

#ifndef SAAB_CDC_SCROLLER_H_
#define SAAB_CDC_SCROLLER_H_

#include <string.h>
#include <rtos.h>

#define WRITE_BUF_SIZE  12
#define TITLE_BUF_SIZE  64
#define ARTIST_BUF_SIZE 64

class StringBuffer {
	int position;
	int to_cut;
public:
	char buffer[WRITE_BUF_SIZE + 1];

	StringBuffer() {
		clear();
	}
	void clear() {
		position = 0;
		buffer[0] = 0;
		to_cut = 0;
	}
	void cut(int n) {
		to_cut = n;
	}
	bool add(const char *s) {
		int s_len = strlen(s);
		if (to_cut >= s_len) {
			to_cut -= s_len;
			return true;
		}
		int start_pos = to_cut;
		to_cut = 0;
		int to_add = s_len - start_pos;
		bool got_all_string = true;
		if (to_add > WRITE_BUF_SIZE - position) {
			to_add = WRITE_BUF_SIZE - position;
			got_all_string = false;
		}
		memcpy(buffer + position, s + start_pos, to_add);
		position += to_add;
		buffer[position] = 0;
		return got_all_string;
	}
};

class Scroller {
	StringBuffer buffer;

	// These are set from the Bluetooth thread, and used from CAN ISR. So they need protection.
	char title[TITLE_BUF_SIZE];
	char text[ARTIST_BUF_SIZE + 3 + TITLE_BUF_SIZE + 3 + 1];
	int position;
	int text_len;
	Semaphore info_lock; // mbed docs say a mutex cannot be used in ISRs, so we'll use a semaphore

public:
	Scroller();
	void clear();
	void set_info(const char *artist,const char *title);
	const char* get();
};

extern Scroller scroller;

#endif /* SAAB_CDC_SCROLLER_H_ */
