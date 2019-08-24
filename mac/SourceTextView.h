#import <Cocoa/Cocoa.h>

#import <string_view>
#import <vector>

#import "expression_inserter.hpp"
#import "source_selection.hpp"
#import "statement_inserter.hpp"

#import "EditorViewController.h"

@class SourceTextView;

@protocol SourceTextViewDataSource

- (marlin::control::source_selection)textView:(SourceTextView *)textView
                                  selectionAt:(marlin::source_loc)loc;

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceTextView *)textView;

- (marlin::control::expression_inserter)expressionInserterForTextView:(SourceTextView *)textView;

@end

@interface SourceTextView : NSView <EditorViewControllerDelegate>

@property(weak) id<SourceTextViewDataSource> dataSource;

- (void)insertBeforeLine:(NSUInteger)line
              withSource:(std::string_view)source
              highlights:(std::vector<marlin::control::highlight_token>)highlights;

- (void)updateInRange:(NSRange)range
           withSource:(std::string_view)source
           highlights:(std::vector<marlin::control::highlight_token>)highlights;

- (NSUInteger)addErrorAtSourceRange:(marlin::source_range)range;

- (void)clearErrors;

@end
