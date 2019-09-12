#import "Types.h"

#include <optional>

#include "DrawContext.h"

NS_ASSUME_NONNULL_BEGIN

@interface ExecuteViewController : ViewController <DrawContextDelegate>

- (void)setExecutable:(NSString *)executable;

@end

NS_ASSUME_NONNULL_END
