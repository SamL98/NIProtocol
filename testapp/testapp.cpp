#include <CoreFoundation/CoreFoundation.h>
#include <niparser.h>
#include <ninotifier.h>

void
notif_callback(CFNotificationCenterRef center,
			   void *observer,
			   CFNotificationName name,
			   CFStringRef object,
			   CFDictionaryRef userInfo)
{
	char    *packet;
	mk2_msg *msgs;
	size_t  packetLen,
			nmsgs, i;

	if (!userInfo) {
		printf("No user info from notification\n");
		return;
	}

	if (!CFDictionaryGetValueIfPresent(userInfo, 
									   (const void*)kPacketKey,
									   (const void**)&packet) || !packet) {
		printf("No packet key in user info\n");
		return;
	}

	if (!CFDictionaryGetValueIfPresent(userInfo, 
									   (const void*)kPacketLenKey,
									   (const void**)&packetLen)) {
		printf("No packet len key in user info\n");
		return;
	}

	msgs = parse_packet(packet, packetLen, &nmsgs);
	if (!msgs) {
		printf("Could not parse messages from packet\n");
		return;
	}

	for (i=0; i<nmsgs; i++) {
		if (msgs[i].type == WheelType)
			printf("Wheel - Button: %u, State: %d\n", msgs[i].msg.wheel_msg.btn, msgs[i].msg.wheel_msg.state);
		else
			printf("Button - Button: %u, Pressure: %u\n", msgs[i].msg.button_msg.btn, msgs[i].msg.button_msg.pressure);
	}
}

int main()
{
	listen((CFNotificationCallback)notif_callback, NULL);
}
