#include <nimessenger.h>
#include <ninotifier.h>
#include "callbacks.h"

#define kSerialNumberPacketDataLen 25

CFMessagePortRef
createNotificationPort(const char *name, 
                       CFMessagePortCallBack callout, 
                       void *info)
{
    Boolean              shouldFreeInfo;
    CFStringRef          cfName;
    CFMessagePortContext ctx;

    cfName = CFStringCreateWithCString(kCFAllocatorDefault,
                                       name,
                                       kCFStringEncodingASCII);

    ctx.version = 0;
    ctx.info = info;
    ctx.retain = CFRetain;
    ctx.release = CFRelease;
    ctx.copyDescription = NULL;

    return CFMessagePortCreateLocal(kCFAllocatorDefault,
                                    cfName,
                                    callout,
                                    &ctx,
                                    &shouldFreeInfo);
}

CFDataRef
bootstrap_notif_port_callback(CFMessagePortRef local, 
                              SInt32 msgid, 
                              CFDataRef data, 
                              void *info)
{
    CFMessagePortRef reqPort;

    if (!data || !CFDataGetBytePtr(data)) {
        printf("No data from hardware agent\n");
        return NULL;
    }

    if (!info) {
        printf("No info in bootstrap notif port callback\n");
        return NULL;
    }

    reqPort = (CFMessagePortRef)info;
    sendMsg(reqPort, 
            (uint8_t *)CFDataGetBytePtr(data), 
            (size_t)CFDataGetLength(data));

    return NULL;
}

CFDataRef
mikro_notif_port_callback(CFMessagePortRef local,
                          SInt32 msgid,
                          CFDataRef data,
                          void *info)
{
    if (!data || !CFDataGetBytePtr(data)) {
        printf("No data from hardware agent\n");
        return NULL;
    }

    char portName[kMikroNotificationPortNameLen];
    CFStringGetCString(CFMessagePortGetName(local), 
					   portName, 
					   kMikroNotificationPortNameLen, 
					   kCFStringEncodingASCII);
    
    //printf("Received %ld bytes of data on %s\n", (long)CFDataGetLength(data), portName);

    if (CFDataGetLength(data) >= 24)
	    broadcast(data);

    return NULL;
}

CFDataRef
agent_notif_port_callback(CFMessagePortRef local,
                          SInt32 msgid,
                          CFDataRef data,
                          void *info)
{
    serial_num_msg_t serial_num_msg;
    CFStringRef      cfAgentNotifPortName;
    Boolean          shouldFreeInfo;
    char             *agentNotifPortName;

    if (!data || CFDataGetLength(data) != kSerialNumberPacketDataLen)
    {
        printf("Invalid data from hardware agent\n");
        return NULL;
    }

    cfAgentNotifPortName = CFMessagePortGetName(local);
    if (!cfAgentNotifPortName)
    {
        printf("Couldn't get port name for agent notification port\n");
        return NULL;
    }

    gReceivedSerial = 1;
    agentNotifPortName = (char *)CFStringGetCStringPtr(cfAgentNotifPortName,
                                                       kCFStringEncodingASCII);

    // Interpret the data as a serial message packet
    serial_num_msg = *(serial_num_msg_t *)CFDataGetBytePtr(data);
    strncpy(gSerialNum, serial_num_msg.num, kSerialNumberLen);
    
    return NULL;
}

void
invalidation_callback(CFMessagePortRef port, void *info)
{
	CFStringRef portName = CFMessagePortGetName(port);
	if (!portName)
		return;

	printf("%s invalidated\n", CFStringGetCStringPtr(portName, kCFStringEncodingASCII));
	CFRelease(portName);
}
