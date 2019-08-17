#import "SourceTextView.h"

#import <optional>
#import <utility>
#import <vector>

#import "prototype_definition.hpp"
#import "toolbox_model.hpp"

#import "LineNumberView.h"
#import "MessageViewController.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "SourceTheme.h"

@interface SourceTextView ()

@property(nonatomic, strong) NSPopover* popover;

@end

@implementation SourceTextView {
  std::optional<marlin::control::source_selection> _selection;
  std::optional<marlin::control::expression_inserter> _expressionInserter;
  std::optional<marlin::control::statement_inserter> _statementInserter;
  NSRange _selectionRange;
  NSInteger _statementInsertionPoint;

  std::vector<NSRange> _errors;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
  if (self = [super initWithCoder:coder]) {
    self.enclosingScrollView.hasHorizontalScroller = YES;
    self.horizontallyResizable = YES;
    self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    self.maxSize = NSMakeSize(FLT_MAX, FLT_MAX);
    self.textContainer.containerSize = NSMakeSize(FLT_MAX, FLT_MAX);
    self.textContainer.widthTracksTextView = NO;

    _selectionRange = NSMakeRange(0, 0);
    _statementInsertionPoint = -1;
  }
  return self;
}

- (void)mouseDown:(NSEvent*)event {
  [super mouseDown:event];

  auto location = [self convertPoint:event.locationInWindow fromView:nil];
  auto index = [self characterIndexForInsertionAtPoint:location];
  _selection = [self.dataSource textView:self selectionAt:[self sourceLocOfIndex:index]];
  _selectionRange = [self rangeOfSourceRange:_selection->get_range()];
  [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
  if (_selection->is_literal()) {
    NSStoryboard* storyboard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    EditorViewController* vc =
        [storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
    vc.delegate = self;

    self.popover = [NSPopover new];
    self.popover.behavior = NSPopoverBehaviorTransient;
    self.popover.contentViewController = vc;
    auto rect = [self rectOfRange:_selectionRange];
    [self.popover showRelativeToRect:rect ofView:self preferredEdge:NSMinYEdge];

    auto [type, data] = _selection->get_literal_content();
    vc.type = type;
    vc.editorTextField.stringValue = [NSString stringWithStringView:data];
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
    CGFloat x = oneCharSize.width * (_statementInserter->get_location().column - 1);
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
  [self drawErrorMessage];
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

- (void)updateInRange:(NSRange)range
           withSource:(std::string_view)source
           highlights:(std::vector<marlin::control::highlight_token>)highlights {
  [self.textStorage beginEditing];
  NSString* newString = [NSString stringWithStringView:source];
  [self.textStorage replaceCharactersInRange:range withString:newString];
  [[SourceTheme new] applyTo:self.textStorage
                       range:NSMakeRange(range.location, newString.length)
              withHighlights:highlights];
  [self.textStorage endEditing];
  [self didChangeText];
}

- (NSUInteger)addErrorAtSourceRange:(marlin::source_range)range {
  auto charRange = [self rangeOfSourceRange:range];
  _errors.push_back(charRange);
  return charRange.location;
}

- (void)clearErrors {
  _errors.clear();
  [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
}

- (void)drawErrorMessage {
  for (auto errorRange : _errors) {
    auto rect = [self rectOfRange:errorRange];
    [NSGraphicsContext saveGraphicsState];
    NSBezierPath* line = [NSBezierPath bezierPath];
    [line moveToPoint:NSMakePoint(rect.origin.x, rect.origin.y + rect.size.height)];
    [line
        lineToPoint:NSMakePoint(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height)];
    [line setLineWidth:3.0];
    CGFloat dashPattern[] = {5, 5};
    [line setLineDash:dashPattern count:2 phase:0];
    [[NSColor redColor] set];
    [line stroke];
    [NSGraphicsContext restoreGraphicsState];
  }
}

#pragma mark - EditorViewControllerDelegate implementation

- (void)viewController:(EditorViewController*)vc
    finishEditWithString:(NSString*)string
                  ofType:(EditorType)type {
  [self.popover close];
  if (string.length > 0 && _selection.has_value()) {
    auto inserter = (*std::move(_selection)).as_expression_inserter();
    _selection = std::nullopt;
    _selectionRange = NSMakeRange(0, 0);

    if (auto update = inserter.insert_literal(type, std::string{string.UTF8String})) {
      [self updateInRange:[self rangeOfSourceRange:update->range]
               withSource:std::move(update->source)
               highlights:std::move(update->highlights)];
    }
  } else {
    _selection = std::nullopt;
    _selectionRange = NSMakeRange(0, 0);
  }
}

@end

@implementation SourceTextView (DragAndDrop)

- (NSArray<NSPasteboardType>*)acceptableDragTypes {
  return @[
    pasteboardOfType(marlin::control::pasteboard_t::statement),
    pasteboardOfType(marlin::control::pasteboard_t::expression)
  ];
}

- (NSArray<NSPasteboardType>*)readablePasteboardTypes {
  return @[
    pasteboardOfType(marlin::control::pasteboard_t::statement),
    pasteboardOfType(marlin::control::pasteboard_t::expression)
  ];
}

- (NSDragOperation)dragOperationForDraggingInfo:(id<NSDraggingInfo>)dragInfo
                                           type:(NSPasteboardType)type {
  auto location = [self convertPoint:dragInfo.draggingLocation fromView:nil];
  auto index = [self characterIndexForInsertionAtPoint:location];

  if (type == pasteboardOfType(marlin::control::pasteboard_t::statement)) {
    if (!_statementInserter.has_value()) {
      _statementInserter = [self.dataSource statementInserterForTextView:self];
    }
    auto [line, column]{[self sourceLocOfIndex:index]};
    _statementInserter->move_to_line(line);
    if (_statementInserter->can_insert()) {
      _statementInsertionPoint =
          [self.textStorage.string lineRangeForRange:NSMakeRange(index, 0)].location;
      [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:NO];
      return NSDragOperationCopy;
    } else {
      _statementInsertionPoint = -1;
      [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:NO];
    }
  } else if (type == pasteboardOfType(marlin::control::pasteboard_t::expression)) {
    if (!_expressionInserter.has_value()) {
      _expressionInserter = [self.dataSource expressionInserterForTextView:self];
    }
    _expressionInserter->move_to_loc([self sourceLocOfIndex:index]);
    if (_expressionInserter->can_insert()) {
      _selectionRange = [self rangeOfSourceRange:_expressionInserter->get_range()];
      [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
      return NSDragOperationCopy;
    } else {
      _selectionRange = NSMakeRange(0, 0);
      [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
    }
  }

  return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  auto type = [sender.draggingPasteboard availableTypeFromArray:self.acceptableDragTypes];
  if (type == pasteboardOfType(marlin::control::pasteboard_t::statement)) {
    if (_statementInserter.has_value() && _statementInserter->can_insert()) {
      auto* data = [sender.draggingPasteboard
          dataForType:pasteboardOfType(marlin::control::pasteboard_t::statement)];
      if (auto update = _statementInserter->insert(data.dataView)) {
        [self updateInRange:NSMakeRange(_statementInsertionPoint, 0)
                 withSource:std::move(update->source)
                 highlights:std::move(update->highlights)];
        return YES;
      }
    }
  } else if (type == pasteboardOfType(marlin::control::pasteboard_t::expression)) {
    if (_expressionInserter.has_value() && _expressionInserter->can_insert()) {
      auto* data = [sender.draggingPasteboard
          dataForType:pasteboardOfType(marlin::control::pasteboard_t::expression)];
      if (auto update = _expressionInserter->insert(data.dataView)) {
        [self updateInRange:_selectionRange
                 withSource:std::move(update->source)
                 highlights:std::move(update->highlights)];
        return YES;
      }
    }
  }
  return NO;
}

- (void)cleanUpAfterDragOperation {
  _statementInserter = std::nullopt;
  _expressionInserter = std::nullopt;
  _statementInsertionPoint = -1;
  _selectionRange = NSMakeRange(0, 0);
  [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
}

@end
