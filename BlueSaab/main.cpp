#include "mbed.h"
#include "rtos.h"
#include "SaabCan.h"
#include "Buttons.h"

DigitalOut myLED (LED1);

int main() {
	myLED = 0;

	buttons.initialize();
	saabCan.initialize();

	while (1) {
		wait(0.3);
		myLED = 0;
	}
}
