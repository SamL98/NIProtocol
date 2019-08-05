#include <CoreFoundation/CoreFoundation.h>
#include "client.h"

#define kMainPortName "NIHWMainHandler"
#define kAgentNotificationPortNameFormat "NIHWS%04x%04dNotification"
#define kAgentRequestPortNameFormat "NIHWS%04x%04dRequest"
#define kMikroNotificationPortNameFormat "NIHWMaschineMikroMK2-%sxxxxNotification"
#define kSerialNumberLen 9
#define kSerialNumberPacketDataLen 25
#define kAgentNotificationPortNameLen 26
#define kAgentRequestPortNameLen 21
#define kMikroPortNameLen 46
#define kNim2 0x4e694d32
#define kPrmy 0x70726d79
#define kTrue 0x74727565
#define kStrt 0x73747274
#define kInitMsgUid 40
#define kNumHandshakeIter 8

struct nonce_msg_t {
    uint32_t nonce;
} nonce_msg;

struct cmd_msg_t {
    uint32_t nonce;
    uint32_t cmd;
} cmd_msg;

struct port_uid_msg_t {
    uint32_t nonce;
    uint32_t uid;
    uint32_t nim2Str;
    uint32_t prmyStr;
    uint32_t unk;
} port_uid_msg;

struct __attribute__((packed)) port_name_msg_t {
    uint32_t nonce;
    uint32_t trueStr;
    uint32_t unk;
    uint32_t len;
    char     name[kAgentNotificationPortNameLen];
} port_name_msg;

struct __attribute__((packed)) mk_port_name_msg_t {
    uint32_t nonce;
    uint32_t trueStr;
    uint32_t unk;
    uint32_t len;
    char     name[kMikroPortNameLen];
} mk_port_name_msg;

struct serial_num_msg_t {
    uint32_t timestamp;
    uint32_t unk;
    uint32_t port_uid;
    uint32_t len;
    char     num[kSerialNumberLen];
} serial_num_msg;


char gReceivedSerial = 0;
uint32_t gNonces[4] = {55797590, 54818048, 54543104, 54817091};
uint16_t gPortUids[kNumHandshakeIter] = {0x1300, 0x1140, 0x808, 0x1200, 0x1110, 0x1350, 0x1500, 0x1200};
maschine_callback gUserDefinedCallback = NULL;

void
registerCallback(maschine_callback callback)
{
    gUserDefinedCallback = callback;
}

void
mikro_notif_port_callback(CFMessagePortRef local,
                          SInt32 msgid,
                          CFDataRef data,
                          void *info)
{
    if (!data) {
        printf("No data from hardware agent\n");
        return;
    }
    
    printf("Received %ld bytes of data\n", (long)CFDataGetLength(data));

    if (gUserDefinedCallback)
        gUserDefinedCallback((uint32_t*)CFDataGetBytePtr(data), CFDataGetLength(data));
}

CFMessagePortRef
getBootstrapPort()
{
    return CFMessagePortCreateRemote(kCFAllocatorDefault,
                                     CFSTR(kMainPortName));
}

void
invalidation_callback(CFMessagePortRef port, void *info)
{
	CFStringRef portName = CFMessagePortGetName(port);
	if (!portName)
		return;

	printf("%s invalidated\n", CFStringGetCStringPtr(portName, kCFStringEncodingASCII));
	CFRelease(portName);
}

void
sendMsg(CFMessagePortRef port, uint8_t *msg, size_t size)
{
    CFDataRef msgData;

    msgData = CFDataCreate(kCFAllocatorDefault,
                           msg,
                           size);
                           
    if (!msgData) {
        printf("Couldn't create message data\n");
        return;
    }

    CFMessagePortSendRequest(port,
                             0,
                             msgData,
                             1000,
                             1000,
                             kCFRunLoopDefaultMode,
                             NULL);

    CFRelease(msgData);
}

void
sendNonceMsg(CFMessagePortRef port, uint32_t nonce)
{
    nonce_msg.nonce = nonce;
    sendMsg(port, (uint8_t *)&nonce_msg, sizeof(nonce_msg));
}

void
sendUidMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid)
{
    port_uid_msg.nonce = nonce;
    port_uid_msg.uid = (uint32_t)uid;
    port_uid_msg.nim2Str = kNim2;
    port_uid_msg.prmyStr = kPrmy;
    port_uid_msg.unk = 0;
    sendMsg(port, (uint8_t *)&port_uid_msg, sizeof(port_uid_msg));
}

void
sendNameMsg(CFMessagePortRef port, uint32_t nonce, char *name)
{
    port_name_msg.nonce = nonce;
	strncpy(port_name_msg.name, name, kAgentNotificationPortNameLen-1);
    port_name_msg.trueStr = kTrue;
    port_name_msg.unk = 0;
    port_name_msg.len = kAgentNotificationPortNameLen;
    sendMsg(port, (uint8_t *)&port_name_msg, sizeof(port_name_msg));
}

void 
sendMKNameMsg(CFMessagePortRef port, uint32_t nonce, char *name)
{
    mk_port_name_msg.nonce = nonce;
    strncpy(mk_port_name_msg.name, name, kMikroPortNameLen - 1);
    mk_port_name_msg.trueStr = kTrue;
    mk_port_name_msg.unk = 0x30;
    mk_port_name_msg.len = kMikroPortNameLen;
    sendMsg(port, (uint8_t *)&mk_port_name_msg, sizeof(mk_port_name_msg));
}

CFMessagePortRef
createNotificationPort(char *name, CFMessagePortCallBack callout)
{
    Boolean shouldFreeInfo;
    CFStringRef cfName;

    printf("Creating port %s\n", name);

    cfName = CFStringCreateWithCString(kCFAllocatorDefault,
                                       name,
                                       kCFStringEncodingASCII);

    return CFMessagePortCreateLocal(kCFAllocatorDefault,
                                    cfName,
                                    callout,
                                    NULL,
                                    &shouldFreeInfo);
}

void agent_notif_port_callback(CFMessagePortRef local,
                               SInt32 msgid,
                               CFDataRef data,
                               void *info)
{
    Boolean shouldFreeInfo;
    char mikroNotifPortName[kMikroPortNameLen];
    CFStringRef cfAgentNotifPortName;
    char *agentNotifPortName;
    CFMessagePortRef mikroNotifPort;

    if (!data || CFDataGetLength(data) != kSerialNumberPacketDataLen)
    {
        printf("Invalid data from hardware agent\n");
        return;
    }

    cfAgentNotifPortName = CFMessagePortGetName(local);
    if (!cfAgentNotifPortName)
    {
        printf("Couldn't get port name for agent notification port\n");
        return;
    }

    gReceivedSerial = 1;
    agentNotifPortName = (char *)CFStringGetCStringPtr(cfAgentNotifPortName,
                                                       kCFStringEncodingASCII);

    // Interpret the data as a serial message packet
    serial_num_msg = *(struct serial_num_msg_t *)CFDataGetBytePtr(data);

    // Create the mikro notification port
    sprintf(mikroNotifPortName, kMikroNotificationPortNameFormat, serial_num_msg.num);

    // This is extremely hacky and bad practice but I could've figure out how to pass
    // the message uid as context when creating the message port.
    memcpy(mikroNotifPortName + 29, agentNotifPortName + 9, 4);

    mikroNotifPort = createNotificationPort(mikroNotifPortName,
                                            (CFMessagePortCallBack)mikro_notif_port_callback);
    if (!mikroNotifPort)
    {
        printf("Couldn't create mikro notification port\n");
        CFRelease(cfMikroNotifPortName);
        return;
    }

    CFMessagePortInvalidate(local);
    CFMessagePortSetDispatchQueue(mikroNotifPort, dispatch_get_main_queue());
}

CFMessagePortRef
waitForRequestPort(char *name)
{
    CFStringRef      cfName;
    CFMessagePortRef port = NULL;
    size_t           tries = 0;
    size_t           max_tries = 10;

    cfName = CFStringCreateWithCString(kCFAllocatorDefault,
                                       name,
                                       kCFStringEncodingASCII);
    if (!cfName) {
        printf("Couldn't create CFStringRef for string %s\n", name);
        return NULL;
    }

    while (!port && tries++ < max_tries) {
        port = CFMessagePortCreateRemote(kCFAllocatorDefault, cfName);
		if (!port) sleep(1);
    }

    CFRelease(cfName);
    return port;
}

void
performHandshakeIteration(uint16_t portUid, uint16_t msgUid)
{
    CFMessagePortRef bsPort;
    CFMessagePortRef reqPort;
    CFMessagePortRef notifPort;
    char             reqPortName[kAgentRequestPortNameLen];
    char             notifPortName[kAgentNotificationPortNameLen];

    // Get a reference to the bootstrap port
    bsPort = getBootstrapPort();
    if (!bsPort) {
        printf("Couldn't get bootstrap port\n");
        return;
    }

    // Send the initial nonce and the uid
    sendNonceMsg(bsPort, gNonces[0]);
    sendUidMsg(bsPort, gNonces[1], portUid);

    // Wait for the hardware agent to open the request port corresponding to the port and msg uid's
    sprintf(reqPortName, kAgentRequestPortNameFormat, portUid, msgUid);
    reqPort = waitForRequestPort(reqPortName);
    if (!reqPort) {
        printf("Couldn't get request port\n");
        goto req_port_fail;
    }

    // Create the notification port corresponding to the current port and msg uid's
    sprintf(notifPortName, kAgentNotificationPortNameFormat, portUid, msgUid);
    notifPort = createNotificationPort(notifPortName, (CFMessagePortCallBack)agent_notif_port_callback);
    if (!notifPort) {
        printf("Couldn't create notification port\n");
        goto notif_port_fail;
    }

	CFMessagePortSetInvalidationCallBack(notifPort, 
										 (CFMessagePortInvalidationCallBack)invalidation_callback);
	CFMessagePortSetDispatchQueue(notifPort,
								  dispatch_get_main_queue());

    // Let the hardware agent know the name of our notification port and send the final nonce
    sendNameMsg(reqPort, gNonces[2], notifPortName);
    sendNonceMsg(reqPort, gNonces[3]);

notif_port_fail:
    CFRelease(reqPort);
req_port_fail:
    CFRelease(bsPort);
}

void
doHandshake()
{
    uint16_t msgUid = kInitMsgUid;
    size_t   i;

    for (i=0; i<kNumHandshakeIter && !gReceivedSerial; i++) {
        performHandshakeIteration(gPortUids[i], msgUid);
        ++msgUid;
    }
}

int main(int argc, const char * argv[]) {
    doHandshake();
    while (1)
        usleep(100);
    return 0;
}
