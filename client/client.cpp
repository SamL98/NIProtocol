#include <nimessenger.h>
#include "callbacks.h"

#define kMainPortName "NIHWMainHandler"
#define kAgentNotificationPortNameFormat "NIHWS%04x%04dNotification"
#define kAgentRequestPortNameFormat "NIHWS%04x%04dRequest"
#define kMikroNotificationPortNameFormat "NIHWMaschineMikroMK2-%s%04dNotification"
#define kMikroRequestPortNameFormat "NIHWMaschineMikroMK2-%s%04dRequest"
#define kAgentRequestPortNameLen 21
#define kInitMsgUid 40
#define kNumHandshakeIter 8
#define kNumRepRepNonce 4

uint32_t gNonces[4] = {55797590, 54818048, 54543104, 54817091};
uint32_t gSerialNonce = 54806784;
uint32_t gSpecialEndNonce = 54749011;
uint32_t gStartNonce = 54739712;
uint16_t gPortUids[kNumHandshakeIter] = {0x1300, 0x1140, 0x808, 0x1200, 0x1110, 0x1350, 0x1500, 0x1200};

CFMessagePortRef
waitForRequestPort(char *name)
{
    // Try 10 times to obtain a reference to hardware agent's request port.
    // Wait 1 second between tries. Usually don't need all 10 tries.

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
    // If we are on the last handshake iteration, we should've sent the request
    // port should be specified by the serial number and msgUid.
    //
    // Otherwise, it should be specified by the portUid and msgUid.

    char *reqPortNamePtr;

    if (handshakeIter == kNumHandshakeIter-1) {
        if (!gReceivedSerial) {
            printf("No serial to use while getting reference to message port\n");
            return NULL;
        }

        char reqPortName[kMikroRequestPortNameLen];
        sprintf(reqPortName, kMikroRequestPortNameFormat, gSerialNum, msgUid);
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
    // Similarly to the notification port, the name of the notification port
    // to open depends on whether or not we are on the last iteration of the handshake.

    CFMessagePortCallBack callback;

    if (handshakeIter == kNumHandshakeIter-1) {
        if (!gReceivedSerial) {
            printf("No serial to use while creating mikro notification port\n");
            return NULL;
        }

        char notifPortName[kMikroNotificationPortNameLen];
        sprintf(notifPortName, kMikroNotificationPortNameFormat, gSerialNum, msgUid);

        memcpy(name, notifPortName, kMikroNotificationPortNameLen);
        callback = (CFMessagePortCallBack)mikro_notif_port_callback;
    }
    else {
        char notifPortName[kAgentNotificationPortNameLen];
        sprintf(notifPortName, kAgentNotificationPortNameFormat, portUid, msgUid);

        memcpy(name, notifPortName, kAgentNotificationPortNameLen);
        callback = (CFMessagePortCallBack)agent_notif_port_callback;
    }

    return createNotificationPort((const char*)name, callback, NULL);
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
    bsPort = getBootstrapPort(kMainPortName);
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

CFMessagePortRef
doHandshake()
{
    CFMessagePortRef reqPort;
    uint16_t msgUid = kInitMsgUid;
    size_t   i;

    for (i=0; i<kNumHandshakeIter; i++) {
        performHandshakeIteration(gPortUids[i], msgUid, i);
        if (i != kNumHandshakeIter-3) ++msgUid;
    }

    reqPort = getRequestPort(0, msgUid-1, i-1);
    if (!reqPort) {
        printf("Couldn't get request port\n");
        return NULL;
    }

    // Send the final message to the MK2 to start
    sendCmdMsg(reqPort, gStartNonce, kStrt);

    return reqPort;
}

void
setupMK2(CFMessagePortRef reqPort)
{
    button_data_t button_data;
    screen_data_t screen_data;
    size_t        i;
    FILE          *fp;  

    initButtonData(&button_data);
    setPadColor(&button_data, 1, (char)255, (char)0, (char)0);

    initScreenData(&screen_data);
    readScreenDataFromFile(&screen_data, "initials_bitwise.bin");

    sendButtonDataMsg(reqPort, button_data);
    sendScreenDataMsg(reqPort, screen_data);
}

int main(int argc, const char * argv[]) {
    CFMessagePortRef reqPort,
                     notifPort;

    if (!(reqPort = doHandshake())) {
        printf("Couldn't obtain request port from handshake\n");
        return 1;
    }

    printf("Finished handshake\n");
    setupMK2(reqPort);
    printf("Performed initial MK2 setup\n");

    if (!(notifPort = createNotificationPort(kSLBootstrapPortName, 
                                             (CFMessagePortCallBack)bootstrap_notif_port_callback,
                                             (void *)reqPort))) {
        printf("Couldn't create SL bootstrap notification port\n");
        return 1;
    }

    printf("Created %s bootstrap port\n", kSLBootstrapPortName);
    CFMessagePortSetDispatchQueue(notifPort,
								  dispatch_get_main_queue());

    CFRunLoopRun();

    return 0;
}
