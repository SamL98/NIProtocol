//
//  ViewController.h
//  Maschine 2
//
//  Created by Sam Lerner on 8/17/19.
//  Copyright © 2019 Sam Lerner. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MessageManager.h"

@interface ViewController : UIViewController

@property (weak, nonatomic) IBOutlet UIButton *pad1;
@property (weak, nonatomic) IBOutlet UIButton *pad2;
@property (weak, nonatomic) IBOutlet UIButton *pad3;
@property (weak, nonatomic) IBOutlet UIButton *pad4;
@property (weak, nonatomic) IBOutlet UIButton *pad5;
@property (weak, nonatomic) IBOutlet UIButton *pad6;
@property (weak, nonatomic) IBOutlet UIButton *pad7;
@property (weak, nonatomic) IBOutlet UIButton *pad8;
@property (weak, nonatomic) IBOutlet UIButton *pad9;
@property (weak, nonatomic) IBOutlet UIButton *pad10;
@property (weak, nonatomic) IBOutlet UIButton *pad11;
@property (weak, nonatomic) IBOutlet UIButton *pad12;
@property (weak, nonatomic) IBOutlet UIButton *pad13;
@property (weak, nonatomic) IBOutlet UIButton *pad14;
@property (weak, nonatomic) IBOutlet UIButton *pad15;

@property MessageManager *messman;

- (void)finishConnectionSetup:(CFNetServiceRef)service;

@end

