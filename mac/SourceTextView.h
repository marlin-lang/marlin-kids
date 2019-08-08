#import <Cocoa/Cocoa.h>

#import "base.hpp"
#import "statement_inserter.hpp"

#import "EditorViewController.h"

@protocol SourceTextViewDataSource;

@interface SourceTextView : NSTextView <EditorViewControllerDelegate>

@property(weak) id<SourceTextViewDataSource> dataSource;

@end

@protocol SourceTextViewDataSource

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceTextView*)textView;

- (marlin::ast::base&)textView:(SourceTextView*)textView
         nodeContainsSourceLoc:(marlin::source_loc)loc;

@end
