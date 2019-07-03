/* C++ class for handling communications with Microchip RN52 Bluetooth module
 * Copyright (C) 2013  Tim Otto
 * Copyright (C) 2018  Girts Linde
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
#ifndef RN52STRINGS_H_
#define RN52STRINGS_H_

// Action commands
#define RN52_CMD_BEGIN              "CMD\r\n"
#define RN52_CMD_EXIT               "END\r\n"
#define RN52_CMD_QUERY              "Q\r"
#define RN52_CMD_DETAILS            "D\r"
#define RN52_CMD_RECONNECTLAST      "B,04\r"
#define RN52_CMD_DISCONNECT         "K,04\r"
#define RN52_CMD_REBOOT             "R,1\r"
#define RN52_CMD_VOLUP              "AV+\r"
#define RN52_CMD_VOLDOWN            "AV-\r"
#define RN52_CMD_DISCOVERY_ON       "@,1\r"
#define RN52_CMD_DISCOVERY_OFF      "@,0\r"
#define RN52_CMD_RESET_PDL          "U\r"               // Reset Paired Device List (PDL)

// RN52 settings commands
#define RN52_SET_IDLE_TIMEOUT       "S^,600\r"          // Shutdown module after 10 minutes of idle time (not connected)
#define RN52_SET_PAIRING_TIMEOUT    "STP,10\r"          // Set pairing timeout to 10 seconds; upon failure switch to next device on PDL
#define RN52_SET_DISCOVERY_MASK     "SD,04\r"           // Enable A2DP/AVRCP profile in discovery mask
#define RN52_SET_CONNECTION_MASK    "SK,04\r"           // Enable A2DP/AVRCP profile in connection mask
#define RN52_SET_COD                "SC,200420\r"       // Sets "CoD" (Class of Device)
#define RN52_SET_DEVICE_NAME        "SN,BlueSaab v6\r"  // Broadcasted and shown in audio source's settings
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
