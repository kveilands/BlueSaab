/* C++ class for handling/emulating node communications on SAAB I-bus
 * Copyright (C) 2018 Girts Linde
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mbed.h"
#include "rtos.h"
#include "SaabCan.h"

CAN iBus(PB_8, PB_9);
CANMessage canRxFrame;
SaabCan saabCan;

void SaabCan::initialize(int hz) {
	if (iBus.frequency(hz) && iBus.mode(CAN::Normal)) {
//		getLog()->log("CAN OK\r\n");
	} else {
//		getLog()->log("CAN NOT OK\r\n");
	}

	iBus.attach(callback(this,&SaabCan::onRx), mbed::CAN::RxIrq);
	send_thread.start(callback(this, &SaabCan::sendFunc));
//	getLog()->registerThread("SaabCan::sendFunc", &send_thread);
}

void SaabCan::sendCanMessage(CANMessage &msg) {
	CANMessage *canTxFrame = new (canFrameQueue.alloc()) CANMessage();
	*canTxFrame = msg;
	canFrameQueue.put(canTxFrame);
}

void SaabCan::sendCanMessage(CANFormat format, unsigned int id, unsigned char len, const char *data) {
	CANMessage *canTxFrame = new (canFrameQueue.alloc()) CANMessage(format, data, len, CANData, format);
	canFrameQueue.put(canTxFrame);
}

void SaabCan::sendCanFrame(int canId, const unsigned char *data) {
	CANMessage *canTxFrame = new (canFrameQueue.alloc()) CANMessage();
	canTxFrame->id = canId;
	for (int i = 0; i < canTxFrame->len; i++) {
		canTxFrame->data[i] = data[i];
	}
	canFrameQueue.put(canTxFrame);
}

extern DigitalOut aliveLed;

void SaabCan::onRx() {
	while (iBus.read(canRxFrame)) {
		for (int i = 0; i < CAN_MAX_CALLBACKS; i++) {
			if (callBacks[i].id == canRxFrame.id) {
				callBacks[i].callBack.call(canRxFrame);
			}
		}
	}
}

void SaabCan::sendFunc() {
	while (true) {
		osEvent evt = canFrameQueue.get();
		if (evt.status == osEventMail) {
			CANMessage *message = (CANMessage*) evt.value.p;
//			getLog()->logFrame(message);
//			unsigned tde = iBus.tderror();

//			int rc =
			iBus.write(*message);
//			getLog()->log("send rc=%d\r\n", rc);
//			unsigned rde = iBus.rderror();
//			tde = iBus.tderror();
//			getLog()->log("    rde=%d\r\n", rde);
//			getLog()->log("    tde=%d\r\n", tde);
//
//			uint32_t esr = iBus.read_ESR();
//			getLog()->log("    ESR=%08x\r\n", esr);
//			getLog()->log("    BOFF=%x\r\n", esr & 4);
//			getLog()->log("    MCR=%08x\r\n", iBus.read_MCR());

			canFrameQueue.free(message);
			aliveLed = !aliveLed;
		}
	}
}

void SaabCan::attach(unsigned int canId, Callback<void(CANMessage&)> callBack) {
	for (int i = 0; i < CAN_MAX_CALLBACKS; i++) {
		if (callBacks[i].id == 0) {
			callBacks[i].callBack = callBack;
			callBacks[i].id = canId;
//			getLog()->log("SaabCan::attach canId %x", canId);
			break;
		}
	}
}
