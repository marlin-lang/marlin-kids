#import "ToolBoxHeaderView.h"

@implementation ToolBoxHeaderView

- (void)drawRect:(NSRect)dirtyRect {
  [super drawRect:dirtyRect];

  [[NSColor colorWithCalibratedWhite:0.8 alpha:1] set];
  NSRectFillUsingOperation(dirtyRect, NSCompositingOperationSourceOver);
}

@end
