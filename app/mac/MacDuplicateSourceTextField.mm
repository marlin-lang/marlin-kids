#import "MacDuplicateSourceTextField.h"

@interface MacDuplicateSourceTextField () <NSDraggingSource>

@end

@implementation MacDuplicateSourceTextField

- (void)setSourceString:(NSString *)sourceString {
  self.stringValue = sourceString;
}

- (void)mouseDragged:(NSEvent *)event {
  [super mouseDragged:event];

  auto pasteboardItem = [NSPasteboardItem new];
  [pasteboardItem setData:self.draggingData.data forType:pasteboardOfType(self.draggingData.type)];
  auto draggingItem = [[NSDraggingItem alloc] initWithPasteboardWriter:pasteboardItem];
  auto imageRep = [self bitmapImageRepForCachingDisplayInRect:self.bounds];
  imageRep.size = self.bounds.size;
  [self cacheDisplayInRect:self.bounds toBitmapImageRep:imageRep];
  auto image = [[NSImage alloc] init];
  [image addRepresentation:imageRep];
  [draggingItem setDraggingFrame:self.bounds contents:image];
  [self beginDraggingSessionWithItems:@[ draggingItem ] event:event source:self];
}

#pragma mark - NSDraggingSource

- (NSDragOperation)draggingSession:(NSDraggingSession *)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  return NSDragOperationEvery;
}

@end
