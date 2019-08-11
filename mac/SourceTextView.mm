#import "SourceTextView.h"

#import <optional>
#import <utility>

#import "SourceTheme.h"

@interface SourceTextView ()

@property(nonatomic, strong) NSPopover* popover;

@end

@implementation SourceTextView {
  NSRange _selectionRange;
  std::optional<marlin::control::statement_inserter> _statementInserter;
  NSInteger _statementInsertionPoint;
  marlin::source_loc _expression_loc;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
  if (self = [super initWithCoder:coder]) {
    _selectionRange = NSMakeRange(0, 0);
    _statementInsertionPoint = -1;
    _expression_loc = {0, 0};
  }
  return self;
}

- (void)mouseDown:(NSEvent*)theEvent {
  [super mouseDown:theEvent];

  auto location = [self convertPoint:theEvent.locationInWindow fromView:nil];
  auto index = [self characterIndexForInsertionAtPoint:location];
  _expression_loc = [self sourceLocOfIndex:index];
  auto& node = [self.dataSource textView:self nodeContainsSourceLoc:_expression_loc];
  _selectionRange = [self rangeOfSourceRange:node.source_code_range];
  [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
  if (node.is<marlin::ast::variable_placeholder>() ||
      node.is<marlin::ast::expression_placeholder>() || node.is<marlin::ast::variable_name>() ||
      node.is<marlin::ast::number_literal>() || node.is<marlin::ast::string_literal>() ||
      node.is<marlin::ast::identifier>()) {
    self.popover = [NSPopover new];
    self.popover.behavior = NSPopoverBehaviorTransient;
    NSStoryboard* storyboard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    EditorViewController* vc =
        [storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
    vc.delegate = self;
    self.popover.contentViewController = vc;
    auto rect = [self rectOfRange:_selectionRange];
    [self.popover showRelativeToRect:rect ofView:self preferredEdge:NSMinYEdge];

    if (node.is<marlin::ast::variable_placeholder>()) {
      vc.type = EditorType::Variable;
    } else if (node.is<marlin::ast::expression_placeholder>()) {
      vc.type = EditorType::Number;
    } else if (node.is<marlin::ast::number_literal>()) {
      vc.type = EditorType::Number;
      vc.editorTextField.stringValue =
          [NSString stringWithCString:node.as<marlin::ast::number_literal>().value.c_str()
                             encoding:NSUTF8StringEncoding];
    } else if (node.is<marlin::ast::variable_name>()) {
      vc.type = EditorType::Variable;
      vc.editorTextField.stringValue =
          [NSString stringWithCString:node.as<marlin::ast::variable_name>().name.c_str()
                             encoding:NSUTF8StringEncoding];
    } else if (node.is<marlin::ast::string_literal>()) {
      vc.type = EditorType::String;
      vc.editorTextField.stringValue =
          [NSString stringWithCString:node.as<marlin::ast::string_literal>().value.c_str()
                             encoding:NSUTF8StringEncoding];
    } else if (node.is<marlin::ast::identifier>()) {
      vc.type = EditorType::Identifier;
      vc.editorTextField.stringValue =
          [NSString stringWithCString:node.as<marlin::ast::identifier>().name.c_str()
                             encoding:NSUTF8StringEncoding];
    }
  }
}

- (void)mouseMoved:(NSEvent*)event {
  [NSCursor.arrowCursor set];
}

- (BOOL)shouldDrawInsertionPoint {
  return false;
}

- (NSRange)selectionRangeForProposedRange:(NSRange)proposedCharRange
                              granularity:(NSSelectionGranularity)granularity {
  return NSMakeRange(0, 0);
}

- (void)drawStatementInsertionPoint {
  if (_statementInsertionPoint >= 0 && _statementInserter.has_value() &&
      _statementInserter->can_insert()) {
    auto* theme = [SourceTheme new];
    NSSize oneCharSize = [@"a" sizeWithAttributes:theme.allAttrs];
    CGFloat x = oneCharSize.width * (_statementInserter->get_insert_location().column - 1);
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
    [line moveToPoint:NSMakePoint(x + self.textContainer.lineFragmentPadding, y)];
    [line lineToPoint:NSMakePoint(x + 200, y)];
    [line setLineWidth:5.0];
    [[NSColor blueColor] set];
    [line stroke];
    [NSGraphicsContext restoreGraphicsState];
  }
}

- (void)drawViewBackgroundInRect:(NSRect)rect {
  [super drawViewBackgroundInRect:rect];
  if (_selectionRange.length > 0) {
    auto selectionRect = [self rectOfRange:_selectionRange];
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
  [self drawStatementInsertionPoint];
}

- (NSRect)rectOfRange:(NSRange)range {
  auto* theme = [SourceTheme new];
  NSSize oneCharSize = [@"a" sizeWithAttributes:theme.allAttrs];
  NSRange glyphRange = [self.layoutManager glyphRangeForCharacterRange:range
                                                  actualCharacterRange:NULL];
  NSRect rect = [self.layoutManager boundingRectForGlyphRange:glyphRange
                                              inTextContainer:self.textContainer];
  rect.origin.x += self.textContainerOrigin.x;
  rect.origin.y += self.textContainerOrigin.y;
  return NSMakeRect(rect.origin.x - oneCharSize.width * 0.25, rect.origin.y + 1,
                    rect.size.width + oneCharSize.width * 0.5, rect.size.height);
}

- (marlin::source_loc)sourceLocOfIndex:(NSUInteger)index {
  NSUInteger currentLineIndex = 0;
  NSUInteger previousLineIndex = 0;
  NSUInteger numberOfLines = 0;
  while (currentLineIndex <= index) {
    ++numberOfLines;
    if (currentLineIndex < self.textStorage.string.length) {
      NSRange lineRange;
      [self.layoutManager lineFragmentRectForGlyphAtIndex:currentLineIndex
                                           effectiveRange:&lineRange];
      previousLineIndex = currentLineIndex;
      currentLineIndex = NSMaxRange(lineRange);
    } else {
      break;
    }
  }
  return {numberOfLines, index - previousLineIndex + 1};
}

- (NSUInteger)indexOfSourceLoc:(marlin::source_loc)loc {
  NSUInteger currentLineIndex = 0;
  NSUInteger numberOfLines = 1;
  while (numberOfLines < loc.line) {
    ++numberOfLines;
    NSRange lineRange;
    [self.layoutManager lineFragmentRectForGlyphAtIndex:currentLineIndex effectiveRange:&lineRange];
    currentLineIndex = NSMaxRange(lineRange);
  }
  return currentLineIndex + loc.column - 1;
}

- (NSRange)rangeOfSourceRange:(marlin::source_range)sourceRange {
  auto begin = [self indexOfSourceLoc:sourceRange.begin];
  auto end = [self indexOfSourceLoc:sourceRange.end];
  return NSMakeRange(begin, end - begin);
}

#pragma mark - EditorViewControllerDelegate implementation

- (void)viewController:(EditorViewController*)vc finishEditWithString:(NSString*)string {
  [self.popover close];
  if (string.length > 0) {
    auto update = [self.dataSource textView:self
                       replacePlaceholderAt:_expression_loc
                                       type:vc.type
                                 withString:string];
    auto begin = [self indexOfSourceLoc:update.range.begin];
    NSString* updatedString = [NSString stringWithCString:update.source.c_str()
                                                 encoding:NSUTF8StringEncoding];
    [self.textStorage replaceCharactersInRange:[self rangeOfSourceRange:update.range]
                                    withString:updatedString];
    [[SourceTheme new] applyTo:self.textStorage
                         range:NSMakeRange(begin, updatedString.length)
                withHighlights:update.highlights];
    _expression_loc = {0, 0};
    _selectionRange = NSMakeRange(0, 0);
  }
}

@end

@implementation SourceTextView (DragAndDrop)

- (NSArray<NSPasteboardType>*)acceptableDragTypes {
  return @[ @"marlin.statement" ];
}

- (NSArray<NSPasteboardType>*)readablePasteboardTypes {
  return @[ @"marlin.statement" ];
}

- (NSDragOperation)dragOperationForDraggingInfo:(id<NSDraggingInfo>)dragInfo
                                           type:(NSPasteboardType)type {
  if (!_statementInserter.has_value()) {
    _statementInserter = [self.dataSource statementInserterForTextView:self];
  }

  auto location = [self convertPoint:dragInfo.draggingLocation fromView:nil];
  auto index = [self characterIndexForInsertionAtPoint:location];
  auto [line, column]{[self sourceLocOfIndex:index]};
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
    auto index = [sender.draggingPasteboard stringForType:@"marlin.statement"].integerValue;
    auto source = _statementInserter->insert(*marlin::control::statement_prototypes[index]);
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
