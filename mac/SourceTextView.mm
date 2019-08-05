#import "SourceTextView.h"

#import "SourceTheme.h"

@implementation SourceTextView {
  NSRange _selectionRange;
  NSInteger _statementInsertionPoint;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
  if (self = [super initWithCoder:coder]) {
    _selectionRange = NSMakeRange(0, 0);
    _statementInsertionPoint = -1;
  }
  return self;
}

- (void)mouseDown:(NSEvent*)theEvent {
  [super mouseDown:theEvent];

  auto eventLocation = [theEvent locationInWindow];
  auto loc = [self convertPoint:eventLocation fromView:nil];
  auto index = [self characterIndexForInsertionAtPoint:loc];
  _selectionRange = [self.dataSource textView:self selectRangeContainsIndex:index];
  [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
}

- (void)mouseMoved:(NSEvent*)event {
  [NSCursor.arrowCursor set];
}

- (void)interpretKeyEvents:(NSArray<NSEvent*>*)eventArray {
  [self.textStorage replaceCharactersInRange:NSMakeRange(0, self.textStorage.string.length)
                                  withString:@"5 + 6"];
}

- (void)drawInsertionPointInRect:(NSRect)rect color:(NSColor*)color turnedOn:(BOOL)flag {
}

- (void)drawStatementInsertionPoint {
  CGFloat y = 0;
  if (_statementInsertionPoint < self.textStorage.string.length) {
    auto rect = [self.layoutManager lineFragmentRectForGlyphAtIndex:_statementInsertionPoint
                                                     effectiveRange:nil];
    y = rect.origin.y;
  } else {
    auto rect = [self.layoutManager lineFragmentRectForGlyphAtIndex:_statementInsertionPoint - 1
                                                     effectiveRange:nil];
    y = rect.origin.y + rect.size.height;
  }
  [NSGraphicsContext saveGraphicsState];
  NSBezierPath* line = [NSBezierPath bezierPath];
  [line moveToPoint:NSMakePoint(0 + self.textContainer.lineFragmentPadding, y)];
  [line lineToPoint:NSMakePoint(200, y)];
  [line setLineWidth:5.0];
  [[NSColor blueColor] set];
  [line stroke];
  [NSGraphicsContext restoreGraphicsState];
}

- (void)drawViewBackgroundInRect:(NSRect)rect {
  [super drawViewBackgroundInRect:rect];
  if (_selectionRange.length > 0) {
    auto selectionRect = self.selectionRect;
    [NSGraphicsContext saveGraphicsState];
    NSBezierPath* path = [NSBezierPath bezierPathWithRoundedRect:selectionRect
                                                         xRadius:5.0f
                                                         yRadius:5.0f];
    NSColor* fillColor = [NSColor colorWithCalibratedRed:237.0 / 255.0
                                                   green:243.0 / 255.0
                                                    blue:252.0 / 255.0
                                                   alpha:1];
    NSColor* strokeColor = [NSColor colorWithCalibratedRed:163.0 / 255.0
                                                     green:188.0 / 255.0
                                                      blue:234.0 / 255.0
                                                     alpha:1];
    [path addClip];
    [fillColor setFill];
    [strokeColor setStroke];
    NSRectFillUsingOperation(selectionRect, NSCompositingOperationSourceOver);
    NSAffineTransform* transform = [NSAffineTransform transform];
    [transform translateXBy:0.5 yBy:0.5];
    [path transformUsingAffineTransform:transform];
    [path stroke];
    [transform translateXBy:-1.5 yBy:-1.5];
    [path transformUsingAffineTransform:transform];
    [path stroke];
    [NSGraphicsContext restoreGraphicsState];
  }
  if (_statementInsertionPoint >= 0) {
    [self drawStatementInsertionPoint];
  }
}

- (NSRect)selectionRect {
  auto* theme = [SourceTheme new];
  NSSize oneCharSize = [@"a" sizeWithAttributes:theme.allAttrs];
  NSRange range = [self.layoutManager glyphRangeForCharacterRange:_selectionRange
                                             actualCharacterRange:NULL];
  NSRect rect = [self.layoutManager boundingRectForGlyphRange:range
                                              inTextContainer:self.textContainer];
  rect.origin.x += self.textContainerOrigin.x;
  rect.origin.y += self.textContainerOrigin.y;
  return NSMakeRect(rect.origin.x - oneCharSize.width * 0.25, rect.origin.y + 1,
                    rect.size.width + oneCharSize.width * 0.5, rect.size.height);
}

- (uint)lineContainsIndex:(NSUInteger)index {
  NSUInteger numberOfLines = 0;
  for (NSUInteger indexOfGlyph = 0; indexOfGlyph < index; numberOfLines++) {
    NSRange lineRange;
    [self.layoutManager lineFragmentRectForGlyphAtIndex:indexOfGlyph effectiveRange:&lineRange];
    indexOfGlyph = NSMaxRange(lineRange);
  }
  return numberOfLines + 1;
}

@end

@implementation SourceTextView (DragAndDrop)

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
  return YES;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  auto location = [self convertPoint:sender.draggingLocation fromView:nil];
  auto index = [self characterIndexForInsertionAtPoint:location];
  auto range = [self.textStorage.string lineRangeForRange:NSMakeRange(index, 0)];
  _statementInsertionPoint = range.location;
  [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:NO];
  return YES;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
  _statementInsertionPoint = -1;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  auto* theme = [SourceTheme new];
  auto index = [sender.draggingPasteboard stringForType:NSPasteboardTypeString].integerValue;
  auto line = [self lineContainsIndex:_statementInsertionPoint];
  auto string = [self.dataSource textView:self insertStatementByIndex:index atLine:line];
  [self.textStorage replaceCharactersInRange:NSMakeRange(_statementInsertionPoint, 0)
                                  withString:string];
  [self.textStorage setAttributes:theme.allAttrs
                            range:NSMakeRange(_statementInsertionPoint, string.length)];
  _statementInsertionPoint = -1;
  [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
  return YES;
}

@end
