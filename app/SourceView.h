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

- (marlin::control::source_selection)sourceView:(SourceView *)view
                                    selectionAt:(marlin::source_loc)loc;

- (marlin::control::statement_inserter)statementInserterForSourceView:(SourceView *)view;

- (marlin::control::expression_inserter)expressionInserterForSourceView:(SourceView *)view;

@end

@protocol SourceViewDelegate

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     fromRect:(Rect)rect
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data;

- (void)dismissEditorViewControllerForSourceView:(SourceView *)view;

@end

@interface SourceView : View<EditorViewControllerDelegate>

@property(weak) id<SourceViewDataSource> dataSource;
@property(weak) id<SourceViewDelegate> delegate;

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

- (BOOL)draggingStatementAtLocation:(Point)location;

- (BOOL)draggingExpressionAtLocation:(Point)location;

- (BOOL)performStatementDropForData:(NSData *)data;

- (BOOL)performExpressionDropForData:(NSData *)data;

- (void)removeDraggingSelection;

- (void)resetAll;

- (void)touchAtLocation:(Point)location;

@end

NS_ASSUME_NONNULL_END
