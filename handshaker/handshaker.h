#define kAgentRequestPortNameFormat "NIHWS%04x%04dRequest"

extern CFMessagePortRef createNotificationPort(const char *name, CFMessagePortCallBack callout, void *info);
extern CFDataRef bootstrap_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
extern CFDataRef mikro_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
extern CFDataRef agent_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);

extern char gStopAfterSerial;
CFMessagePortRef doHandshake(char *serial);