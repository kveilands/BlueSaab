/* C++ class for STM32F103RB microcontroller logging
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

#ifndef SERIALLOG_H_
#define SERIALLOG_H_

#include <mbed.h>
#include <rtos.h>
#include <vector>

enum LogEntryType {
	leCANMessage = 0,
	leShortString = 1
};

struct LogEntry {
	uint32_t time;
	const char* format;
	int32_t value;
};

struct ThreadsEntry {
	const char *name;
	Thread *thread;
	ThreadsEntry(const char *name, Thread *thread): name(name), thread(thread) {}
};

class SerialLog {
	char rxChar;
	Callback<void(char)> rxCallback;
	Serial &serial;
	Mail<LogEntry, 64> mail_box;
	std::vector<ThreadsEntry> monitored_threads;

	void onSerialRx(int p);

public:
	SerialLog(Serial &serial): rxChar(0), serial(serial) {}
	void run();
	void log(const char* format, int value=0);
	void logFrame(CANMessage *frame);
	void logShortString(const char *s);
	void registerThread(const char *name, Thread *thread) {
		monitored_threads.push_back(ThreadsEntry(name,thread));
	}
	void printThreads();
	void attachRXCallback(Callback<void(char)> cb) { rxCallback = cb; };
};

SerialLog *getLog();


#endif /* SERIALLOG_H_ */
