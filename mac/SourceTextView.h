#import <Cocoa/Cocoa.h>

#import "base.hpp"
#import "source_modifications.hpp"
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

- (marlin::control::source_replacement)textView:(SourceTextView*)textView
                           replacePlaceholderAt:(marlin::source_loc)loc
                                           type:(EditorType)type
                                     withString:(NSString*)string;

@end
