#import "SourceView.h"

#import "LineNumberView.h"

NS_ASSUME_NONNULL_BEGIN

@interface IosSourceView : SourceView

@property(weak, nonatomic) LineNumberView* lineNumberView;

- (instancetype)initWithEnclosingScrollView:(UIScrollView*)enclosingScrollView
                                 dataSource:(id<SourceViewDataSource>)dataSource;

@end

NS_ASSUME_NONNULL_END
