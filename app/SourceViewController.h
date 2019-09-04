#import "Types.h"

#import "Document.h"
#import "ExecuteViewController.h"
#import "SourceView.h"

NS_ASSUME_NONNULL_BEGIN

@interface SourceViewController
    : ViewController<CollectionViewDelegateFlowLayout, SourceViewDataSource>

@property(weak) IBOutlet CollectionView* toolboxView;

#ifdef IOS
@property(strong, nonatomic) Document* document;
#else
@property(weak, nonatomic) Document* document;
#endif

- (void)execute;

@end

NS_ASSUME_NONNULL_END
