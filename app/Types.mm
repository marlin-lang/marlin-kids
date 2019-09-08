#import "Types.h"

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
