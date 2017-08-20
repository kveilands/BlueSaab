#include "mbed.h"
#include "rtos.h"
#include "SaabCan.h"
#include "Buttons.h"
#include "CDCStatus.h"

DigitalOut myLED (LED1);

int main() {
	myLED = 0;

	saabCan.initialize();
	buttons.initialize();
	cdcStatus.initialize();

	while (1) {
		wait(0.3);
		myLED = 0;
	}
}
