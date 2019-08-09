#include <CoreFoundation/CoreFoundation.h>

CFMessagePortRef createNotificationPort(char *name, CFMessagePortCallBack callout);
CFDataRef mikro_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
CFDataRef agent_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
void invalidation_callback(CFMessagePortRef port, void *info);