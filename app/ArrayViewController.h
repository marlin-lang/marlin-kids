#import <Types.h>

NS_ASSUME_NONNULL_BEGIN

@class ArrayViewController;

@protocol ArrayViewControllerDelegate

- (void)arrayViewController:(ArrayViewController *)vc
     finishEditingWithCount:(NSUInteger)count;

@end

@interface ArrayViewController : ViewController

@property(weak) id<ArrayViewControllerDelegate> delegate;

@property (weak) IBOutlet NSTextField *countTextField;

- (void)setMinimalCount:(NSUInteger)minimalCount;

@end

NS_ASSUME_NONNULL_END

