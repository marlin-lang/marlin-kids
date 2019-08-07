#import <Cocoa/Cocoa.h>

#import "statement_inserter.hpp"

#import "EditorViewController.h"

@protocol SourceTextViewDataSource;

@interface SourceTextView : NSTextView<EditorViewControllerDelegate>

@property(weak) id<SourceTextViewDataSource> dataSource;

@end

@protocol SourceTextViewDataSource

- (NSRange)textView:(SourceTextView*)textView selectRangeContainsIndex:(NSUInteger)index;

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceTextView*)textView;

@end
