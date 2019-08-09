#include "messenger.h"
#include "callbacks.h"

#define kSerialNumberPacketDataLen 25

CFMessagePortRef
createNotificationPort(char *name, CFMessagePortCallBack callout)
{
    Boolean shouldFreeInfo;
    CFStringRef cfName;

    cfName = CFStringCreateWithCString(kCFAllocatorDefault,
                                       name,
                                       kCFStringEncodingASCII);

    return CFMessagePortCreateLocal(kCFAllocatorDefault,
                                    cfName,
                                    callout,
                                    NULL,
                                    &shouldFreeInfo);
}

CFDataRef
mikro_notif_port_callback(CFMessagePortRef local,
                          SInt32 msgid,
                          CFDataRef data,
                          void *info)
{
    if (!data) {
        printf("No data from hardware agent\n");
        return NULL;
    }

    char portName[kMikroNotificationPortNameLen];
    CFStringGetCString(CFMessagePortGetName(local), portName, kMikroNotificationPortNameLen, kCFStringEncodingASCII);
    
    printf("Received %ld bytes of data on %s\n", (long)CFDataGetLength(data), portName);

    return NULL;
}

CFDataRef
agent_notif_port_callback(CFMessagePortRef local,
                          SInt32 msgid,
                          CFDataRef data,
                          void *info)
{
    Boolean shouldFreeInfo;
    CFStringRef cfAgentNotifPortName;
    char *agentNotifPortName;

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
    serial_num_msg = *(struct serial_num_msg_t *)CFDataGetBytePtr(data);
    
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