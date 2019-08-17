#import <Foundation/Foundation.h>

#import "byte_span.hpp"

@interface NSData (DataView)

+ (instancetype)dataWithDataView:(marlin::store::data_view)data;

- (instancetype)initWithDataView:(marlin::store::data_view)data;

- (marlin::store::data_view)dataView;

@end
