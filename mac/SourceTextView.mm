#import "SourceTextView.h"

#import <optional>

#import "SourceTheme.h"

@interface SourceTextView ()

@property(nonatomic, strong) NSPopover* popover;

@end

@implementation SourceTextView {
  std::optional<marlin::control::statement_inserter> _statementInserter;
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
  self.popover = [NSPopover new];
  self.popover.behavior = NSPopoverBehaviorApplicationDefined;
  NSStoryboard* storyboard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
  EditorViewController* vc =
      [storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
  vc.delegate = self;
  self.popover.contentViewController = vc;
  [self.popover showRelativeToRect:NSMakeRect(100, 100, 100, 100)
                            ofView:self
                     preferredEdge:NSMinYEdge];
}

- (void)mouseMoved:(NSEvent*)event {
  [NSCursor.arrowCursor set];
}

- (void)interpretKeyEvents:(NSArray<NSEvent*>*)eventArray {
  // [self.textStorage replaceCharactersInRange:NSMakeRange(0, self.textStorage.string.length)
  //                                 withString:@"5 + 6"];
}

- (BOOL)shouldDrawInsertionPoint {
  return false;
}

- (NSRange)selectionRangeForProposedRange:(NSRange)proposedCharRange
                              granularity:(NSSelectionGranularity)granularity {
  return NSMakeRange(0, 0);
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
  if (_statementInsertionPoint >= 0 && _statementInserter.has_value() &&
      _statementInserter->can_insert()) {
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
  NSUInteger indexOfGlyph = 0;
  NSUInteger numberOfLines = 0;
  while (indexOfGlyph <= index) {
    ++numberOfLines;
    if (indexOfGlyph < self.textStorage.string.length) {
      NSRange lineRange;
      [self.layoutManager lineFragmentRectForGlyphAtIndex:indexOfGlyph effectiveRange:&lineRange];
      indexOfGlyph = NSMaxRange(lineRange);
    } else {
      break;
    }
  }
  return numberOfLines;
}

- (void)viewController:(EditorViewController*)vc finishEditWithString:(NSString*)string {
  [self.popover close];
}

@end

@implementation SourceTextView (DragAndDrop)

- (NSDragOperation)dragOperationForDraggingInfo:(id<NSDraggingInfo>)dragInfo
                                           type:(NSPasteboardType)type {
  if (type != NSStringPboardType) {
    return NSDragOperationNone;
  }

  if (!_statementInserter.has_value()) {
    _statementInserter = [self.dataSource statementInserterForTextView:self];
  }

  auto location = [self convertPoint:dragInfo.draggingLocation fromView:nil];
  auto index = [self characterIndexForInsertionAtPoint:location];
  auto line = [self lineContainsIndex:index];
  _statementInserter->move_to_line(line);
  if (_statementInserter->can_insert()) {
    auto range = [self.textStorage.string lineRangeForRange:NSMakeRange(index, 0)];
    _statementInsertionPoint = range.location;
    [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:NO];
    return NSDragOperationCopy;
  } else {
    _statementInsertionPoint = -1;
    [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:NO];
    return NSDragOperationNone;
  }
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  if (_statementInserter.has_value() && _statementInserter->can_insert()) {
    auto index = [sender.draggingPasteboard stringForType:NSPasteboardTypeString].integerValue;
    auto source = _statementInserter->insert(marlin::control::statement_prototypes[index]);
    NSString* string = [NSString stringWithCString:source.source.c_str()
                                          encoding:NSUTF8StringEncoding];
    auto range = NSMakeRange(_statementInsertionPoint, 0);
    [self.textStorage replaceCharactersInRange:range withString:string];
    [[SourceTheme new] applyTo:self.textStorage range:range withHighlights:source.highlights];

    _statementInsertionPoint = -1;
    [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
    return YES;
  } else {
    return NO;
  }
}

- (void)cleanUpAfterDragOperation {
  _statementInserter = std::nullopt;
  _statementInsertionPoint = -1;
  [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
}

@end
