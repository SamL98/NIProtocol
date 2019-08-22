//
//  BonjourManager.m
//  Maschine 2
//
//  Created by Sam Lerner on 8/17/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#import "BonjourManager.h"

@implementation BonjourManager

void
resolve_callback(CFNetServiceRef service,
                 CFStreamError *error,
                 void *info)
{
    BonjourManager *bonjman;
    
    if (!info) {
        printf("No info in resolve callback\n");
        return;
    }
    
    bonjman = (__bridge BonjourManager *)info;
    
    if ([bonjman clientCallback])
        [bonjman clientCallback](service, error, [bonjman callerPtr]);
}

void
browse_callback(CFNetServiceBrowserRef browser,
                CFOptionFlags flags,
                CFTypeRef domainOrService,
                CFStreamError *error,
                void *info)
{
    CFNetServiceClientContext ctx;
    CFNetServiceRef           service;
    
    if (!info) {
        printf("No info in resolve callback\n");
        return;
    }
    
    ctx.version = 0;
    ctx.info = info;
    ctx.retain = NULL;
    ctx.release = NULL;
    ctx.copyDescription = NULL;
    
    service = (CFNetServiceRef)domainOrService;
    
    CFNetServiceSetClient(service,
                          resolve_callback,
                          &ctx);
    
    CFNetServiceScheduleWithRunLoop(service,
                                    CFRunLoopGetCurrent(),
                                    kCFRunLoopDefaultMode);
    
    if (!CFNetServiceResolveWithTimeout(service, 0, error)) {
        printf("Couldn't resolve service\n");
        CFNetServiceUnscheduleFromRunLoop(service,
                                          CFRunLoopGetCurrent(),
                                          kCFRunLoopDefaultMode);
        CFNetServiceSetClient(service, 0, 0);
    }
}

-(id)init:(NSString *)domain withType:(NSString *)type withCallback:(CFNetServiceClientCallBack)callback withCaller:(void *)callerPtr
{
    self = [super init];
    [self setDomain:domain];
    [self setType:type];
    [self setClientCallback:callback];
    [self setCallerPtr:callerPtr];
    return self;
}

-(int)connect
{
    CFNetServiceClientContext  ctx;
    CFStreamError              error;
    CFNetServiceBrowserRef     browser;
    
    // Create a context structure with ourself as the info
    ctx.version = 0;
    ctx.info = (__bridge void *)self;
    ctx.retain = NULL;
    ctx.release = NULL;
    ctx.copyDescription = NULL;
    
    browser = CFNetServiceBrowserCreate(kCFAllocatorDefault,
                                        (CFNetServiceBrowserClientCallBack)browse_callback,
                                        &ctx);
    if (!browser) {
        printf("Couldn't create net service browser\n");
        return 1;
    }
    
    CFNetServiceBrowserScheduleWithRunLoop(browser,
                                           CFRunLoopGetCurrent(),
                                           kCFRunLoopDefaultMode);
    
    if (!CFNetServiceBrowserSearchForServices(browser,
                                              (__bridge CFStringRef)[self domain],
                                              (__bridge CFStringRef)[self type],
                                              &error)) {
        printf("Couldn't search for services\n");
        CFNetServiceBrowserUnscheduleFromRunLoop(browser,
                                                 CFRunLoopGetCurrent(),
                                                 kCFRunLoopDefaultMode);
        CFRelease(browser);
    }
    
    CFRunLoopRun();
    CFRelease(browser);
    
    return 0;
}

@end
