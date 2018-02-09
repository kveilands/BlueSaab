/*
 * Virtual C++ Class for RovingNetworks RN-52 Bluetooth modules
 * Copyright (C) 2013  Tim Otto
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Created by: Tim Otto
 * Created on: Jun 21, 2013
 * Modified by: Sam Thompson
 * Last modified on: Dec 16, 2016
 */

#include "RN52impl.h"
#include "RN52strings.h"
#include <string.h>

#include "SerialLog.h"

#define DEBUGMODE 0

///**
// * Reads the input (if any) from UART over software serial connection
// */
//
//void RN52impl::readFromUART() {
//    while (softSerial.available()) {
//        char c = softSerial.read();
//        //Serial.print(c);
//        fromUART(c);
////        if (currentCommand) {
//        cmdResponseDeadline = millis() + cmdResponseTimeout;
////        }
//    }
//}


/**
 * Formats a message and writes it to UART over software serial
 */

void RN52impl::toUART(const char* c){
	serial.puts(c);
//    for(int i = 0; i < len; i++) {
//        softSerial.write(c[i]);
//        //Serial.print(c[i]);
//    }
};

void RN52impl::fromSPP(const char* c, int len){
    // bytes received from phone via SPP
    
    // to send bytes back to the phone call rn52.toSPP
};

void RN52impl::setMode(Mode mode){
    if (mode == COMMAND) {
        ///digitalWrite(BT_CMD_PIN, LOW);
    	bt_cmd_pin = 0;
#if (DEBUGMODE==1)
        ///Serial.println(F("RN52: Set command mode."));
#endif
    } else if (mode == DATA) {
        ///digitalWrite(BT_CMD_PIN, HIGH);
    	bt_cmd_pin = 1;
#if (DEBUGMODE==1)
        ///Serial.println(F("RN52: Set data mode. "));
#endif
    }
};

void RN52impl::onError(int location, Error error){
    ///Serial.print(F("RN52 Error "));
    ///Serial.print(error);
    ///Serial.print(F(" at location: "));
    ///Serial.println(location);
};

void RN52impl::onGPIO2() {
    queueCommand(RN52_CMD_QUERY);
}

void RN52impl::onProfileChange(BtProfile profile, bool connected) {
    switch(profile) {
        case A2DP:bt_a2dp = connected;
            if (connected && playing) {
                sendAVCRP(RN52::RN52driver::PLAYPAUSE);
            }
            break;
        case SPP:bt_spp = connected;break;
        case IAP:bt_iap = connected;break;
        case HFP:bt_hfp = connected;break;
    }
}

//void RN52impl::update() {
//    readFromUART();
//    if (digitalRead(BT_EVENT_INDICATOR_PIN) == 0) {
//        if (getMode()==DATA && (millis() - lastEventIndicatorPinStateChange) > 100) {
//            lastEventIndicatorPinStateChange = millis();
//            onGPIO2();
//#if (DEBUGMODE==1)
//            ///Serial.println(F("Event Indicator Pin signalled."));
//#endif
//        }
//    }
//    if ( (long)( millis() - cmdResponseDeadline ) >= 0) {
//        if (currentCommand) {
//            // timed out. Bail on command if there is one, and reset
//            cmdResponseDeadline = millis() + cmdResponseTimeout;
//            ///Serial.println(F("Warning: Command timed out: "));
//            abortCurrentCommand();
//        }
//    }
//}

void RN52impl::initialize() {
	curRXEntry = rx_mail_box.alloc();
	serial.read((uint8_t*)curRXEntry->buf, sizeof(curRXEntry->buf), callback(this, &RN52impl::onSerialRX),
			SERIAL_EVENT_RX_ALL
			//SERIAL_EVENT_RX_CHARACTER_MATCH | SERIAL_EVENT_RX_COMPLETE
			, '\n');

	thread.start(callback(this,&RN52impl::run));
	getLog()->registerThread("RN52impl::run", &thread);

    // For hardware versions with 9600 baud locked, can set RN52 configuration after enabling it.
    //bool configRN52postEnable = false;
  
    // Initializing ATMEGA pins
    //pinMode(BT_PWREN_PIN,OUTPUT);               // RN52 will not be restartable if rebooted with PWREN low.
	bt_pwren_pin = 1;

    // No point in pulling low again.
    // According to RN52 DS70005120A p14 (section 2.5), cannot power down vreg.
    // Leaving high still allows pair timeout sleep.
    //pinMode(BT_EVENT_INDICATOR_PIN,INPUT);
    //pinMode(BT_CMD_PIN, OUTPUT);

    // TODO: not connected to STM?
	//pinMode(BT_FACT_RST_PIN,OUTPUT);            // Some REALLY crazy stuff is going on if this pin is set as output and pulled low. Leave it alone! Trust me...

    //pinMode(HW_REV_CHK_PIN,INPUT);              // We do an analogRead() on this pin to determine HW version of the module and take action accordingly

	// TODO: still there? this does not belong to bluetooth though
    //pinMode(SN_XCEIVER_RS_PIN,OUTPUT);

    //digitalWrite(BT_EVENT_INDICATOR_PIN,HIGH);  // Default state of GPIO2, per data sheet, is HIGH
    //digitalWrite(BT_CMD_PIN,HIGH);              // Default state of GPIO9, per data sheet, is HIGH
    bt_cmd_pin = 1;
    // TODO: digitalWrite(BT_FACT_RST_PIN,HIGH);         // Default state of GPIO4, per data sheet, is LOW, but this is "voice command mode".

    getLog()->log("Configuring RN52...\r\n");
	set_discovery_mask();
	set_connection_mask();
	set_cod();
	set_device_name();
	set_extended_features();
	set_max_volume();
	set_pair_timeout();
	reboot();
	//processCmdQueue();
	Thread::wait(5000);
    getLog()->log("RN52 configuration completed!\r\n");

    bt_event_pin.fall(callback(this,&RN52impl::onGPIO2));
}

void RN52impl::processCmdQueue() {
#if (DEBUGMODE==1)
    ///Serial.println(F("Processing command queue."));
#endif
    do {
        ///update();
    } while (getQueueSize() || currentCommand != NULL); //FIXME: fails if only 1 cmd in the queue to start.
}

RXEntry*  RN52impl::waitForRXLine(uint32_t timeout) {
//	getLog()->log("RN52impl::waitForRXLine()\r\n");

	osEvent evt = rx_mail_box.get(timeout);
	if (evt.status == osEventMail) {
		//getLog()->log("rxGET\r\n");
		RXEntry *e = (RXEntry*)evt.value.p;
		char *curBuf = e->buf;
		curBuf[RX_BUF_SIZE-1] = 0; // zero terminate the whole buffer to be safe
		char *pos = strchr(curBuf,'\n');
		if (pos) {
			*(pos+1) = 0; // zero terminate the received data
		}
		return e;
	}
	return NULL;
}

void RN52impl::onSerialRX(int p) {
	//getLog()->log("RN52impl::onSerialRx(p=%x)\r\n", p);
	if (p & (SERIAL_EVENT_RX_CHARACTER_MATCH | SERIAL_EVENT_RX_COMPLETE)) {
		RXEntry *newEntry = rx_mail_box.alloc();
		if (newEntry == NULL) {
			getLog()->log("NO MAIL TO ALLOC, drop and reuse\r\n");
		} else {
			rx_mail_box.put(curRXEntry);
			//getLog()->log("rxPUT\r\n");
			curRXEntry = newEntry;
		}
	}

	serial.read((uint8_t*)curRXEntry->buf, sizeof(curRXEntry->buf), callback(this, &RN52impl::onSerialRX),
			SERIAL_EVENT_RX_ALL
			//SERIAL_EVENT_RX_CHARACTER_MATCH | SERIAL_EVENT_RX_COMPLETE
			, '\n');
}

void RN52impl::clearRXMail() {
	int n = 0;
	for(;;) {
		osEvent evt = rx_mail_box.get(0);
		if (evt.status == osEventMail) {
			RXEntry *e = (RXEntry*)evt.value.p;
			rx_mail_box.free(e);
			n++;
		} else {
			break;
		}
	}
	if ( n >0 )
		getLog()->log("%d mails cleared\r\n", n);
}

static bool isCmd(const char *buffer, const char *cmd) {
    return strncmp(buffer, cmd, strlen(cmd)) == 0;
}

char title[72];
char artist[72];

void RN52impl::run() {
	getLog()->log("RN52impl::run() started\r\n");

	osEvent evt;
	evt.status = osOK;
	for(;;) {
		//getLog()->log("RN52impl::run() waiting for cmd (1)...\r\n");
		if (evt.status != osEventMessage) { // osEventMessage would mean it has a cmd we haven't sent yet
			evt = rtosQueue.get();
		}
		getLog()->log("Entering cmd mode\r\n");
		clearRXMail();
		bt_cmd_pin.write(0);
		RXEntry* gotBuf = waitForRXLine(500); // get CMD
		if (gotBuf) {
			getLog()->logShortString(gotBuf->buf);
			bool gotCMD = isCmd(gotBuf->buf, RN52_CMD_BEGIN);
			rx_mail_box.free(gotBuf);

			if (gotCMD) {
				for(;;) {
					if (evt.status == osEventMessage) {
						const char *cmd = (const char*)evt.value.p;
						getLog()->log("send: %s", (int)cmd);
						toUART(cmd);
						evt.status = osOK;

						if (isCmd(cmd, RN52_CMD_GET_TRACK_DATA)) { // gather track info until timeout
							int lines = 0;
							while ( true ) {
								gotBuf = waitForRXLine(100);
								if (gotBuf) {
									lines++;
									// The strings after Title= and Artist= seem to be limited to 60 chars,
									// plus \r\r\n. Which makes it max 70 chars total.
									if (isCmd(gotBuf->buf, "Title=")) {
										strncpy(title,gotBuf->buf,sizeof(title)); // may not 0 terminate
										title[sizeof(title)-1] = 0;
										getLog()->log(title);
									} else if (isCmd(gotBuf->buf, "Artist=")) {
										strncpy(artist,gotBuf->buf,sizeof(artist)); // may not 0 terminate
										artist[sizeof(artist)-1] = 0;
										getLog()->log(artist);
									} else {
										getLog()->logShortString(gotBuf->buf);
									}
									rx_mail_box.free(gotBuf);
								} else {
									getLog()->log("track info response %d lines\r\n", lines);
									break;
								}
							}
						} else
						if (isCmd(cmd, RN52_CMD_QUERY)) {
							gotBuf = waitForRXLine(500);
							if (gotBuf) {
								getLog()->logShortString(gotBuf->buf);
								if (strlen((char *)gotBuf->buf) != 6 || !parseQResponse((char *)gotBuf->buf)) {
									queueCommand(RN52_CMD_QUERY);
								}
								rx_mail_box.free(gotBuf);
							}
						} else {
							gotBuf = waitForRXLine(500);
							if (gotBuf) {
								getLog()->logShortString(gotBuf->buf);
								rx_mail_box.free(gotBuf);
							}
						}
					}
					//getLog()->log("RN52impl::run() waiting for cmd (2)...\r\n");
					evt = rtosQueue.get(500);
					if (evt.status == osEventTimeout) {
						break;
					}
				} // for(;;)
			}
		}
		getLog()->log("Exiting cmd mode\r\n");
		bt_cmd_pin.write(1);
		gotBuf = waitForRXLine(500); // get END or timeout
		if (gotBuf) {
			getLog()->logShortString(gotBuf->buf);
			rx_mail_box.free(gotBuf);
		}
	}

//	while (1) {
//		getLog()->log("RN52impl::run() waiting for cmd...\r\n");
//		// wait for a command from queue
//		osEvent evt = rtosQueue.get();
//		if (evt.status == osEventMessage) {
//			const char *cmd = (const char*)evt.value.p;
//			getLog()->log("RN52impl::run() from queue:");
//			getLog()->log(cmd);
//		}
//		// send out the command
//
//		// wait for the command reply
//
//		// parse the reply
//		//impl.fromUART(data, size);
//	}
}
