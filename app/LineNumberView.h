#import "Types.h"

#import "SourceView.h"

NS_ASSUME_NONNULL_BEGIN

@interface LineNumberView : View  // NSRulerView

- (instancetype)initWithSourceView:(SourceView*)view;

- (void)addError:(NSString*)message atLine:(NSUInteger)line;

- (void)clearErrors;

@end

NS_ASSUME_NONNULL_END
