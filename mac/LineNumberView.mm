#import "LineNumberView.h"

@implementation LineNumberView

- (instancetype)initWithTextView:(NSTextView*)textView {
  if (self = [super initWithScrollView:textView.enclosingScrollView orientation:NSVerticalRuler]) {
    self.clientView = textView;
    self.ruleThickness = 40;
  }
  return self;
}

- (void)drawHashMarksAndLabelsInRect:(NSRect)rect {
  auto* textView = (NSTextView*)self.clientView;
  auto* layoutManager = textView.layoutManager;
  auto visibleRange = [layoutManager glyphRangeForBoundingRect:textView.visibleRect
                                               inTextContainer:textView.textContainer];
  auto firstVisibleIndex = [layoutManager characterIndexForGlyphAtIndex:visibleRange.location];
  auto line = 0;
  auto lineIndex = visibleRange.location;
  while (lineIndex < NSMaxRange(visibleRange)) {
    auto lineRange = [textView.string lineRangeForRange:NSMakeRange(lineIndex, 0)];
    auto effectiveRange = NSMakeRange(0, 0);
    auto lineRect = [layoutManager lineFragmentRectForGlyphAtIndex:lineRange.location
                                                    effectiveRange:&effectiveRange];
    lineIndex = NSMaxRange(effectiveRange);
    auto* stringValue = [NSString stringWithFormat:@"%d", line];
    auto* string = [[NSAttributedString alloc] initWithString:stringValue attributes:nil];
    [string drawAtPoint:NSMakePoint(0, lineRect.origin.y)];
    ++line;
  }
}

@end
