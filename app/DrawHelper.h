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

inline void drawRectangle(CGRect rect, Color* fillColor) {
  drawWithBlock([rect, fillColor]() {
    auto* path = [BezierPath bezierPathWithRect:rect];
    [fillColor set];
    [path fill];
  });
}

inline void drawRoundRectangle(CGRect rect, CGFloat cornerRadius, Color* fillColor,
                               Color* strokeColor) {
  drawWithBlock([rect, cornerRadius, fillColor, strokeColor]() {
#ifdef IOS
    auto* path = [BezierPath bezierPathWithRoundedRect:rect cornerRadius:cornerRadius];
#else
    auto* path = [BezierPath bezierPathWithRoundedRect:rect
                                               xRadius:cornerRadius
                                               yRadius:cornerRadius];
#endif
    if (fillColor) {
      [fillColor setFill];
      [path fill];
    }
    if (strokeColor) {
      [strokeColor setStroke];
      [path stroke];
    }
  });
}

inline void drawOval(CGRect rect, Color* fillColor) {
  drawWithBlock([rect, fillColor]() {
    auto* path = [BezierPath bezierPathWithOvalInRect:rect];
    [fillColor set];
    [path fill];
  });
}

inline void drawLine(CGPoint start, CGPoint end, CGFloat width, Color* color) {
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
