#import "SourceTextView.h"

#import "SourceTheme.h"

@implementation SourceTextView {
  NSRange _selectionRange;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
  if (self = [super initWithCoder:coder]) {
    _selectionRange = NSMakeRange(0, 0);
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
  NSLog(@"events: %lu", eventArray.count);
  [self.textStorage replaceCharactersInRange:NSMakeRange(0, self.textStorage.string.length)
                                  withString:@"5 + 6"];
}

- (void)drawInsertionPointInRect:(NSRect)rect color:(NSColor*)color turnedOn:(BOOL)flag {
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

@end

@implementation SourceTextView (DragAndDrop)

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
  return YES;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  return YES;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  auto* theme = [SourceTheme new];
  auto string = [sender.draggingPasteboard stringForType:NSPasteboardTypeString];
  [self.textStorage replaceCharactersInRange:NSMakeRange(0, 0) withString:string];
  [self.textStorage setAttributes:theme.allAttrs range:NSMakeRange(0, string.length)];
  return YES;
}

@end
