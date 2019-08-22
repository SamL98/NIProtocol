//
//  SocketManager.h
//  Maschine 2
//
//  Created by Sam Lerner on 8/20/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#ifndef SocketManager_h
#define SocketManager_h

@interface SocketManager : NSObject

@property BOOL connected;
@property int sockfd;

-(int)connect:(CFNetServiceRef)service;
-(int)sendMsg:(void *)msg withLength:(size_t)len;

@end

#endif /* SocketManager_h */
