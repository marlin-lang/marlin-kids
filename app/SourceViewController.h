#import "Types.h"

#import "SourceView.h"

NS_ASSUME_NONNULL_BEGIN

@interface SourceViewController
    : ViewController<CollectionViewDelegateFlowLayout, SourceViewDataSource>

@property(weak) IBOutlet CollectionView* toolboxView;

- (void)execute;

@end

NS_ASSUME_NONNULL_END
