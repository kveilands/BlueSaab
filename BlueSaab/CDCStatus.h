#ifndef CDCSTATUS_H_
#define CDCSTATUS_H_

#include <rtos.h>

class CDCStatus {
	Thread thread;
	void run();
	void sendCdcStatus(bool event, bool remote, bool cdcActive);

public:
	void initialize();
	void onIhuStatusFrame(CANMessage& frame);
};

extern CDCStatus cdcStatus;

#endif /* CDCSTATUS_H_ */
