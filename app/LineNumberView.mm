#import "LineNumberView.h"

#import "DrawHelper.h"
#import "Theme.h"

@interface LineNumberView ()

#ifdef IOS
@property(weak, nonatomic) SourceView* clientView;
#endif

@end

@implementation LineNumberView {
  NSMutableDictionary* _errors;
  CGFloat _inset;
}

- (instancetype)initWithSourceView:(SourceView*)view {
#ifdef IOS
  if (self = [super init]) {
    self.backgroundColor = [Color colorWithWhite:0.85 alpha:1.0];
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
    
    - (NSUInteger)lineNumberOfLocation:(CGPoint)loc {
      auto* sourceView = (SourceView*)self.clientView;
      auto sourceViewLocation = [self convertPoint:loc toView:sourceView];
      auto [line, column] = [sourceView sourceLocationOfPoint:sourceViewLocation];
      return line;
    }

    - (NSString *)errorMessageOfLine:(NSUInteger)line {
        return [_errors objectForKey:[NSNumber numberWithInteger:line]];
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

#ifdef IOS
- (void)drawRect:(CGRect)rect {
  [super drawRect:rect];
  [self drawHashMarksAndLabelsInRect:rect];
}
#endif

- (void)drawHashMarksAndLabelsInRect:(CGRect)rect {
  auto* sourceView = (SourceView*)self.clientView;
  auto startPoint = [self convertPoint:rect.origin toView:sourceView];
  auto [startLine, startColumn] = [sourceView sourceLocationOfPoint:startPoint];
  auto endPoint =
      [self convertPoint:CGPointMake(rect.origin.x, rect.origin.y + rect.size.height - 1)
                  toView:sourceView];
  auto [endLine, endColumn] = [sourceView sourceLocationOfPoint:endPoint];
  auto offset = [self convertPoint:CGPointZero fromView:sourceView];
  auto height = sourceView.lineHeight;
  auto errorSize = height - _inset * 2;
  auto y = [sourceView lineTopOfNumber:startLine] + offset.y;
  if (startLine > 0) {
    for (auto line = startLine; line <= endLine; ++line) {
      auto errorRect = CGRectMake(_inset, y + _inset, errorSize, errorSize);
      [self drawErrorIndicatorOfLine:line atRect:errorRect];

      auto* string = [NSString stringWithFormat:@"%lu", line];
      auto* attrString = [[NSAttributedString alloc] initWithString:string
                                                         attributes:currentTheme().lineNumberAttrs];
      [attrString
          drawAtPoint:CGPointMake(self.ruleThickness - _inset - attrString.size.width, y + _inset)];
      y += height;
    }
  }
}

- (void)drawErrorIndicatorOfLine:(NSUInteger)number atRect:(CGRect)rect {
  if ([_errors objectForKey:[NSNumber numberWithInteger:number]]) {
    drawOval(rect, Color.redColor);
  }
}

@end
