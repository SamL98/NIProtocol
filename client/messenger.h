#include <CoreFoundation/CoreFoundation.h>

#define kNim2 0x4e694d32
#define kPrmy 0x70726d79
#define kTrue 0x74727565
#define kStrt 0x73747274
#define kSerialNumberLen 9
#define kAgentNotificationPortNameLen 26
#define kMikroNotificationPortNameLen 46
#define kMikroRequestPortNameLen 41

struct nonce_msg_t {
    uint32_t nonce;
};

struct cmd_msg_t {
    uint32_t nonce;
    uint32_t cmd;
};

struct port_uid_msg_t {
    uint32_t nonce;
    uint32_t uid;
    uint32_t nim2Str;
    uint32_t prmyStr;
    uint32_t unk;
};

struct __attribute__((packed)) port_name_msg_t {
    uint32_t nonce;
    uint32_t trueStr;
    uint32_t unk;
    uint32_t len;
    char     name[kAgentNotificationPortNameLen];
};

struct __attribute__((packed)) mk_serial_msg_t {
	uint32_t nonce;
	uint32_t uid;
	uint32_t nim2Str;
	uint32_t prmyStr;
	uint32_t len;
	char 	 serial[kSerialNumberLen];
};

struct __attribute__((packed)) mk_port_name_msg_t {
    uint32_t nonce;
    uint32_t trueStr;
    uint32_t unk;
    uint32_t len;
    char     name[kMikroNotificationPortNameLen];
};

struct serial_num_msg_t {
    uint32_t timestamp;
    uint32_t unk;
    uint32_t port_uid;
    uint32_t len;
    char     num[kSerialNumberLen];
};

extern char gReceivedSerial;
extern struct nonce_msg_t nonce_msg;
extern struct cmd_msg_t cmd_msg;
extern struct port_uid_msg_t port_uid_msg;
extern struct port_name_msg_t port_name_msg;
extern struct mk_serial_msg_t mk_serial_msg;
extern struct mk_port_name_msg_t mk_port_name_msg;
extern struct serial_num_msg_t serial_num_msg;

void sendNonceMsg(CFMessagePortRef port, uint32_t nonce);
void sendUidMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid);
void sendNameMsg(CFMessagePortRef port, uint32_t nonce, char *name);
void sendMKSerialMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid);
void sendMKNameMsg(CFMessagePortRef port, uint32_t nonce, char *name);