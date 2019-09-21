#import "MacSourceView.h"

#include <optional>

#import "NSString+StringView.h"
#import "Pasteboard.h"

@interface MacSourceView () <NSDraggingSource>

@end

@implementation MacSourceView {
  NSDragOperation _operation;
  std::optional<NSInteger> _currentDraggingSourceSession;
}

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
    auto* pasteboardItem = [NSPasteboardItem new];
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
    pasteboardOfType(marlin::control::pasteboard_t::expression),
    pasteboardOfType(marlin::control::pasteboard_t::reference)
  ];
}

// TODO: set cursors

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
  [NSCursor.arrowCursor push];
  return [self draggingUpdated:sender];
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  if (sender.draggingSequenceNumber == _currentDraggingSourceSession) {
    _operation = NSDragOperationDelete;
  } else {
    _operation = NSDragOperationNone;
  }
  if (const auto type = [self pasteboardTypeFromDraggingInfo:sender]) {
    const auto location = [self convertPoint:sender.draggingLocation fromView:nil];
    if ([self draggingPasteboardOfType:*type toLocation:location]) {
      if (sender.draggingSequenceNumber == _currentDraggingSourceSession) {
        _operation = NSDragOperationMove;
      } else {
        _operation = NSDragOperationCopy;
      }
    }
  }

  switch (_operation) {
    case NSDragOperationDelete:
      [NSCursor.disappearingItemCursor set];
      break;
    case NSDragOperationNone:
      [NSCursor.operationNotAllowedCursor set];
      break;
    default:
      [NSCursor.arrowCursor set];
  }

  [self setNeedsDisplayInRect:self.bounds];
  return _operation;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  [NSCursor pop];
  if (_operation == NSDragOperationCopy || _operation == NSDragOperationMove) {
    if (auto type = [self pasteboardTypeFromDraggingInfo:sender]) {
      auto data = [sender.draggingPasteboard dataForType:pasteboardOfType(*type)];
      bool removingSource = _operation == NSDragOperationMove;
      return [self dropPasteboardOfType:*type withData:data removingCurrentSource:removingSource];
    } else {
      return NO;
    }
  } else if (_operation == NSDragOperationDelete) {
    return [self removeDraggingSource];
  } else {
    return NO;
  }
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
  [NSCursor pop];
  [self resetDraggingDestination];
}

#pragma mark - NSDraggingSource implementation

- (NSDragOperation)draggingSession:(NSDraggingSession*)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  return NSDragOperationEvery;
}

- (void)draggingSession:(NSDraggingSession*)session willBeginAtPoint:(NSPoint)screenPoint {
  _currentDraggingSourceSession = session.draggingSequenceNumber;
}

- (void)draggingSession:(NSDraggingSession*)session
           endedAtPoint:(NSPoint)screenPoint
              operation:(NSDragOperation)operation {
  _currentDraggingSourceSession = std::nullopt;
  [self resetDraggingSource];

  switch (operation) {
    case NSDragOperationCopy:
      NSLog(@"performed drag: copy");
      break;
    case NSDragOperationMove:
      NSLog(@"performed drag: move");
      break;
    case NSDragOperationDelete:
      NSLog(@"performed drag: delete");
      break;
    case NSDragOperationNone:
      NSLog(@"performed drag: none");
      break;
    default:
      NSLog(@"performed drag: other");
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
  } else if ([type isEqualToString:pasteboardOfType(marlin::control::pasteboard_t::reference)]) {
    return marlin::control::pasteboard_t::reference;
  } else {
    return std::nullopt;
  }
}

@end
