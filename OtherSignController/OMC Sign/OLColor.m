//
//  OLColor.m
//  OMC Sign
//
//  Created by Mike Estee on 12/12/13.
//  Copyright (c) 2013 Other Machine Co. All rights reserved.
//

#import "OLColor.h"

@implementation NSColor(OLColor)

- (NSString*)stringValue
{
    NSColor *color = [self colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
    
    int r = color.redComponent*255;
    int g = color.greenComponent*255;
    int b = color.blueComponent*255;
    
    return [NSString stringWithFormat:@"%2.2x%2.2x%2.2x",r,g,b];
}

@end
