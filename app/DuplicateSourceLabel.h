#import "Types.h"

#import "Pasteboard.h"

NS_ASSUME_NONNULL_BEGIN

@interface DuplicateSourceLabel : Label

@property DraggingData draggingData;

- (void)setSourceString:(NSString*)sourceString;

@end

NS_ASSUME_NONNULL_END
