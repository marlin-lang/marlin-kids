#import "SourceTextView.h"

#import <optional>
#import <vector>

#import "prototype_definition.hpp"
#import "toolbox_model.hpp"

#import "LineNumberView.h"
#import "MessageViewController.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "SourceTheme.h"

@implementation SourceTextView {
  NSPopover* _popover;
  NSEdgeInsets _insets;
  NSMutableArray* _strings;

  bool _potentialDrag;
  std::optional<marlin::control::statement_inserter> _statementInserter;
  std::optional<marlin::control::expression_inserter> _expressionInserter;
  std::optional<marlin::control::source_selection> _selection;
  std::optional<marlin::control::source_selection> _draggingSelection;

  NSUInteger _insertLineNumber;
  marlin::source_range _selectionRange;

  std::vector<NSRange> _errors;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
  if (self = [super initWithCoder:coder]) {
    self.frame = NSZeroRect;
    _strings = [NSMutableArray new];
    _insets = NSEdgeInsetsMake(5, 5, 5, 5);
    _insertLineNumber = 0;

    _potentialDrag = false;

    [self setupDragDrop];
  }
  return self;
}

- (BOOL)isFlipped {
  return YES;
}

- (void)insertBeforeLine:(NSUInteger)line
              withSource:(std::string_view)source
              highlights:(std::vector<marlin::control::highlight_token>)highlights {
  auto lineIndex = line - 1;
  if (lineIndex > _strings.count) {
    lineIndex = _strings.count;
  }
  auto lineBegin = 0;
  CGFloat maxLineWidth = 0;
  auto highlightIndex = 0;
  while (lineBegin < source.size()) {
    auto lineEnd = source.find_first_of('\n', lineBegin);
    auto str_view = std::string_view{&source[lineBegin], lineEnd - lineBegin};
    auto* str =
        [[NSMutableAttributedString alloc] initWithString:[NSString stringWithStringView:str_view]];
    std::vector<marlin::control::highlight_token> lineHighlights;
    while (highlightIndex < highlights.size() && highlights[highlightIndex].offset < lineEnd) {
      auto highlight = highlights[highlightIndex];
      highlight.offset -= lineBegin;
      lineHighlights.push_back(highlight);
      ++highlightIndex;
    }
    [[SourceTheme new] applyTo:str
                         range:NSMakeRange(0, str.string.length)
                withHighlights:lineHighlights];
    maxLineWidth = fmax(maxLineWidth, str.size.width);
    [_strings insertObject:str atIndex:lineIndex];
    lineBegin = lineEnd + 1;
    ++lineIndex;
  }
  auto width = fmax(maxLineWidth + _insets.left + _insets.right, self.bounds.size.width);
  auto height = self.lineHeight * _strings.count + _insets.top + _insets.bottom;
  [self setFrameSize:NSMakeSize(width, height)];
  [self setNeedsDisplayInRect:self.bounds];
}

- (void)updateInSourceRange:(marlin::source_range)sourceRange
                 withSource:(std::string_view)source
                 highlights:(std::vector<marlin::control::highlight_token>)highlights {
  NSAssert(sourceRange.begin.line == sourceRange.end.line, @"Only support one line expression");
  NSAssert(sourceRange.begin.line > 0 && sourceRange.begin.line <= _strings.count, @"");
  NSMutableAttributedString* str = [_strings objectAtIndex:sourceRange.begin.line - 1];
  auto range =
      NSMakeRange(sourceRange.begin.column - 1, sourceRange.end.column - sourceRange.begin.column);
  [str replaceCharactersInRange:range withString:[NSString stringWithStringView:source]];
  range.length = source.size();
  [[SourceTheme new] applyTo:str range:range withHighlights:highlights];
  auto width = str.size.width + _insets.left + _insets.right;
  if (width > self.frame.size.width) {
    [self setFrameSize:NSMakeSize(width, self.frame.size.height)];
  }
  [self setNeedsDisplayInRect:self.bounds];
}

- (marlin::source_loc)sourceLocationOfPoint:(NSPoint)point {
  if (_strings.count > 0) {
    auto oneCharSize = [@"a" sizeWithAttributes:[SourceTheme new].allAttrs];
    size_t line = fmin(_strings.count - 1, fmax(0, point.y - _insets.top) / oneCharSize.height) + 1;
    NSAttributedString* string = [_strings objectAtIndex:line - 1];
    size_t column =
        fmin(string.string.length - 1, fmax(0, point.x - _insets.left) / oneCharSize.width) + 1;
    return {line, column};
  } else {
    return {0, 0};
  }
}

- (CGFloat)lineHeight {
  auto oneCharSize = [@"a" sizeWithAttributes:[SourceTheme new].allAttrs];
  return oneCharSize.height;
}

- (CGFloat)lineTopOfNumber:(NSUInteger)number {
  return (number - 1) * self.lineHeight + _insets.top;
}

- (void)drawRect:(NSRect)dirtyRect {
  [self drawBackgroundInRect:dirtyRect];
  auto lineHeight = self.lineHeight;
  NSUInteger beginIndex = fmax(0, dirtyRect.origin.y - _insets.top) / lineHeight;
  NSUInteger endIndex =
      fmin(fmax(0, dirtyRect.origin.y + dirtyRect.size.height - 1 - _insets.top) / lineHeight,
           _strings.count - 1);
  for (auto index = beginIndex; index <= endIndex; ++index) {
    auto y = index * lineHeight + _insets.top;
    NSAttributedString* string = [_strings objectAtIndex:index];
    [string drawAtPoint:NSMakePoint(_insets.left, y)];
  }
}

- (void)drawBackgroundInRect:(NSRect)rect {
  [self drawSelection];
  [self drawStatementInsertionPointInRect:rect];
}

- (void)drawStatementInsertionPointInRect:(NSRect)rect {
  if (_insertLineNumber > 0 && _statementInserter.has_value() && _statementInserter->can_insert()) {
    auto oneCharSize = [@"a" sizeWithAttributes:[SourceTheme new].allAttrs];
    auto x = oneCharSize.width * (_statementInserter->get_location().column - 1);
    auto y = oneCharSize.height * (_insertLineNumber - 1) + _insets.top;
    if (NSPointInRect(NSMakePoint(x, y), rect)) {
      [NSGraphicsContext saveGraphicsState];
      NSBezierPath* line = [NSBezierPath bezierPath];
      [line moveToPoint:NSMakePoint(x, y)];
      [line lineToPoint:NSMakePoint(x + 200, y)];
      [line setLineWidth:5.0];
      [[NSColor blueColor] set];
      [line stroke];
      [NSGraphicsContext restoreGraphicsState];
    }
  }
}

- (void)drawSelection {
  if (_selectionRange.end > _selectionRange.begin) {
    auto selectionRect = [self rectOfSourceRange:_selectionRange];
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

- (void)mouseDown:(NSEvent*)event {
  [super mouseDown:event];

  auto location = [self convertPoint:event.locationInWindow fromView:nil];
  _selection = [self.dataSource textView:self selectionAt:[self sourceLocationOfPoint:location]];
  _selectionRange = _selection->get_range();
  [self setNeedsDisplayInRect:self.bounds];

  if (_selection->is_literal()) {
    NSStoryboard* storyboard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    EditorViewController* vc =
        [storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
    vc.delegate = self;

    _popover = [NSPopover new];
    _popover.behavior = NSPopoverBehaviorTransient;
    _popover.contentViewController = vc;
    auto rect = [self rectOfSourceRange:_selectionRange];
    [_popover showRelativeToRect:rect ofView:self preferredEdge:NSMinYEdge];

    auto [type, data] = _selection->get_literal_content();
    vc.type = type;
    vc.editorTextField.stringValue = [NSString stringWithStringView:data];
  }
}

- (NSRect)rectOfSourceRange:(marlin::source_range)range {
  NSAssert(range.end.line >= range.begin.line, @"Range should be valid");
  const CGFloat inset = 0.25;
  NSSize oneCharSize = [@"a" sizeWithAttributes:[SourceTheme new].allAttrs];
  if (range.end.line == range.begin.line) {
    auto x =
        (range.begin.column - 1) * oneCharSize.width + _insets.left - oneCharSize.width * inset;
    auto y = (range.begin.line - 1) * oneCharSize.height + _insets.top;
    auto width =
        (range.end.column - range.begin.column) * oneCharSize.width + oneCharSize.width * inset * 2;
    return NSMakeRect(x, y, width, oneCharSize.height);
  } else {
    CGFloat maxWidth = 0;
    for (auto line = range.begin.line; line <= range.end.line; ++line) {
      NSAttributedString* string = [_strings objectAtIndex:line - 1];
      maxWidth = fmax(maxWidth, string.size.width);
    }
    auto x = fmax(0, (range.begin.column - 1) * oneCharSize.width - oneCharSize.width * inset);
    auto y = (range.begin.line - 1) * self.lineHeight + _insets.top;
    auto width = maxWidth + oneCharSize.width * inset * 2 - x;
    auto height = (range.end.line - range.begin.line + 1) * self.lineHeight;
    return NSMakeRect(x, y, width, height);
  }
}

- (void)resetSelection {
  _selection = std::nullopt;
  _selectionRange = {};
}

- (NSUInteger)addErrorAtSourceRange:(marlin::source_range)range {
  /*auto charRange = [self rangeOfSourceRange:range];
  _errors.push_back(charRange);
  return charRange.location;*/
  return 0;
}

- (void)clearErrors {
  // _errors.clear();
  // [self setNeedsDisplayInRect:self.bounds avoidAdditionalLayout:YES];
}

/*- (void)drawErrorMessage {
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
}*/

#pragma mark - EditorViewControllerDelegate implementation

- (void)viewController:(EditorViewController*)vc
    finishEditWithString:(NSString*)string
                  ofType:(EditorType)type {
  [_popover close];
  if (string.length > 0 && _selection.has_value()) {
    auto inserter = (*std::move(_selection)).as_expression_inserter();
    [self resetSelection];

    if (auto update = inserter.insert_literal(type, std::string{string.UTF8String})) {
      [self updateInSourceRange:update->range
                     withSource:std::move(update->source)
                     highlights:std::move(update->highlights)];
    }
  } else {
    [self resetSelection];
  }
}

#pragma mark - Drag and Drop

- (NSArray<NSPasteboardType>*)acceptableDragTypes {
  return @[
    pasteboardOfType(marlin::control::pasteboard_t::statement),
    pasteboardOfType(marlin::control::pasteboard_t::expression)
  ];
}

- (void)setupDragDrop {
  [self registerForDraggedTypes:self.acceptableDragTypes];
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  auto location = [self convertPoint:sender.draggingLocation fromView:nil];
  auto source_loc = [self sourceLocationOfPoint:location];

  auto* type = [sender.draggingPasteboard availableTypeFromArray:self.acceptableDragTypes];
  if ([type isEqualToString:pasteboardOfType(marlin::control::pasteboard_t::statement)]) {
    if (!_statementInserter.has_value()) {
      _statementInserter = [self.dataSource statementInserterForTextView:self];
    }
    _statementInserter->move_to_line(source_loc.line);
    if (_statementInserter->can_insert()) {
      _insertLineNumber = source_loc.line;
      [self setNeedsDisplayInRect:self.bounds];
      return NSDragOperationMove;
    } else {
      _insertLineNumber = 0;
      return NSDragOperationNone;
    }
  } else if ([type isEqualToString:pasteboardOfType(marlin::control::pasteboard_t::expression)]) {
    if (!_expressionInserter.has_value()) {
      _expressionInserter = [self.dataSource expressionInserterForTextView:self];
    }
    _expressionInserter->move_to_loc(source_loc);
    if (_expressionInserter->can_insert()) {
      _selectionRange = _expressionInserter->get_range();
      [self setNeedsDisplayInRect:self.bounds];
      return NSDragOperationMove;
    } else {
      _selectionRange = {};
      [self setNeedsDisplayInRect:self.bounds];
      return NSDragOperationNone;
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
        [self insertBeforeLine:_insertLineNumber
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
        [self updateInSourceRange:_selectionRange
                       withSource:update->source
                       highlights:update->highlights];
        return YES;
      }
    }
  }
  return NO;
}

- (void)draggingEnded:(id<NSDraggingInfo>)sender {
  _insertLineNumber = 0;
  [self resetSelection];
  [self setNeedsDisplayInRect:self.bounds];
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
  _insertLineNumber = 0;
  [self resetSelection];
  [self setNeedsDisplayInRect:self.bounds];
}

@end

@implementation SourceTextView (DragAndDrop)

- (BOOL)writeSelectionToPasteboard:(NSPasteboard*)pboard types:(NSArray<NSPasteboardType>*)types {
  assert(_selection.has_value());

  if (_selection->is_statement()) {
    [pboard setData:[NSData dataWithDataView:_selection->get_data()]
            forType:pasteboardOfType(marlin::control::pasteboard_t::statement)];
    return true;
  } else if (_selection->is_expression()) {
    [pboard setData:[NSData dataWithDataView:_selection->get_data()]
            forType:pasteboardOfType(marlin::control::pasteboard_t::expression)];
    return true;
  } else {
    return false;
  }
}

- (void)draggingSession:(NSDraggingSession*)session
           endedAtPoint:(NSPoint)screenPoint
              operation:(NSDragOperation)operation {
  /*if (_draggingSelection.has_value() && operation == NSDragOperationMove) {
    if (auto update = _draggingSelection->remove_from_document()) {
      _draggingSelection = std::nullopt;
      auto range = [self rangeOfSourceRange:update->range];
      [self updateInRange:range
               withSource:std::move(update->source)
               highlights:std::move(update->highlights)];
    }
  }*/
}

@end
