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
@property MessageManager *messman;

- (void)finishConnectionSetup:(CFNetServiceRef)service;

@end

