#import "ToolboxHeaderView.h"

@implementation ToolboxHeaderView

- (void)drawRect:(NSRect)dirtyRect {
  [super drawRect:dirtyRect];

    [[NSColor colorWithCalibratedWhite:0.8 alpha:1] setFill];
    NSRectFill(dirtyRect);
}

@end
