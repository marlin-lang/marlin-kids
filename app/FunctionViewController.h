#import <Types.h>

NS_ASSUME_NONNULL_BEGIN

@class FunctionViewController;

@protocol FunctionViewControllerDelegate

- (void)viewController:(FunctionViewController *)vc
    finishEditingWithName:(NSString *)name
               parameters:(NSArray<NSString *> *)parameters;

@end

@interface FunctionViewController : ViewController

@property(weak) id<FunctionViewControllerDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
