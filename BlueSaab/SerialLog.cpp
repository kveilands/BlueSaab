#include "SerialLog.h"

//Serial pcSerial(USBTX, USBRX, 115200);
Serial pcSerial(PA_2, PA_3, 115200);

SerialLog *logInstance = NULL;

SerialLog *getLog() {
	if (logInstance == NULL) {
		logInstance = new SerialLog(pcSerial);
	}
	return logInstance;
}

void SerialLog::run() {
	uint32_t last_threads_printed = us_ticker_read();
	while(true) {
        osEvent evt = mail_box.get(1000); // time out to print thread info periodically
        if (evt.status == osEventMail) {
            LogEntry *e = (LogEntry*)evt.value.p;
            if (e->format) {
            	serial.printf(e->format, e->value);
            } else { // CANMessage
            	CANMessage *m = (CANMessage *)e->value;
            	serial.printf("%#x: %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
            		m->id, m->data[0], m->data[1], m->data[2], m->data[3], m->data[4], m->data[5], m->data[6], m->data[7]);
            	delete m;
            }
            mail_box.free(e);
        }
        uint32_t time_now = us_ticker_read();
        int time_since_print = time_now - last_threads_printed;
        if (time_since_print > 1000000) {
        	last_threads_printed = time_now;
        	for(unsigned i=0; i<monitored_threads.size(); i++) {
        		serial.printf("thread %20s size %4u, max used %u\r\n",
        				monitored_threads[i].name,
        				monitored_threads[i].thread->stack_size(),
						monitored_threads[i].thread->max_stack());
        	}
        }
	}
}

void SerialLog::log(const char* format, int value) {
	LogEntry *e = mail_box.alloc();
	e->format = format;
	e->value = value;
    mail_box.put(e);
}

void SerialLog::logFrame(CANMessage* frame) {
	CANMessage *saved_frame = new CANMessage();
	*saved_frame = *frame;
	log(NULL, (int)saved_frame);
}
