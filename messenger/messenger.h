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
#define kScreenDataLen 1024
#define kButtonDataLen 78
#define kPadButtonLen 48
#define kPadButtonOffset 30

typedef struct {
    uint32_t nonce;
} nonce_msg_t;

typedef struct {
    uint32_t nonce;
    uint32_t cmd;
} cmd_msg_t;

typedef struct {
    uint32_t nonce;
    uint32_t uid;
    uint32_t nim2Str;
    uint32_t prmyStr;
    uint32_t unk;
} port_uid_msg_t;

typedef struct __attribute__((packed)) {
    uint32_t nonce;
    uint32_t trueStr;
    uint32_t unk;
    uint32_t len;
    char     name[kAgentNotificationPortNameLen];
} port_name_msg_t;

typedef struct __attribute__((packed)) {
	uint32_t nonce;
	uint32_t uid;
	uint32_t nim2Str;
	uint32_t prmyStr;
	uint32_t len;
	char 	 serial[kSerialNumberLen];
} mk_serial_msg_t;

typedef struct __attribute__((packed)) {
    uint32_t nonce;
    uint32_t trueStr;
    uint32_t unk;
    uint32_t len;
    char     name[kMikroNotificationPortNameLen];
} mk_port_name_msg_t;

typedef struct __attribute__((packed)) {
    uint32_t timestamp;
    uint32_t unk;
    uint32_t port_uid;
    uint32_t len;
    char     num[kSerialNumberLen];
} serial_num_msg_t;

typedef struct __attribute__((packed)) {
    uint32_t nonce;
    uint32_t len;
    char     button_data[kButtonDataLen];
} button_data_msg_t;

typedef struct __attribute__((packed)) {
	uint32_t nonce;
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
    uint32_t len;
    char     screen_data[kScreenDataLen];
} screen_data_msg_t;

typedef struct __attribute__((packed)) {
	char ctrl_buttons[kPadButtonOffset];
	char pad_buttons[kPadButtonLen];
} button_data_t;

typedef struct {
    char pixels[kScreenDataLen];
} screen_data_t;

extern char gReceivedSerial;
extern char gSerialNum[kSerialNumberLen];

CFMessagePortRef getBootstrapPort(const char *name);
void sendNonceMsg(CFMessagePortRef port, uint32_t nonce);
void sendUidMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid);
void sendNameMsg(CFMessagePortRef port, uint32_t nonce, char *name);
void sendMKSerialMsg(CFMessagePortRef port, uint32_t nonce, uint16_t uid);
void sendMKNameMsg(CFMessagePortRef port, uint32_t nonce, char *name);
void sendCmdMsg(CFMessagePortRef port, uint32_t nonce, uint32_t cmd);
void sendButtonDataMsg(CFMessagePortRef port, button_data_t button_data);
void sendScreenDataMsg(CFMessagePortRef port, screen_data_t screen_data);

void initButtonData(button_data_t *button_data);
void initScreenData(screen_data_t *screen_data);

void setPadColor(button_data_t *button_data, uint32_t btn_num, char r, char g, char b);
void readScreenDataFromFile(screen_data_t *screen_data, const char *fname);