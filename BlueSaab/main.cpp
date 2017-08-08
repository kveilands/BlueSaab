#include "mbed.h"
#include "rtos.h"
#include "SaabCan.h"

int main() {
	saabCan.initialize();
	while (1) {
		saabCan.handleRxFrame();
	}
}
