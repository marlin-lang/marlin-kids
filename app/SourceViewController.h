#import "Types.h"

#import "LineNumberView.h"
#import "SourceView.h"
#import "ToolboxViewController.h"

NS_ASSUME_NONNULL_BEGIN

@interface SourceViewController
    : ViewController<CollectionViewDelegateFlowLayout, SourceViewDataSource>

@property(weak, nonatomic) ToolboxViewController* toolboxViewController;
@property(weak, nonatomic) LineNumberView* lineNumberView;

- (void)execute;

@end

NS_ASSUME_NONNULL_END
