//
//  MessageManager.h
//  Maschine 2
//
//  Created by Sam Lerner on 8/21/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#ifndef MessageManager_h
#define MessageManager_h

#include "SocketManager.h"

@interface MessageManager : NSObject

@property SocketManager *sockman;

-(id)init:(SocketManager *)sockman;
-(int)sendPad:(uint)num withPressure:(uint)pressure;

@end

#endif /* MessageManager_h */
