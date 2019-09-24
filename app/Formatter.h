#import "Types.h"

@interface NumberFormatter : NSNumberFormatter

@end

@interface ArrayCountFormatter : NumberFormatter

@property NSUInteger minimalCount;

@end

@interface StringFormatter : NSFormatter

@end

@interface VariableFormatter : NSFormatter

@end
