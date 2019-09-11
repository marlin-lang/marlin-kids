#import "SourceView.h"

#include <optional>
#include <vector>

#include "prototype_definition.hpp"
#include "toolbox_model.hpp"

#import "DrawHelper.h"
#import "LineNumberView.h"
#import "MessageViewController.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "Theme.h"

@implementation SourceView {
  EdgeInsets _insets;

  NSMutableArray* _strings;

  std::optional<marlin::control::statement_inserter> _statementInserter;
  std::optional<NSUInteger> _statementInsertionLine;
  std::optional<marlin::control::expression_inserter> _expressionInserter;
  std::optional<marlin::source_range> _expressionInsertionRange;

  std::optional<marlin::control::source_selection> _selection;
  BOOL _isDraggingFromSelection;

  std::vector<marlin::source_range> _errors;
}

- (instancetype)init {
  if (self = [super init]) {
    [self initialize];
  }
  return self;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
  if (self = [super initWithCoder:coder]) {
    [self initialize];
  }
  return self;
}

- (void)initialize {
  self.frame = CGRectZero;
  _strings = [NSMutableArray new];
  _insets = EdgeInsetsMake(5, 5, 5, 5);
  _isDraggingFromSelection = NO;
}

- (void)insertStatementsBeforeLine:(NSUInteger)line
                        withSource:(std::string_view)source
                        highlights:(std::vector<marlin::control::highlight_token>)highlights {
  auto lineIndex = line - 1;
  if (lineIndex > _strings.count) {
    lineIndex = _strings.count;
  }
  size_t lineBegin = 0;
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
    applyTheme(currentTheme(), str, NSMakeRange(0, str.string.length), lineHighlights);
    maxLineWidth = fmax(maxLineWidth, str.size.width);
    [_strings insertObject:str atIndex:lineIndex];
    lineBegin = lineEnd + 1;
    ++lineIndex;
  }
  auto width = fmax(maxLineWidth + _insets.left + _insets.right, self.bounds.size.width);
  auto height = self.lineHeight * _strings.count + _insets.top + _insets.bottom;
  [self setFrame:CGRectMake(self.frame.origin.x, self.frame.origin.y, width, height)];
  [self setNeedsDisplayInRect:self.bounds];
}

- (void)updateExpressionInSourceRange:(marlin::source_range)sourceRange
                           withSource:(std::string_view)source
                           highlights:(std::vector<marlin::control::highlight_token>)highlights {
  NSAssert(sourceRange.begin.line == sourceRange.end.line, @"Only support one line expression");
  NSAssert(sourceRange.begin.line > 0 && sourceRange.begin.line <= _strings.count, @"");
  NSMutableAttributedString* str = [_strings objectAtIndex:sourceRange.begin.line - 1];
  auto range =
      NSMakeRange(sourceRange.begin.column - 1, sourceRange.end.column - sourceRange.begin.column);
  [str replaceCharactersInRange:range withString:[NSString stringWithStringView:source]];
  range.length = source.size();
  applyTheme(currentTheme(), str, range, highlights);
  auto width = str.size.width + _insets.left + _insets.right;
  if (width > self.frame.size.width) {
    [self setFrame:CGRectMake(self.frame.origin.x, self.frame.origin.y, width,
                              self.frame.size.height)];
  }
  [self setNeedsDisplayInRect:self.bounds];
}

- (void)removeStatementFromLine:(NSUInteger)from toLine:(NSUInteger)to {
  NSAssert(from > 0 && from <= _strings.count, @"%lu out of range", from);
  NSAssert(to > 0 && to <= _strings.count, @"");
  NSAssert(from <= to, @"");
  auto* indexSet = [NSMutableIndexSet new];
  [indexSet addIndexesInRange:NSMakeRange(from - 1, to - from + 1)];
  [_strings removeObjectsAtIndexes:indexSet];
  [self setNeedsDisplayInRect:self.bounds];
}

- (void)removeExpressionInSourceRange:(marlin::source_range)sourceRange {
  NSAssert(sourceRange.begin.line == sourceRange.end.line, @"Only support one line expression");
  NSAssert(sourceRange.begin.line > 0 && sourceRange.begin.line <= _strings.count, @"");
  NSMutableAttributedString* str = [_strings objectAtIndex:sourceRange.begin.line - 1];
  auto range =
      NSMakeRange(sourceRange.begin.column - 1, sourceRange.end.column - sourceRange.begin.column);
  [str replaceCharactersInRange:range withString:@""];
  [self setNeedsDisplayInRect:self.bounds];
}

- (marlin::source_loc)sourceLocationOfPoint:(CGPoint)point {
  if (_strings.count > 0) {
    auto oneCharSize = characterSizeWithAttributes(currentTheme().allAttrs);
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
  auto oneCharSize = characterSizeWithAttributes(currentTheme().allAttrs);
  return oneCharSize.height;
}

- (CGFloat)lineTopOfNumber:(NSUInteger)number {
  return (number - 1) * self.lineHeight + _insets.top;
}

- (void)addErrorInSourceRange:(marlin::source_range)range {
  _errors.push_back(range);
  [self setNeedsDisplayInRect:self.bounds];
}

- (void)clearErrors {
  _errors.clear();
  [self setNeedsDisplayInRect:self.bounds];
}

#pragma mark - EditorViewControllerDelegate implementation

- (void)viewController:(EditorViewController*)vc
    finishEditWithString:(NSString*)string
                  ofType:(EditorType)type {
  [self.delegate dismissEditorViewControllerForSourceView:self];
  if (string.length > 0 && _selection) {
    auto inserter = (*std::move(_selection)).as_expression_inserter();
    _selection.reset();

    if (auto update = inserter.insert_literal(type, std::string{string.UTF8String})) {
      [self updateExpressionInSourceRange:update->range
                               withSource:std::move(update->source)
                               highlights:std::move(update->highlights)];
    }
  } else {
    _selection.reset();
  }
}

#pragma mark - Private methods

- (BOOL)isFlipped {
  return YES;
}

- (void)removeDraggingSelection {
  if (_isDraggingFromSelection) {
    auto range = _selection->get_range();
    if (auto update = _selection->remove_from_document()) {
      if (_selection->is_statement()) {
        [self removeStatementFromLine:range.begin.line toLine:range.end.line];
      } else if (_selection->is_expression()) {
        [self removeExpressionInSourceRange:update->range];
      }
      _selection.reset();
    }
    _isDraggingFromSelection = NO;
  }
}

- (void)drawRect:(CGRect)dirtyRect {
  [super drawRect:dirtyRect];
  [self drawBackgroundInRect:dirtyRect];
  auto lineHeight = self.lineHeight;
  NSUInteger beginIndex = fmax(0, dirtyRect.origin.y - _insets.top) / lineHeight;
  NSUInteger endIndex =
      fmin(fmax(0, dirtyRect.origin.y + dirtyRect.size.height - 1 - _insets.top) / lineHeight,
           _strings.count - 1);
  for (auto index = beginIndex; index <= endIndex; ++index) {
    auto y = index * lineHeight + _insets.top;
    NSAttributedString* string = [_strings objectAtIndex:index];
    [string drawAtPoint:CGPointMake(_insets.left, y)];
  }
}

- (void)drawBackgroundInRect:(CGRect)rect {
  [self drawSelectionInRect:rect];
  [self drawExpressionInsertionInRect:rect];
  [self drawStatementInsertionPointInRect:rect];
  [self drawErrorMessage];
}

- (void)drawStatementInsertionPointInRect:(CGRect)rect {
  if (_statementInsertionLine && _statementInserter && _statementInserter->can_insert()) {
    auto oneCharSize = characterSizeWithAttributes(currentTheme().allAttrs);
    auto x = _insets.left + oneCharSize.width * (_statementInserter->get_location().column - 1);
    auto y = oneCharSize.height * (*_statementInsertionLine - 1) + _insets.top;
    if (CGRectContainsPoint(rect, CGPointMake(x, y))) {
      drawLine(CGPointMake(x, y), CGPointMake(x + 200, y), 5, [Color blueColor]);
    }
  }
}

- (void)drawSelectionInRect:(CGRect)rect {
  if (_selection) {
    [self drawSourceRange:_selection->get_range() inRect:rect forSelection:YES];
  }
}

- (void)drawExpressionInsertionInRect:(CGRect)rect {
  if (_expressionInsertionRange && _expressionInserter && _expressionInserter->can_insert()) {
    [self drawSourceRange:*_expressionInsertionRange inRect:rect forSelection:NO];
  }
}

- (void)drawSourceRange:(marlin::source_range)range
                 inRect:(CGRect)rect
           forSelection:(BOOL)isSelection {
  auto sourceRect = [self rectOfSourceRange:range];
  if (CGRectIntersectsRect(rect, sourceRect)) {
    Color* fillColor = [Color colorWithRed:237.0 / 255.0
                                     green:243.0 / 255.0
                                      blue:252.0 / 255.0
                                     alpha:1];
    Color* strokeColor = isSelection ? [Color colorWithRed:163.0 / 255.0
                                                     green:188.0 / 255.0
                                                      blue:234.0 / 255.0
                                                     alpha:1]
                                     : Color.redColor;
    drawRoundRectangle(sourceRect, 5, fillColor, strokeColor);
  }
}

- (void)drawErrorMessage {
#ifndef IOS
  for (auto errorRange : _errors) {
    auto rect = [self rectOfSourceRange:errorRange];
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
#endif
}

- (void)touchAtLocation:(CGPoint)location {
  _selection = [self.dataSource sourceView:self selectionAt:[self sourceLocationOfPoint:location]];
  [self setNeedsDisplayInRect:self.bounds];

  if (_selection->is_literal()) {
    auto rect = [self rectOfSourceRange:_selection->get_range()];
    auto [type, data] = _selection->get_literal_content();
    [self.delegate showEditorViewControllerForSourceView:self
                                                fromRect:rect
                                                withType:type
                                                    data:data];
  }
}

#ifndef IOS
- (void)mouseDragged:(NSEvent*)event {
  [super mouseDragged:event];

  /*if (!_isDraggingFromSelection && _selection) {
    _isDraggingFromSelection = YES;
    auto* pasteboardItem = [NSPasteboardItem new];
    if (_selection->is_statement()) {
      [pasteboardItem
          setDataProvider:self
                 forTypes:@[ pasteboardOfType(marlin::control::pasteboard_t::statement) ]];
    } else if (_selection->is_expression()) {
      [pasteboardItem
          setDataProvider:self
                 forTypes:@[ pasteboardOfType(marlin::control::pasteboard_t::expression) ]];
    }
    auto* draggingItem = [[NSDraggingItem alloc] initWithPasteboardWriter:pasteboardItem];
    [draggingItem setDraggingFrame:NSMakeRect(0, 0, 100, 100)];
    [self beginDraggingSessionWithItems:@[ draggingItem ] event:event source:self];
  }*/
}
#endif

- (CGRect)rectOfSourceRange:(marlin::source_range)range {
  NSAssert(range.end.line >= range.begin.line, @"Range should be valid");
  const CGFloat inset = 0.25;
  auto oneCharSize = characterSizeWithAttributes(currentTheme().allAttrs);
  if (range.end.line == range.begin.line) {
    auto x =
        (range.begin.column - 1) * oneCharSize.width + _insets.left - oneCharSize.width * inset;
    auto y = (range.begin.line - 1) * oneCharSize.height + _insets.top;
    auto width =
        (range.end.column - range.begin.column) * oneCharSize.width + oneCharSize.width * inset * 2;
    return CGRectMake(x, y, width, oneCharSize.height);
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
    return CGRectMake(x, y, width, height);
  }
}

#pragma mark - Drag and drop

- (BOOL)draggingStatementAtLocation:(CGPoint)location {
  auto source_loc = [self sourceLocationOfPoint:location];

  if (!_statementInserter) {
    _statementInserter = [self.dataSource statementInserterForSourceView:self];
  }
  _statementInserter->move_to_line(source_loc.line);
  if (_statementInserter->can_insert()) {
    _statementInsertionLine = source_loc.line;
    return YES;
  }
  _statementInsertionLine.reset();
  return NO;
}

- (BOOL)draggingExpressionAtLocation:(CGPoint)location {
  auto source_loc = [self sourceLocationOfPoint:location];

  if (!_expressionInserter) {
    _expressionInserter = [self.dataSource expressionInserterForSourceView:self];
  }
  _expressionInserter->move_to_loc(source_loc);
  if (_expressionInserter->can_insert()) {
    _expressionInsertionRange = _expressionInserter->get_range();
    return YES;
  }
  _expressionInsertionRange.reset();
  return NO;
}

- (BOOL)performStatementDropForData:(NSData*)data {
  if (_statementInsertionLine && _statementInserter && _statementInserter->can_insert()) {
    if (auto update = _statementInserter->insert(data.dataView)) {
      [self insertStatementsBeforeLine:update->range.begin.line
                            withSource:std::move(update->source)
                            highlights:std::move(update->highlights)];
      _statementInserter.reset();
      [self removeDraggingSelection];
      return YES;
    }
  }
  [self removeDraggingSelection];
  return NO;
}

- (BOOL)performExpressionDropForData:(NSData*)data {
  if (_expressionInserter && _expressionInserter->can_insert()) {
    if (auto update = _expressionInserter->insert(data.dataView)) {
      [self updateExpressionInSourceRange:update->range
                               withSource:std::move(update->source)
                               highlights:std::move(update->highlights)];
      _statementInserter.reset();
      [self removeDraggingSelection];
      return YES;
    }
  }
  [self removeDraggingSelection];
  return NO;
}

- (void)resetAll {
  _statementInsertionLine.reset();
  _expressionInsertionRange.reset();
  _isDraggingFromSelection = NO;
  [self setNeedsDisplayInRect:self.bounds];
}

@end