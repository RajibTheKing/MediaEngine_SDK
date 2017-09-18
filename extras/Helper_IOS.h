//
//  NSObject_Helper_IOS.h
//  AudioVideoEngine
//
//  Created by Rajib Chandra Das on 2/23/16.
//  Copyright © 2016 ipvision. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "Tools.h"
using namespace std;

@interface Helper_IOS: NSObject
{
    //NSString *m_fileName;
    //FILE *m_FileForDumpIOS;
    //Tools m_Tools;
}

- (void)InitializeFilePointerIOS;
+ (id)GetInstance;
- (id) init;
- (void)WriteToFile:(unsigned char *)data dataLength:(int)datalen;
- (void)WriteToFile:(const char *)path withData:(unsigned char *)data dataLength:(int)datalen;
- (void)MakeFolderInDocument:(NSString *)nsFolderName;
- (void) runScript:(NSString*)scriptName;
@end


static Helper_IOS *helper_IOS = nil;