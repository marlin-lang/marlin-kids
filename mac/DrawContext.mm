#import "DrawContext.h"

void DrawContext::initWithImage(NSImage* image, id<DrawContextDelegate> delegate) {
  imageRep = [[NSBitmapImageRep alloc] initWithCGImage:[image CGImageForProposedRect:nil
                                                                              context:nil
                                                                                hints:nil]];
  _delegate = delegate;
  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext
      setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:imageRep]];
  auto* rect =
      [NSBezierPath bezierPathWithRect:NSMakeRect(0, 0, image.size.width, image.size.height)];
  [NSColor.blackColor set];
  [rect fill];
  [NSGraphicsContext restoreGraphicsState];
    [_delegate refreshImage];
}

void DrawContext::drawLine(NSPoint from, NSPoint to) {
    execute([this, &from, &to]() {
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext
         setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:imageRep]];
        [NSColor.whiteColor set];
        [NSBezierPath strokeLineFromPoint:from toPoint:to];
        [NSGraphicsContext restoreGraphicsState];
    });
}
