#import "Theme.h"

@implementation DefaultTheme

- (instancetype)init {
  if (self = [super init]) {
    auto* font = [Font fontWithName:@"Courier" size:22];
    _allAttrs = @{NSFontAttributeName : font};
    _lineNumberAttrs = @{
      NSFontAttributeName : [Font fontWithName:@"Courier" size:18],
      NSForegroundColorAttributeName : Color.blackColor
    };
    _consoleAttrs = @{
      NSFontAttributeName : [Font fontWithName:@"Courier" size:13],
      NSForegroundColorAttributeName : Color.blackColor
    };
    _keywordAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : Color.purpleColor};
    _opAttrs = @{NSFontAttributeName : font, NSForegroundColorAttributeName : Color.brownColor};
    _booleanAttrs = @{NSFontAttributeName : font, NSForegroundColorAttributeName : Color.blueColor};
    _numberAttrs = @{NSFontAttributeName : font, NSForegroundColorAttributeName : Color.greenColor};
    _stringAttrs = @{NSFontAttributeName : font, NSForegroundColorAttributeName : Color.cyanColor};
    _placeholderAttrs =
        @{NSFontAttributeName : font, NSForegroundColorAttributeName : Color.grayColor};
  }
  return self;
}

@end

static id<Theme> _currentTheme;

id<Theme> currentTheme() { return _currentTheme; }

void setCurrentTheme(id<Theme> theme) { _currentTheme = theme; }

Size characterSizeWithAttributes(NSDictionary<NSAttributedStringKey, id>* attrs) {
  return [@"a" sizeWithAttributes:attrs];
}

void applyTheme(id<Theme> theme, NSMutableAttributedString* attributedString, NSRange range,
                const std::vector<marlin::control::highlight_token>& highlights) {
  [attributedString setAttributes:theme.allAttrs range:range];
  for (const auto& highlight : highlights) {
    auto highlight_range = NSMakeRange(range.location + highlight.offset, highlight.length);
    switch (highlight.type) {
      case marlin::control::highlight_token_type::keyword:
        [attributedString setAttributes:theme.keywordAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::op:
        [attributedString setAttributes:theme.opAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::boolean:
        [attributedString setAttributes:theme.booleanAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::number:
        [attributedString setAttributes:theme.numberAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::string:
        [attributedString setAttributes:theme.stringAttrs range:highlight_range];
        break;
      case marlin::control::highlight_token_type::placeholder:
        [attributedString setAttributes:theme.placeholderAttrs range:highlight_range];
        break;
    }
  }
}
