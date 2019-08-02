#import <Cocoa/Cocoa.h>

#import <optional>

#import <document.hpp>

@interface Document : NSDocument

- (marlin::document&)content;

@end
