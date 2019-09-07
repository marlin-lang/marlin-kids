#import "SourceView.h"

NS_ASSUME_NONNULL_BEGIN

@interface IosSourceView : SourceView

- (instancetype)initWithEnclosingScrollView:(UIScrollView*)enclosingScrollView
                                 dataSource:(id<SourceViewDataSource>)dataSource;

@end

NS_ASSUME_NONNULL_END
