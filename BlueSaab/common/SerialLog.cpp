#include "SerialLog.h"

//Serial pcSerial(USBTX, USBRX, 115200);
Serial pcSerial(PA_2, PA_3, 115200); // uart2 (nucleo)
//Serial pcSerial(PB_10, PB_11, 115200); // uart3

SerialLog *logInstance = NULL;

SerialLog *getLog() {
	if (logInstance == NULL) {
		logInstance = new SerialLog(pcSerial);
	}
	return logInstance;
}


void SerialLog::onSerialRx(int p) {
	if (p & SERIAL_EVENT_RX_COMPLETE) {
		if (rxCallback) {
			rxCallback.call(rxChar);
		}
	}

	serial.read((uint8_t*)&rxChar, 1, callback(this, &SerialLog::onSerialRx), SERIAL_EVENT_RX_COMPLETE);
}

void SerialLog::run() {
	serial.read((uint8_t*)&rxChar, 1, callback(this, &SerialLog::onSerialRx), SERIAL_EVENT_RX_COMPLETE);

	uint32_t last_threads_printed = us_ticker_read();
	while(true) {
		osEvent evt = mail_box.get(1000); // time out to print thread info periodically
		if (evt.status == osEventMail) {
			LogEntry *e = (LogEntry*)evt.value.p;
			serial.printf("%05d ", e->time/1000);
			switch ((int)e->format) {
				case leCANMessage: {
					CANMessage *m = (CANMessage *) e->value;
					if (m->type == CANData)
						serial.printf(
								"%#x %c %d: %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
								m->id, (m->format == CANStandard) ? 's' : 'x',
								m->len, m->data[0], m->data[1], m->data[2],
								m->data[3], m->data[4], m->data[5], m->data[6],
								m->data[7]);
					else
						serial.printf("%#x %c: remote\r\n", m->id,
								(m->format == CANStandard) ? 's' : 'x');
					delete m;
					break;
				}
				case leShortString: {
					char *p = (char*)&e->value;
					for(int i=0; i<4; i++) {
						if (*p == 0 || *p == '\r' || *p == '\n')
							break;
						serial.putc(*p);
						p++;
					}
					serial.puts("\r\n");
					break;
				}
				default: {
					serial.printf(e->format, e->value);
					size_t l = strlen(e->format);
					if (l==0 || e->format[l-1]!='\n')
						serial.putc('\n');
				}
			}
			mail_box.free(e);
		}
		uint32_t time_now = us_ticker_read();
		int time_since_print = time_now - last_threads_printed;
		if (time_since_print > 1000000) {
			last_threads_printed = time_now;
			printThreads();
		}
	}
}

void SerialLog::printThreads() {
	for(unsigned i=0; i<monitored_threads.size(); i++) {
		serial.printf("thread %20s size %4u, max used %u\r\n",
				monitored_threads[i].name,
				monitored_threads[i].thread->stack_size(),
				monitored_threads[i].thread->max_stack());
	}
}

void SerialLog::log(const char* format, int value) {
	LogEntry *e = mail_box.alloc();
	e->time = us_ticker_read();
	e->format = format;
	e->value = value;
	mail_box.put(e);
}

void SerialLog::logFrame(CANMessage* frame) {
	CANMessage *saved_frame = new CANMessage();
	*saved_frame = *frame;
	log((char *)leCANMessage, (int)saved_frame);
}

void SerialLog::logShortString(const char *s) {
	uint32_t buf;
	strncpy((char *)&buf, s, 4);
	log((char *)leShortString, buf);
}
