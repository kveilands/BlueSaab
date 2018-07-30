#include "CDCStatus.h"
#include "SaabCan.h"
#include "MessageSender.h"
#include "Bluetooth.h"
#include "SidResource.h"

CDCStatus cdcStatus;

unsigned char cdcPoweronCmd[NODE_STATUS_TX_MSG_SIZE][8] = {
		{ 0x32, 0x00, 0x00, 0x03, 0x01, 0x02, 0x00, 0x00 },
		{ 0x42, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00 },
		{ 0x52, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00 },
		{ 0x62, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00 }
};
unsigned char cdcActiveCmd[NODE_STATUS_TX_MSG_SIZE][8] = {
		{ 0x32, 0x00, 0x00, 0x16, 0x01, 0x02, 0x00, 0x00 },
		{ 0x42, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00 },
		{ 0x52, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00 },
		{ 0x62, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00 }
};
unsigned char cdcPowerdownCmd[NODE_STATUS_TX_MSG_SIZE][8] = {
		{ 0x32, 0x00, 0x00, 0x19, 0x01, 0x00, 0x00, 0x00 },
		{ 0x42, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x00 },
		{ 0x52, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x00 },
		{ 0x62, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x00 }
};

unsigned char soundCmd[] = {0x80,0x04,0x00,0x00,0x00,0x00,0x00,0x00};

MessageSender cdcPoweronCmdSender(0x4, NODE_STATUS_TX_CDC,cdcPoweronCmd, 4, NODE_STATUS_TX_INTERVAL);
MessageSender cdcActiveCmdSender(0x1, NODE_STATUS_TX_CDC, cdcActiveCmd, 4, NODE_STATUS_TX_INTERVAL);
MessageSender cdcPowerdownCmdSender(0x8, NODE_STATUS_TX_CDC, cdcPowerdownCmd, 4, NODE_STATUS_TX_INTERVAL);

void CDCStatus::initialize() {
	saabCan.attach(NODE_STATUS_RX_IHU, callback(this, &CDCStatus::onIhuStatusFrame));
	saabCan.attach(CDC_CONTROL, callback(this, &CDCStatus::onCDCControlFrame));
	getLog()->log("CDCStatus::initialize()\r\n");
	thread.start(callback(this, &CDCStatus::run));
	getLog()->registerThread("CDCStatus::run", &thread);
}

void CDCStatus::onCDCControlFrame(CANMessage& frame) {
	if (frame.data[0] == 0x80) {
		switch (frame.data[1]) {
		case 0x24:
			cdcActive = true;
			sidResource.activate();
			saabCan.sendCanFrame(SOUND_REQUEST, soundCmd);
			thread.signal_set(0x2);
			bluetooth.reconnect();
			break;
		case 0x14:
			sidResource.deactivate();
			cdcActive = false;
			thread.signal_set(0x2);
			bluetooth.disconnect();
			break;
		}
	}
}

void CDCStatus::onIhuStatusFrame(CANMessage& frame) {

	/*
	 Here be dragons... This part of the code is responsible for causing lots of headache
	 We look at the bottom half of 3rd byte of '6A1' frame to determine what the "reply" should be
	 */

	switch (frame.data[3] & 0x0F) {
	case (0x3):
		cdcPoweronCmdSender.send();
		break;
	case (0x2):
		cdcActiveCmdSender.send();
		break;
	case (0x8):
		cdcPowerdownCmdSender.send();
		break;
	}
}

void CDCStatus::run() {
	getLog()->log("CDCStatus::run()\r\n");
	bool cdcStatusResendNeeded = false;
	bool cdcStatusResendDueToCdcCommand = false;

	while(1) {
		getLog()->log("cdcActive %d\r\n", cdcActive);
		sendCdcStatus(cdcStatusResendNeeded, cdcStatusResendDueToCdcCommand, cdcActive);
		Thread::wait(50);
		cdcStatusResendNeeded = false;
		cdcStatusResendDueToCdcCommand = false;
		osEvent result = Thread::signal_wait(0x2, CDC_STATUS_TX_BASETIME-50);
		if (result.status == osEventSignal) {
			cdcStatusResendNeeded = true;
			cdcStatusResendDueToCdcCommand = true;
		}
	}
}

void CDCStatus::sendCdcStatus(bool event, bool remote, bool cdcActive) {

	/* Format of GENERAL_STATUS_CDC frame:
	 ID: CDC node ID
	 [0]:
	 byte 0, bit 7: FCI NEW DATA: 0 - sent on base time, 1 - sent on event
	 byte 0, bit 6: FCI REMOTE CMD: 0 - status change due to internal operation, 1 - status change due to CDC_COMMAND frame
	 byte 0, bit 5: FCI DISC PRESENCE VALID: 0 - disc presence signal is not valid, 1 - disc presence signal is valid
	 [1]: Disc presence validation (boolean)
	 byte 1-2, bits 0-15: DISC PRESENCE: (bitmap) 0 - disc absent, 1 - disc present. Bit 0 is disc 1, bit 1 is disc 2, etc.
	 [2]: Disc presence (bitmap)
	 byte 1-2, bits 0-15: DISC PRESENCE: (bitmap) 0 - disc absent, 1 - disc present. Bit 0 is disc 1, bit 1 is disc 2, etc.
	 [3]: Disc number currently playing
	 byte 3, bits 7-4: DISC MODE
	 byte 3, bits 3-0: DISC NUMBER
	 [4]: Track number currently playing
	 [5]: Minute of the current track
	 [6]: Second of the current track
	 [7]: CD changer status; D0 = Married to the car
	 */

	unsigned char cdcGeneralStatusCmd[8] = { 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xD0 };
	cdcGeneralStatusCmd[0] = ((event ? 0x07 : 0x00) | (remote ? 0x00 : 0x01)) << 5;
	cdcGeneralStatusCmd[1] = (cdcActive ? 0xFF : 0x00); // Validation for presence of six discs in the magazine
	cdcGeneralStatusCmd[2] = (cdcActive ? 0x3F : 0x01); // There are six discs in the magazine
	cdcGeneralStatusCmd[3] = (cdcActive ? 0x41 : 0x01); // ToDo: check 0x01 | (discMode << 4) | 0x01

	saabCan.sendCanFrame(GENERAL_STATUS_CDC, cdcGeneralStatusCmd);
}
