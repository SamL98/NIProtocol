#include <niparser.h>
#include "messenger.h"

#define kButtonNonce 57439488
#define kScreenNonce 56914756

char gReceivedSerial = 0;
char gSerialNum[kSerialNumberLen] = "\0";

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
    nonce_msg_t nonce_msg = {};
    nonce_msg.nonce = nonce;
    sendMsg(port, (uint8_t *)&nonce_msg, sizeof(nonce_msg));
}

void
sendUidMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid)
{
    port_uid_msg_t port_uid_msg = {};
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
    port_name_msg_t port_name_msg = {};
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

    mk_serial_msg_t mk_serial_msg = {};
    mk_serial_msg.nonce = nonce;
    mk_serial_msg.uid = (uint32_t)uid;
    mk_serial_msg.nim2Str = kNim2;
    mk_serial_msg.prmyStr = kPrmy;
    mk_serial_msg.len = kSerialNumberLen-1;
    strncpy(mk_serial_msg.serial, gSerialNum, kSerialNumberLen-1);
    sendMsg(port, (uint8_t *)&mk_serial_msg, sizeof(mk_serial_msg));
}

void 
sendMKNameMsg(CFMessagePortRef port, uint32_t nonce, char *name)
{
    mk_port_name_msg_t mk_port_name_msg = {};
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
    cmd_msg_t cmd_msg;
    cmd_msg.nonce = nonce;
    cmd_msg.cmd = cmd;
    sendMsg(port, (uint8_t *)&cmd_msg, sizeof(cmd_msg));
}

void
sendButtonDataMsg(CFMessagePortRef port, button_data_t button_data)
{
    button_data_msg_t button_data_msg = {};
    button_data_msg.nonce = kButtonNonce;
    button_data_msg.len = kButtonDataLen;
    memcpy(button_data_msg.button_data, &button_data, kButtonDataLen);
    sendMsg(port, (uint8_t*)&button_data_msg, sizeof(button_data_msg));
}

void 
sendScreenDataMsg(CFMessagePortRef port, screen_data_t screen_data)
{
    screen_data_msg_t screen_data_msg = {};
    screen_data_msg.nonce = kScreenNonce;
    screen_data_msg.unk1 = 0x10000000;
    screen_data_msg.unk2 = 0;
    screen_data_msg.unk3 = kProjUid;
    screen_data_msg.len = kScreenDataLen;
    memcpy(screen_data_msg.screen_data, &screen_data, kScreenDataLen);
    sendMsg(port, (uint8_t*)&screen_data_msg, sizeof(screen_data_msg));
}

void
initButtonData(button_data_t *button_data)
{
    uint8_t *data_ptr = (uint8_t *)button_data;
    size_t  i;

    for (i=0; i<kButtonDataLen; i++)
        *(data_ptr+i) = 0;
}

void
initScreenData(screen_data_t *screen_data)
{
    uint8_t *data_ptr = (uint8_t *)screen_data;
    size_t  i;

    for (i=0; i<kScreenDataLen; i++)
        *(data_ptr+i) = 0;
}

void
setPadColor(button_data_t *button_data, uint32_t btn_num, char r, char g, char b)
{
    uint32_t btn_code;
    size_t   base_idx;

    btn_code = btn_num_to_code(btn_num);
    base_idx = btn_code * 3;
    button_data->pad_buttons[base_idx] = r;
    button_data->pad_buttons[base_idx+1] = g;
    button_data->pad_buttons[base_idx+2] = b;
}

void
readScreenDataFromFile(screen_data_t *screen_data, const char *fname)
{
    FILE *fp;

    if ((fp = fopen(fname, "r"))) {
		fread(screen_data, 1, kScreenDataLen, fp);
		fclose(fp);
	}
}