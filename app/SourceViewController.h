#import "Types.h"

#import "Document.h"
#import "ExecuteViewController.h"
#import "SourceView.h"

NS_ASSUME_NONNULL_BEGIN

@interface SourceViewController : ViewController <CollectionViewDataSource,
                                                  CollectionViewDelegateFlowLayout,
                                                  SourceViewDataSource>

@property(weak, nonatomic) Document* document;

- (void)execute;

@end

NS_ASSUME_NONNULL_END
