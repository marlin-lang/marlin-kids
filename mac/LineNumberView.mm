#import "LineNumberView.h"

#import "MessageViewController.h"
#import "SourceTheme.h"

@implementation LineNumberView {
  NSPopover* _popover;
  NSMutableDictionary* _errors;
}

- (instancetype)initWithTextView:(NSTextView*)textView {
  if (self = [super initWithScrollView:textView.enclosingScrollView orientation:NSVerticalRuler]) {
    self.clientView = textView;
    self.ruleThickness = 45;
    _errors = [NSMutableDictionary new];
  }
  return self;
}

- (void)addError:(NSString*)message atIndex:(NSUInteger)index {
  auto* number = [NSNumber numberWithInteger:[self lineNumberContainsCharacterAtIndex:index]];
  if (NSMutableString* string = [_errors objectForKey:number]) {
    [string appendString:@"\n"];
    [string appendString:message];
  } else {
    [_errors setObject:[message mutableCopy] forKey:number];
  }
}

- (void)clearErrors {
  [_errors removeAllObjects];
}

- (void)mouseDown:(NSEvent*)event {
  [super mouseDown:event];

  auto location = [self convertPoint:event.locationInWindow fromView:nil];
  auto lineNumber = [self lineNumberOfLocation:location];
  if (NSString* message = [_errors objectForKey:[NSNumber numberWithInteger:lineNumber]]) {
    auto* storyboard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    MessageViewController* vc =
        [storyboard instantiateControllerWithIdentifier:@"MessageViewController"];
    _popover = [NSPopover new];
    _popover.behavior = NSPopoverBehaviorTransient;
    _popover.contentViewController = vc;
    auto* textView = (NSTextView*)self.clientView;
    auto offset = [self convertPoint:NSZeroPoint fromView:textView];
    auto height = self.lineHeight;
    auto y = textView.textContainerInset.height + (lineNumber - 1) * height + offset.y;
    auto size = self.lineHeight - 10;
    auto errorRect = NSMakeRect(5, y + 5, size, size);
    [_popover showRelativeToRect:errorRect ofView:self preferredEdge:NSMinYEdge];
    vc.messageTextField.stringValue = message;
  }
}

- (void)drawHashMarksAndLabelsInRect:(NSRect)rect {
  auto* textView = (NSTextView*)self.clientView;
  auto* layoutManager = textView.layoutManager;
  auto visibleRange = [layoutManager glyphRangeForBoundingRect:textView.visibleRect
                                               inTextContainer:textView.textContainer];
  auto beginLine = [self lineNumberContainsCharacterAtIndex:visibleRange.location];
  auto endLine = [self lineNumberContainsCharacterAtIndex:NSMaxRange(visibleRange) - 1];
  auto* theme = [SourceTheme new];
  auto offset = [self convertPoint:NSZeroPoint fromView:textView];
  auto height = self.lineHeight;
  auto errorSize = height - 10;
  auto y = textView.textContainerInset.height + (beginLine - 1) * height + offset.y;
  for (auto line = beginLine; line <= endLine; ++line) {
    auto errorRect = NSMakeRect(5, y + 5, errorSize, errorSize);
    [self drawErrorIndicatorOfLine:line atRect:errorRect];

    auto* string = [NSString stringWithFormat:@"%lu", line];
    auto* attrString = [[NSAttributedString alloc] initWithString:string
                                                       attributes:theme.lineNumberAttrs];
    [attrString drawAtPoint:NSMakePoint(self.ruleThickness - 5 - attrString.size.width, y + 5)];
    y += height;
  }
}

- (void)drawErrorIndicatorOfLine:(NSUInteger)number atRect:(NSRect)rect {
  if ([_errors objectForKey:[NSNumber numberWithInteger:number]]) {
    [NSGraphicsContext saveGraphicsState];
    auto* circle = [NSBezierPath bezierPathWithOvalInRect:rect];
    [[NSColor redColor] set];
    [circle fill];
    [NSGraphicsContext restoreGraphicsState];
  }
}

- (CGFloat)lineHeight {
  auto* textView = (NSTextView*)self.clientView;
  auto* layoutManager = textView.layoutManager;
  auto lineRect = [layoutManager lineFragmentRectForGlyphAtIndex:0 effectiveRange:nil];
  return lineRect.size.height;
}

- (NSUInteger)lineNumberContainsCharacterAtIndex:(NSUInteger)index {
  auto* textView = (NSTextView*)self.clientView;
  auto* layoutManager = textView.layoutManager;
  auto lineRect = [layoutManager lineFragmentRectForGlyphAtIndex:index effectiveRange:nil];
  return (lineRect.origin.y - textView.textContainerInset.height) / lineRect.size.height + 1;
}

- (NSUInteger)lineNumberOfLocation:(NSPoint)loc {
  auto* textView = (NSTextView*)self.clientView;
  auto* layoutManager = textView.layoutManager;
  auto point = [self convertPoint:loc fromView:textView];
  auto lineRect = [layoutManager lineFragmentRectForGlyphAtIndex:0 effectiveRange:nil];
  return (point.y - textView.textContainerInset.height) / lineRect.size.height + 1;
}

@end
