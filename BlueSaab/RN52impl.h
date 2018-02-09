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
 * Modified on: December 15, 2016
 */

#ifndef RN52impl_H
#define RN52impl_H

#include "mbed.h"
#include "RN52driver.h"

/**
 * Atmel 328 pin definitions:
 */

//const int BT_FACT_RST_PIN = A0;             // RN52 factory reset pin GPIO4
//const int HW_REV_CHK_PIN = A1;              // HW revision check pin
//const int SN_XCEIVER_RS_PIN = A3;           // Connected to "Sleep control" (RS) pin on SN65HVD251D CAN transciever
//const int BT_EVENT_INDICATOR_PIN = 3;       // RN52 GPIO2 pin for reading current status of the module
//const int BT_CMD_PIN = 4;                   // RN52 GPIO9 pin for enabling command mode
//const int BT_PWREN_PIN = 9;                 // RN52 Power enable pin
//const int UART_TX_PIN = 5;                  // UART Tx
//const int UART_RX_PIN = 6;                  // UART Rx

const int RX_BUF_SIZE = 80;

struct RXEntry {
	char buf[RX_BUF_SIZE];
};

class RN52impl : public RN52::RN52driver {
	Mail<RXEntry, 4> rx_mail_box;
	RXEntry *curRXEntry;

    // called by RN52lib when the connected Bluetooth devices uses a profile
    void onProfileChange(BtProfile profile, bool connected);
    
    ///SoftwareSerial softSerial =  SoftwareSerial(UART_RX_PIN, UART_TX_PIN);
    Serial serial;
    
    unsigned long lastEventIndicatorPinStateChange;
    unsigned long cmdResponseDeadline;
    
    bool playing;
    bool bt_iap;
    bool bt_spp;
    bool bt_a2dp;
    bool bt_hfp;
    
    DigitalOut bt_cmd_pin;
    DigitalOut bt_pwren_pin;
    InterruptIn bt_event_pin;

    Thread thread;

public:
    
    RN52impl()
		: curRXEntry(NULL)
    	//, serial(PA_2, PA_3, 115200) // tx, rx - uart2
		, serial(PA_9, PA_10, 115200) // tx, rx - uart1
		, bt_cmd_pin(PA_7)
		, bt_pwren_pin(PC_8)
		, bt_event_pin(PB_0)
		, thread(osPriorityNormal, 512)
	{
        playing = true;
        bt_iap = false;
        bt_spp = false;
        bt_a2dp = false;
        bt_hfp = false;
        lastEventIndicatorPinStateChange = 0;
        cmdResponseDeadline = 0;
    }
    
    ///void readFromUART();
    // this is used by RN52lib to send data to the RN52 module
    // the implementation of this method needs to write to the
    // connected serial port
    void toUART(const char* c);
    // this method is called by RN52lib when data arrives via
    // the SPP profile
    void fromSPP(const char* c, int len);
    // this method is called by RN52lib whenever it needs to
    // switch between SPP and command mode
    void setMode(Mode mode);
    void onError(int location, Error error);
    // GPIO2 of RN52 is toggled on state change, eg. a Bluetooth
    // devices connects
    void onGPIO2();
    void initialize();
    ///void update();

    void run();

private:
    void processCmdQueue();
    RXEntry* waitForRXLine(uint32_t timeout);
    void onSerialRX(int p);
    void clearRXMail();
};

#endif
