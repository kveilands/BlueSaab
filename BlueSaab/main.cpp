#include "mbed.h"
#include "rtos.h"
#include "SaabCan.h"

DigitalOut myLED (LED1);

int main() {
	myLED = 0;
	saabCan.initialize();
	while (1) {
//		saabCan.handleRxFrame();
		wait(0.3);
		myLED = 0;
	}
}
