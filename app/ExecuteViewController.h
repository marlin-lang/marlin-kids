#import "Types.h"

#include <optional>

#include "DrawContext.h"
#include "exec_env.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface ExecuteViewController : ViewController <DrawContextDelegate>

- (void)setEnvironment:(marlin::control::exec_environment)environment;

@end

NS_ASSUME_NONNULL_END
