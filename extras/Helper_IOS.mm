//
//  NSObject_Helper_IOS.h
//  AudioVideoEngine
//
//  Created by Rajib Chandra Das on 2/23/16.
//  Copyright © 2016 ipvision. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Helper_IOS.h"


@implementation Helper_IOS

- (id) init
{
    self = [super init];
    NSLog(@"Inside Helper_IOS Constructor");
    
    
    //m_fileName = @"IOS_FileDump.dump";   //This is the file Name
    
    //[self InitializeFilePointerIOS];
    
    //[self MakeFolderInDocument:@"/Encode"];
    //[self MakeFolderInDocument:@"/Decode"];
    
    return self;
}

+ (id)GetInstance
{
    if(!helper_IOS)
    {
        helper_IOS = [[Helper_IOS alloc] init];
    }
    return helper_IOS;
}

-(void)InitializeFilePointerIOS
{
    /*
    NSFileHandle *handle;
    NSArray *Docpaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [Docpaths objectAtIndex:0];
    NSString *filePathyuv = [documentsDirectory stringByAppendingPathComponent:m_fileName];
    handle = [NSFileHandle fileHandleForUpdatingAtPath:filePathyuv];
    char *filePathcharyuv = (char*)[filePathyuv UTF8String];
    m_FileForDumpIOS = fopen(filePathcharyuv, "wb");
    */
}

- (void)WriteToFile:(unsigned char *)data dataLength:(int)datalen
{
    /*
    printf("Writing to yuv");
    fwrite(data, 1, datalen, m_FileForDumpIOS);
     */
}

- (void)MakeFolderInDocument:(NSString *)nsFolderName
{
    NSLog(@"TheKing MakeFolder Method is Called");
    
    NSArray *Docpaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [Docpaths objectAtIndex:0];
    NSString *filePathAndDirectory = [documentsDirectory stringByAppendingPathComponent:nsFolderName];
    
    NSError *error;
    
    if (![[NSFileManager defaultManager] createDirectoryAtPath:filePathAndDirectory
                                   withIntermediateDirectories:NO
                                                    attributes:nil
                                                         error:&error])
    {
        NSLog(@"Create directory error: %@", error);
    }
    
}

- (void)WriteToFile:(const char *)path withData:(unsigned char *)data dataLength:(int)datalen
{
    FILE *FpDump;
    NSString *fileName = [NSString stringWithUTF8String: path];
    
    NSFileHandle *handle;
    NSArray *Docpaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [Docpaths objectAtIndex:0];
    NSString *filePathyuv = [documentsDirectory stringByAppendingPathComponent:fileName];
    handle = [NSFileHandle fileHandleForUpdatingAtPath:filePathyuv];
    char *filePathcharyuv = (char*)[filePathyuv UTF8String];
    FpDump = fopen(filePathcharyuv, "wb");
    
    
    fwrite(data, 1, datalen, FpDump);
}

@end
