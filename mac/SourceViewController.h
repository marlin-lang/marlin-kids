#import <Cocoa/Cocoa.h>

#import "Document.h"
#import "SourceTextView.h"

@interface SourceViewController
    : NSViewController<NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout,
                       NSTextViewDelegate, SourceTextViewDataSource>

@property(nonatomic, weak) Document* document;

@end
