#include "mbed.h"
#include "rtos.h"
#include "SaabCan.h"
#include "Buttons.h"
#include "CDCStatus.h"
#include "MessageSender.h"
#include "Bluetooth.h"
#include "SidResource.h"

DigitalOut aliveLed(PA_1);

Thread logThread(osPriorityLow, 1024);

int main() {
	logThread.start(callback(getLog(), &SerialLog::run));
	getLog()->registerThread("logThread", &logThread);

	aliveLed = 1;

	saabCan.initialize(47619);
	bluetooth.initialize();
	buttons.initialize();
	cdcStatus.initialize();
	sidResource.initialize();

	while (1) {
		Thread::wait(1000);
	}
}
