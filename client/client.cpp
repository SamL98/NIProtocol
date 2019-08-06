#include "messenger.h"
#include "callbacks.h"

#define kMainPortName "NIHWMainHandler"
#define kAgentNotificationPortNameFormat "NIHWS%04x%04dNotification"
#define kAgentRequestPortNameFormat "NIHWS%04x%04dRequest"
#define kMikroNotificationPortNameFormat "NIHWMaschineMikroMK2-%s%04dNotification"
#define kMikroRequestPortNameFormat "NIHWMaschineMikroMK2-%s%04dRequest"
#define kAgentRequestPortNameLen 21
#define kInitMsgUid 40
#define kNumHandshakeIter 8


uint32_t gNonces[4] = {55797590, 54818048, 54543104, 54817091};
uint32_t gSerialNonce = 54806784;
uint32_t gSpecialEndNonce = 54749011;
uint16_t gPortUids[kNumHandshakeIter] = {0x1300, 0x1140, 0x808, 0x1200, 0x1110, 0x1350, 0x1500, 0x1200};

CFMessagePortRef
getBootstrapPort()
{
    return CFMessagePortCreateRemote(kCFAllocatorDefault,
                                     CFSTR(kMainPortName));
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

CFMessagePortRef
getRequestPort(uint16_t portUid, uint16_t msgUid, size_t handshakeIter)
{
    char *reqPortNamePtr;

    if (handshakeIter == kNumHandshakeIter-1) {
        if (!gReceivedSerial) {
            printf("No serial to use while getting reference to message port\n");
            return NULL;
        }

        char reqPortName[kMikroRequestPortNameLen];
        sprintf(reqPortName, kMikroRequestPortNameFormat, serial_num_msg.num, msgUid);
        reqPortNamePtr = reqPortName;
    }
    else {
        char reqPortName[kAgentRequestPortNameLen];
        sprintf(reqPortName, kAgentRequestPortNameFormat, portUid, msgUid);
        reqPortNamePtr = reqPortName;
    }

    // Wait for the hardware agent to open the request port corresponding to the port and msg uid's
    return waitForRequestPort(reqPortNamePtr);
}

CFMessagePortRef
setNotificationPort(char *name, uint16_t portUid, uint16_t msgUid, size_t handshakeIter)
{
    CFMessagePortCallBack callback;

    if (handshakeIter == kNumHandshakeIter-1) {
        if (!gReceivedSerial) {
            printf("No serial to use while creating mikro notification port\n");
            return NULL;
        }

        char notifPortName[kMikroNotificationPortNameLen];
        sprintf(notifPortName, kMikroNotificationPortNameFormat, serial_num_msg.num, msgUid);

        memcpy(name, notifPortName, kMikroNotificationPortNameLen);
        callback = (CFMessagePortCallBack)mikro_notif_port_callback;
    }
    else {
        char notifPortName[kAgentNotificationPortNameLen];
        sprintf(notifPortName, kAgentNotificationPortNameFormat, portUid, msgUid);

        memcpy(name, notifPortName, kAgentNotificationPortNameLen);
        callback = (CFMessagePortCallBack)agent_notif_port_callback;
    }

    return createNotificationPort(name, callback);
}

void
performHandshakeIteration(uint16_t portUid, 
                          uint16_t msgUid, 
                          size_t handshakeIter)
{
    CFMessagePortRef bsPort;
    CFMessagePortRef reqPort;
    CFMessagePortRef notifPort;
    char             notifPortName[kMikroNotificationPortNameLen];
    uint32_t         finalNonce;

    // Get a reference to the bootstrap port
    bsPort = getBootstrapPort();
    if (!bsPort) {
        printf("Couldn't get bootstrap port\n");
        return;
    }

    // Send the initial nonce and the uid
    sendNonceMsg(bsPort, gNonces[0]);

    // On the last handshake iteration, we send back the serial number we previously received
    if (handshakeIter == kNumHandshakeIter-1) 
    {
        if (!gReceivedSerial) {
            printf("Don't have a valid serial to respond with\n");
            return;
        }
        sendMKSerialMsg(bsPort, gSerialNonce, portUid);
    }
    else
        sendUidMsg(bsPort, gNonces[1], portUid);

    // I have no clue why they do this but they do
    if (handshakeIter == kNumHandshakeIter-3)
        goto req_port_fail;

    // Hopefully get a reference to the created request port
    reqPort = getRequestPort(portUid, msgUid, handshakeIter);
    if (!reqPort) {
        printf("Couldn't get request port\n");
        goto req_port_fail;
    }

    // Create the notification port corresponding to the current port and msg uid's
    notifPort = setNotificationPort(notifPortName, portUid, msgUid, handshakeIter);
    if (!notifPort) {
        printf("Couldn't create notification port\n");
        goto notif_port_fail;
    }

	CFMessagePortSetInvalidationCallBack(notifPort, 
										 (CFMessagePortInvalidationCallBack)invalidation_callback);
	CFMessagePortSetDispatchQueue(notifPort,
								  dispatch_get_main_queue());

    // Let the hardware agent know the name of our notification port and send the final nonce
    if (handshakeIter == kNumHandshakeIter-1) {
        sendMKNameMsg(reqPort, gNonces[2], notifPortName);
        finalNonce = gSpecialEndNonce;
    }
    else {
        sendNameMsg(reqPort, gNonces[2], notifPortName);
        finalNonce = gNonces[3];
    }

    sendNonceMsg(reqPort, finalNonce);

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

    for (i=0; i<kNumHandshakeIter; i++) {
        performHandshakeIteration(gPortUids[i], msgUid, i);
        if (i != kNumHandshakeIter-3)
            ++msgUid;
    }
}

int main(int argc, const char * argv[]) {
    doHandshake();
    while (1)
        usleep(100);
    return 0;
}
