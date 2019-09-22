#import <Types.h>

NS_ASSUME_NONNULL_BEGIN

@class DuplicateViewController;

@protocol DuplicateViewControllerDelegate

- (void)performDeleteForDuplicateViewController:(DuplicateViewController *)vc;

@end

@interface DuplicateViewController : ViewController

@property(weak, nonatomic) id<DuplicateViewControllerDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
