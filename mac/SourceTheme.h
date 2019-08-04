#import <Cocoa/Cocoa.h>

#import "source_modifications.hpp"

@interface SourceTheme : NSObject

@property(readonly) NSString* NSSelectionAttributeName;

@property(readonly) NSDictionary* allAttrs;
@property(readonly) NSDictionary* keywordAttrs;
@property(readonly) NSDictionary* opAttrs;
@property(readonly) NSDictionary* booleanAttrs;
@property(readonly) NSDictionary* numberAttrs;
@property(readonly) NSDictionary* stringAttrs;

- (void)applyTo:(NSTextStorage*)textStorage
             range:(NSRange)range
 withHighlights:(const std::vector<marlin::control::highlight_token>&)highlights;

@end
