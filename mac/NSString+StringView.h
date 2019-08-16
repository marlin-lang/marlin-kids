#import <Foundation/Foundation.h>

#import <string_view>

@interface NSString (StringView)

+ (instancetype)stringWithStringView:(std::string_view)sv;

- (instancetype)initWithStringView:(std::string_view)sv;

@end
