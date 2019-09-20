#import <Types.h>

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "function_definition.hpp"
#include "prototypes.hpp"

#import "Document.h"
#import "EditorViewController.h"
#import "FunctionViewController.h"

NS_ASSUME_NONNULL_BEGIN

@class SourceView;

struct DraggingData {
  marlin::control::pasteboard_t type;
  NSData *data;

  DraggingData(marlin::control::pasteboard_t _type, NSData *_data) : type{_type}, data{_data} {}
};

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
                          withFunctionSignature:(marlin::function_definition)signature;

- (void)dismissPopoverViewControllerForSourceView:(SourceView *)view;

@end

@interface SourceView : View <EditorViewControllerDelegate, FunctionViewControllerDelegate>

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
