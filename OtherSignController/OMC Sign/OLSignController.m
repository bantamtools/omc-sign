//
//  OLSignController.m
//  OMC Sign
//
//  Created by Mike Estee on 12/11/13.
//  Copyright (c) 2013 Other Machine Co. All rights reserved.
//

#import "OLSignController.h"

#import <Foundation/Foundation.h>

float lerp( float x, float y, float s )
{
    return x + (s * (y-x));
}


@implementation OLSignController

- (void)awakeFromNib
{
    self.other = [NSColor blackColor];
    self.machine = [NSColor blackColor];
    self.co = [NSColor blackColor];
    self.O = [NSColor blackColor];
    self.M = [NSColor blackColor];
    self.C = [NSColor blackColor];
    self.left = [NSColor blackColor];
    self.right = [NSColor blackColor];
}

- (IBAction)changeColor:(id)sender
{
    NSMutableString *colors = [NSMutableString string];
    
//    int count = 218;
    
    for( int i=0; i<33 ; i++ ) [colors appendFormat:@"%@,",[self.other stringValue]];
    for( int i=33; i<71; i++ ) [colors appendFormat:@"%@,",[self.right stringValue]];
    
    for( int i=71; i<81; i++ ) [colors appendFormat:@"%@,",[self.co stringValue]];
    for( int i=81; i<127; i++ ) [colors appendFormat:@"%@,",[self.machine stringValue]];

    for( int i=127; i<165; i++ ) [colors appendFormat:@"%@,",[self.left stringValue]];

    for( int i=165; i<179; i++ ) [colors appendFormat:@"%@,",[self.O stringValue]];
    for( int i=179; i<206; i++ ) [colors appendFormat:@"%@,",[self.M stringValue]];
    for( int i=206; i<218; i++ ) [colors appendFormat:@"%@,",[self.C stringValue]];
    
    
    
    
    [colors appendString:@"\n"];   
    [self write:colors];
}

- (void)open
{
	NSInputStream  *inputStream = nil;
    NSOutputStream *outputStream = nil;
    [NSStream getStreamsToHost:[NSHost hostWithAddress:[self.address stringValue]]
                          port:1717
                   inputStream:&inputStream
                  outputStream:&outputStream];
    
    self.input = inputStream;
    self.output = outputStream;
	
	[self.input retain];
	[self.output retain];
	
	[self.input setDelegate:self];
	[self.output setDelegate:self];
	
	[self.input scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[self.output scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	
	[self.input open];
	[self.output open];
}


- (void)close
{
	[self.input close];
	[self.output close];
	
	[self.input removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[self.output removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	
	[self.input setDelegate:nil];
	[self.output setDelegate:nil];
	
	[self.input release];
	[self.output release];
	
	self.input = nil;
	self.output = nil;
}


- (IBAction)connect:(id)sender
{
    [self open];
}


- (IBAction)disconnect:(id)sender
{
    [self close];
}



- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)event
{
	switch(event)
    {
		case NSStreamEventHasSpaceAvailable:
        {
			if(stream == self.output)
				NSLog(@"outputStream is ready.");
			break;
		}
		case NSStreamEventHasBytesAvailable:
        {
			if(stream == self.input)
            {
				NSLog(@"inputStream is ready.");
				
				uint8_t buf[1024];
				size_t len = 0;
				
				len = [self.input read:buf maxLength:1024];
				
				if(len > 0)
                {
					NSMutableData* data = [[NSMutableData alloc] initWithLength:0];
					
					[data appendBytes: (const void *)buf length:len];
					
					NSString *s = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
					
					[self read:s];
					
					[data release];
				}
			}
			break;
		}
		default: {
			NSLog(@"Stream is sending an Event: %i", (int)event);
			
			break;
		}
	}
}


- (void)read:(NSString *)s
{
	NSLog(@"Reading in the following:");
	NSLog(@"%@", s);
}


- (void)write:(NSString *)s
{
	uint8_t *buf = (uint8_t *)[s UTF8String];
	[self.output write:buf maxLength:strlen((char *)buf)];
    
    NSLog(@"wrote: %@", s);
}
@end
