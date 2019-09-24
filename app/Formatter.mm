#import "Formatter.h"

@implementation NumberFormatter

- (instancetype)init {
  if (self = [super init]) {
    self.minimumFractionDigits = 0;
    self.maximumFractionDigits = 10;
  }
  return self;
}

- (BOOL)getObjectValue:(out id _Nullable __autoreleasing *)obj
             forString:(NSString *)string
      errorDescription:(out NSString *__autoreleasing _Nullable *)error {
  if (string.length == 0) {
    return NO;
  }
  return [super getObjectValue:obj forString:string errorDescription:error];
}

@end

@implementation ArrayCountFormatter

- (BOOL)getObjectValue:(out id _Nullable __autoreleasing *)obj
             forString:(NSString *)string
      errorDescription:(out NSString *__autoreleasing _Nullable *)error {
  auto isValid = [super getObjectValue:obj forString:string errorDescription:error];
  return isValid && string.integerValue >= self.minimalCount && string.integerValue <= 32;
}

@end

@implementation StringFormatter

- (BOOL)getObjectValue:(out id _Nullable __autoreleasing *)obj
             forString:(NSString *)string
      errorDescription:(out NSString *__autoreleasing _Nullable *)error {
  return YES;
}

@end

@implementation VariableFormatter

- (BOOL)getObjectValue:(out id _Nullable __autoreleasing *)obj
             forString:(NSString *)string
      errorDescription:(out NSString *__autoreleasing _Nullable *)error {
  if (string.length == 0) {
    return NO;
  }
  for (auto i = 0; i < string.length; ++i) {
    auto ch = [string characterAtIndex:i];
    if (i == 0 && isdigit(ch)) {
      return NO;
    }
    if (ch != '_' && !isalnum(ch)) {
      return NO;
    }
  }
  return YES;
}

@end
