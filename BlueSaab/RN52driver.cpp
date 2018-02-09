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

///#include <Arduino.h>
#include <ctype.h>
#include <string.h>
#include "RN52driver.h"
#include "RN52strings.h"
///#include "Scroller.h"
#include "SerialLog.h"

#define DEBUGMODE  0

unsigned long cmdResponseTimeout = CMD_TIMEOUT; // Abandon command and reset if no response/no valid response received within this period.

namespace RN52 {
    
    static int getVal(char c);
    
    RN52driver::RN52driver() :
    currentCommand(NULL),
	/*mode(DATA), enterCommandMode(false), enterDataMode(false),*/
	trueMode(tmDATA),
	state(0), profile(0), a2dpConnected(false),
    sppConnected(false), streamingAudio(false), sppTxBufferPos(0), cmdRxBufferPos(0), commandQueuePos(0)
    {}
    
    int RN52driver::fromUART(const char c)
    {
        //if (mode == DATA && !enterCommandMode) {
        if (trueMode == tmDATA) {
            fromSPP(&c, 1);
            return 1;
        } else {
            return parseCmdResponse(&c, 1);
            
        }
    }
    
    int RN52driver::fromUART(const char *data, int size)
    {
        //if (mode == DATA && !enterCommandMode) {
        if (trueMode == tmDATA) {
            fromSPP(data, size);
        } else {
            int parsed = parseCmdResponse(data, size);
            if (parsed < size && parsed > 0) {
                // Not all UART bytes were processed by the CommandMode parser.
                // This means there was an END\r\n, so feed the remainder to SPP
                //if (mode == DATA) {
                if (trueMode == tmDATA) {
                    fromSPP(data + parsed, size - parsed);
                } else {
                    onError(1, OVERFLOWrn52);
                    return -1;
                }
            }
        }
        return size;
    }
    
//    int RN52driver::toSPP(const char c)
//    {
//        if(!sppConnected) {
//            onError(2, NOTCONNECTED);
//            return -2;
//        }
//
//        if (mode == DATA && !enterCommandMode)
//            toUART(&c, 1);
//        else {
//            // in command mode, buffer
//            int left = SPP_TX_BUFFER_SIZE - sppTxBufferPos;
//            if (left < 1) {
//                onError(2, OVERFLOWrn52);
//                return -1;
//            }
//
//            sppTxBuffer[sppTxBufferPos++] = c;
//        }
//        return 1;
//    }
//
//    int RN52driver::toSPP(const char *data, int size)
//    {
//        if(!sppConnected) {
//            onError(3, NOTCONNECTED);
//            return -2;
//        }
//
//        if (mode == DATA && !enterCommandMode)
//            toUART(data, size);
//        else {
//            // in command mode, buffer
//            int left = SPP_TX_BUFFER_SIZE - sppTxBufferPos;
//            if (left < size) {
//                onError(3, OVERFLOWrn52);
//                return -1;
//            }
//
//            memcpy(sppTxBuffer+sppTxBufferPos, data, size);
//            sppTxBufferPos += size;
//        }
//        return size;
//    }
    
    static bool isCmd(const char *buffer, const char *cmd) {
        return strncmp(buffer, cmd, strlen(cmd)) == 0;
    }
    
    void RN52driver::abortCurrentCommand() {
        if (isCmd(currentCommand, RN52_CMD_GET_TRACK_DATA)) {
            ///scroller.complete_update();
        }
        currentCommand = NULL;
        prepareDataMode();
    }

    void RN52driver::trimRightEOL() {
        for (;;) {
            int lastCharPos = cmdRxBufferPos - 1;
            if (lastCharPos < 0)
                break;
            if (cmdRxBuffer[lastCharPos] != '\r' && cmdRxBuffer[lastCharPos] != '\n') {
                // done trimming
                break;
            }
            cmdRxBufferPos = lastCharPos;
            cmdRxBuffer[cmdRxBufferPos] = 0;
        }
    }

    // We have a line in cmdRxBuffer with \r\n at the end. See what we can do.
    void RN52driver::parseResponseLine()
    {
        // TODO handle other responses, depending on the command sent before
        if (currentCommand == NULL) {
            cmdRxBuffer[cmdRxBufferPos - 2] = 0;
        } else if (isCmd(currentCommand, RN52_CMD_QUERY)) {
            currentCommand = NULL;
            if (cmdRxBufferPos != 6 || !parseQResponse(cmdRxBuffer)) {
                queueCommand(RN52_CMD_QUERY);
            }
        } else if (isCmd(currentCommand, RN52_CMD_DETAILS)) {
            if (isCmd(cmdRxBuffer, "BTA=")) {
                ///Serial.print(cmdRxBuffer + 4);
                currentCommand = NULL;
            }
        } else if (isCmd(currentCommand, RN52_CMD_GET_TRACK_DATA)) {
            if (isCmd(cmdRxBuffer, "Title=")) {
                trimRightEOL();
                ///scroller.set_title(cmdRxBuffer + 6);
            } else if (isCmd(cmdRxBuffer, "Artist=")) {
                trimRightEOL();
                ///scroller.set_artist(cmdRxBuffer + 7);
                ///scroller.complete_update();
                currentCommand = NULL;
            }
        } else if (isCmd(currentCommand, RN52_CMD_REBOOT)) {
            currentCommand = NULL;
        } else {
            // misc command (AVCRP, connect/disconnect, etc)
            if (isCmd(cmdRxBuffer, RN52_RX_OK)) {
                // OK
            } else if (isCmd(cmdRxBuffer, RN52_RX_ERROR)) {
                // Error
            } else if (isCmd(cmdRxBuffer, RN52_RX_WHAT)) {
                // WTF!?
            } else {
                ///Serial.print(F("Current command: "));
                ///Serial.println(currentCommand);
                cmdRxBuffer[cmdRxBufferPos - 2] = 0;
                onError(4, PROTOCOL);
#if (DEBUGMODE==1)
                Serial.print(F("Invalid Response: "));
                Serial.println(cmdRxBuffer);
//                            debug("Invalid response:");
//                            debug(cmdRxBuffer);
#endif
            }
            currentCommand = NULL;
        }

    }

    int RN52driver::parseCmdResponse(const char *data, int size)
    {
        int parsed = 0;
        while (parsed < size) {
            if (cmdRxBufferPos == CMD_RX_BUFFER_SIZE) {
                onError(4, OVERFLOWrn52);
                return -1;
            }

            // If the incoming line is longer than CMD_RX_BUFFER_SIZE
            // try to force it to end with \r\n
            if (cmdRxBufferPos == CMD_RX_BUFFER_SIZE - 1) {
                // in the last byte accept only \n
                if (data[parsed] != '\n') {
                    // drop any other chars
                    parsed++;
                    continue;
                }
            } else if (cmdRxBufferPos == CMD_RX_BUFFER_SIZE - 2) {
                // in the byte before that accept \r or \n
                if (data[parsed] != '\r' && data[parsed] != '\n') {
                    // drop any other chars
                    parsed++;
                    continue;
                }
            }
            
            cmdRxBuffer[cmdRxBufferPos++] = data[parsed++];
            //if (mode == DATA) {
            if (trueMode == tmCOMMAND_REQUESTED) {
#if (DEBUGMODE==1)
                Serial.print(F("Data: "));
                Serial.write(cmdRxBuffer, cmdRxBufferPos);
                Serial.println(F(""));
#endif
                // did not receive CMD\r\n yet
                if (cmdRxBufferPos == 5) {
                    if (isCmd(cmdRxBuffer, RN52_CMD_BEGIN)) {
                        //mode = COMMAND;
                        //enterCommandMode = false;
                    	trueMode = tmCOMMAND;
                        cmdRxBufferPos = 0;
                    } else {
                        //toSPP(cmdRxBuffer[0]);
                        for (int i = 1; i < 5; i++)
                            cmdRxBuffer[i - 1] = cmdRxBuffer[i];
                        cmdRxBufferPos--;
                    }
                }
            } else if (cmdRxBufferPos >= 2 &&
				cmdRxBuffer[cmdRxBufferPos - 1] == '\n' && cmdRxBuffer[cmdRxBufferPos - 2] == '\r') {
				// this is a line
#if (DEBUGMODE==1)
				Serial.print(F("CMD Response: "));
				Serial.write(cmdRxBuffer, cmdRxBufferPos);
				Serial.println(F(""));
#endif
				if (trueMode == tmDATA_REQUESTED) {
					if (isCmd(cmdRxBuffer, RN52_CMD_EXIT)) {
						//mode = DATA;
						//enterDataMode = false;
						trueMode = tmDATA;

						cmdRxBufferPos = 0;

						if (commandQueuePos > 0) {
							// there's outgoing command requests (yet or again)
							prepareCommandMode();
						}
						break;
					}
				} else if (trueMode == tmCOMMAND) {
					parseResponseLine();
				}

				cmdRxBufferPos = 0;
            }
        }


        //if (mode == COMMAND && !enterDataMode) {
        if (trueMode == tmCOMMAND) {
            if (currentCommand == NULL) {
                if (commandQueuePos > 0) {
                    // send next command
                    currentCommand = commandQueue[0];
                    for(int i = 1; i < commandQueuePos; i++)
                        commandQueue[i - 1] = commandQueue[i];
                    commandQueuePos--;
                    if (isCmd(currentCommand, RN52_CMD_REBOOT)) {
                        cmdResponseTimeout = REBOOT_TIMEOUT;
                    } else {
                        cmdResponseTimeout = CMD_TIMEOUT;
                    }
                    ///toUART(currentCommand, strlen(currentCommand));
                    toUART(currentCommand);
                } else {
                    //enterDataMode = true;
                    prepareDataMode();
                }
            }
        }
        return parsed;
    }
    
    int RN52driver::queueCommand(const char *cmd) {
    	getLog()->log("queue: %s\r\n", (int)cmd);
    	rtosQueue.put(cmd);
//        prepareCommandMode();
        return 0;

//        if (commandQueuePos == CMD_QUEUE_SIZE) {
//            onError(5, OVERFLOWrn52);
//            return -1;
//        }
//
//        commandQueue[commandQueuePos++] = cmd;
//        prepareCommandMode();
//        return 0;
    }
    
    bool RN52driver::parseQResponse(const char data[4]) {
        for (int i = 0; i < 4; i++) {
            if (!isxdigit(data[i]))
                return false;
        }

        int profile =
        (getVal(data[0]) << 4 | getVal(data[1])) & 0x0f;
        int state =
        (getVal(data[2]) << 4 | getVal(data[3])) & 0x0f;
        
        //profIAPConnected = profile & 0x01;
        bool lastSppConnected = sppConnected;
        bool lastA2dpConnected = a2dpConnected;
        
        sppConnected = profile & 0x02;
        a2dpConnected = profile & 0x04;
        //profHFPHSPConnected = profile & 0x08;
        bool trackChanged = getVal(data[0]) & 0x02;
        
        bool changed = (this->state != state) || (this->profile != profile);
        //bool profilesChanged = this->profile != profile;
        bool streamingChanged = (this->state == 13 && state != 13) || (this->state != 3 && state == 13);
        this->state = state;
        this->profile = profile;
        
        if (changed)
            onStateChange(state, profile);
        
        if (streamingChanged)
            onStreaming(state == 13);
        if (lastSppConnected != sppConnected)
            onProfileChange(SPP, sppConnected);
        if (lastA2dpConnected != a2dpConnected)
            onProfileChange(A2DP, a2dpConnected);
        if (trackChanged)
            get_track_data();
        return true;
    }
    
    void RN52driver::prepareCommandMode() {
        //if (mode == COMMAND) {
        if (trueMode == tmCOMMAND) {
#if (DEBUGMODE==1)
        Serial.println(F("DEBUG: prepareCommandMode(): Already in command mode."));
#endif
            return;
        }
        //enterCommandMode = true;
        trueMode = tmCOMMAND_REQUESTED;
        setMode(COMMAND);
#if (DEBUGMODE==1)
        Serial.println(F("DEBUG: RN52 'SPP -> CMD'."));
#endif
    }
    
    void RN52driver::prepareDataMode() {
        //if (mode == DATA)
        if (trueMode == tmDATA)
            return;
        
//        if (enterCommandMode) {
//#if (DEBUGMODE==1)
//            Serial.println(F("DEBUG: Command mode was attempted but never reached."));
//#endif
//            // command mode was attempted but never reached
//            enterCommandMode = false;
//        }
        trueMode = tmDATA_REQUESTED;
        setMode(DATA);
#if (DEBUGMODE==1)
        Serial.println(F("DEBUG: RN52 'CMD -> SPP'."));
#endif
    }
    
    int RN52driver::sendAVCRP(AVCRP cmd)
    {
        if(!a2dpConnected) {
            onError(6, NOTCONNECTED);
#if (DEBUGMODE==1)
            Serial.println(F("ERROR: RN52 A2DP not connected."));
#endif
            return -2;
        }
        switch(cmd) {
            case PLAYPAUSE:
                queueCommand(RN52_CMD_AVCRP_PLAYPAUSE);
#if (DEBUGMODE==1)
                Serial.println(F("DEBUG: Sending 'Play/Pause' command to RN52."));
#endif
                break;
            case PREV:
                queueCommand(RN52_CMD_AVCRP_PREV);
#if (DEBUGMODE==1)
                Serial.println(F("DEBUG: Sending 'Previous Track' command to RN52."));
#endif
                break;
            case NEXT:
                queueCommand(RN52_CMD_AVCRP_NEXT);
#if (DEBUGMODE==1)
                Serial.println(F("DEBUG: Sending 'Next Track' command to RN52."));
#endif
                break;
            case VASSISTANT:
                queueCommand(RN52_CMD_AVCRP_VASSISTANT);
#if (DEBUGMODE==1)
                Serial.println(F("DEBUG: Sending 'Invoke voice assistant' command to RN52."));
#endif
                break;
            case VOLUP:
                queueCommand(RN52_CMD_VOLUP);
                break;
            case VOLDOWN:
                queueCommand(RN52_CMD_VOLDOWN);
                break;
            default:
                break;
        }
        return 0;
    }
    
    void RN52driver::reconnectLast(){
        queueCommand(RN52_CMD_RECONNECTLAST);
#if (DEBUGMODE==1)
        Serial.println(F("DEBUG: RN52 connecting to the last known device."));
#endif
    }
    void RN52driver::disconnect(){
        queueCommand(RN52_CMD_DISCONNECT);
#if (DEBUGMODE==1)
        Serial.println(F("DEBUG: RN52 disconnecting from the 'active' device."));
#endif
    }
    void RN52driver::visible(bool visible){
        if (visible) {
            queueCommand(RN52_CMD_DISCOVERY_ON);
#if (DEBUGMODE==1)
            Serial.println(F("DEBUG: RN52 discoverable = ON."));
#endif
        }
        else {
            queueCommand(RN52_CMD_DISCOVERY_OFF);
#if (DEBUGMODE==1)
            Serial.println(F("DEBUG: RN52 discoverable = OFF (connectable)."));
#endif
        }
    }
    
    void RN52driver::set_discovery_mask() {
#if (DEBUGMODE==1)
        Serial.print(F("Setting discovery mask to: "));
        Serial.println(RN52_SET_DISCOVERY_MASK);
#endif
        queueCommand(RN52_SET_DISCOVERY_MASK);
    }
    
    void RN52driver::set_connection_mask() {
#if (DEBUGMODE==1)
        Serial.print(F("Setting connection mask to: "));
        Serial.println(RN52_SET_CONNECTION_MASK);
#endif
        queueCommand(RN52_SET_CONNECTION_MASK);
    }
    
    
    void RN52driver::set_cod() {
#if (DEBUGMODE==1)
        Serial.print(F("Setting class of device to: "));
        Serial.println(RN52_SET_COD);
#endif
        queueCommand(RN52_SET_COD);
    }
    
    
    void RN52driver::set_device_name() {
#if (DEBUGMODE==1)
        Serial.print(F("Setting device name to: "));
        Serial.println(RN52_SET_DEVICE_NAME);
#endif
        queueCommand(RN52_SET_DEVICE_NAME);
    }
    
    
    void RN52driver::set_baudrate() {
#if (DEBUGMODE==1)
        Serial.print(F("Setting RN52 baudrate to: "));
        Serial.println(RN52_SET_BAUDRATE_9600);
#endif
        queueCommand(RN52_SET_BAUDRATE_9600);
    }
    
    
    void RN52driver::set_max_volume() {
#if (DEBUGMODE==1)
        Serial.println(F("Turning RN52 volume gain to max..."));
#endif
        queueCommand(RN52_SET_MAXVOL);
    }
    
    void RN52driver::set_extended_features() {
#if (DEBUGMODE==1)
        Serial.print(F("Setting extended features to: "));
        Serial.println(RN52_SET_EXTENDED_FEATURES);
#endif
        queueCommand(RN52_SET_EXTENDED_FEATURES);
    }
    
    void RN52driver::set_pair_timeout() {
#if (DEBUGMODE==1)
        Serial.print(F("Setting pair timeout to: "));
        Serial.println(RN52_SET_PAIR_TIMEOUT);
#endif
        queueCommand(RN52_SET_PAIR_TIMEOUT);
    }
    
    void RN52driver::reboot() {
#if (DEBUGMODE==1)
        Serial.println(F("Rebooting RN52..."));
#endif
        queueCommand(RN52_CMD_REBOOT);
    }
    
    void RN52driver::print_mac() {
        queueCommand(RN52_CMD_DETAILS);
    }

    void RN52driver::get_track_data() {
        ///scroller.start_update();
        queueCommand(RN52_CMD_GET_TRACK_DATA);
    }

    void RN52driver::refreshState() {
        queueCommand(RN52_CMD_QUERY);
    }
    
    static int getVal(char c)
    {
        if(c >= '0' && c <= '9')
            return (c - '0');
        else
            return (c-'A'+10);
    }
    
} /* namespace RN52 */
