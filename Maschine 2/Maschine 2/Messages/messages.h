//
//  messages.h
//  Maschine 2
//
//  Created by Sam Lerner on 8/21/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#ifndef messages_h
#define messages_h

#define kPadCtrl 0x3504e00

typedef struct {
    uint32_t ctrl;
    uint32_t counter;
    uint32_t msg_unk;
    uint32_t nmsgs;
    uint32_t btn;
    uint32_t btn_unk;
    uint32_t pressure;
} pad_msg;

#endif /* messages_h */
