#import <Types.h>

NS_ASSUME_NONNULL_BEGIN

@class ArrayViewController;

@protocol ArrayViewControllerDelegate

- (void)arrayViewController:(ArrayViewController *)vc finishEditingWithCount:(NSUInteger)count;

@end

@interface ArrayViewController : ViewController

@property(weak) IBOutlet TextField *countTextField;

@property(weak) id<ArrayViewControllerDelegate> delegate;

@property(getter=isValid) BOOL valid;

- (void)setCount:(NSUInteger)count;

- (void)setMinimalCount:(NSUInteger)minimalCount;

- (void)validate;

@end

NS_ASSUME_NONNULL_END
