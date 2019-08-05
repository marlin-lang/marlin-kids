#import <Cocoa/Cocoa.h>

@protocol SourceTextViewDataSource;

@interface SourceTextView : NSTextView

@property(weak) id<SourceTextViewDataSource> dataSource;

@end

@protocol SourceTextViewDataSource

- (NSRange)textView:(SourceTextView*)textView selectRangeContainsIndex:(NSUInteger)index;

- (NSString*)textView:(SourceTextView*)textView
    insertStatementByIndex:(NSUInteger)index
                    atLine:(NSUInteger)line;

@end
