#import <Cocoa/Cocoa.h>

#import "Document.h"
#import "ExecuteViewController.h"
#import "SourceTextView.h"

NS_ASSUME_NONNULL_BEGIN

@interface SourceViewController : NSViewController <NSCollectionViewDataSource,
                                                    NSCollectionViewDelegateFlowLayout,
                                                    NSTextViewDelegate,
                                                    SourceTextViewDataSource>

@property(nonatomic, weak) Document* document;

- (void)execute;

@end

NS_ASSUME_NONNULL_END
