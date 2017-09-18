#ifndef SERIALLOG_H_
#define SERIALLOG_H_

#include <mbed.h>
#include <rtos.h>

#include <vector>

struct LogEntry {
	const char* format;
	int value;
};

struct ThreadsEntry {
	const char *name;
	Thread *thread;
	ThreadsEntry(const char *name, Thread *thread): name(name), thread(thread) {}
};

class SerialLog {
	Serial &serial;
	Mail<LogEntry, 10> mail_box;
	std::vector<ThreadsEntry> monitored_threads;
public:
	SerialLog(Serial &serial) :serial(serial) {}
	void run();
	void log(const char* format, int value=0);
	void logFrame(CANMessage *frame);
	void registerThread(const char *name, Thread *thread) {
		monitored_threads.push_back(ThreadsEntry(name,thread));
	}
};

SerialLog *getLog();


#endif /* SERIALLOG_H_ */
