#import <Cocoa/Cocoa.h>

#import "Document.h"
#import "ExecuteViewController.h"
#import "SourceTextView.h"

@interface SourceViewController : NSViewController <NSCollectionViewDataSource,
                                                    NSCollectionViewDelegateFlowLayout,
                                                    NSTextViewDelegate,
                                                    ExecuteViewControllerDelegate,
                                                    SourceTextViewDataSource>

@property(nonatomic, weak) Document* document;

- (void)execute;

@end
