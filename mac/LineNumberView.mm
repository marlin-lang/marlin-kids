#import "LineNumberView.h"

#import "MessageViewController.h"
#import "SourceTheme.h"

@implementation LineNumberView {
  NSPopover* _popover;
  NSMutableDictionary* _errors;

  CGFloat _inset;
}

- (instancetype)initWithTextView:(SourceTextView*)textView {
  if (self = [super initWithScrollView:textView.enclosingScrollView orientation:NSVerticalRuler]) {
    self.clientView = textView;
    self.ruleThickness = 45;
    _errors = [NSMutableDictionary new];
    _inset = 5;
  }
  return self;
}

- (void)addError:(NSString*)message atLine:(NSUInteger)line {
  auto* number = [NSNumber numberWithInteger:line];
  if (NSMutableString* string = [_errors objectForKey:number]) {
    [string appendString:@"\n"];
    [string appendString:message];
  } else {
    [_errors setObject:[message mutableCopy] forKey:number];
  }
}

- (void)clearErrors {
  [_errors removeAllObjects];
  [self setNeedsDisplay:YES];
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
    auto* textView = (SourceTextView*)self.clientView;
    auto offset = [self convertPoint:NSZeroPoint fromView:textView];
    auto y = [textView lineTopOfNumber:lineNumber] + offset.y;
    auto size = textView.lineHeight - _inset * 2;
    auto errorRect = NSMakeRect(_inset, y + _inset, size, size);
    [_popover showRelativeToRect:errorRect ofView:self preferredEdge:NSMinYEdge];
    vc.messageTextField.stringValue = message;
  }
}

- (void)drawHashMarksAndLabelsInRect:(NSRect)rect {
  auto* textView = (SourceTextView*)self.clientView;
  auto startPoint = [self convertPoint:rect.origin toView:textView];
  auto [startLine, startColumn] = [textView sourceLocationOfPoint:startPoint];
  auto endPoint =
      [self convertPoint:NSMakePoint(rect.origin.x, rect.origin.y + rect.size.height - 1)
                  toView:textView];
  auto [endLine, endColumn] = [textView sourceLocationOfPoint:endPoint];
  auto offset = [self convertPoint:NSZeroPoint fromView:textView];
  auto height = textView.lineHeight;
  auto errorSize = height - _inset * 2;
  auto y = [textView lineTopOfNumber:startLine] + offset.y;
  for (auto line = startLine; line <= endLine; ++line) {
    auto errorRect = NSMakeRect(_inset, y + _inset, errorSize, errorSize);
    [self drawErrorIndicatorOfLine:line atRect:errorRect];

    auto* string = [NSString stringWithFormat:@"%lu", line];
    auto* attrString =
        [[NSAttributedString alloc] initWithString:string
                                        attributes:[SourceTheme new].lineNumberAttrs];
    [attrString
        drawAtPoint:NSMakePoint(self.ruleThickness - _inset - attrString.size.width, y + _inset)];
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

- (NSUInteger)lineNumberOfLocation:(NSPoint)loc {
  auto* textView = (SourceTextView*)self.clientView;
  auto textViewLocation = [self convertPoint:loc toView:textView];
  auto [line, column] = [textView sourceLocationOfPoint:textViewLocation];
  return line;
}

@end
