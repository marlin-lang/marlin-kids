#import "MacSourceView.h"

#import "NSString+StringView.h"
#import "Pasteboard.h"

@interface MacSourceView () <NSDraggingSource>

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

- (void)mouseDragged:(NSEvent*)event {
  [super mouseDragged:event];
  if (auto draggingData = self.currentDraggingData) {
    auto* pasteboardItem = [[NSPasteboardItem alloc] init];
    [pasteboardItem setData:draggingData->data forType:pasteboardOfType(draggingData->type)];
    auto draggingItem = [[NSDraggingItem alloc] initWithPasteboardWriter:pasteboardItem];
    [draggingItem setDraggingFrame:NSMakeRect(0, 0, 100, 100)];
    [self beginDraggingSessionWithItems:@[ draggingItem ] event:event source:self];
  }
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

#pragma mark - NSDraggingSource implementation

- (NSDragOperation)draggingSession:(NSDraggingSession*)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  switch (context) {
    case NSDraggingContextWithinApplication:
      return NSDragOperationCopy;
    case NSDraggingContextOutsideApplication:
      return NSDragOperationMove;
  }
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
