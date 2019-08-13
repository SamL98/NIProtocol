#include <CoreFoundation/CoreFoundation.h>
#include <niparser.h>
#include <ninotifier.h>
#include <nimessenger.h>

CFMessagePortRef gBsPort = NULL;
uint32_t gPrevBtn = 0;

size_t callbackcount = 0;

void
notif_callback(CFNotificationCenterRef center,
			   void *observer,
			   CFNotificationName name,
			   CFStringRef object,
			   CFDictionaryRef userInfo)
{
	CFDataRef   	 data;
	char			 *packet;
	mk2_msg 		 msg;
	size_t  		 packetLen;
	button_data_t 	 button_data;
	int				 parseResult;

	initButtonData(&button_data);

	if (!gBsPort) {
		if (!(gBsPort = getBootstrapPort(kSLBootstrapPortName))) {
			printf("Couldn't get reference to SL bootstrap port\n");
			return;
		}
	}

	if (!userInfo) {
		printf("No user info from notification\n");
		return;
	}

	if (!CFDictionaryGetValueIfPresent(userInfo, 
									   (const void*)CFSTR(kDataKey),
									   (const void**)&data) || !data) {
		printf("No data key in user info\n");
		return;
	}

	packet = (char *)CFDataGetBytePtr(data);
	packetLen = CFDataGetLength(data);

	if (!packet || packetLen < 0) {
		printf("No data from packet\n");
		return;
	}

	if (parse_packet(packet, packetLen, &msg)) {
		printf("Could not parse message from packet\n");
		return;
	}

	display_msg(msg);

	if (msg.type == PadType && msg.msg.pad_msg.btn != gPrevBtn)
		gPrevBtn = msg.msg.pad_msg.btn;
		setPadColor(&button_data, gPrevBtn, char(255), 0, 0);
		sendButtonDataMsg(gBsPort, button_data);
	}
}

int main()
{
	listen((CFNotificationCallback)notif_callback);
	CFRunLoopRun();
}
