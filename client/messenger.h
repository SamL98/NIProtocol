#include <CoreFoundation/CoreFoundation.h>

#define kNim2 0x4e694d32
#define kPrmy 0x70726d79
#define kTrue 0x74727565
#define kStrt 0x73747274
#define kSerialNumberLen 9
#define kAgentNotificationPortNameLen 26
#define kMikroNotificationPortNameLen 46
#define kMikroRequestPortNameLen 41
#define kNewProjLen 12
#define kSelBtn 0x1
#define kProjUid 0x800040
#define kLenProjData 1024
#define kLenRepData 78

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

struct __attribute__((packed)) new_proj_msg_t {
	uint32_t nonce;
	uint32_t unk1;
	uint32_t unk2;
	uint32_t len;
	char 	 cmd[kNewProjLen];
};

struct __attribute__((packed)) rep_msg_t {
    uint32_t nonce;
    uint32_t len;
    char     rep_data[kLenRepData];
};

struct __attribute__((packed)) proj_msg_t {
	uint32_t nonce;
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
    uint32_t len;
    char     proj_data[kLenProjData];
};

extern char gReceivedSerial;
extern struct nonce_msg_t nonce_msg;
extern struct cmd_msg_t cmd_msg;
extern struct port_uid_msg_t port_uid_msg;
extern struct port_name_msg_t port_name_msg;
extern struct mk_serial_msg_t mk_serial_msg;
extern struct mk_port_name_msg_t mk_port_name_msg;
extern struct serial_num_msg_t serial_num_msg;
extern struct new_proj_msg_t new_proj_msg;
extern struct rep_msg_t rep_msg;
extern struct proj_msg_t proj_msg;

void sendNonceMsg(CFMessagePortRef port, uint32_t nonce);
void sendUidMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid);
void sendNameMsg(CFMessagePortRef port, uint32_t nonce, char *name);
void sendMKSerialMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid);
void sendMKNameMsg(CFMessagePortRef port, uint32_t nonce, char *name);
void sendCmdMsg(CFMessagePortRef port, uint32_t nonce, uint32_t cmd);
void sendNewProjMsg(CFMessagePortRef port, uint32_t nonce);
void sendRepMsg(CFMessagePortRef port, uint32_t nonce, char *in_rep_data);
void sendProjMsg(CFMessagePortRef port, uint32_t nonce, char *in_proj_data);