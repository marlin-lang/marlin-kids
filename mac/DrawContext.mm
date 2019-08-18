#import "DrawContext.h"

void DrawContext::initWithImage(NSImage* image, id<DrawContextDelegate> delegate) {
  _imageRep = [[NSBitmapImageRep alloc] initWithCGImage:[image CGImageForProposedRect:nil
                                                                              context:nil
                                                                                hints:nil]];
  _delegate = delegate;
  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext
      setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:_imageRep]];
  auto* rect =
      [NSBezierPath bezierPathWithRect:NSMakeRect(0, 0, image.size.width, image.size.height)];
  [NSColor.blackColor set];
  [rect fill];
  [NSGraphicsContext restoreGraphicsState];
  [_delegate applyImageRep:_imageRep];
}

void DrawContext::drawLine(NSPoint from, NSPoint to) {
  dispatch_sync(dispatch_get_main_queue(), ^{
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext
        setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:_imageRep]];
    [NSColor.whiteColor set];
    [NSBezierPath strokeLineFromPoint:from toPoint:to];
    [NSGraphicsContext restoreGraphicsState];
    [_delegate applyImageRep:_imageRep];
  });
}
