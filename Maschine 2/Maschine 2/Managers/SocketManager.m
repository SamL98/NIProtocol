//
//  SocketManager.m
//  Maschine 2
//
//  Created by Sam Lerner on 8/20/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#import "SocketManager.h"

@implementation SocketManager

-(id)init
{
    self = [super init];
    [self setConnected:NO];
    return self;
}

-(int)connect:(CFNetServiceRef)service
{
    CFArrayRef addresses;
    CFDataRef  addrData;
    struct     sockaddr *sa;
    int        sockaddrlen;
    int        sockfd;
    
    if (!(addresses= CFNetServiceGetAddressing(service))) {
        printf("Couldn't get the addresses found by the net service\n");
        return 1;
    }
    
    if (CFArrayGetCount(addresses) < 1) {
        printf("No addresses found for the net service\n");
        return 1;
    }
    
    addrData = (CFDataRef)CFArrayGetValueAtIndex(addresses, 0);
    sa = (struct sockaddr *)CFDataGetBytePtr(addrData);
    
    if (sa->sa_family == AF_INET)
        sockaddrlen = sizeof(struct sockaddr_in);
    else if (sa->sa_family == AF_INET6)
        sockaddrlen = sizeof(struct sockaddr_in6);
    else {
        printf("Unknown sockaddr family: %d\n", sa->sa_family);
        return 1;
    }
    
    if ((sockfd = socket(sa->sa_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("Couldn't create socket to connect to net service\n");
        return 1;
    }
    
    if (connect(sockfd, sa, sockaddrlen) < 0) {
        printf("Couldn't connect to network service\n");
        return 1;
    }
    
    [self setSockfd:sockfd];
    [self setConnected:YES];
    
    return 0;
}

-(int)sendMsg:(void *)msg withLength:(size_t)len
{
    size_t bytesSent;
    
    if (![self connected]) {
        printf("Cannot send a message to an unconnected socket\n");
        return 1;
    }
    
    if ((bytesSent = send([self sockfd], msg, len, 0)) < 0) {
        printf("Error occurred while sending on socket\n");
        return 1;
    }
        
    if (bytesSent < len)
        return [self sendMsg:msg+bytesSent withLength:len-bytesSent];
    
    return 0;
}

@end
