#include <CoreFoundation/CoreFoundation.h>

#define kNotifNameLen 16
#define kMK2NotifName "MK2Notification"

#define kDataKeyLen 5
#define kDataKey "data"

void broadcast(CFDataRef data);
void listen(CFNotificationCallback callback);
