#import <Cocoa/Cocoa.h>

#import "Document.h"
#import "SourceTextView.h"

@interface SourceViewController : NSViewController <NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout, SourceTextViewDataSource>

@property(nonatomic, weak) Document* document;

- (void)setNeedsUpdate;

@end
