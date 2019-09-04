#import "Document.h"

#import "NSData+DataView.h"
#import "NSObject+Casting.h"
#import "SourceViewController.h"

@interface Document () {
  std::optional<marlin::control::document> _content;
}

@property(strong, nonatomic) NSData *initialData;

@end

@implementation Document

+ (BOOL)autosavesInPlace {
  return YES;
}

- (std::optional<marlin::control::source_update>)initialize {
  if (auto result{self.initialData == nil
                      ? marlin::control::document::make_document()
                      : marlin::control::document::make_document(self.initialData.dataView)}) {
    auto [doc, source]{*std::move(result)};
    self.initialData = nil;
    _content = std::move(doc);
    return source;
  } else {
    return std::nullopt;
  }
}

- (marlin::control::document &)content {
  return *_content;
}

#ifdef IOS

- (id)contentsForType:(NSString *)typeName error:(NSError **)errorPtr {
  return [[NSData alloc] init];
}

- (BOOL)loadFromContents:(id)contents ofType:(NSString *)typeName error:(NSError **)errorPtr {
  return YES;
}

#else

- (void)makeWindowControllers {
  NSWindowController *controller = [[NSStoryboard storyboardWithName:@"Main" bundle:nil]
      instantiateControllerWithIdentifier:@"WindowController"];
  [self addWindowController:controller];
  if (auto *vc = [SourceViewController cast:controller.contentViewController]) {
    vc.document = self;
  }
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
  return [NSData dataWithDataView:_content->write()];
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
  self.initialData = data;
  return YES;
}

#endif

@end
