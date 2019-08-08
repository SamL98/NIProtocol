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
#define kNumRepRepNonce 4

uint32_t gNonces[4] = {55797590, 54818048, 54543104, 54817091};
uint32_t gSerialNonce = 54806784;
uint32_t gSpecialEndNonce = 54749011;
uint32_t gStartNonce = 54739712;
uint32_t gNewProjNonce = 55145294;
uint32_t gNullNonce = 55145300;
uint32_t gProjNonce = 56914756;
uint32_t gRepNonce = 57439488;
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

int
getMsgData(char *dataPtr, char *fname, size_t len)
{
    FILE   *fp;
    size_t bytesRead;

    fp = fopen(fname, "r");
    if (!fp) {
        printf("Couldn't open %s\n", fname);
        return 1;
    }

    bytesRead = fread(dataPtr, 1, len, fp);
    if (bytesRead < len) {
        printf("Could only read %lu bytes from %s\n", bytesRead, fname);
        return 1;
    }

    return 0;
}

void
doHandshake()
{
    CFMessagePortRef reqPort;
    uint16_t msgUid = kInitMsgUid;
    size_t   i;
    char     *nullProjData;
    char     projData[kLenProjData];
    char     repDataNull[kLenRepData];
    char     repData[kLenRepData];
    int      dataStatus;

    for (i=0; i<kNumHandshakeIter; i++) {
        performHandshakeIteration(gPortUids[i], msgUid, i);
        if (i != kNumHandshakeIter-3)
            ++msgUid;
    }

    reqPort = getRequestPort(0, msgUid-1, i-1);
    if (!reqPort) {
        printf("Couldn't get request port\n");
        return;
    }

    nullProjData = (char*)calloc(kLenProjData, sizeof(char));
    if (!nullProjData) {
        printf("Couldn't allocate null project data\n");
        return;
    }

    dataStatus = getMsgData(projData, "proj_data.bin", kLenProjData);
    if (dataStatus) {
        printf("Couldn't get project data\n");
        return;
    }

    dataStatus = getMsgData(repDataNull, "rep_data_null.bin", kLenRepData);
    if (dataStatus) {
        printf("Couldn't get null rep data\n");
        return;
    }

    dataStatus = getMsgData(repData, "rep_data.bin", kLenRepData);
    if (dataStatus) {
        printf("Couldn't get rep data\n");
        return;
    }

    //sendNewProjMsg(reqPort, gNewProjNonce);
    sendCmdMsg(reqPort, gStartNonce, kStrt);
    // sendNewProjMsg(reqPort, gNewProjNonce);
    // sendCmdMsg(reqPort, gNullNonce, 0);
    // sendProjMsg(reqPort, gProjNonce, nullProjData);

    // for (i=0; i<3; i++)
    //     sendRepMsg(reqPort, gRepNonce, repDataNull);

    //sendRepMsg(reqPort, gRepNonce, repData);
    sendProjMsg(reqPort, gProjNonce, projData);

    free(nullProjData);
}

int main(int argc, const char * argv[]) {
    doHandshake();
    CFRunLoopRun();
    return 0;
}
