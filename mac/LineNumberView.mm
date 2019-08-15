#import "LineNumberView.h"

#import "SourceTheme.h"

@implementation LineNumberView

- (instancetype)initWithTextView:(NSTextView*)textView {
  if (self = [super initWithScrollView:textView.enclosingScrollView orientation:NSVerticalRuler]) {
    self.clientView = textView;
    self.ruleThickness = 45;
  }
  return self;
}

- (void)drawHashMarksAndLabelsInRect:(NSRect)rect {
  auto* textView = (NSTextView*)self.clientView;
  auto offset = [self convertPoint:NSZeroPoint fromView:textView];
  auto* layoutManager = textView.layoutManager;
  auto visibleRange = [layoutManager glyphRangeForBoundingRect:textView.visibleRect
                                               inTextContainer:textView.textContainer];
  auto lineRange = [textView.string lineRangeForRange:NSMakeRange(visibleRange.location, 0)];
  auto lineRect = [layoutManager lineFragmentRectForGlyphAtIndex:lineRange.location
                                                  effectiveRange:nil];
  NSUInteger lineNumber = lineRect.origin.y / lineRect.size.height + 1;
  auto lineIndex = visibleRange.location;
  auto* theme = [SourceTheme new];
  while (lineIndex < NSMaxRange(visibleRange)) {
    lineRange = [textView.string lineRangeForRange:NSMakeRange(lineIndex, 0)];
    auto effectiveRange = NSMakeRange(0, 0);
    lineRect = [layoutManager lineFragmentRectForGlyphAtIndex:lineRange.location
                                               effectiveRange:&effectiveRange];
    lineIndex = NSMaxRange(effectiveRange);
    auto* string = [NSString stringWithFormat:@"%lu", (unsigned long)lineNumber];
    auto* attrString = [[NSAttributedString alloc] initWithString:string
                                                       attributes:theme.lineNumberAttrs];
    auto y = lineRect.origin.y + offset.y + (lineRect.size.height - attrString.size.height) / 2;
    [attrString drawAtPoint:NSMakePoint(self.ruleThickness - 5 - attrString.size.width, y)];
    ++lineNumber;
  }
}

@end
