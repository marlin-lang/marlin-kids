#import "MacSourceView.h"

#import "NSString+StringView.h"
#import "Pasteboard.h"

constexpr NSDragOperation INTERNAL_DRAGGING_MASK = NSDragOperationMove | NSDragOperationDelete;
constexpr NSDragOperation EXTERNAL_DRAGGING_MASK = NSDragOperationCopy;

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
  [self touchDownAtLocation:location];
}

- (void)mouseUp:(NSEvent*)event {
  [super mouseDown:event];

  [self touchUp];
}

- (void)mouseDragged:(NSEvent*)event {
  [super mouseDragged:event];

  auto location = [self convertPoint:event.locationInWindow fromView:nil];
  if (auto draggingData = [self startDraggingAtLocation:location]) {
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

// TODO: draggingSourceOperationMask is not really working
// consider fixing or changing to another implementation

// TODO: set cursors

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  if (auto type = [self pasteboardTypeFromDraggingInfo:sender]) {
    auto location = [self convertPoint:sender.draggingLocation fromView:nil];
    if ([self draggingPasteboardOfType:*type toLocation:location]) {
      [self setNeedsDisplayInRect:self.bounds];
      // NSLog(@"%d", sender.draggingSourceOperationMask == INTERNAL_DRAGGING_MASK);
      if (sender.draggingSourceOperationMask & NSDragOperationMove) {
        return NSDragOperationMove;
      } else {
        return NSDragOperationCopy;
      }
    }
  }
  [self setNeedsDisplayInRect:self.bounds];
  if (sender.draggingSourceOperationMask & NSDragOperationDelete) {
    return NSDragOperationDelete;
  } else {
    return NSDragOperationNone;
  }
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  if (auto type = [self pasteboardTypeFromDraggingInfo:sender]) {
    auto data = [sender.draggingPasteboard dataForType:pasteboardOfType(*type)];
    bool withinApplication = sender.draggingSourceOperationMask == INTERNAL_DRAGGING_MASK;
    return [self dropPasteboardOfType:*type withData:data removingCurrentSource:withinApplication];
  } else {
    return NO;
  }
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
  [self resetDraggingDestination];
}

#pragma mark - NSDraggingSource implementation

- (NSDragOperation)draggingSession:(NSDraggingSession*)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  switch (context) {
    case NSDraggingContextWithinApplication:
      // NSLog(@"drag within! %ld", INTERNAL_DRAGGING_MASK);
      return INTERNAL_DRAGGING_MASK;
    case NSDraggingContextOutsideApplication:
      // NSLog(@"drag outside! %ld", EXTERNAL_DRAGGING_MASK);
      return EXTERNAL_DRAGGING_MASK;
  }
}

- (void)draggingSession:(NSDraggingSession*)session
           endedAtPoint:(NSPoint)screenPoint
              operation:(NSDragOperation)operation {
  [self resetDraggingSource];
  switch (operation) {
    case NSDragOperationCopy:
      NSLog(@"copy");
      break;
    case NSDragOperationMove:
      NSLog(@"move");
      break;
    case NSDragOperationDelete:
      NSLog(@"delete");
      break;
    case NSDragOperationNone:
      NSLog(@"none");
      break;
    default:
      NSLog(@"other");
      break;
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
