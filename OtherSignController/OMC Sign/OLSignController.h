//
//  OLSignController.h
//  OMC Sign
//
//  Created by Mike Estee on 12/11/13.
//  Copyright (c) 2013 Other Machine Co. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OLColor.h"

@interface OLSignController : NSObject<NSStreamDelegate>

@property (retain) NSInputStream *input;
@property (retain) NSOutputStream *output;

@property (retain) NSColor *other;
@property (retain) NSColor *machine;
@property (retain) NSColor *co;
@property (retain) NSColor *O;

@property (retain) NSColor *M;
@property (retain) NSColor *C;

@property (retain) NSColor *left;
@property (retain) NSColor *right;


@property (assign) IBOutlet NSTextField *address;

- (IBAction)connect:(id)sender;
- (IBAction)disconnect:(id)sender;

- (IBAction)changeColor:(id)sender;

@end
