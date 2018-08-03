/* C++ class for handling communications with Microchip RN52 Bluetooth module
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

#ifndef RN52DRIVER_H
#define RN52DRIVER_H

#include <rtos.h>

namespace RN52 {

class RN52driver {
	public:
		enum BtProfile {IAP, SPP, A2DP, HFP};
		enum Mode {COMMAND, DATA};
		enum TrueMode {tmDATA, tmCOMMAND_REQUESTED, tmCOMMAND, tmDATA_REQUESTED};
		enum Error {TIMEOUT, OVERFLOWrn52, NOTCONNECTED, PROTOCOL};
		enum AVCRP {PLAYPAUSE, NEXT, PREV, VASSISTANT, VOLUP, VOLDOWN};

		RN52driver();
		virtual ~RN52driver() {}
		bool isA2DPConnected() {return a2dpConnected;}
		bool isSPPConnected() {return sppConnected;}
		bool isStreamingAudio() {return streamingAudio;}

		void reconnectLast();
		void disconnect();
		void setDiscoveryMask();
		void setConnectionMask();
		void setCod();
		void setDeviceName();
		void setBaudrate();
		void setMaxVolume();
		void setExtendedFeatures();
		void setPairTimeout();
		void reboot();
		void getDetails();
		void getTrackData();
		void visible(bool visible);
		int sendAVCRP(AVCRP cmd);

		Queue<const char, 20> rtosQueue;
		bool parseQResponse(const char data[4]);

	protected:
		void refreshState();
		int queueCommand(const char *cmd);

	private:
		int state;
		int profile;
		bool a2dpConnected;
		bool sppConnected;
		bool streamingAudio;
		virtual void onStateChange(int state, int profile) {};
		virtual void onProfileChange(BtProfile profile, bool connected) {};
		virtual void onStreaming(bool streaming) {};
		virtual void toUART(const char* c) = 0;
		virtual void debug(const char *c) {};
		virtual void onError(int location, Error error) {};
	};

} /* namespace RN52 */
#endif /* RN52DRIVER_H */
