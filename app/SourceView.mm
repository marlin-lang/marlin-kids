#import "SourceView.h"

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
  std::optional<marlin::control::source_selection> _draggingSelection;

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
  self.frameSize = CGSizeZero;
  _strings = [NSMutableArray arrayWithObject:[NSMutableAttributedString new]];
  _insets = EdgeInsetsMake(5, 5, 5, 5);

  _inserter = {{self}};
}

- (void)initializeWithDisplay:(marlin::format::display)display {
  [self insertLinesBeforeLine:1 withDisplay:std::move(display) isInitialize:true];
}

- (void)performUpdates:(std::vector<marlin::control::source_update>)updates {
  for (auto& update : updates) {
    [self performUpdate:std::move(update)];
  }
}

- (void)performUpdate:(marlin::control::source_update)update {
  // For now, we assume that there are only 3 types of updates:
  //  - Insert statements/blocks
  //  - Replace/remove expressions
  //  - Remove statements/blocks
  NSLog(@"%ld, %ld, %ld, %ld, %@", update.range.begin.line, update.range.begin.column,
        update.range.end.line, update.range.end.column,
        [NSString stringWithStringView:update.display.source]);
  if (update.range.begin == update.range.end) {
    [self insertLinesBeforeLine:update.range.begin.line
                    withDisplay:std::move(update.display)
                   isInitialize:false];
  } else if (update.range.begin.line == update.range.end.line) {
    [self updateExpressionInSourceRange:update.range withDisplay:std::move(update.display)];
  } else {
    assert(update.display.source.length() == 0);
    [self removeLinesFromLine:update.range.begin.line toLine:update.range.end.line];
  }
}

- (void)insertLinesBeforeLine:(NSUInteger)line
                  withDisplay:(marlin::format::display)display
                 isInitialize:(bool)isInitialize {
  auto lineIndex = line - 1;
  if (lineIndex > _strings.count) {
    lineIndex = _strings.count;
  }
  size_t lineBegin = 0;
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
    [_strings insertObject:str atIndex:lineIndex];
    lineBegin = lineEnd + 1;
    ++lineIndex;
  }
  self.frameSize = self.currentSize;
  [self setNeedsDisplay:YES];
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
  self.frameSize = self.currentSize;
  [self setNeedsDisplay:YES];
  [self.delegate sourceViewChanged:self];
}

- (void)removeLinesFromLine:(NSUInteger)from toLine:(NSUInteger)to {
  NSAssert(from > 0 && from <= _strings.count, @"%lu out of range", from);
  NSAssert(to > 0 && to <= _strings.count, @"");
  NSAssert(from <= to, @"");
  auto indexSet = [NSMutableIndexSet new];
  [indexSet addIndexesInRange:NSMakeRange(from - 1, to - from)];
  [_strings removeObjectsAtIndexes:indexSet];
  self.frameSize = self.currentSize;
  [self setNeedsDisplay:YES];
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
  [self setNeedsDisplay:YES];
}

- (void)clearErrors {
  _errors.clear();
  [self setNeedsDisplay:YES];
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
    [self performUpdate:std::move(update)];
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
    [self performUpdates:std::move(updates)];
  } else {
    _selection.reset();
  }
}

#pragma mark - Private methods

- (BOOL)isFlipped {
  return YES;
}

- (CGSize)currentSize {
  CGFloat width = 0;
  for (NSAttributedString* string in _strings) {
    width = fmax(width, string.size.width + _insets.left + _insets.right);
  }
  auto height = self.lineHeight * _strings.count + _insets.top + _insets.bottom;
  return CGSizeMake(width, height);
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
  [self drawDraggingSelectionInRect:rect];
  [self drawBlockInsertionPointInRect:rect];
  [self drawStatementInsertionPointInRect:rect];
  [self drawExpressionInsertionInRect:rect];
  [self drawErrorMessage];
}

- (void)drawSelectionInRect:(CGRect)rect {
  if (_selection.has_value()) {
    [self drawSourceRange:_selection->get_range() inRect:rect forSelection:YES];
  }
}

- (void)drawDraggingSelectionInRect:(CGRect)rect {
  if (_draggingSelection.has_value()) {
    [self drawSourceRange:_draggingSelection->get_range() inRect:rect forSelection:YES];
  }
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

- (void)touchDownAtLocation:(CGPoint)location {
  NSAssert(self.dataSource.document != nil, @"");
  _selection = {self.dataSource.document.content, [self sourceLocationOfPoint:location]};
  [self setNeedsDisplay:YES];
}

- (void)touchUp {
  if (_selection.has_value()) {
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
}

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

- (std::optional<DraggingData>)startDraggingAtLocation:(CGPoint)location {
  NSAssert(!_draggingSelection.has_value(), @"");

  if (_selection.has_value()) {
    auto rect = [self rectOfSourceRange:_selection->get_range()];
    if (!CGRectContainsPoint(rect, location)) {
      _draggingSelection = (*std::exchange(_selection, std::nullopt)).as_dragging_selection();
      if (auto type = _draggingSelection->dragging_type()) {
        return DraggingData(*type, [NSData dataWithDataView:_draggingSelection->get_data()]);
      } else {
        _draggingSelection.reset();
        return std::nullopt;
      }
    } else {
      return std::nullopt;
    }
  } else {
    return std::nullopt;
  }
}

- (BOOL)draggingPasteboardOfType:(marlin::control::pasteboard_t)type toLocation:(CGPoint)location {
  if (_selection.has_value()) {
    [self.delegate dismissPopoverViewControllerForSourceView:self];
    _selection.reset();
  }

  NSAssert(_inserter.has_value(), @"");
  auto exclusion = _draggingSelection.has_value() ? &*_draggingSelection : nullptr;
  return _inserter->move_to_loc(type, [self sourceLocationOfPoint:location], exclusion);
}

- (BOOL)dropPasteboardOfType:(marlin::control::pasteboard_t)type
                    withData:(NSData*)data
       removingCurrentSource:(BOOL)removing {
  NSAssert(_inserter.has_value(), @"");
  if (removing) {
    auto lineUpdate = [self removeDraggingSelection];
    _inserter->update_lines(std::move(lineUpdate));
  }
  auto update = _inserter->insert(type, data.dataView);
  const bool result = update.size() > 0;
  [self performUpdates:std::move(update)];
  return result;
}

- (BOOL)removeDraggingSource {
  return [self removeDraggingSelection].start_line > 0;
}

- (marlin::control::line_update)removeDraggingSelection {
  if (_draggingSelection.has_value()) {
    if (_draggingSelection->is_removable()) {
      auto line_update = _draggingSelection->removal_line_update();
      auto updates = (*std::exchange(_draggingSelection, std::nullopt)).remove_from_document();
      [self performUpdates:std::move(updates)];
      return line_update;
    } else {
      assert(false);
      _draggingSelection.reset();
    }
  }
  return {};
}

- (void)resetDraggingDestination {
  _inserter->reset_all();
  [self setNeedsDisplay:YES];
}

- (void)resetDraggingSource {
  _draggingSelection.reset();
  [self setNeedsDisplay:YES];
}

@end
