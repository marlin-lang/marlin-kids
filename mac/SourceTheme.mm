#import "SourceTheme.h"

#import "document.hpp"

@implementation SourceTheme

- (instancetype)init {
  if (self = [super init]) {
    _NSSelectionAttributeName = @"Selection";

    auto *font = [NSFont fontWithName:@"Courier" size:25];
    _allAttrs = @{NSFontAttributeName : font};
    _keywordAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.purpleColor};
    _opAttrs = @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.brownColor};
    _booleanAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.blueColor};
    _numberAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.greenColor};
    _stringAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.cyanColor};
  }
  return self;
}

- (void)applyTo:(NSTextStorage *)textStorage
             range:(NSRange)range
    withHighlights:(const std::vector<marlin::control::highlight_token> &)highlights {
  [textStorage setAttributes:self.allAttrs range:range];
  for (const auto &highlight : highlights) {
    auto highlight_range = NSMakeRange(range.location + highlight.offset, highlight.length);
    switch (highlight.type) {
      case marlin::control::highlight_token_type::keyword:
        [textStorage setAttributes:self.keywordAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::op:
        [textStorage setAttributes:self.opAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::boolean:
        [textStorage setAttributes:self.booleanAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::number:
        [textStorage setAttributes:self.numberAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::string:
        [textStorage setAttributes:self.stringAttrs range:highlight_range];
        break;
      default:
        break;
    }
  }
}

@end
