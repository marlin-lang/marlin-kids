#import <Cocoa/Cocoa.h>

#import <optional>

#import "document.hpp"

@interface Document : NSDocument

- (marlin::control::source_initialization)initialize;

- (marlin::control::document&)content;

@end
