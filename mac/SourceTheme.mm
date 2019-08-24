#import "SourceTheme.h"

#import "document.hpp"

@implementation SourceTheme

- (instancetype)init {
  if (self = [super init]) {
    _NSSelectionAttributeName = @"Selection";

    auto *font = [NSFont fontWithName:@"Courier" size:25];
    _allAttrs = @{NSFontAttributeName : font};
    _lineNumberAttrs = @{
      NSFontAttributeName : [NSFont fontWithName:@"Courier" size:18],
      NSForegroundColorAttributeName : NSColor.blackColor
    };
    _keywordAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.purpleColor};
    _opAttrs = @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.brownColor};
    _booleanAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.blueColor};
    _numberAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.greenColor};
    _stringAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.cyanColor};
    _placeholderAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : NSColor.grayColor};
  }
  return self;
}

- (void)applyTo:(NSMutableAttributedString *)attributedString
             range:(NSRange)range
    withHighlights:(const std::vector<marlin::control::highlight_token> &)highlights {
  [attributedString setAttributes:self.allAttrs range:range];
  for (const auto &highlight : highlights) {
    auto highlight_range = NSMakeRange(range.location + highlight.offset, highlight.length);
    switch (highlight.type) {
      case marlin::control::highlight_token_type::keyword:
        [attributedString setAttributes:self.keywordAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::op:
        [attributedString setAttributes:self.opAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::boolean:
        [attributedString setAttributes:self.booleanAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::number:
        [attributedString setAttributes:self.numberAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::string:
        [attributedString setAttributes:self.stringAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::placeholder:
        [attributedString setAttributes:self.placeholderAttrs range:highlight_range];
        break;
    }
  }
}

@end
