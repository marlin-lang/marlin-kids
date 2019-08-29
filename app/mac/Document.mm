#import "Document.h"

#include <optional>

#import "NSObject+Casting.h"
#import "SourceViewController.h"

@implementation Document {
    DocumentImpl _impl;
}

+ (BOOL)autosavesInPlace {
  return YES;
}

- (void)makeWindowControllers {
  NSWindowController *controller = [[NSStoryboard storyboardWithName:@"Main" bundle:nil]
      instantiateControllerWithIdentifier:@"Document Window Controller"];
  [self addWindowController:controller];
  if (auto *vc = [SourceViewController cast:controller.contentViewController]) {
    vc.document = self;
  }
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
  return [NSData dataWithDataView:_impl.content->write()];
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
  _impl.initialData = data;
  return YES;
}

- (std::optional<marlin::control::source_update>)initialize {
    return _impl.initialize();
}

- (marlin::control::document&)content {
    NSAssert(_impl.content, @"content should be initialized");
    return *_impl.content;
}

@end
