#import "IosDocument.h"

@implementation IosDocument

- (id)contentsForType:(NSString *)typeName error:(NSError **)errorPtr {
  return [NSData dataWithDataView:self.content.write()];
}

- (BOOL)loadFromContents:(id)contents ofType:(NSString *)typeName error:(NSError **)errorPtr {
  self.initialData = contents;
  return YES;
}

@end
