#import "Types.h"

#import "SourceView.h"

NS_ASSUME_NONNULL_BEGIN

@interface LineNumberView : RulerView

#ifdef IOS
@property CGFloat ruleThickness;
#endif

- (instancetype)initWithSourceView:(SourceView*)view;

- (void)addError:(NSString*)message atLine:(NSUInteger)line;

- (void)clearErrors;

@end

NS_ASSUME_NONNULL_END
