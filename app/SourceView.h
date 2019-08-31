#import <Types.h>

#include <string_view>
#include <vector>

#include "expression_inserter.hpp"
#include "source_selection.hpp"
#include "statement_inserter.hpp"

#import "EditorViewController.h"

NS_ASSUME_NONNULL_BEGIN

@class SourceView;

@protocol SourceViewDataSource

- (marlin::control::source_selection)textView:(SourceView *)view
                                  selectionAt:(marlin::source_loc)loc;

- (marlin::control::statement_inserter)statementInserterForTextView:(SourceView *)view;

- (marlin::control::expression_inserter)expressionInserterForTextView:(SourceView *)view;

@end

#ifdef IOS

@interface SourceView : View <EditorViewControllerDelegate>

#else

@interface SourceView
    : View <EditorViewControllerDelegate, NSPasteboardItemDataProvider, NSDraggingSource>

#endif

@property(weak) id<SourceViewDataSource> dataSource;

@property(readonly) CGFloat lineHeight;

- (void)insertStatementsBeforeLine:(NSUInteger)line
                        withSource:(std::string_view)source
                        highlights:(std::vector<marlin::control::highlight_token>)highlights;

- (void)updateExpressionInSourceRange:(marlin::source_range)sourceRange
                           withSource:(std::string_view)source
                           highlights:(std::vector<marlin::control::highlight_token>)highlights;

- (void)removeStatementFromLine:(NSUInteger)from toLine:(NSUInteger)to;

- (void)removeExpressionInSourceRange:(marlin::source_range)sourceRange;

- (marlin::source_loc)sourceLocationOfPoint:(Point)point;

- (CGFloat)lineHeight;

- (CGFloat)lineTopOfNumber:(NSUInteger)number;

- (void)addErrorInSourceRange:(marlin::source_range)range;

- (void)clearErrors;

@end

NS_ASSUME_NONNULL_END
