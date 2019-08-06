#include <CoreFoundation/CoreFoundation.h>

typedef void(*maschine_callback)(uint32_t *data, size_t length);

void registerCallback(maschine_callback callback);
CFMessagePortRef createNotificationPort(char *name, CFMessagePortCallBack callout);
CFDataRef mikro_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
CFDataRef agent_notif_port_callback(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
void invalidation_callback(CFMessagePortRef port, void *info);