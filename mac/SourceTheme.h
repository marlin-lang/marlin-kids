#import <Cocoa/Cocoa.h>

#import "source_update.hpp"

@interface SourceTheme : NSObject

@property(readonly) NSString* NSSelectionAttributeName;

@property(readonly) NSDictionary* allAttrs;

@property(readonly) NSDictionary* lineNumberAttrs;

@property(readonly) NSDictionary* keywordAttrs;
@property(readonly) NSDictionary* opAttrs;
@property(readonly) NSDictionary* booleanAttrs;
@property(readonly) NSDictionary* numberAttrs;
@property(readonly) NSDictionary* stringAttrs;
@property(readonly) NSDictionary* placeholderAttrs;

- (void)applyTo:(NSMutableAttributedString*)attributedString
             range:(NSRange)range
    withHighlights:(const std::vector<marlin::control::highlight_token>&)highlights;

@end
