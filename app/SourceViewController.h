#import "Types.h"

#import "LineNumberView.h"
#import "SourceView.h"

NS_ASSUME_NONNULL_BEGIN

@interface SourceViewController
    : ViewController <CollectionViewDelegateFlowLayout, SourceViewDataSource>

@property(weak, nonatomic) IBOutlet CollectionView* toolboxView;

@property(weak, nonatomic) LineNumberView* lineNumberView;

- (void)execute;

@end

NS_ASSUME_NONNULL_END
