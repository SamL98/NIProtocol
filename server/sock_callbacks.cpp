#include <niparser.h>
#include <nimessenger.h>
#include <sys/socket.h>
#include "sock_callbacks.h"

#define kMaxPacketLen 28

extern CFMessagePortRef gM2NotifPort;

void conn_read_callback(CFSocketRef s,
						CFSocketCallBackType callbackType,
						CFDataRef address,
						void *data,
						void *info)
{
	char	  packet[kMaxPacketLen];
	int	  	  packetLen;
	mk2_msg	  msg;

	if ((packetLen = recv(CFSocketGetNative(s), 
						  packet, 
						  kMaxPacketLen, 
						  0)) <= 0) 
	{
		if (packetLen == 0) {
			CFSocketInvalidate(s);
			CFRelease(s);
		}
		return;
	}

	if (packetLen < 24) {
		printf("Packet of length %d is too short\n", packetLen);
		return;
	}

	if (parse_packet(packet,
					 (size_t)packetLen,
					 &msg)) {
		printf("Couldn't parse packet\n");
		return;
	}

	display_msg(msg);
	sendMsg(gM2NotifPort, (uint8_t *)packet, packetLen);
}

void conn_accept_callback(CFSocketRef s,
					 	  CFSocketCallBackType callbackType,
					 	  CFDataRef address,
					 	  void *data,
					 	  void *info)
{
	struct sockaddr_storage sa;
	CFRunLoopSourceRef 		cfRunLoopSrc;
	CFSocketRef 	   		cfsock;
	socklen_t 				sasize = sizeof(sa);
	int	  	  				new_fd;

	if (!gM2NotifPort) {
		printf("Null M2 notification port\n");
		return;
	}

	if ((new_fd = accept(CFSocketGetNative(s), 
						 (struct sockaddr *)&sa, 
						 &sasize)) < 0) {
		printf("Couldn't accept\n");
		return;
	}

	cfsock = CFSocketCreateWithNative(kCFAllocatorDefault,
									  new_fd,
									  kCFSocketReadCallBack,
									  (CFSocketCallBack)conn_read_callback,
									  NULL);

	if (!(cfRunLoopSrc = CFSocketCreateRunLoopSource(kCFAllocatorDefault,
													 cfsock,
													 0))) {
		printf("Couldn't create a run loop source for the socket\n");
		CFRelease(cfsock);
		return;
	}

	CFRunLoopAddSource(CFRunLoopGetCurrent(),
					   cfRunLoopSrc,
					   kCFRunLoopDefaultMode);
}