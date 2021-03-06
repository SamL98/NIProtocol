#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>
#include <nimessenger.h>
#include <nihandshaker.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "mach_callbacks.h"
#include "sock_callbacks.h"
#include "bonjour_callbacks.h"

#define kM2NotifPortMsgUid 46
#define kListenPort "6969"

CFSocketRef 
create_cfsocket()
{
	CFSocketRef 	cfsock = NULL;
	struct addrinfo hints, 
					*res;
	int 			sockfd, 
					status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, 
							  kListenPort, 
							  &hints, 
							  &res))) {
		printf("Couldn't get addr info: %s\n", gai_strerror(status));
		return NULL;
	}

	if ((sockfd = socket(res->ai_family, 
						 res->ai_socktype, 
						 res->ai_protocol)) < 0) {
		printf("Couldn't get valid socket fd\n");
		goto ret;
	}

	if ((status = bind(sockfd, 
					   res->ai_addr, 
					   res->ai_addrlen)) < 0) {
		printf("Couldn't bind socket to port: %s\n", kListenPort);
		goto ret;
	}

	listen(sockfd, 5);

	cfsock = CFSocketCreateWithNative(kCFAllocatorDefault,
									  sockfd,
									  kCFSocketReadCallBack,//kCFSocketDataCallBack,
									  (CFSocketCallBack)conn_accept_callback,
									  NULL);

ret:
	freeaddrinfo(res);
	return cfsock;
}

int
get_serial(char *serial)
{
	memset(serial, 0, kSerialNumberLen);
	gStopAfterSerial = 1;
	doHandshake(serial);
	return strlen(serial) == kSerialNumberLen-1;
}

CFNetServiceRef
create_netservice(int port, CFNetServiceClientCallBack callback)
{
	CFNetServiceClientContext ctx;
	CFNetServiceRef 		  cfnetservice;
	CFStreamError			  error;

	cfnetservice = CFNetServiceCreate(NULL,
									  CFSTR("local."),
							  		  CFSTR("_freehand._tcp"),
							  		  CFSTR("M2"),
							  		  port);

	ctx.version = 0;
	ctx.info = NULL;
	ctx.retain = NULL;
	ctx.release = NULL;
	ctx.copyDescription = NULL;
	CFNetServiceSetClient(cfnetservice, callback, &ctx);

	CFNetServiceScheduleWithRunLoop(cfnetservice,
									CFRunLoopGetCurrent(),
									kCFRunLoopDefaultMode);

	if (!CFNetServiceRegisterWithOptions(cfnetservice,
										 0,
										 &error)) 
	{
		CFNetServiceUnscheduleFromRunLoop(cfnetservice,
										  CFRunLoopGetCurrent(),
										  kCFRunLoopDefaultMode);
										
		CFRelease(cfnetservice);
		cfnetservice = NULL;
	}

	return cfnetservice;
}

int main(int argc, const char * argv[]) {
	CFRunLoopSourceRef cfRunLoopSrc;
	CFSocketRef 	   cfsock;
	CFNetServiceRef	   cfnetservice;
	int				   retval = 0;

	// If the port NIHWMainHandler is available, then that means the hardware agent is running.
	// 		In this case, we can simply perform the handshake from the client side with the hardware
	//		agent until we obtain the serial number. Then we can open the M2 notification port using that serial.
	//
	// Otherwise, we will create the NIHWMainHandler port and perform the handshake from the server side,
	// passing a hardcoded serial number. Then M2 will send us the name of its notification port that we can then open.
	// We open a request port for M2 but ignore all requests.
	if (getBootstrapPort(kMainPortName))
	{
		printf("Hardware agent running\n");

		char serial[kSerialNumberLen];
		char m2NotifPortName[kMikroNotificationPortNameLen];

		printf("Trying to obtain serial number\n");

		if (!get_serial(serial)) {
			printf("Couldn't get serial number\n");
			return 1;
		}

		printf("Received serial number: %s\n", serial);

		sprintf(m2NotifPortName, kMikroNotificationPortNameFormat, serial, kM2NotifPortMsgUid);
		m2NotifPortName[kMikroNotificationPortNameLen-1] = 0;

		if (!(gM2NotifPort = getBootstrapPort(m2NotifPortName))) {
			printf("Couldn't obtain M2 notification port\n");
			return 1;
		}

		printf("Obtained reference to notification port: %s\n", m2NotifPortName);
	}
	else {
		printf("Hardware agent not running\n");

		CFMessagePortRef bsPort = createRequestPort(kMainPortName, 
						  							(CFMessagePortCallBack)bootstrap_req_port_callback, 
						  							NULL);
		
		CFMessagePortSetDispatchQueue(bsPort, dispatch_get_main_queue());

		printf("Created port %s\n", kMainPortName);
	}

	if (!(cfsock = create_cfsocket())) {
		printf("Couldn't create CFSocket\n");
		return 1;
	}

	printf("Created socket listening on %s\n", kListenPort);

	if (!(cfRunLoopSrc = CFSocketCreateRunLoopSource(kCFAllocatorDefault,
													 cfsock,
													 0))) {
		printf("Couldn't create a run loop source for the socket\n");
		retval = 1;
		goto release_sock;
	}

	CFRunLoopAddSource(CFRunLoopGetCurrent(),
					   cfRunLoopSrc,
					   kCFRunLoopDefaultMode);

	if (!(cfnetservice = create_netservice(atoi(kListenPort),
										   (CFNetServiceClientCallBack)register_callback))) {
		printf("Couldn't create cfnetservice\n");
		retval = 1;
		goto release_rlsrc;
	}

    CFRunLoopRun();

release_rlsrc:
	CFRelease(cfRunLoopSrc);
	
release_sock:
	CFSocketInvalidate(cfsock);
	CFRelease(cfsock);
    return retval;
}
