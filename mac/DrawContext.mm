#import "DrawContext.h"

void DrawContext::initWithImage(NSImage* image, id<DrawContextDelegate> delegate) {
  _size = image.size;
  _imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                      pixelsWide:_size.width
                                                      pixelsHigh:_size.height
                                                   bitsPerSample:8
                                                 samplesPerPixel:4
                                                        hasAlpha:YES
                                                        isPlanar:NO
                                                  colorSpaceName:NSDeviceRGBColorSpace
                                                     bytesPerRow:4 * _size.width
                                                    bitsPerPixel:32];
  _delegate = delegate;
  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext
      setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:_imageRep]];
  auto* rect = [NSBezierPath bezierPathWithRect:NSMakeRect(0, 0, _size.width, _size.height)];
  [NSColor.blackColor set];
  [rect fill];
  [NSGraphicsContext restoreGraphicsState];
  [_delegate setNeedRefreshImage];
}

void DrawContext::draw_line(double from_x, double from_y, double to_x, double to_y) {
  auto from = NSMakePoint(static_cast<CGFloat>(from_x), static_cast<CGFloat>(from_y));
  auto to = NSMakePoint(static_cast<CGFloat>(to_x), static_cast<CGFloat>(to_y));

  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext
      setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:_imageRep]];
  [NSColor.whiteColor set];
  NSBezierPath.defaultLineWidth = 2;
  [NSBezierPath strokeLineFromPoint:from toPoint:to];
  [NSGraphicsContext restoreGraphicsState];
  [_delegate setNeedRefreshImage];
}
