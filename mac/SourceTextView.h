#import <Cocoa/Cocoa.h>

#import <string>
#import <vector>

#import "expression_inserter.hpp"
#import "source_selection.hpp"
#import "statement_inserter.hpp"

#import "EditorViewController.h"

@protocol SourceTextViewDataSource;

@interface SourceTextView : NSTextView <EditorViewControllerDelegate>

@property(weak) id<SourceTextViewDataSource> dataSource;

- (void)updateInRange:(NSRange)range
           withSource:(std::string)source
           highlights:(std::vector<marlin::control::highlight_token>)highlights;

@end

@protocol SourceTextViewDataSource

- (marlin::control::source_selection)textView:(SourceTextView *)textView
                                  selectionAt:(marlin::source_loc)loc;

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceTextView *)textView;

- (marlin::control::expression_inserter)expressionInserterForTextView:(SourceTextView *)textView;

@end
