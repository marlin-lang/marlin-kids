#import <Cocoa/Cocoa.h>

#import <optional>

#import "document.hpp"

@interface Document : NSDocument

- (std::optional<marlin::control::source_initialization>)initialize;

- (marlin::control::document&)content;

@end
