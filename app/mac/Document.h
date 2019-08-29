#import <Cocoa/Cocoa.h>

#import "DocumentImpl.h"

@interface Document : NSDocument

- (std::optional<marlin::control::source_update>)initialize;

- (marlin::control::document&)content;

@end
