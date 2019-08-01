#include <CoreFoundation/CoreFoundation.h>
#define kMainPortName "NIHWMainHandler"
#define kAgentNotificationPortNameFormat "NIHWS%04x%04dNotification"
#define kAgentRequestPortNameFormat "NIHWS%04x%04dRequest"
#define kMikroNotificationPortNameFormat "NIHWMaschineMikroMK2-%sNotification"
#define kSerialNumberLen 9
#define kSerialNumberPacketDataLen 25
#define kAgentNotificationPortNameLen 26
#define kAgentRequestPortNameLen 21
#define kMikroPortNameLen 36
#define kNim2 0x4e694d32
#define kPrmy 0x70726d79
#define kTrue 0x74727565
#define kStrt 0x73747274
#define kInitMsgUid 40

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

struct port_name_msg_t {
    uint32_t nonce;
    uint32_t trueStr;
    uint32_t unk;
    uint32_t len;
    char     *name;
} port_name_msg;

struct serial_num_msg_t {
    uint32_t timestamp;
    uint32_t unk;
    uint32_t port_uid;
    uint32_t len;
    char     num[kSerialNumberLen];
} serial_num_msg;


uint32_t gNonces[4] = {55797590, 54818048, 54543104, 54817091};
uint16_t gPortUids[8] = {0x1300, 0x1140, 0x808, 0x1200, 0x1110, 0x1350, 0x1500, 0x1200};

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
}

void
agent_notif_port_callback(CFMessagePortRef local,
                          SInt32 msgid,
                          CFDataRef data,
                          void *info)
{
    Boolean          shouldFreeInfo;
    char             mikroNotifPortName[kMikroPortNameLen];
    CFStringRef      cfMikroNotifPortName;
    CFMessagePortRef mikroNotifPort;
    
    if (!data || CFDataGetLength(data) != kSerialNumberPacketDataLen) {
        printf("Invalid data from hardware agent\n");
        return;
    }
    
    // Interpret the data as a serial message packet
    serial_num_msg = *(struct serial_num_msg_t *)CFDataGetBytePtr(data);
    
    // Create the mikro notification port
    sprintf(mikroNotifPortName, kMikroNotificationPortNameFormat, serial_num_msg.num);
    cfMikroNotifPortName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                     mikroNotifPortName,
                                                     kCFStringEncodingASCII);
    mikroNotifPort = CFMessagePortCreateLocal(kCFAllocatorDefault,
                                              cfMikroNotifPortName,
                                              (CFMessagePortCallBack)mikro_notif_port_callback,
                                              NULL,
                                              &shouldFreeInfo);
    if (!mikroNotifPort) {
        printf("Couldn't create mikro notification port\n");
        CFRelease(cfMikroNotifPortName);
        return;
    }
}

CFMessagePortRef
getBootstrapPort()
{
    return CFMessagePortCreateRemote(kCFAllocatorDefault,
                                     CFSTR(kMainPortName));
}

void
sendMsg(CFMessagePortRef port, uint8_t *msg, size_t size)
{
    CFDataRef msgData;

    msgData = CFDataCreate(kCFAllocatorDefault,
                           msg,
                           size);
                           
    if (!msgData)
    {
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
    port_name_msg.name = name;
    port_name_msg.trueStr = kTrue;
    port_name_msg.unk = 0;
    sendMsg(port, (uint8_t *)&port_name_msg, sizeof(port_name_msg));
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
        sleep(1);
    }

    CFRelease(cfName);
    return port;
}

void
performHandshakeIteration(uint16_t portUid, uint16_t msgUid)
{
    CFMessagePortRef bsPort;
    CFMessagePortRef reqPort;
    char             reqPortName[kAgentRequestPortNameLen];
    char             notifPortName[kAgentNotificationPortNameLen];

    // Get a reference to the bootstrap port
    bsPort = getBootstrapPort();
    if (!bsPort) {
        printf("Couldn't get bootstrap port\n");
        return;
    }

    sendNonceMsg(bsPort, gNonces[0]);
    sendUidMsg(bsPort, gNonces[1], portUid);

    sprintf(reqPortName, kAgentRequestPortNameFormat, portUid, msgUid);
    reqPort = waitForRequestPort(reqPortName);
    if (!reqPort) {
        printf("Couldn't get request port: %s\n", reqPortName);
        goto req_port_fail;
    }

    sprintf(notifPortName, kAgentNotificationPortNameFormat, portUid, msgUid);
    sendNameMsg(reqPort, gNonces[2], notifPortName);
    sendNonceMsg(reqPort, gNonces[3]);

req_port_fail:
    CFRelease(bsPort);
}

void
doHandshake()
{
    size_t   iter = 0;
    uint16_t msgUid = kInitMsgUid;

    performHandshakeIteration(gPortUids[iter], msgUid);
}

int main(int argc, const char * argv[]) {
    doHandshake();
    return 0;
}
