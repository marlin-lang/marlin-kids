#import <Cocoa/Cocoa.h>

#import "SourceTextView.h"

NS_ASSUME_NONNULL_BEGIN

@interface LineNumberView : NSRulerView

- (instancetype)initWithTextView:(SourceTextView*)textView;

- (void)addError:(NSString*)message atIndex:(NSUInteger)index;

- (void)clearErrors;

@end

NS_ASSUME_NONNULL_END
