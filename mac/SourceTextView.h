#import <Cocoa/Cocoa.h>

#import "statement_inserter.hpp"

@protocol SourceTextViewDataSource;

@interface SourceTextView : NSTextView

@property(weak) id<SourceTextViewDataSource> dataSource;

@end

@protocol SourceTextViewDataSource

- (NSRange)textView:(SourceTextView*)textView selectRangeContainsIndex:(NSUInteger)index;

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceTextView*)textView;

@end
