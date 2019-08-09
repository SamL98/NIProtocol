#include "notifier.h"

void 
broadcast(char *packet, size_t packetLen, const char *notifName)
{
	CFDictionaryRef userInfo;
	CFStringRef		cfNotifName;
	size_t			numValues;
	const char 		*keys[numValues+1];
	void 			*values[numValues+1];

	if (!notifName)
		notifName = kMK2NotifName;

	cfNotifName = CFStringCreateWithCString(kCFAllocatorDefault,
											notifName,
											kCFStringEncodingASCII);
	if (!cfNotifName) {
		printf("Couldn't create cf notification name\n");
		return;
	}

	numValues = 2;

	keys[0] = kPacketKey;
	keys[1] = kPacketLenKey;
	keys[2] = 0;

	values[0] = (void *)packet;
	values[1] = (void *)packetLen;
	values[2] = 0;

	userInfo = CFDictionaryCreate(kCFAllocatorDefault,
								  (const void **)keys,
								  (const void **)values,
								  numValues,
								  NULL,
								  NULL);
	if (!userInfo) {
		printf("Couldn't create user info dict\n");
		goto release_name;
	}

	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
										 cfNotifName,
										 NULL,
										 userInfo,
										 true);

	CFRelease(userInfo);
release_name:
	CFRelease(cfNotifName);
}

void
listen(CFNotificationCallback callback, const char *notifName)
{
	CFStringRef cfNotifName;

	if (!notifName)
		notifName = kMK2NotifName;

	cfNotifName = CFStringCreateWithCString(kCFAllocatorDefault,
											notifName,
											kCFStringEncodingASCII);
	if (!cfNotifName) {
		printf("Couldn't create cf notification name\n");
		goto release_name;
	}

	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
									NULL,
									callback,
									cfNotifName,
									NULL,
									CFNotificationSuspensionBehaviorDeliverImmediately);

release_name:
	CFRelease(cfNotifName);
}
