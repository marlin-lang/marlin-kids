#import "SourceTheme.h"

#import "document.hpp"

@implementation SourceTheme

- (instancetype)init {
  if (self = [super init]) {
    _NSSelectionAttributeName = @"Selection";

    auto *font = [NSFont fontWithName:@"Courier" size:25];
    _allAttrs = @{NSFontAttributeName : font};
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

@end
