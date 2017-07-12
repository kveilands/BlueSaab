/*
 * C++ Class for scrolling track data
 * Copyright (C) 2017 Girts Linde
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Scroller.h"

Scroller scroller;

Scroller::Scroller() {
	position = 0;
	update_completed = false;
	artist[0] = 0;
	artist[ARTIST_BUF_SIZE - 1] = 0;
	title[0] = 0;
	title[TITLE_BUF_SIZE - 1] = 0;
}

void Scroller::start_update() {
	update_completed = false;
	artist[0] = 0;
	title[0] = 0;
}
void Scroller::complete_update() {
	update_completed = true;
	position = 0;
}
void Scroller::set_artist(const char *s) {
	strncpy(artist, s, ARTIST_BUF_SIZE - 1);
}
void Scroller::set_title(const char *s) {
	strncpy(title, s, TITLE_BUF_SIZE - 1);
}

const char* Scroller::get() {
	if (!update_completed) {
		buffer.clear();
		return buffer.buffer;
	}

	buffer.clear();
	buffer.cut(position);
	bool got_all_artist = buffer.add(artist);
	if (artist[0] != 0 && title[0] != 0) {
		buffer.add(" - ");
	}
	bool got_all_title = buffer.add(title);
	bool got_all = ((title[0] != 0 && got_all_title)
			|| (title[0] == 0 && got_all_artist));

	if (got_all) {
		position = 0;
	} else {
		position++;
	}

	return buffer.buffer;
}
