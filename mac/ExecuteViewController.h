#import <Cocoa/Cocoa.h>

#import <optional>

#import "exec_env.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface ExecuteViewController : NSViewController

@property std::optional<marlin::control::exec_environment> environment;

@end

NS_ASSUME_NONNULL_END
