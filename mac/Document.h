#import <Cocoa/Cocoa.h>

#import <optional>

#import <document.hpp>

@interface Document : NSDocument

- (marlin::control::document&)content;

@end
