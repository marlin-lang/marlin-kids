#import "MacSourceView.h"

#include "toolbox_model.hpp"

#import "NSString+StringView.h"
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

- (void)mouseDown:(NSEvent*)event {
  [super mouseDown:event];

  auto location = [self convertPoint:event.locationInWindow fromView:nil];
  [self touchAtLocation:location];
}

- (NSArray<NSPasteboardType>*)acceptableDragTypes {
  return @[
    pasteboardOfType(marlin::control::pasteboard_t::block),
    pasteboardOfType(marlin::control::pasteboard_t::statement),
    pasteboardOfType(marlin::control::pasteboard_t::expression)
  ];
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  if (auto type = [self pasteboardTypeFromDraggingInfo:sender]) {
    auto location = [self convertPoint:sender.draggingLocation fromView:nil];
    if ([self draggingPasteboardOfType:*type toLocation:location]) {
      [self setNeedsDisplayInRect:self.bounds];
      return NSDragOperationMove;
    }
  }
  [self setNeedsDisplayInRect:self.bounds];
  return NSDragOperationDelete;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  if (auto type = [self pasteboardTypeFromDraggingInfo:sender]) {
    auto* data = [sender.draggingPasteboard dataForType:pasteboardOfType(*type)];
    return [self dropPasteboardOfType:*type withData:data];
  } else {
    [self removeDraggingSelection];
    return NO;
  }
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
  if (_selection->is_block()) {
      [pasteboard setData:[NSData dataWithDataView:_selection->get_data()]
                  forType:pasteboardOfType(marlin::control::pasteboard_t::block)];
  } else if (_selection->is_statement()) {
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

#pragma mark - Private methods

- (std::optional<marlin::control::pasteboard_t>)pasteboardTypeFromDraggingInfo:
    (id<NSDraggingInfo>)sender {
  auto* type = [sender.draggingPasteboard availableTypeFromArray:self.acceptableDragTypes];
  if ([type isEqualToString:pasteboardOfType(marlin::control::pasteboard_t::block)]) {
    return marlin::control::pasteboard_t::block;
  } else if ([type isEqualToString:pasteboardOfType(marlin::control::pasteboard_t::statement)]) {
    return marlin::control::pasteboard_t::statement;
  } else if ([type isEqualToString:pasteboardOfType(marlin::control::pasteboard_t::expression)]) {
    return marlin::control::pasteboard_t::expression;
  } else {
    return std::nullopt;
  }
}

@end
