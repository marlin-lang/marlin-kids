#import "NSString+StringView.h"

@implementation NSString (StringView)

+ (instancetype)stringWithStringView:(std::string_view)sv {
  return [[NSString alloc] initWithStringView:sv];
}

- (instancetype)initWithStringView:(std::string_view)sv {
  return [self initWithBytes:sv.data() length:sv.size() encoding:NSUTF8StringEncoding];
}

- (std::string_view)stringView {
  return {self.UTF8String, self.length};
}

@end
