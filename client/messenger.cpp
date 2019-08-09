#include "messenger.h"

char gReceivedSerial = 0;
struct nonce_msg_t nonce_msg = {};
struct cmd_msg_t cmd_msg = {};
struct port_uid_msg_t port_uid_msg = {};
struct port_name_msg_t port_name_msg = {};
struct mk_serial_msg_t mk_serial_msg = {};
struct mk_port_name_msg_t mk_port_name_msg = {};
struct serial_num_msg_t serial_num_msg = {};
struct new_proj_msg_t new_proj_msg = {};
struct button_data_msg_t button_data_msg = {};
struct screen_data_msg_t screen_data_msg = {};

CFMessagePortRef
getBootstrapPort(const char *name)
{
    CFStringRef cfName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                   name,
                                                   kCFStringEncodingASCII);
    return CFMessagePortCreateRemote(kCFAllocatorDefault,
                                     cfName);
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

    if (!port || !CFMessagePortIsValid(port)) {
        printf("Either null or invalid message port\n");
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
sendMKSerialMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid)
{
    if (!gReceivedSerial) {
        printf("Do not have a valid serial number to respond with\n");
        return;
    }

    mk_serial_msg.nonce = nonce;
    mk_serial_msg.uid = (uint32_t)uid;
    mk_serial_msg.nim2Str = kNim2;
    mk_serial_msg.prmyStr = kPrmy;
    mk_serial_msg.len = kSerialNumberLen-1;
    strncpy(mk_serial_msg.serial, serial_num_msg.num, kSerialNumberLen-1);
    sendMsg(port, (uint8_t *)&mk_serial_msg, sizeof(mk_serial_msg));
}

void 
sendMKNameMsg(CFMessagePortRef port, uint32_t nonce, char *name)
{
    mk_port_name_msg.nonce = nonce;
    mk_port_name_msg.trueStr = kTrue;
    mk_port_name_msg.unk = 0x30;
    mk_port_name_msg.len = kMikroNotificationPortNameLen;
    strncpy(mk_port_name_msg.name, name, kMikroNotificationPortNameLen-1);
    sendMsg(port, (uint8_t *)&mk_port_name_msg, sizeof(mk_port_name_msg));
}

void
sendCmdMsg(CFMessagePortRef port, uint32_t nonce, uint32_t cmd)
{
    cmd_msg.nonce = nonce;
    cmd_msg.cmd = cmd;
    sendMsg(port, (uint8_t *)&cmd_msg, sizeof(cmd_msg));
}

void 
sendNewProjMsg(CFMessagePortRef port, uint32_t nonce)
{
    new_proj_msg.nonce = nonce;
    new_proj_msg.unk1 = 0;
    new_proj_msg.unk2 = 0;
    new_proj_msg.len = kNewProjLen;
    memcpy(new_proj_msg.cmd, "New Project", kNewProjLen-1);
    sendMsg(port, (uint8_t*)&new_proj_msg, sizeof(new_proj_msg));
}

void
sendButtonDataMsg(CFMessagePortRef port, uint32_t nonce, char *button_data)
{
    button_data_msg.nonce = nonce;
    button_data_msg.len = kButtonDataLen;
    memcpy(button_data_msg.button_data, button_data, kButtonDataLen);
    sendMsg(port, (uint8_t*)&button_data_msg, sizeof(button_data_msg));
}

void 
sendScreenDataMsg(CFMessagePortRef port, uint32_t nonce, char *screen_data)
{
    screen_data_msg.nonce = nonce;
    screen_data_msg.unk1 = 0x10000000;
    screen_data_msg.unk2 = 0;
    screen_data_msg.unk3 = kProjUid;
    screen_data_msg.len = kScreenDataLen;
    memcpy(screen_data_msg.screen_data, screen_data, kScreenDataLen);
    sendMsg(port, (uint8_t*)&screen_data_msg, sizeof(screen_data_msg));
}