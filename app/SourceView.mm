#import "SourceView.h"

#include <optional>
#include <vector>

#include "source_inserters.hpp"
#include "source_selection.hpp"

#import "DrawHelper.h"
#import "LineNumberView.h"
#import "MessageViewController.h"
#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"
#import "Theme.h"

struct DocumentGetter {
  DocumentGetter(SourceView* sourceView) : _sourceView{sourceView} {}

  marlin::control::document& operator()() {
    assert(_sourceView != nil);
    return _sourceView.dataSource.document.content;
  }

 private:
  __weak SourceView* _sourceView;
};

@implementation SourceView {
  EdgeInsets _insets;

  NSMutableArray* _strings;

  std::optional<marlin::control::source_inserters<DocumentGetter>> _inserter;

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

  _inserter = {{self}};
}

- (void)insertStatementsBeforeLine:(NSUInteger)line withDisplay:(marlin::format::display)display {
  [self insertStatementsBeforeLine:line withDisplay:std::move(display) isInitialize:false];
}

- (void)insertStatementsBeforeLine:(NSUInteger)line
                       withDisplay:(marlin::format::display)display
                      isInitialize:(bool)isInitialize {
  auto lineIndex = line - 1;
  if (lineIndex > _strings.count) {
    lineIndex = _strings.count;
  }
  size_t lineBegin = 0;
  CGFloat maxLineWidth = 0;
  size_t highlightIndex = 0;
  while (lineBegin < display.source.size()) {
    auto lineEnd = display.source.find_first_of('\n', lineBegin);
    std::string_view str_view{&display.source[lineBegin], lineEnd - lineBegin};
    auto str =
        [[NSMutableAttributedString alloc] initWithString:[NSString stringWithStringView:str_view]];
    std::vector<marlin::format::highlight_token> lineHighlights;
    while (highlightIndex < display.highlights.size() &&
           display.highlights[highlightIndex].offset < lineEnd) {
      auto highlight = display.highlights[highlightIndex];
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
  self.frame = CGRectMake(self.frame.origin.x, self.frame.origin.y, width, height);
  [self setNeedsDisplayInRect:self.bounds];
  if (!isInitialize) {
    [self.delegate sourceViewChanged:self];
  }
}

- (void)updateExpressionInSourceRange:(marlin::source_range)sourceRange
                          withDisplay:(marlin::format::display)display {
  NSAssert(sourceRange.begin.line == sourceRange.end.line, @"Only support one line expression");
  NSAssert(sourceRange.begin.line > 0 && sourceRange.begin.line <= _strings.count, @"");
  NSMutableAttributedString* str = [_strings objectAtIndex:sourceRange.begin.line - 1];
  auto range =
      NSMakeRange(sourceRange.begin.column - 1, sourceRange.end.column - sourceRange.begin.column);
  [str replaceCharactersInRange:range withString:[NSString stringWithStringView:display.source]];
  range.length = display.source.size();
  applyTheme(currentTheme(), str, range, display.highlights);
  auto width = str.size.width + _insets.left + _insets.right;
  if (width > self.frame.size.width) {
    self.frame =
        CGRectMake(self.frame.origin.x, self.frame.origin.y, width, self.frame.size.height);
  }
  [self setNeedsDisplayInRect:self.bounds];
  [self.delegate sourceViewChanged:self];
}

- (void)removeStatementFromLine:(NSUInteger)from toLine:(NSUInteger)to {
  NSAssert(from > 0 && from <= _strings.count, @"%lu out of range", from);
  NSAssert(to > 0 && to <= _strings.count, @"");
  NSAssert(from <= to, @"");
  auto indexSet = [NSMutableIndexSet new];
  [indexSet addIndexesInRange:NSMakeRange(from - 1, to - from + 1)];
  [_strings removeObjectsAtIndexes:indexSet];
  [self setNeedsDisplayInRect:self.bounds];
  [self.delegate sourceViewChanged:self];
}

- (void)removeExpressionInSourceRange:(marlin::source_range)sourceRange {
  NSAssert(sourceRange.begin.line == sourceRange.end.line, @"Only support one line expression");
  NSAssert(sourceRange.begin.line > 0 && sourceRange.begin.line <= _strings.count, @"");
  NSMutableAttributedString* str = [_strings objectAtIndex:sourceRange.begin.line - 1];
  auto range =
      NSMakeRange(sourceRange.begin.column - 1, sourceRange.end.column - sourceRange.begin.column);
  [str replaceCharactersInRange:range withString:@""];
  [self setNeedsDisplayInRect:self.bounds];
  [self.delegate sourceViewChanged:self];
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

#pragma mark - EditorViewControllerDelegate

- (void)viewController:(EditorViewController*)vc
    finishEditWithString:(NSString*)string
                  ofType:(EditorType)type {
  [self.delegate dismissPopoverViewControllerForSourceView:self];
  if (string.length > 0 && _selection.has_value()) {
    auto update = (*std::exchange(_selection, std::nullopt))
                      .as_expression_inserter()
                      .insert_literal(type, string.stringView);
    [self updateExpressionInSourceRange:update.range withDisplay:std::move(update.display)];
  } else {
    _selection.reset();
  }
}

#pragma mark - FunctionViewControllerDelegate

- (void)viewController:(FunctionViewController*)vc
    finishEditingWithName:(NSString*)name
               parameters:(NSArray<NSString*>*)parameters {
  [self.delegate dismissPopoverViewControllerForSourceView:self];
  if (name.length > 0 && _selection.has_value()) {
    marlin::function_definition signature{std::string{name.stringView}};
    for (NSString* parameter in parameters) {
      if (parameter.length > 0) {
        signature.parameters.emplace_back(parameter.stringView);
      }
    }
    auto updates = (*std::exchange(_selection, std::nullopt)).replace_function_signature(signature);
    for (auto& update : updates) {
      [self updateExpressionInSourceRange:update.range withDisplay:std::move(update.display)];
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
    if (_selection->is_block() || _selection->is_statement()) {
      if (auto update = (*std::exchange(_selection, std::nullopt)).remove_from_document()) {
        [self removeStatementFromLine:update->range.begin.line toLine:update->range.end.line];
      }
    } else if (_selection->is_expression()) {
      if (auto update = (*std::exchange(_selection, std::nullopt)).remove_from_document()) {
        [self removeExpressionInSourceRange:update->range];
      }
    }
  }
  _isDraggingFromSelection = NO;
}

- (void)drawRect:(CGRect)dirtyRect {
  [super drawRect:dirtyRect];
  [self drawBackgroundInRect:dirtyRect];
  dirtyRect = self.bounds;
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
  [self drawBlockInsertionPointInRect:rect];
  [self drawStatementInsertionPointInRect:rect];
  [self drawExpressionInsertionInRect:rect];
  [self drawErrorMessage];
}

- (void)drawBlockInsertionPointInRect:(CGRect)rect {
  NSAssert(_inserter.has_value(), @"");
  if (auto location = _inserter->block_insert_location()) {
    [self drawInsertionPoint:*location inRect:rect];
  }
}

- (void)drawStatementInsertionPointInRect:(CGRect)rect {
  NSAssert(_inserter.has_value(), @"");
  if (auto location = _inserter->statement_insert_location()) {
    [self drawInsertionPoint:*location inRect:rect];
  }
}

- (void)drawExpressionInsertionInRect:(CGRect)rect {
  NSAssert(_inserter.has_value(), @"");
  if (auto range = _inserter->expression_insert_range()) {
    [self drawSourceRange:*range inRect:rect forSelection:NO];
  }
}

- (void)drawSelectionInRect:(CGRect)rect {
  if (_selection.has_value()) {
    [self drawSourceRange:_selection->get_range() inRect:rect forSelection:YES];
  }
}

- (void)drawInsertionPoint:(marlin::source_loc)location inRect:(CGRect)rect {
  auto oneCharSize = characterSizeWithAttributes(currentTheme().allAttrs);
  auto x = _insets.left + oneCharSize.width * (location.column - 1);
  auto y = oneCharSize.height * (location.line - 1) + _insets.top;
  if (CGRectContainsPoint(rect, CGPointMake(x, y))) {
    drawLine(CGPointMake(x, y), CGPointMake(x + 200, y), 5, [Color blueColor]);
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
  NSAssert(self.dataSource.document != nil, @"");
  _selection = {self.dataSource.document.content, [self sourceLocationOfPoint:location]};
  [self setNeedsDisplayInRect:self.bounds];

  auto rect = [self rectOfSourceRange:_selection->get_range()];
  if (_selection->is_literal()) {
    auto [type, data] = _selection->get_literal_content();
    [self.delegate showEditorViewControllerForSourceView:self
                                                fromRect:rect
                                                withType:type
                                                    data:data];
  } else if (_selection->is_function_signature()) {
    [self.delegate showFunctionViewControllerForSourceView:self
                                                  fromRect:rect
                                     withFunctionSignature:_selection->get_function_signature()];
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

- (BOOL)draggingPasteboardOfType:(marlin::control::pasteboard_t)type toLocation:(CGPoint)location {
  if (_selection.has_value()) {
    [self.delegate dismissPopoverViewControllerForSourceView:self];
    _selection.reset();
  }

  NSAssert(_inserter.has_value(), @"");
  return _inserter->move_to_loc(type, [self sourceLocationOfPoint:location]);
}

- (BOOL)dropPasteboardOfType:(marlin::control::pasteboard_t)type withData:(NSData*)data {
  NSAssert(_inserter.has_value(), @"");
  if (auto update = _inserter->insert(type, data.dataView)) {
    if (type == marlin::control::pasteboard_t::expression) {
      [self updateExpressionInSourceRange:update->range withDisplay:std::move(update->display)];
    } else {
      [self insertStatementsBeforeLine:update->range.begin.line
                           withDisplay:std::move(update->display)];
    }
    [self removeDraggingSelection];
    return YES;
  } else {
    [self removeDraggingSelection];
    return NO;
  }
}

- (void)resetAll {
  _inserter->reset_all();
  _isDraggingFromSelection = NO;
  [self setNeedsDisplayInRect:self.bounds];
}

@end
