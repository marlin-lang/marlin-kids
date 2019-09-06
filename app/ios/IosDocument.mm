#import "IosDocument.h"

@implementation IosDocument

- (id)contentsForType:(NSString *)typeName error:(NSError **)errorPtr {
  return [[NSData alloc] init];
}

- (BOOL)loadFromContents:(id)contents ofType:(NSString *)typeName error:(NSError **)errorPtr {
  return YES;
}

@end
