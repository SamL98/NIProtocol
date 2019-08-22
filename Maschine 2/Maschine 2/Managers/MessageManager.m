//
//  MessageManager.m
//  Maschine 2
//
//  Created by Sam Lerner on 8/21/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MessageManager.h"
#import "messages.h"

@implementation MessageManager

-(id)init:(SocketManager *)sockman
{
    self = [super init];
    [self setSockman:sockman];
    return self;
}

-(int)sendPad:(uint)btnNum withPressure:(uint)pressure
{
    pad_msg  msg;
    uint32_t r, c, btnCode;
    
    btnNum -= 1;
    r = 3 - btnNum / 4;
    c = btnNum % 4;
    btnCode = r * 4 + c;
    
    msg.ctrl = kPadCtrl;
    msg.counter = 0;
    msg.msg_unk = 0;
    msg.nmsgs = 1;
    msg.btn = btnCode;
    msg.btn_unk = 0;
    msg.pressure = pressure;
    
    return [[self sockman] sendMsg:(void *)&msg withLength:sizeof(msg)];
}

@end
