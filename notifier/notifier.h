#include <CoreFoundation/CoreFoundation.h>

#define kMK2NotifName "MK2Notification"
#define kPacketKey "packet"
#define kPacketLenKey "packet_len"

void broadcast(char *packet, size_t packetLen, const char *notifName);
void listen(CFNotificationCallback callback, const char *notifName);
