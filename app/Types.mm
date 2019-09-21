#import "Types.h"

@implementation NSObject (Casting)

+ (instancetype)cast:(id)object {
  return [object isKindOfClass:self] ? object : nil;
}

@end

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

@implementation NSData (DataView)

+ (instancetype)dataWithDataView:(marlin::store::data_view)data {
  return [[NSData alloc] initWithDataView:std::move(data)];
}

- (instancetype)initWithDataView:(marlin::store::data_view)data {
  return [self initWithBytes:data.begin() length:data.size()];
}

- (marlin::store::data_view)dataView {
  return {static_cast<const std::byte*>(self.bytes), self.length};
}

@end

#ifdef IOS

@implementation UISegmentedControl (MacInterface)

- (NSUInteger)selectedSegment {
  return self.selectedSegmentIndex;
}

@end

@implementation UITextField (MacInterface)

- (NSString*)stringValue {
  return self.text;
}

- (void)setStringValue:(NSString*)stringValue {
  self.text = stringValue;
}

@end

@implementation UITextView (MacInterface)

- (NSString*)string {
  return self.text;
}

@end

#endif
