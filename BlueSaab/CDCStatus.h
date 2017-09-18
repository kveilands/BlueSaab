#ifndef CDCSTATUS_H_
#define CDCSTATUS_H_

#include <rtos.h>

class CDCStatus {
	volatile bool cdcActive;
	Thread thread;

	void run();
	void sendCdcStatus(bool event, bool remote, bool cdcActive);

public:
	CDCStatus():cdcActive(false), thread(osPriorityNormal, 768) {}
	void initialize();
	void onIhuStatusFrame(CANMessage& frame);
	void onCDCControlFrame(CANMessage& frame);
};

extern CDCStatus cdcStatus;

#endif /* CDCSTATUS_H_ */
