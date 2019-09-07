#import "LineNumberView.h"

#import "DrawHelper.h"
#import "MessageViewController.h"
#import "Theme.h"

@interface LineNumberView ()

#ifdef IOS
@property(weak, nonatomic) SourceView* clientView;
#endif

@end

@implementation LineNumberView {
  // NSPopover* _popover;

  NSMutableDictionary* _errors;

  CGFloat _inset;
}

- (instancetype)initWithSourceView:(SourceView*)view {
#ifdef IOS
  if (self = [super init]) {
#else
  if (self = [super initWithScrollView:view.enclosingScrollView orientation:NSVerticalRuler]) {
#endif
    self.ruleThickness = 45;
    self.clientView = view;
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
  [self setNeedsDisplayInRect:self.bounds];
}

/*- (void)mouseDown:(NSEvent*)event {
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
      auto* textView = (SourceView*)self.clientView;
    auto offset = [self convertPoint:NSZeroPoint fromView:textView];
    auto y = [textView lineTopOfNumber:lineNumber] + offset.y;
    auto size = textView.lineHeight - _inset * 2;
    auto errorRect = NSMakeRect(_inset, y + _inset, size, size);
    [_popover showRelativeToRect:errorRect ofView:self preferredEdge:NSMinYEdge];
    vc.messageTextField.stringValue = message;
  }
}*/

#ifdef IOS
- (void)drawRect:(Rect)dirtyRect {
  [self drawHashMarksAndLabelsInRect:dirtyRect];
}
#endif

- (void)drawHashMarksAndLabelsInRect:(Rect)rect {
  auto* sourceView = (SourceView*)self.clientView;
  auto startPoint = [self convertPoint:rect.origin toView:sourceView];
  auto [startLine, startColumn] = [sourceView sourceLocationOfPoint:startPoint];
  auto endPoint = [self convertPoint:MakePoint(rect.origin.x, rect.origin.y + rect.size.height - 1)
                              toView:sourceView];
  auto [endLine, endColumn] = [sourceView sourceLocationOfPoint:endPoint];
  auto offset = [self convertPoint:ZeroPoint fromView:sourceView];
  auto height = sourceView.lineHeight;
  auto errorSize = height - _inset * 2;
  auto y = [sourceView lineTopOfNumber:startLine] + offset.y;
  for (auto line = startLine; line <= endLine; ++line) {
    auto errorRect = MakeRect(_inset, y + _inset, errorSize, errorSize);
    [self drawErrorIndicatorOfLine:line atRect:errorRect];

    auto* string = [NSString stringWithFormat:@"%lu", line];
    auto* attrString = [[NSAttributedString alloc] initWithString:string
                                                       attributes:currentTheme().lineNumberAttrs];
    [attrString
        drawAtPoint:MakePoint(self.ruleThickness - _inset - attrString.size.width, y + _inset)];
    y += height;
  }
}

- (void)drawErrorIndicatorOfLine:(NSUInteger)number atRect:(Rect)rect {
  if ([_errors objectForKey:[NSNumber numberWithInteger:number]]) {
    drawOval(rect, Color.redColor);
  }
}

- (NSUInteger)lineNumberOfLocation:(Point)loc {
  auto* sourceView = (SourceView*)self.clientView;
  auto sourceViewLocation = [self convertPoint:loc toView:sourceView];
  auto [line, column] = [sourceView sourceLocationOfPoint:sourceViewLocation];
  return line;
}

@end
