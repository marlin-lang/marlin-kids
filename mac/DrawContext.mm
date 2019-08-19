#import "DrawContext.h"

void DrawContext::initWithImage(NSImage* image, id<DrawContextDelegate> delegate) {
  imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                     pixelsWide:image.size.width
                                                     pixelsHigh:image.size.height
                                                  bitsPerSample:8
                                                samplesPerPixel:4
                                                       hasAlpha:YES
                                                       isPlanar:NO
                                                 colorSpaceName:NSDeviceRGBColorSpace
                                                    bytesPerRow:4 * image.size.width
                                                   bitsPerPixel:32];
  _delegate = delegate;
  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext
      setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:imageRep]];
  auto* rect =
      [NSBezierPath bezierPathWithRect:NSMakeRect(0, 0, image.size.width, image.size.height)];
  [NSColor.blackColor set];
  [rect fill];
  [NSGraphicsContext restoreGraphicsState];
  [_delegate setNeedRefreshImage];
}

void DrawContext::drawLine(NSPoint from, NSPoint to) {
  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext
      setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:imageRep]];
  [NSColor.whiteColor set];
  NSBezierPath.defaultLineWidth = 2;
  [NSBezierPath strokeLineFromPoint:from toPoint:to];
  [NSGraphicsContext restoreGraphicsState];
  [_delegate setNeedRefreshImage];
}
