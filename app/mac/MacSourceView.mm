#import "MacSourceView.h"

#include "toolbox_model.hpp"

#import "Pasteboard.h"

@interface MacSourceView () <NSPasteboardItemDataProvider, NSDraggingSource>

@end

@implementation MacSourceView

- (instancetype)initWithCoder:(NSCoder*)decoder {
  if (self = [super initWithCoder:decoder]) {
    [self registerForDraggedTypes:self.acceptableDragTypes];
  }
  return self;
}

- (NSArray<NSPasteboardType>*)acceptableDragTypes {
  return @[
    pasteboardOfType(marlin::control::pasteboard_t::statement),
    pasteboardOfType(marlin::control::pasteboard_t::expression)
  ];
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  auto location = [self convertPoint:sender.draggingLocation fromView:nil];

  auto* type = [sender.draggingPasteboard availableTypeFromArray:self.acceptableDragTypes];
  if ([type isEqualToString:pasteboardOfType(marlin::control::pasteboard_t::statement)]) {
    if ([self draggingStatementAtLocation:location]) {
      [self setNeedsDisplayInRect:self.bounds];
      return NSDragOperationMove;
    }
  } else if ([type isEqualToString:pasteboardOfType(marlin::control::pasteboard_t::expression)]) {
    if ([self draggingExpressionAtLocation:location]) {
      [self setNeedsDisplayInRect:self.bounds];
      return NSDragOperationMove;
    }
  }
  [self setNeedsDisplayInRect:self.bounds];
  return NSDragOperationDelete;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  auto type = [sender.draggingPasteboard availableTypeFromArray:self.acceptableDragTypes];
  if (type == pasteboardOfType(marlin::control::pasteboard_t::statement)) {
    auto* data = [sender.draggingPasteboard
        dataForType:pasteboardOfType(marlin::control::pasteboard_t::statement)];
    return [self performStatementDropForData:data];
  } else if (type == pasteboardOfType(marlin::control::pasteboard_t::expression)) {
    auto* data = [sender.draggingPasteboard
        dataForType:pasteboardOfType(marlin::control::pasteboard_t::expression)];
    return [self performExpressionDropForData:data];
  }
  [self removeDraggingSelection];
  return YES;
}

- (void)draggingEnded:(id<NSDraggingInfo>)sender {
  [self resetAll];
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
  [self resetAll];
}

#pragma mark - NSPasteboardItemDataProvider

- (void)pasteboard:(NSPasteboard*)pasteboard
                  item:(NSPasteboardItem*)item
    provideDataForType:(NSPasteboardType)type {
  /*NSAssert(_isDraggingFromSelection, @"Should be in dragging");
  if (_selection->is_statement()) {
      [pasteboard setData:[NSData dataWithDataView:_selection->get_data()]
                  forType:pasteboardOfType(marlin::control::pasteboard_t::statement)];
  } else if (_selection->is_expression()) {
      [pasteboard setData:[NSData dataWithDataView:_selection->get_data()]
                  forType:pasteboardOfType(marlin::control::pasteboard_t::expression)];
  }*/
}

#pragma mark - NSDraggingSource implementation

- (NSDragOperation)draggingSession:(NSDraggingSession*)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  return NSDragOperationMove;
}

@end
