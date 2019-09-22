#import <Types.h>

#import "Pasteboard.h"

NS_ASSUME_NONNULL_BEGIN

@class DuplicateViewController;

@protocol DuplicateViewControllerDelegate

- (void)performDeleteForDuplicateViewController:(DuplicateViewController *)vc;

@end

@interface DuplicateViewController : ViewController

@property(weak, nonatomic) id<DuplicateViewControllerDelegate> delegate;

- (void)setDraggingData:(const DraggingData &)draggingData;

@end

NS_ASSUME_NONNULL_END
