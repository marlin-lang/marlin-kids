#import "Types.h"

#ifdef IOS

@implementation UITextView (MacInterface)

- (NSString*)string {
    return self.text;
}

@end

#endif
