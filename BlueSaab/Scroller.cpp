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

#include "Scroller.h"
#include "utf_convert.h"

Scroller scroller;

Scroller::Scroller(): info_lock(1) {
	position = 0;
	title[0] = 0;
	title[sizeof(title) - 1] = 0;
	text[0] = 0;
	text[sizeof(text) - 1] = 0;
	text_len = 0;
}

void Scroller::clear() {
	info_lock.wait();
	title[0] = 0;
	text[0] = 0;
	position = 0;
	text_len = 0;
	info_lock.release();
}

void Scroller::set_info(const char *artist_,const char *title_) {
	info_lock.wait();
	utf_convert(artist_, text, ARTIST_BUF_SIZE);
	utf_convert(title_, title, sizeof(title));
	if (text[0] && title[0]) {
		// We have the artist and the title, so add a dash between them
		strcat(text, " - ");
	}
	strcat(text, title);
	text_len = strlen(text);
	if (text_len > 12) {
		// It doesn't fit, we'll need to scroll, so add a dash at the end
		strcat(text, " - ");
		text_len += 3;
	}

	position = 0;
	info_lock.release();
}

const char* Scroller::get() {
	buffer.clear();

	info_lock.wait();

	buffer.cut(position);
	bool got_all = buffer.add(text);
	if (position == 0 && got_all) { // all fits without scrolling, nothing more to do
		info_lock.release();
		return buffer.buffer;
	}

	if (got_all)
		buffer.add(text);

	position++;
	if (position >= text_len) {
		position -= text_len;
	}
	info_lock.release();
	return buffer.buffer;
}

/*
#include <assert.h>

void test_scroller() {
	Scroller s;

	s.set_info("", "");
	assert(strcmp(s.get(), "") == 0);
	assert(strcmp(s.get(), "") == 0);

	// just artist - short
	s.set_info("a1234", "");
	assert(strcmp(s.get(), "a1234") == 0);
	assert(strcmp(s.get(), "a1234") == 0);

	// just artist - long
	s.set_info("a123456789ABC", "");
	assert(strcmp(s.get(), "a123456789AB") == 0);
	assert(strcmp(s.get(), "123456789ABC") == 0);
	assert(strcmp(s.get(), "23456789ABC ") == 0);
	assert(strcmp(s.get(), "3456789ABC -") == 0);
	assert(strcmp(s.get(), "456789ABC - ") == 0);
	assert(strcmp(s.get(), "56789ABC - a") == 0);

	// just title - short
	s.set_info("", "t1234");
	assert(strcmp(s.get(), "t1234") == 0);
	assert(strcmp(s.get(), "t1234") == 0);

	// just title - long
	s.set_info("", "t123456789ABC");
	assert(strcmp(s.get(), "t123456789AB") == 0);
	assert(strcmp(s.get(), "123456789ABC") == 0);
	assert(strcmp(s.get(), "23456789ABC ") == 0);
	assert(strcmp(s.get(), "3456789ABC -") == 0);
	assert(strcmp(s.get(), "456789ABC - ") == 0);
	assert(strcmp(s.get(), "56789ABC - t") == 0);

	s.set_info("a1234", "t1234");
	assert(strcmp(s.get(), "a1234 - t123") == 0);
	assert(strcmp(s.get(), "1234 - t1234") == 0);
	assert(strcmp(s.get(), "234 - t1234 ") == 0);
	assert(strcmp(s.get(), "34 - t1234 -") == 0);
	assert(strcmp(s.get(), "4 - t1234 - ") == 0);
	assert(strcmp(s.get(), " - t1234 - a") == 0);
	assert(strcmp(s.get(), "- t1234 - a1") == 0);
	assert(strcmp(s.get(), " t1234 - a12") == 0);
	assert(strcmp(s.get(), "t1234 - a123") == 0);
	assert(strcmp(s.get(), "1234 - a1234") == 0);
	assert(strcmp(s.get(), "234 - a1234 ") == 0);
	assert(strcmp(s.get(), "34 - a1234 -") == 0);
	assert(strcmp(s.get(), "4 - a1234 - ") == 0);
	assert(strcmp(s.get(), " - a1234 - t") == 0);
	assert(strcmp(s.get(), "- a1234 - t1") == 0);
	assert(strcmp(s.get(), " a1234 - t12") == 0);
	assert(strcmp(s.get(), "a1234 - t123") == 0);
	assert(strcmp(s.get(), "1234 - t1234") == 0);
	assert(strcmp(s.get(), "234 - t1234 ") == 0);
	assert(strcmp(s.get(), "34 - t1234 -") == 0);
	assert(strcmp(s.get(), "4 - t1234 - ") == 0);
	assert(strcmp(s.get(), " - t1234 - a") == 0);
	assert(strcmp(s.get(), "- t1234 - a1") == 0);

	// If it fits - don't scroll
	s.set_info("a123", "t1234");
	assert(strcmp(s.get(), "a123 - t1234") == 0);
	assert(strcmp(s.get(), "a123 - t1234") == 0);

	s.set_info("a1234", "t12345");
	assert(strncmp(s.get(), "a1234 - t123", 12) == 0);
	assert(strncmp(s.get(), "1234 - t1234", 12) == 0);
	assert(strncmp(s.get(), "234 - t12345", 12) == 0);
	assert(strncmp(s.get(), "34 - t12345 ", 12) == 0);

	s.set_info("a1", "t123456789ABCD");
	assert(strncmp(s.get(), "a1 - t123456", 12) == 0);
	assert(strncmp(s.get(), "1 - t1234567", 12) == 0);
	assert(strncmp(s.get(), " - t12345678", 12) == 0);
	assert(strncmp(s.get(), "- t123456789", 12) == 0);
	s.get(); s.get(); s.get();
	assert(strncmp(s.get(), "23456789ABCD", 12) == 0);
	assert(strncmp(s.get(), "3456789ABCD ", 12) == 0);
	assert(strncmp(s.get(), "456789ABCD -", 12) == 0);
	assert(strncmp(s.get(), "56789ABCD - ", 12) == 0);
	assert(strncmp(s.get(), "6789ABCD - a", 12) == 0);

	s.set_info("a123456789ABC", "t1");
	assert(strncmp(s.get(), "a123456789AB", 12) == 0);
	s.get(); s.get(); s.get(); s.get(); s.get();
	assert(strncmp(s.get(), "6789ABC - t1", 12) == 0);
	assert(strncmp(s.get(), "789ABC - t1 ", 12) == 0);
	assert(strncmp(s.get(), "89ABC - t1 -", 12) == 0);
	assert(strncmp(s.get(), "9ABC - t1 - ", 12) == 0);
	assert(strncmp(s.get(), "ABC - t1 - a", 12) == 0);
}
*/
