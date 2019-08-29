#import <Cocoa/Cocoa.h>

#import <optional>

#import "DrawContext.h"
#import "exec_env.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface ExecuteViewController : NSViewController <DrawContextDelegate>

- (void)setEnvironment:(marlin::control::exec_environment)environment;

@end

NS_ASSUME_NONNULL_END
