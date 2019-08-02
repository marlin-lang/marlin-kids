#import <Cocoa/Cocoa.h>

@protocol SourceTextViewDataSource;

@interface SourceTextView : NSTextView

@property (weak) id<SourceTextViewDataSource> dataSource;

@end

@protocol SourceTextViewDataSource

- (NSRange)textView:(SourceTextView*)textView selectRageContainsIndex:(NSUInteger)index;

@end
