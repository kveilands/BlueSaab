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
 * Last modified on: Dec 15, 2016
 */

#ifndef RN52STRINGS_H_
#define RN52STRINGS_H_

// Action commands
#define RN52_CMD_BEGIN              "CMD\r\n"
#define RN52_CMD_EXIT               "END\r\n"
#define RN52_CMD_QUERY              "Q\r"
#define RN52_CMD_DETAILS            "D\r"
#define RN52_CMD_RECONNECTLAST      "B,06\r"
#define RN52_CMD_DISCONNECT         "K,06\r"
#define RN52_CMD_REBOOT             "R,1\r"
#define RN52_CMD_VOLUP              "AV+\r"
#define RN52_CMD_VOLDOWN            "AV-\r"
#define RN52_CMD_DISCOVERY_ON       "@,1\r"
#define RN52_CMD_DISCOVERY_OFF      "@,0\r"

// RN52 settings commands
#define RN52_SET_PAIR_TIMEOUT       "S^,0\r"            // Shutdown module if pairing doesn't happen. 0 means don't enable this feature
#define RN52_SET_DISCOVERY_MASK     "SD,06\r"           // A2DP/AVRCP + SPP profiles
#define RN52_SET_CONNECTION_MASK    "SK,06\r"           // A2DP/AVRCP + SPP profiles
#define RN52_SET_COD                "SC,200420\r"       // Sets "CoD" (Class of Device)
#define RN52_SET_DEVICE_NAME        "SN,BlueSaab\r"     // Broadcasted and shown in audio source's settigns
#define RN52_SET_BAUDRATE_9600      "SU,01\r"           // Enables serial communications on RN52 @ 9600bps
#define RN52_SET_MAXVOL             "SS,0F\r"           // Sets the volume gain to MAX level 15 (default 11)
#define RN52_SET_EXTENDED_FEATURES  "S%,1084\r"
/*
	Bit 0 – Enable AVRCP buttons for EK
	Bit 1 – Enable reconnect on power-on
>	Bit 2 – Discoverable on start up
	Bit 3 – Codec indicators PIO7 (AAC) and PIO6 (aptX)

	Bit 4 – Reboot after disconnect
	Bit 5 – Mute volume up/down tones
	Bit 6 – Enable voice command button on PIO4
>	Bit 7 – Disable system tones

	Bit 8 – Power off after pairing timeout
	Bit 9 – Reset after power off
	Bit 10 – Enable list reconnect after panic
	Bit 11 – Enable latch event indicator PIO2

>	Bit 12 – Enable track change event
	Bit 13 – Enable tones playback at fixed volume
	Bit 14 – Enable auto-accept passkey in Keyboard I/O Authentication mode
*/

// AVRCP commands
#define RN52_CMD_AVCRP_NEXT         "AT+\r"
#define RN52_CMD_AVCRP_PREV         "AT-\r"
#define RN52_CMD_AVCRP_VASSISTANT   "P\r"
#define RN52_CMD_AVCRP_PLAYPAUSE    "AP\r"
#define RN52_CMD_GET_TRACK_DATA     "AD\r"

// RN52 reply messages
#define RN52_RX_OK                  "AOK\r\n"
#define RN52_RX_ERROR               "ERR\r\n"
#define RN52_RX_WHAT                "?\r\n"
#define RN52_RX_REBOOT              "Reboot!"


#endif /* RN52STRINGS_H_ */
