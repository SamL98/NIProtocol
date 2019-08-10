#include "notifier.h"

void 
broadcast(CFDataRef data)
{
	CFMutableDictionaryRef userInfo;
	size_t				   numValues;

	numValues = 1;
	userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault,
								  		 numValues,
								  		 &kCFCopyStringDictionaryKeyCallBacks,
								  		 &kCFTypeDictionaryValueCallBacks);
	
	CFDictionaryAddValue(userInfo, CFSTR(kDataKey), data);

	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
										 CFSTR(kMK2NotifName),
										 NULL,
										 userInfo,
										 true);

release_user_info:
	CFRelease(userInfo);
}

void
listen(CFNotificationCallback callback)
{
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
									NULL,
									callback,
									CFSTR(kMK2NotifName),
									NULL,
									CFNotificationSuspensionBehaviorDeliverImmediately);
}
