#include <CoreFoundation/CoreFoundation.h>

CFMessagePortRef createNotificationPort(const char *name, CFMessagePortCallBack callout, void *info);
CFDataRef bootstrap_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
CFDataRef mikro_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
CFDataRef agent_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);