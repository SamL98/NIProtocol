#include <nimessenger.h>
#include <ninotifier.h>
#include <nihandshaker.h>
#include "mach_callbacks.h"

#define kAgentNotificationPortNameFormat "NIHWS%04x%04dNotification"
#define kMikroNotificationPortNameFormat "NIHWMaschineMikroMK2-%s%04dNotification"
#define kMikroRequestPortNameFormat "NIHWMaschineMikroMK2-%s%04dRequest"
#define kSerialNumberPacketDataLen 25

uint16_t gMsgUid = 40;
uint32_t gSerialNonce = 54808107;
uint32_t gCmdNonce = 54808064;
CFMessagePortRef gM2NotifPort = NULL;

CFMessagePortRef
createRequestPort(const char *name, 
                  CFMessagePortCallBack callout, 
                  void *info)
{
    Boolean              shouldFreeInfo;
    CFStringRef          cfName;
	CFMessagePortRef	 port;
    CFMessagePortContext ctx;

    cfName = CFStringCreateWithCString(kCFAllocatorDefault,
                                       name,
                                       kCFStringEncodingASCII);

    ctx.version = 0;
    ctx.info = info;
    ctx.retain = NULL;//CFRetain;
    ctx.release = NULL;//CFRelease;
    ctx.copyDescription = NULL;

    port = CFMessagePortCreateLocal(kCFAllocatorDefault,
                                    cfName,
                                    callout,
                                    &ctx,
                                    &shouldFreeInfo);

	if (!shouldFreeInfo)
		free(info);

	return port;
}

CFDataRef
handleNonceMsg(uint8_t *dataPtr, size_t dataLen)
{
	if (*(uint32_t*)dataPtr != 55797590)
		return NULL;

	nonce_response_t response;
	
	response.unk1 = kNonceResponseUnk1;
	response.unk2 = kNonceResponseUnk2;

	return CFDataCreate(kCFAllocatorDefault,
						(uint8_t *)&response,
						sizeof(response));
}

CFDataRef
handleUidMsg(uint8_t *dataPtr, size_t dataLen)
{
	CFMessagePortRef reqPort;
	port_uid_msg_t	 port_uid_msg;
	char 			 reqPortName[kMikroRequestPortNameLen];
	char			 notifPortName[kMikroNotificationPortNameLen];
	uint8_t		 	 *response;
	size_t			 responseLen;

	port_uid_msg = *(port_uid_msg_t *)dataPtr;

	// Account for the weird 3rd to last handshake iteration
	if (port_uid_msg.uid == 0x1350)
		return NULL;

	// Account for the difference during the last 
	else if (port_uid_msg.uid == 0x1200 && dataLen == sizeof(mk_serial_msg_t)) {
		serial_response_t response_st;
		mk_serial_msg_t   mk_serial_msg;
		
		mk_serial_msg = *(mk_serial_msg_t *)dataPtr;

		sprintf(reqPortName, kMikroRequestPortNameFormat, mk_serial_msg.serial, gMsgUid);
		reqPortName[kMikroRequestPortNameLen-1] = 0;

		sprintf(notifPortName, kMikroNotificationPortNameFormat, mk_serial_msg.serial, gMsgUid);
		notifPortName[kMikroNotificationPortNameLen-1] = 0;

		response_st.trueStr = kTrue;
		response_st.reqPortNameLen = kMikroRequestPortNameLen;
		strncpy(response_st.reqPortName, reqPortName, kMikroRequestPortNameLen);
		response_st.notifPortNameLen = kMikroNotificationPortNameLen;
		strncpy(response_st.notifPortName, notifPortName, kMikroNotificationPortNameLen);
		response_st.unk = 0;

		response = (uint8_t *)&response_st;
		responseLen = sizeof(response_st);
	}
	else {
		port_uid_response_t response_st;

		sprintf(reqPortName, kAgentRequestPortNameFormat, port_uid_msg.uid, gMsgUid);
		reqPortName[kAgentRequestPortNameLen-1] = 0;

		sprintf(notifPortName, kAgentNotificationPortNameFormat, port_uid_msg.uid, gMsgUid);
		notifPortName[kAgentNotificationPortNameLen-1] = 0;

		response_st.trueStr = kTrue;
		response_st.reqPortNameLen = kAgentRequestPortNameLen;
		strncpy(response_st.reqPortName, reqPortName, kAgentRequestPortNameLen);
		response_st.notifPortNameLen = kAgentNotificationPortNameLen;
		strncpy(response_st.notifPortName, notifPortName, kAgentNotificationPortNameLen);
		response_st.unk = 0;

		response = (uint8_t *)&response_st;
		responseLen = sizeof(response_st);
	}

	if (!(reqPort = createRequestPort(reqPortName, 
									  (CFMessagePortCallBack)req_port_callback, 
									  NULL))) {
		printf("Couldn't create request port: %s\n", reqPortName);
		exit(1);
	}

	CFMessagePortSetDispatchQueue(reqPort, dispatch_get_main_queue());
	++gMsgUid;

    return CFDataCreate(kCFAllocatorDefault,
						response,
						responseLen);
}

CFDataRef
handleNameMsg(uint8_t *dataPtr, size_t dataLen)
{
	port_name_response_t response;

	response.trueStr = kTrue;

	return CFDataCreate(kCFAllocatorDefault,
						(uint8_t *)&response,
						sizeof(response));
}

CFDataRef
bootstrap_req_port_callback(CFMessagePortRef local, 
                            SInt32 msgid, 
                            CFDataRef data, 
                            void *info)
{
	uint8_t	*dataPtr;
	size_t  dataLen;

    if (!data || !(dataPtr = (uint8_t *)CFDataGetBytePtr(data))) {
        printf("No data from hardware agent\n");
        return NULL;
    }

	dataLen = CFDataGetLength(data);

	switch (dataLen) {
		case sizeof(nonce_msg_t): return handleNonceMsg(dataPtr, dataLen);
		case sizeof(port_uid_msg_t): return handleUidMsg(dataPtr, dataLen);
		case sizeof(mk_serial_msg_t): return handleUidMsg(dataPtr, dataLen);
		default:
			printf("Don't know how to handle %lu bytes of data\n", dataLen);
			return NULL;
	}
}

CFDataRef
req_port_callback(CFMessagePortRef local,
                  SInt32 msgid,
                  CFDataRef data,
                  void *info)
{
    uint8_t *dataPtr;
	size_t  dataLen;

    if (!data || !(dataPtr = (uint8_t *)CFDataGetBytePtr(data))) {
        printf("No data in request port callback\n");
        return NULL;
    }

	dataLen = CFDataGetLength(data);

	if (dataLen == sizeof(port_name_msg_t)) 
	{
		CFMessagePortRef notifPort;
		serial_num_msg_t serial_num_msg;
		port_name_msg_t  port_name_msg;
		
		port_name_msg = *(port_name_msg_t *)dataPtr;

		if (!strcmp(port_name_msg.name, "NIHWS12000043Notification")) 
		{
			serial_num_msg.nonce = gSerialNonce;
			serial_num_msg.unk = 0;
			serial_num_msg.port_uid = 0x1200;
			serial_num_msg.len = 9;
			strncpy(serial_num_msg.num, "88CC589C", kSerialNumberLen);
			serial_num_msg.num[kSerialNumberLen-1] = 0;

			if (!(notifPort = getBootstrapPort(port_name_msg.name))) {
				printf("Couldn't get notification port from name: %s\n", port_name_msg.name);
				return NULL;
			}

			printf("Sending serial\n");
			sendMsg(notifPort, (uint8_t*)&serial_num_msg, sizeof(serial_num_msg));
		}
	}
	else if (dataLen == sizeof(mk_port_name_msg_t)) {
		CFMessagePortRef   notifPort;
		mk_port_name_msg_t mk_port_name_msg;
		cmd_msg_t		   cmd_msg;

		mk_port_name_msg = *(mk_port_name_msg_t *)dataPtr;
		if (!(gM2NotifPort = getBootstrapPort(mk_port_name_msg.name))) {
			printf("Couldn't get mk notification port from name: %s\n", mk_port_name_msg.name);
			return NULL;
		}

		printf("Obtained reference to notification port: %s\n", mk_port_name_msg.name);

		cmd_msg.nonce = gCmdNonce;
		cmd_msg.cmd = kTrue;

		sendMsg(gM2NotifPort, (uint8_t*)&cmd_msg, sizeof(cmd_msg));
	}
	else if (dataLen == sizeof(cmd_msg_t)) {
		if (!gM2NotifPort)
			return NULL;

		cmd_msg_t recv_cmd_msg;
		cmd_msg_t send_cmd_msg;

		recv_cmd_msg = *(cmd_msg_t *)dataPtr;
		if (recv_cmd_msg.cmd != kStrt)
			return NULL;

		send_cmd_msg.nonce = 54742528;
		send_cmd_msg.cmd = kTrue;

		sendMsg(gM2NotifPort, (uint8_t*)&send_cmd_msg, sizeof(send_cmd_msg));
	}
	else {
		printf("Don't know how to handle %lu bytes\n", dataLen);
		return NULL;
	}
    
    return handleNameMsg(dataPtr, dataLen);
}