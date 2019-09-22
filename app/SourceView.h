#import <Types.h>

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "function_definition.hpp"
#include "prototypes.hpp"

#import "Document.h"
#import "DuplicateViewController.h"
#import "EditorViewController.h"
#import "FunctionViewController.h"
#import "Pasteboard.h"

NS_ASSUME_NONNULL_BEGIN

@class SourceView;

@protocol SourceViewDataSource

- (Document *_Nullable)document;

@end

@protocol SourceViewDelegate

- (void)sourceViewChanged:(SourceView *)view;

- (void)showDuplicateViewControllerForSourceView:(SourceView *)view
                                      withString:(NSString *)string
                                    draggingData:(const DraggingData &)draggingData;

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data;

- (void)showFunctionViewControllerForSourceView:(SourceView *)view
                          withFunctionSignature:(marlin::function_definition)signature;

- (void)dismissEditorViewControllerForSourceView:(SourceView *)view;

- (void)dismissChildViewControllersForSourceView:(SourceView *)view;

@end

@interface SourceView : View <DuplicateViewControllerDelegate,
                              EditorViewControllerDelegate,
                              FunctionViewControllerDelegate>

@property(weak) id<SourceViewDataSource> dataSource;
@property(weak) id<SourceViewDelegate> delegate;

@property(readonly) CGFloat lineHeight;

- (void)initializeWithDisplay:(marlin::format::display)display;

- (marlin::source_loc)sourceLocationOfPoint:(CGPoint)point;

- (CGFloat)lineHeight;

- (CGFloat)lineTopOfNumber:(NSUInteger)number;

- (void)addErrorInSourceRange:(marlin::source_range)range;

- (void)clearErrors;

- (std::optional<DraggingData>)startDraggingAtLocation:(CGPoint)location;

- (BOOL)draggingPasteboardOfType:(marlin::control::pasteboard_t)type toLocation:(CGPoint)location;

- (BOOL)dropPasteboardOfType:(marlin::control::pasteboard_t)type
                    withData:(NSData *)data
       removingCurrentSource:(BOOL)removing;

- (BOOL)removeDraggingSource;

- (void)resetDraggingDestination;

- (void)resetDraggingSource;

- (void)touchDownAtLocation:(CGPoint)location;

- (void)touchUp;

@end

NS_ASSUME_NONNULL_END
