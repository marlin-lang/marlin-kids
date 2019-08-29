#import "NSObject+Casting.h"

@implementation NSObject (Casting)

+ (instancetype)cast:(id)object {
  return [object isKindOfClass:self] ? object : nil;
}

@end
