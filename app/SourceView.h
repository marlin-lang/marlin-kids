#import <Types.h>

#include <string_view>
#include <vector>

#include "prototypes.hpp"
#include "source_selection.hpp"

#import "Document.h"
#import "EditorViewController.h"
#import "FunctionViewController.h"

NS_ASSUME_NONNULL_BEGIN

@class SourceView;

@protocol SourceViewDataSource

- (Document *_Nullable)document;

@end

@protocol SourceViewDelegate

- (void)sourceViewChanged:(SourceView *)view;

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     fromRect:(CGRect)rect
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data;

- (void)showFunctionViewControllerForSourceView:(SourceView *)view
                                       fromRect:(CGRect)rect
                          withFunctionSignature:
                              (marlin::control::source_selection::function_signature)signature;

- (void)dismissPopoverViewControllerForSourceView:(SourceView *)view;

@end

@interface SourceView : View<EditorViewControllerDelegate, FunctionViewControllerDelegate>

@property(weak) id<SourceViewDataSource> dataSource;
@property(weak) id<SourceViewDelegate> delegate;

@property(readonly) CGFloat lineHeight;

- (void)insertStatementsBeforeLine:(NSUInteger)line withDisplay:(marlin::format::display)display;

- (void)insertStatementsBeforeLine:(NSUInteger)line
                       withDisplay:(marlin::format::display)display
                      isInitialize:(bool)isInitialize;

- (void)updateExpressionInSourceRange:(marlin::source_range)sourceRange
                          withDisplay:(marlin::format::display)display;

- (void)removeStatementFromLine:(NSUInteger)from toLine:(NSUInteger)to;

- (void)removeExpressionInSourceRange:(marlin::source_range)sourceRange;

- (marlin::source_loc)sourceLocationOfPoint:(CGPoint)point;

- (CGFloat)lineHeight;

- (CGFloat)lineTopOfNumber:(NSUInteger)number;

- (void)addErrorInSourceRange:(marlin::source_range)range;

- (void)clearErrors;

- (BOOL)draggingPasteboardOfType:(marlin::control::pasteboard_t)type toLocation:(CGPoint)location;

- (BOOL)dropPasteboardOfType:(marlin::control::pasteboard_t)type withData:(NSData *)data;

- (void)removeDraggingSelection;

- (void)resetAll;

- (void)touchAtLocation:(CGPoint)location;

@end

NS_ASSUME_NONNULL_END
