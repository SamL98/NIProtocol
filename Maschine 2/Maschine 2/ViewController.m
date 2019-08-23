//
//  ViewController.m
//  Maschine 2
//
//  Created by Sam Lerner on 8/17/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#import "ViewController.h"
#import "BonjourManager.h"
#import "SocketManager.h"

void
connect_callback(CFNetServiceRef service,
                 CFStreamError *error,
                 void *info)
{
    if (!info) {
        printf("Null info in connect_callback\n");
        return;
    }
    
    [(__bridge ViewController *)info finishConnectionSetup:service];
}

@interface ViewController ()

@end

@implementation ViewController

- (void)enablePads:(BOOL)enabled
{
    size_t   i;
    char     selName[6];
    SEL      padSel;
    UIButton *pad;
    
    for (i=1; i<16; i++) {
        sprintf(selName, "pad%lu", i);
        
        if (i<10) selName[4] = 0;
        else      selName[5] = 0;
        
        padSel = NSSelectorFromString([NSString stringWithCString:selName encoding:NSASCIIStringEncoding]);
        
        pad = (UIButton *)[self performSelector:padSel];
        [pad setEnabled:enabled];
    }
}

- (void)finishConnectionSetup:(CFNetServiceRef)service
{
    SocketManager *sockman = [[SocketManager alloc] init];
    
    if ([sockman connect:service]) {
        printf("Socket manager couldn't connect to net service\n");
        return;
    }
    
    printf("Socket manager successfully connected to net service\n");
    
    [self setMessman:[[MessageManager alloc] init:sockman]];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self enablePads:YES];
    });
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self enablePads:NO];
    
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_UTILITY, 0), ^{
        [[[BonjourManager alloc] init:@"local."
                             withType:@"_freehand._tcp"
                             withCallback:connect_callback
                             withCaller:(__bridge void *)self] connect];
    });
}

- (IBAction)padTap:(id)sender
{
    NSInteger tag;
    uint      padNum;
    
    if (!(tag = [sender tag])) {
        printf("Sender has no tag\n");
        return;
    }
    
    padNum = (uint)tag;
    printf("Pad %u tapped\n", padNum);
    
    [[self messman] sendPad:padNum withPressure:0xffffffff];
}

@end
