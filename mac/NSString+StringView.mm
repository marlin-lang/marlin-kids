#import "NSString+StringView.h"

@implementation NSString (StringView)

+ (instancetype)stringWithStringView:(std::string_view)sv {
  return [[NSString alloc] initWithStringView:sv];
}

- (instancetype)initWithStringView:(std::string_view)sv {
  return [self initWithBytes:sv.data() length:sv.size() encoding:NSUTF8StringEncoding];
}

@end
