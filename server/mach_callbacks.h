#include <CoreFoundation/CoreFoundation.h>

#define kNonceResponseUnk1 0x20200
#define kNonceResponseUnk2 0x3

typedef struct {
	uint32_t unk1;
	uint32_t unk2;
} nonce_response_t;

typedef struct __attribute__((packed)) {
	uint32_t trueStr;
	uint32_t reqPortNameLen;
	char	 reqPortName[kAgentRequestPortNameLen];
	uint32_t notifPortNameLen;
	char	 notifPortName[kAgentNotificationPortNameLen];
	uint32_t unk;
} port_uid_response_t;

typedef struct __attribute__((packed)) {
	uint32_t trueStr;
	uint32_t reqPortNameLen;
	char	 reqPortName[kMikroRequestPortNameLen];
	uint32_t notifPortNameLen;
	char	 notifPortName[kMikroNotificationPortNameLen];
	uint32_t unk;
} serial_response_t;

typedef struct {
	uint32_t trueStr;
} port_name_response_t;

extern CFMessagePortRef gM2NotifPort;

CFMessagePortRef createRequestPort(const char *name, CFMessagePortCallBack callout, void *info);
CFDataRef bootstrap_req_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
CFDataRef req_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);