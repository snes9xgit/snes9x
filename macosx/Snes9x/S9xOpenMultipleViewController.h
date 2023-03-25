//
//  S9xOpenMultipleViewController.h
//  Snes9x
//
//  Created by Buckley on 3/25/23.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface S9xOpenMultipleViewController : NSViewController

@property (nonatomic, strong) NSURL *fileURL1;
@property (nonatomic, strong) NSURL *fileURL2;

@property (nonatomic, strong) void(^completionBlock)(NSArray<NSURL *> *fileURLs);

@end

NS_ASSUME_NONNULL_END
