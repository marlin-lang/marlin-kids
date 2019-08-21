#import <Cocoa/Cocoa.h>

#import <optional>

#import "document.hpp"

@interface Document : NSDocument

- (std::optional<marlin::control::source_update>)initialize;

- (marlin::control::document&)content;

@end
