#import "DuplicateSourceView.h"

@interface DuplicateSourceView () <NSDraggingSource>

@end

@implementation DuplicateSourceView

- (void)mouseDragged:(NSEvent*)event {
  [super mouseDragged:event];

  auto pasteboardItem = [NSPasteboardItem new];
  [pasteboardItem setData:self.draggingData.data forType:pasteboardOfType(self.draggingData.type)];
  auto draggingItem = [[NSDraggingItem alloc] initWithPasteboardWriter:pasteboardItem];
  [draggingItem setDraggingFrame:NSMakeRect(0, 0, 100, 100)];
  [self beginDraggingSessionWithItems:@[ draggingItem ] event:event source:self];
}

#pragma mark - NSDraggingSource

- (NSDragOperation)draggingSession:(NSDraggingSession*)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  return NSDragOperationEvery;
}

@end
