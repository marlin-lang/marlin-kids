#import <Types.h>

#include <optional>

#include "document.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface Document : AppleDocument

@property(strong, nonatomic, nullable) NSData *initialData;

- (std::optional<marlin::control::source_update>)initialize;

- (marlin::control::document &)content;

@end

NS_ASSUME_NONNULL_END
