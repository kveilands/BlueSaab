/* C++ class for emulating CD changer communications on SAAB I-bus
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

#ifndef CDCSTATUS_H_
#define CDCSTATUS_H_

#include <rtos.h>

class CDCStatus {
	volatile bool cdcActive;
	Thread thread;

	void run();
	void sendCdcStatus(bool event, bool remote, bool cdcActive);

public:
	CDCStatus() :
			cdcActive(false), thread(osPriorityNormal, 256) {}
	void initialize();
	void onIhuStatusFrame(CANMessage& frame);
	void onCDCControlFrame(CANMessage& frame);
};

extern CDCStatus cdcStatus;

#endif /* CDCSTATUS_H_ */
