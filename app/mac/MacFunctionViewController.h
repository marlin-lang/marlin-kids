#import "FunctionViewController.h"

#include "source_selection.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface MacFunctionViewController : FunctionViewController

- (void)setFunctionSignature:(marlin::control::source_selection::function_signature)signature;

@end

NS_ASSUME_NONNULL_END
