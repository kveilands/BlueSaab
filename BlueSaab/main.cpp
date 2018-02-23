#include "mbed.h"
#include "rtos.h"
#include "SaabCan.h"
#include "Buttons.h"
#include "CDCStatus.h"
#include "MessageSender.h"
#include "Bluetooth.h"

//DigitalOut aliveLed (LED1);
DigitalOut aliveLed (PA_1);

//extern MessageSender cdcActiveCmdSender;

Thread logThread(osPriorityLow, 1024);

int main() {
	logThread.start(callback(getLog(),&SerialLog::run));
	getLog()->registerThread("logThread", &logThread);

	aliveLed = 1;

	saabCan.initialize(47619);
	bluetooth.initialize();
	buttons.initialize();
	cdcStatus.initialize();

	while (1) {
		Thread::wait(1000);
	}

//	while (1) {
//		Thread::wait(5300);

//
//		uint32_t t0 = us_ticker_read();
//		for(int i=0; i<4; i++) {
//			if (i > 0)
//				Thread::wait(200);
//			int32_t dt = us_ticker_read() - t0;
//			getLog()->log2("MessageSender::run MAIN, dt1 = %d\r\n", dt/1000);
//			saabCan.sendCanFrame(frameId, frames[i]);
//		}

//		cdcActiveCmdSender.send();
//		myLED = 1;
//	}
}
