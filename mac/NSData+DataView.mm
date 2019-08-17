#import "NSData+DataView.h"

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
