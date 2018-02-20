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
