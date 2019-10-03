#import "SourceView.h"

#include "source_inserters.hpp"
#include "source_selection.hpp"

#import "DrawHelper.h"
#import "LineNumberView.h"
#import "MessageViewController.h"
#import "Pasteboard.h"
#import "Theme.h"

using optional_source_selection = std::optional<marlin::control::source_selection>;

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

  optional_source_selection _selection;
  optional_source_selection _draggingSelection;

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
  [self insertLinesBeforeLine:1 withDisplay:std::move(display)];
  self.frameSize = self.currentSize;
  [self setNeedsDisplayInRect:self.bounds];
}

- (void)performUpdates:(std::vector<marlin::control::source_update>)updates {
  for (auto& update : updates) {
    // For now, we assume that there are only 3 types of updates:
    //  - Insert statements/blocks
    //  - Replace/remove expressions
    //  - Remove statements/blocks
    NSLog(@"%ld, %ld, %ld, %ld, %@", update.range.begin.line, update.range.begin.column,
          update.range.end.line, update.range.end.column,
          [NSString stringWithStringView:update.display.source]);
    if (update.range.begin == update.range.end) {
      [self insertLinesBeforeLine:update.range.begin.line withDisplay:std::move(update.display)];
    } else if (update.range.begin.line == update.range.end.line) {
      [self updateExpressionInSourceRange:update.range withDisplay:std::move(update.display)];
    } else {
      assert(update.display.source.length() == 0);
      [self removeLinesFromLine:update.range.begin.line toLine:update.range.end.line];
    }
  }
  self.frameSize = self.currentSize;
  [self setNeedsDisplayInRect:self.bounds];
  [self clearErrors];
  [self.delegate sourceViewChanged:self];
}

- (void)insertLinesBeforeLine:(NSUInteger)line withDisplay:(marlin::format::display)display {
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
}

- (void)removeLinesFromLine:(NSUInteger)from toLine:(NSUInteger)to {
  NSAssert(from > 0 && from <= _strings.count, @"%lu out of range", from);
  NSAssert(to > 0 && to <= _strings.count, @"");
  NSAssert(from <= to, @"");
  auto indexSet = [NSMutableIndexSet new];
  [indexSet addIndexesInRange:NSMakeRange(from - 1, to - from)];
  [_strings removeObjectsAtIndexes:indexSet];
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

#pragma mark - ArrayViewControllerDelegate

- (void)arrayViewController:(ArrayViewController*)vc finishEditingWithCount:(NSUInteger)count {
  if (_selection.has_value()) {
    auto updates = _selection->set_new_array_elements_count(count);
    [self performUpdates:std::move(updates)];
    [self updateDuplicateViewControllerForSelection:*_selection];
  }
}

#pragma mark - ColorViewControllerDelegate

- (void)colorViewController:(ColorViewController*)vc finishEditingWithColor:(Color*)color {
  if (_selection.has_value() && _selection->is_color_literal()) {
    auto updates = _selection->set_color_literal(
        [color colorLiteralWithMode:_selection->get_color_literal().mode]);
    [self performUpdates:std::move(updates)];
    [self updateDuplicateViewControllerForSelection:*_selection];
  }
}

#pragma mark - DuplicateViewControllerDelegate

- (void)performDeleteForDuplicateViewController:(DuplicateViewController*)vc {
  if (_selection.has_value()) {
    NSAssert(_selection->is_removable(), @"");
    [self.delegate dismissEditorViewControllerForSourceView:self];
    auto updates = (*std::move(_selection)).remove_from_document();
    [self performUpdates:std::move(updates.source_updates)];
    self.selection = std::move(updates.selection_update);
    if (_selection.has_value()) {
      [self showEditorViewControllersForSelection:*_selection];
    }
  }
}

#pragma mark - EditorViewControllerDelegate

- (void)editorViewController:(EditorViewController*)vc
        finishEditWithString:(NSString*)string
                      ofType:(EditorType)type {
  if (_selection.has_value()) {
    auto update = (*std::move(_selection)).insert_literal(type, string.stringView);
    [self performUpdates:std::move(update.source_updates)];
    self.selection = std::move(update.selection_update);
  }
}

#pragma mark - FunctionViewControllerDelegate

- (void)functionViewController:(FunctionViewController*)vc
         finishEditingWithName:(NSString*)name
                    parameters:(NSArray<NSString*>*)parameters {
  if (_selection.has_value()) {
    marlin::function_definition signature{std::string{name.stringView}};
    for (NSString* parameter in parameters) {
      if (parameter.length > 0) {
        signature.parameters.emplace_back(parameter.stringView);
      }
    }
    auto updates = (*std::move(_selection)).replace_function_signature(signature);
    [self performUpdates:std::move(updates.source_updates)];
    self.selection = std::move(updates.selection_update);
  }
}

#pragma mark - Private methods

- (BOOL)isFlipped {
  return YES;
}

- (void)setSelection:(optional_source_selection)selection {
  _selection = selection;
  if (_selection.has_value()) {
    [self updateDuplicateViewControllerForSelection:*_selection];
    // show editor view controller after set selection if needed
  } else {
    [self.delegate dismissDuplicateViewControllerForSourceView:self];
    [self.delegate dismissEditorViewControllerForSourceView:self];
  }
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
  [self drawLineInsertionPointsInRect:rect];
  [self drawExprInsertionRangesInRect:rect];
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

- (void)drawLineInsertionPointsInRect:(CGRect)rect {
  NSAssert(_inserter.has_value(), @"");
  for (auto& location : _inserter->line_insert_locations()) {
    [self drawInsertionPoint:location inRect:rect];
  }
}

- (void)drawExprInsertionRangesInRect:(CGRect)rect {
  NSAssert(_inserter.has_value(), @"");
  for (auto& range : _inserter->expr_insert_ranges()) {
    [self drawSourceRange:range inRect:rect forSelection:NO];
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
  const auto loc = [self sourceLocationOfPoint:location];
  if (_selection.has_value() && _selection->get_range().contains(loc)) {
    // Here the click happens at the currently-edited node, so the dismissal of editor may
    // invalidate the new selection. We must perform the dismissal before re-generating the
    // selection.
    marlin::control::source_selection selection{self.dataSource.document.content, loc};
    auto location = _selection->locate_selection(selection);

    // update selection by finishing editing
    [self.delegate dismissDuplicateViewControllerForSourceView:self];
    [self.delegate dismissEditorViewControllerForSourceView:self];
    // set selection
    self.selection = _selection->selection_by_location(std::move(location));
    [self showEditorViewControllersForSelection:*_selection];
    [self setNeedsDisplayInRect:self.bounds];
  } else {
    // Here the click happens outside of the currently-edited node, so we assume that the new
    // selection will not be invalidated by dismissal of the editor.
    // Notice that this is currently ensured by the fact that side effects of the editor's dismissal
    // (e.g. changing the calls when a function signature is changed) will not result in
    // invalidation of node references.
    // If such is not to be upheld in the future, it must be reflected here how a click in the side
    // effect areas are interpreted.

    // selection by touch point first
    marlin::control::source_selection selection{self.dataSource.document.content, loc};
    // update selection by finishing editing
    [self.delegate dismissDuplicateViewControllerForSourceView:self];
    [self.delegate dismissEditorViewControllerForSourceView:self];
    // set selection
    self.selection = std::move(selection);
    [self showEditorViewControllersForSelection:*_selection];
    [self setNeedsDisplayInRect:self.bounds];
  }
}

- (void)updateDuplicateViewControllerForSelection:
    (const marlin::control::source_selection&)selection {
  if (auto type = selection.dragging_type(true)) {
    auto string = [self snapshotOfSelection:selection];
    auto draggingData = DraggingData{*type, [NSData dataWithDataView:selection.get_data(true)]};
    [self.delegate showDuplicateViewControllerForSourceView:self
                                                 withString:string
                                               draggingData:draggingData];
  } else {
    [self.delegate dismissDuplicateViewControllerForSourceView:self];
  }
}

- (void)showEditorViewControllersForSelection:(const marlin::control::source_selection&)selection {
  if (selection.is_literal()) {
    auto [type, data] = selection.get_literal_content();
    [self.delegate showEditorViewControllerForSourceView:self withType:type data:data];
  } else if (selection.is_function_signature()) {
    [self.delegate showFunctionViewControllerForSourceView:self
                                     withFunctionSignature:selection.get_function_signature()];
  } else if (selection.is_new_array()) {
    [self.delegate showArrayViewControllerForSourceView:self
                                              withCount:selection.get_new_array_element_count()
                                           minimalCount:selection.get_new_array_minimum_count()];
  } else if (selection.is_color_literal()) {
    auto colorLiteral = selection.get_color_literal();
    auto showAlpha = colorLiteral.mode == marlin::ast::color_mode::rgba ||
                     colorLiteral.mode == marlin::ast::color_mode::hsla;
    [self.delegate
        showColorViewControllerForSourceView:self
                                   withColor:[Color colorWithColorLiteral:std::move(colorLiteral)]
                                   showAlpha:showAlpha];
  } else {
    [self.delegate dismissEditorViewControllerForSourceView:self];
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

- (NSString*)snapshotOfSelection:(const marlin::control::source_selection&)selection {
  auto range = selection.get_range();
  NSAssert(range.end.line >= range.begin.line, @"");
  auto lines = range.end.line - range.begin.line;
  if (range.end.column > range.begin.column) {
    ++lines;
  }
  if (lines == 1) {
    NSAttributedString* attrString = [_strings objectAtIndex:range.begin.line - 1];
    return
        [attrString.string substringWithRange:NSMakeRange(range.begin.column - 1,
                                                          range.end.column - range.begin.column)];
  } else if (lines > 1) {
    auto string = [NSMutableString new];
    NSAttributedString* attrString = [_strings objectAtIndex:range.begin.line - 1];
    [string appendString:[attrString.string substringFromIndex:range.begin.column - 1]];
    [string appendString:@"\n"];
    if (lines > 2) {
      [string appendString:@"  ...\n"];
    }
    auto endLine = range.end.column > range.begin.column ? range.end.line - 1 : range.end.line - 2;
    attrString = [_strings objectAtIndex:endLine];
    [string appendString:[attrString.string substringFromIndex:range.begin.column - 1]];
    return string;
  } else {
    NSAssert(NO, @"");
    return nil;
  }
}

#pragma mark - Drag and drop

- (std::optional<DraggingData>)startDraggingAtLocation:(CGPoint)location {
  NSAssert(!_draggingSelection.has_value(), @"");

  std::optional<DraggingData> data;
  if (_selection.has_value()) {
    if (const auto rect = [self rectOfSourceRange:_selection->get_range()];
        !CGRectContainsPoint(rect, location)) {
      _draggingSelection = (*std::move(_selection)).as_dragging_selection();
      if (const auto type = _draggingSelection->dragging_type(false)) {
        self.selection = std::nullopt;
        data = DraggingData(*type, [NSData dataWithDataView:_draggingSelection->get_data()]);
      } else {
        // We assert that non-draggable selections are unchanged through as_dragging_selection(), so
        // there is no need to refresh the editor.
        self.selection = std::exchange(_draggingSelection, std::nullopt);
      }
    }
  }
  return data;
}

- (BOOL)draggingPasteboardOfType:(marlin::control::pasteboard_t)type toLocation:(CGPoint)location {
  if (_selection.has_value()) {
    self.selection = std::nullopt;
  }

  NSAssert(_inserter.has_value(), @"");
  const auto exclusion = _draggingSelection.has_value() ? &*_draggingSelection : nullptr;
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
  const bool result = update.source_updates.size() > 0;
  [self performUpdates:std::move(update.source_updates)];
  self.selection = std::move(update.selection_update);
  if (_selection.has_value()) {
    [self showEditorViewControllersForSelection:*_selection];
  }
  return result;
}

- (BOOL)removeDraggingSource {
  return [self removeDraggingSelection].start_line > 0;
}

- (marlin::control::line_update)removeDraggingSelection {
  if (_draggingSelection.has_value()) {
    NSAssert(_draggingSelection->is_removable(), @"");
    auto line_update = _draggingSelection->removal_line_update();
    auto updates = (*std::exchange(_draggingSelection, std::nullopt)).remove_from_document();
    [self performUpdates:std::move(updates.source_updates)];
    return line_update;
  }
  return {};
}

- (void)resetDraggingDestination {
  _inserter->reset_all();
  [self setNeedsDisplayInRect:self.bounds];
}

- (void)resetDraggingSource {
  _draggingSelection.reset();
  [self setNeedsDisplayInRect:self.bounds];
}

@end
