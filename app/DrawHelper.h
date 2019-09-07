#import "Types.h"

template <typename Block>
inline void drawWithBlock(Block block) {
#ifdef IOS
  CGContextRef context = UIGraphicsGetCurrentContext();
  CGContextSaveGState(context);
#else
  [NSGraphicsContext saveGraphicsState];
#endif

  block();

#ifdef IOS
  CGContextRestoreGState(context);
#else
  [NSGraphicsContext restoreGraphicsState];
#endif
}

inline void drawRectangle(Rect rect, Color* fillColor) {
  drawWithBlock([rect, fillColor]() {
    auto* path = [BezierPath bezierPathWithRect:rect];
    [fillColor set];
    [path fill];
  });
}

inline void drawOval(Rect rect, Color* fillColor) {
  drawWithBlock([rect, fillColor]() {
    auto* path = [BezierPath bezierPathWithOvalInRect:rect];
    [fillColor set];
    [path fill];
  });
}

inline void drawLine(Point start, Point end, CGFloat width, Color* color) {
  drawWithBlock([start, end, width, color]() {
    auto* line = [BezierPath bezierPath];
    [line moveToPoint:start];
#ifdef IOS
    [line addLineToPoint:end];
#else
    [line lineToPoint:end];
#endif
    [line setLineWidth:width];
    [color set];
    [line stroke];
  });
}
