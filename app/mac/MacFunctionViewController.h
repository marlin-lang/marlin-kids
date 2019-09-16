#import "FunctionViewController.h"

#include "function_definition.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface MacFunctionViewController : FunctionViewController

- (void)setFunctionSignature:(marlin::function_definition)signature;

@end

NS_ASSUME_NONNULL_END
