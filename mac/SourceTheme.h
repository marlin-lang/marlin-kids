#import <Cocoa/Cocoa.h>

@interface SourceTheme : NSObject

@property(readonly) NSString* NSSelectionAttributeName;

@property(readonly) NSDictionary* allAttrs;
@property(readonly) NSDictionary* opAttrs;
@property(readonly) NSDictionary* booleanAttrs;
@property(readonly) NSDictionary* numberAttrs;
@property(readonly) NSDictionary* stringAttrs;

@end
