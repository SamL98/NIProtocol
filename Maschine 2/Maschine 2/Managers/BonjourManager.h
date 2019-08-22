//
//  BonjourManager.h
//  Maschine 2
//
//  Created by Sam Lerner on 8/17/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#ifndef BonjourManager_h
#define BonjourManager_h

#import <CFNetwork/CFNetwork.h>

@interface BonjourManager : NSObject

@property NSString *domain;
@property NSString *type;
@property CFNetServiceClientCallBack clientCallback;
@property void *callerPtr;

-(id)init:(NSString*)domain withType:(NSString*)type withCallback:(CFNetServiceClientCallBack)callback withCaller:(void *)callerPtr;
-(int)connect;

@end

#endif /* BonjourManager_h */
